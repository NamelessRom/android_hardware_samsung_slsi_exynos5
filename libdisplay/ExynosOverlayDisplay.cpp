#include "ExynosOverlayDisplay.h"
#include "ExynosHWCUtils.h"
#include "ExynosMPPModule.h"

#ifdef G2D_COMPOSITION
#include "ExynosG2DWrapper.h"
#endif

#define LOG_NDEBUG 0

ExynosOverlayDisplay::ExynosOverlayDisplay(int numMPPs, struct exynos5_hwc_composer_device_1_t *pdev) :
    ExynosDisplay(numMPPs)
{
    mNumMPPs = numMPPs;
    mMPPs = new ExynosMPPModule*[mNumMPPs];
    for (int i = 0; i < mNumMPPs; i++)
        mMPPs[i] = new ExynosMPPModule(this, i);

    mOtfMode = OTF_OFF;
    this->mHwc = pdev;
}

ExynosOverlayDisplay::~ExynosOverlayDisplay()
{
    for (int i = 0; i < mNumMPPs; i++)
        delete mMPPs[i];
    delete[] mMPPs;
}

bool ExynosOverlayDisplay::isOverlaySupported(hwc_layer_1_t &layer, size_t i)
{
    int mMPPIndex = 0;

    if (layer.flags & HWC_SKIP_LAYER) {
        ALOGD("\tlayer %u: skipping", i);
        return false;
    }

    //if (!layer.planeAlpha) //atm not needed, since we have provisory hwc 1.1
    //    return false;

    private_handle_t *handle = private_handle_t::dynamicCast(layer.handle);

    mMPPIndex = getMPPForUHD(layer);

    if (!handle) {
        ALOGD("%s:\tlayer %u: handle is NULL", __FUNCTION__, i);
        return false;
    }

    //isSrcCropFloat() commented out, since we've hwc 1.1, but scaler supports it
    /*if (isSrcCropFloat(layer.sourceCropf)) {
        ALOGV("\tlayer %u: has floating crop info [%7.1f %7.1f %7.1f %7.1f]", i,
            layer.sourceCropf.left, layer.sourceCropf.top, layer.sourceCropf.right,
            layer.sourceCropf.bottom);
        return false;
    }*/

    if (visibleWidth(mMPPs[mMPPIndex], layer, handle->format, this->mXres) < BURSTLEN_BYTES) {
        ALOGD("\tlayer %u: visible area is too narrow", i);
        return false;
    }

#ifdef USE_FB_PHY_LINEAR
    if (layer.displayFrame.left < 0 || layer.displayFrame.top < 0 ||
        layer.displayFrame.right > this->mXres || layer.displayFrame.bottom > this->mYres) {
        ALOGD("%s: USE_FB_PHY_LINEAR return false" , __FUNCTION__);
        return false;
    }
#endif

    if (mMPPs[mMPPIndex]->isProcessingRequired(layer, handle->format)) {
        //TODO
        ALOGD("%s: Processing is required, but not yet supported", __FUNCTION__);
        return false;
        //int down_ratio = mMPPs[mMPPIndex]->getDownscaleRatio(this->mXres, this->mYres);
        /* Check whether GSC can handle using local or M2M */
        /*if (!((mMPPs[mMPPIndex]->isProcessingSupported(layer, handle->format, false, down_ratio)) ||
            (mMPPs[mMPPIndex]->isProcessingSupported(layer, handle->format, true, down_ratio)))) {
            ALOGV("\tlayer %u: gscaler required but not supported", i);
            return false;
        }*/
    } else {
        if (!isFormatSupported(handle->format)) {
            ALOGD("%s:\tlayer %u: pixel format %u not supported", __FUNCTION__, i, handle->format);
            return false;
        }
    }

    if ((layer.blending != HWC_BLENDING_NONE) &&
            mMPPs[mMPPIndex]->isFormatSupportedByGsc(handle->format) &&
            !isFormatRgb(handle->format)) {
        ALOGD("%s:\tlayer %u blending on non RGB",__FUNCTION__, i);
        return false;
    }

    if (!isBlendingSupported(layer.blending)) {
        ALOGD("%s:\tlayer %u: blending %d not supported", __FUNCTION__, i, layer.blending);
        return false;
    }

    if (CC_UNLIKELY(isOffscreen(layer, mXres, mYres))) {
        ALOGW("%s:\tlayer %u: off-screen", __FUNCTION__, i);
        return false;
    }

    ALOGD("%s:\tlayer %u is supported", __FUNCTION__, i);
    return true;
}

int ExynosOverlayDisplay::prepare(hwc_display_contents_1_t* contents)
{
    ALOGD("%s: preparing %u layers for FIMD", __FUNCTION__, contents->numHwLayers);

    memset(mPostData.gsc_map, 0, sizeof(mPostData.gsc_map));
    memset(mPostData.decon_dma_type, -1, sizeof(mPostData.decon_dma_type));

    mPostData.numRgbOvly = 0;

    mForceFb = mHwc->force_gpu;

    //if geometry has not changed, there is no need to do any work here
    /*if (!contents || (!(contents->flags & HWC_GEOMETRY_CHANGED))) {
        ALOGD("%s: geometry not changed, exiting", __FUNCTION__);
        return 0;
    }*/

// TODO: add skip static layers logic (odroid sources look good)

    determineSupportedOverlays(contents);
    determineBandwidthSupport(contents);
    assignWindows(contents);

    //if (!gsc_used)
    //    exynos5_cleanup_gsc_m2m(ctx);

    if (!mFbNeeded)
        mPostData.fb_window = NO_FB_NEEDED;

    return 0;
}

void ExynosOverlayDisplay::configureOtfWindow(hwc_rect_t &displayFrame,
        int32_t blending, int32_t planeAlpha, int format, decon_win_config &cfg)
{
}

void ExynosOverlayDisplay::configureHandle(private_handle_t *handle,
        uint32_t unknown, hwc_layer_1 &layer, int fence_fd, decon_win_config &cfg, enum decon_idma_type idma_type)
{
    uint32_t x, y;
    uint32_t w = WIDTH(layer.displayFrame);
    uint32_t h = HEIGHT(layer.displayFrame);
    uint8_t bpp = formatToBpp(handle->format);

    if (layer.displayFrame.left < 0) {
        unsigned int crop = -layer.displayFrame.left;
        ALOGD("layer off left side of screen; cropping %u pixels from left edge",
                crop);
        x = 0;
        w -= crop;
    } else {
        x = layer.displayFrame.left;
    }

    if (layer.displayFrame.right > this->mXres) {
        unsigned int crop = layer.displayFrame.right - this->mXres;
        ALOGD("layer off right side of screen; cropping %u pixels from right edge",
                crop);
        w -= crop;
    }

    if (layer.displayFrame.top < 0) {
        unsigned int crop = -layer.displayFrame.top;
        ALOGD("layer off top side of screen; cropping %u pixels from top edge",
                crop);
        y = 0;
        h -= crop;
    } else {
        y = layer.displayFrame.top;
    }

    if (layer.displayFrame.bottom > this->mYres) {
        int crop = layer.displayFrame.bottom - this->mYres;
        ALOGD("layer off bottom side of screen; cropping %u pixels from bottom edge",
                crop);
        h -= crop;
    }

    cfg.state = cfg.DECON_WIN_STATE_BUFFER;
    cfg.fd_idma[0] = handle->fd;
    cfg.fd_idma[1] = handle->fd1;
    cfg.fd_idma[2] = handle->fd2;

    cfg.src.x = (uint32_t) layer.sourceCrop.left;
    cfg.src.y = (uint32_t) layer.sourceCrop.top;
    cfg.src.w = (uint32_t) WIDTH(layer.sourceCrop);
    cfg.src.h = (uint32_t) HEIGHT(layer.sourceCrop);

    if (handle->format == 0x121) { //TODO: find constant and add UNLIKELY
        cfg.src.f_w = handle->stride * 2;
        cfg.src.f_h = handle->vstride / 2;
    } else {
        cfg.src.f_w = handle->stride;
        cfg.src.f_h = handle->vstride;
    }

    cfg.dst.x = (uint32_t) layer.displayFrame.left;
    cfg.dst.y = (uint32_t) layer.displayFrame.top;
    cfg.dst.w = (uint32_t) WIDTH(layer.displayFrame);
    cfg.dst.h = (uint32_t) HEIGHT(layer.displayFrame);
    cfg.dst.f_w = cfg.dst.w;
    cfg.dst.f_h = cfg.dst.h;

    cfg.format = halFormatToS3CFormat(handle->format);
    cfg.blending = halBlendingToS3CBlending(layer.blending);
    cfg.fence_fd = fence_fd;
    cfg.idma_type = idma_type;

    cfg.plane_alpha = 255;
    if (layer.planeAlpha && (layer.planeAlpha < 254)) {
        cfg.plane_alpha = layer.planeAlpha;
    }

    cfg.vpp_parm.eq_mode = BT_601_NARROW;
    cfg.vpp_parm.rot = (enum vpp_rotate) layer.transform;
}

void ExynosOverlayDisplay::configureOverlay(hwc_layer_1_t *layer, decon_win_config &cfg, enum decon_idma_type idma_type)
{
    ALOGD("%s", __FUNCTION__);
    if (layer->compositionType == HWC_BACKGROUND) {
        hwc_color_t color = layer->backgroundColor;
        cfg.state = cfg.DECON_WIN_STATE_COLOR;
        cfg.color = (color.r << 16) | (color.g << 8) | color.b;
        cfg.dst.x = 0;
        cfg.dst.y = 0;
        cfg.dst.w = this->mXres;
        cfg.dst.f_w = this->mXres;
        cfg.dst.h = this->mYres;
        cfg.dst.f_h = this->mYres;
        return;
    }

    if (layer->compositionType == HWC_FRAMEBUFFER_TARGET) {
        if (layer->acquireFenceFd >= 0) {
            if (sync_wait(layer->acquireFenceFd, 1000) < 0)
                ALOGW("%s: sync_wait error", __FUNCTION__);

            //usleep(3000);
            close(layer->acquireFenceFd);
            layer->acquireFenceFd = -1;
        }
    }

    /* TODO - needed? if ((layer->acquireFenceFd >= 0) && this->mForceFbYuvLayer) {
        if (sync_wait(layer->acquireFenceFd, 1000) < 0)
            ALOGE("sync_wait error");
        close(layer->acquireFenceFd);
        layer->acquireFenceFd = -1;
    }*/

    private_handle_t *handle = private_handle_t::dynamicCast(layer->handle);
    configureHandle(handle, 0, *layer, layer->acquireFenceFd, cfg, idma_type);
}


int ExynosOverlayDisplay::postFrame(hwc_display_contents_1_t* contents)
{
    exynos5_hwc_post_data_t *pdata = &mPostData;
    struct decon_win_config_data win_data;
    struct decon_win_config *config = win_data.config;
    int win_map = 0;
    int tot_ovly_wins = 0;

#ifdef G2D_COMPOSITION
    int num_g2d_overlayed = 0;
#endif

    memset(config, 0, sizeof(win_data.config));
    for (size_t i = 0; i < NUM_HW_WINDOWS; i++)
        config[i].fence_fd = -1;

    for (size_t i = 0; i < NUM_HW_WINDOWS; i++) {
        if ( pdata->overlay_map[i] != -1)
            tot_ovly_wins++;
    }
    if (mVirtualOverlayFlag)
        tot_ovly_wins++;

    for (size_t i = 0; i < NUM_HW_WINDOWS; i++) {
        int layer_idx = pdata->overlay_map[i];

        if (layer_idx != -1) {
            hwc_layer_1_t &layer = contents->hwLayers[layer_idx];

            //winmap is same as idma_type
            //win_map = getDeconWinMap(i, tot_ovly_wins);
            win_map = pdata->decon_dma_type[layer_idx];

            if (pdata->gsc_map[i].mode == exynos5_gsc_map_t::GSC_M2M) {
                //provisory close fence
                close(layer.acquireFenceFd);
                layer.acquireFenceFd = -1;

                if (postGscM2M(layer, config, win_map, i) < 0)
                    continue;
            } else {
                //TODO - needed? Don't think so - waitForRenderFinish(&layer.handle, 1);
                ALOGD("%s: layer %u win_map(%d) idma_type[%d]=%d", __FUNCTION__, layer_idx, win_map, layer_idx, (int)pdata->decon_dma_type[layer_idx]);
                configureOverlay(&layer, config[win_map], pdata->decon_dma_type[layer_idx]);
            }
        }

        if (i == 0 && config[i].blending != DECON_BLENDING_NONE) {
            ALOGD("blending not supported on window 0; forcing BLENDING_NONE");
            config[i].blending = DECON_BLENDING_NONE;
        }

        ALOGD("window %u configuration:", i);
        dumpConfig(config[win_map]);
    }

    /* TODO - needed? - if (this->mVirtualOverlayFlag) {
        handleStaticLayers(contents, win_data, tot_ovly_wins);
    }*/

    int ret = ioctl(this->mDisplayFd, S3CFB_WIN_CONFIG, &win_data);
    for (size_t i = 0; i < NUM_HW_WINDOWS; i++)
        if (config[i].fence_fd != -1)
            close(config[i].fence_fd);
    if (ret < 0) {
        ALOGE("ioctl S3CFB_WIN_CONFIG failed: %s", strerror(errno));
        return ret;
    }
    if (contents->numHwLayers == 1) {
        hwc_layer_1_t &layer = contents->hwLayers[0];
        if (layer.acquireFenceFd >= 0)
            close(layer.acquireFenceFd);
    }

    if (!mGscLayers || this->mOtfMode == OTF_TO_M2M) {
        cleanupGscs();
    }

#ifdef G2D_COMPOSITION
    if (!this->mG2dComposition)
        exynos5_cleanup_g2d(mHwc, 0);

    this->mG2dBypassCount = max( this->mG2dBypassCount-1, 0);
#endif

    memcpy(this->mLastConfig, &win_data.config, sizeof(win_data.config));
    memcpy(this->mLastGscMap, pdata->gsc_map, sizeof(pdata->gsc_map));
    if (!this->mVirtualOverlayFlag)
        this->mLastFbWindow = pdata->fb_window;
    for (size_t i = 0; i < NUM_HW_WINDOWS; i++) {
        int layer_idx = pdata->overlay_map[i];
        if (layer_idx != -1) {
            hwc_layer_1_t &layer = contents->hwLayers[layer_idx];
            win_map = getDeconWinMap(i, tot_ovly_wins);
            this->mLastHandles[win_map] = layer.handle;
        }
    }

    return win_data.fence;
}

int ExynosOverlayDisplay::clearDisplay()
{
    struct decon_win_config_data win_data;
    memset(&win_data, 0, sizeof(win_data));

    int ret = ioctl(this->mDisplayFd, S3CFB_WIN_CONFIG, &win_data);
    LOG_ALWAYS_FATAL_IF(ret < 0,
            "ioctl S3CFB_WIN_CONFIG failed to clear screen: %s",
            strerror(errno));
    // the causes of an empty config failing are all unrecoverable

    return win_data.fence;
}

int ExynosOverlayDisplay::set(hwc_display_contents_1_t* contents)
{
    hwc_layer_1_t *fb_layer = NULL;
    int err = 0;

    if (this->mPostData.fb_window != NO_FB_NEEDED) {
        for (size_t i = 0; i < contents->numHwLayers; i++) {
            if (contents->hwLayers[i].compositionType == HWC_FRAMEBUFFER_TARGET) {
                this->mPostData.overlay_map[this->mPostData.fb_window] = i;
                fb_layer = &contents->hwLayers[i];
                this->mPostData.decon_dma_type[i] = getRGBDeconDMAType(this->mPostData.numRgbOvly);
                ALOGD("%s: i(%d) fb_window(%d) framebuffer decon_dma_type[%d]=%d", __FUNCTION__, i, this->mPostData.fb_window, i, this->mPostData.decon_dma_type[i]);
                this->mPostData.numRgbOvly++;
                break;
            }
        }

        if (CC_UNLIKELY(!fb_layer)) {
            ALOGE("%s: framebuffer target expected, but not provided", __FUNCTION__);
            err = -EINVAL;
        }
    }

    int fence;
    if (!err) {
        fence = postFrame(contents);
        if (fence < 0)
            err = fence;
    }

    if (err)
        fence = clearDisplay();

    for (size_t i = 0; i < NUM_HW_WINDOWS; i++) {
        if (this->mPostData.overlay_map[i] != -1) {
            hwc_layer_1_t &layer = contents->hwLayers[this->mPostData.overlay_map[i]];

            int dup_fd = dup(fence);
            if (dup_fd < 0)
                ALOGW("%s: release fence dup failed: %s", __FUNCTION__, strerror(errno));
            if (this->mPostData.gsc_map[i].mode == exynos5_gsc_map_t::GSC_M2M) {
                // TODO
                /*int gsc_idx = this->mPostData.gsc_map[i].idx;
                ExynosMPP &gsc = *mMPPs[gsc_idx];
                gsc.mDstBufFence[gsc.mCurrentBuf] = dup_fd;
                gsc.mCurrentBuf = (gsc.mCurrentBuf + 1) % NUM_GSC_DST_BUFS;*/
            } else if (this->mPostData.gsc_map[i].mode == exynos5_gsc_map_t::GSC_LOCAL) {
                /* Only use close(dup_fd) case, working fine. */
                if (dup_fd >= 0)
                    close(dup_fd);
                continue;
#ifdef G2D_COMPOSITION
            } else if (this->mG2dComposition && this->mG2d.win_used[i]) {
                int idx = this->mG2d.win_used[i] - 1;
                this->mWinBufFence[idx][this->mG2dCurrentBuffer[idx]] = dup_fd;
                this->mG2dCurrentBuffer[idx] =  (this->mG2dCurrentBuffer[idx] + 1) % NUM_GSC_DST_BUFS;
#endif
            } else {
                layer.releaseFenceFd = dup_fd;
            }
        }
    }
    contents->retireFenceFd = fence;

    return err;
}

int ExynosOverlayDisplay::getCompModeSwitch()
{
    unsigned int tot_win_size = 0, updateFps = 0;
    unsigned int lcd_size = this->mXres * this->mYres;
    uint64_t TimeStampDiff;
    float Temp;

    if (!mHwc->hwc_ctrl.dynamic_recomp_mode) {
        mHwc->LastModeSwitchTimeStamp = 0;
        mHwc->CompModeSwitch = NO_MODE_SWITCH;
        return 0;
    }

    /* initialize the Timestamps */
    if (!mHwc->LastModeSwitchTimeStamp) {
        mHwc->LastModeSwitchTimeStamp = mHwc->LastUpdateTimeStamp;
        mHwc->CompModeSwitch = NO_MODE_SWITCH;
        return 0;
    }

    /* If video layer is there, skip the mode switch */
    for (size_t i = 0; i < NUM_HW_WINDOWS; i++) {
        if (this->mLastGscMap[i].mode != exynos5_gsc_map_t::GSC_NONE) {
            if (mHwc->CompModeSwitch != HWC_2_GLES) {
                return 0;
            } else {
                mHwc->CompModeSwitch = GLES_2_HWC;
                mHwc->updateCallCnt = 0;
                mHwc->LastModeSwitchTimeStamp = mHwc->LastUpdateTimeStamp;
                //ALOGI("[DYNAMIC_RECOMP] GLES_2_HWC by video layer");
                return GLES_2_HWC;
            }
        }
    }

    /* Mode Switch is not required if total pixels are not more than the threshold */
    if ((uint32_t)mHwc->totPixels <= lcd_size * HWC_FIMD_BW_TH) {
        if (mHwc->CompModeSwitch != HWC_2_GLES) {
            return 0;
        } else {
            mHwc->CompModeSwitch = GLES_2_HWC;
            mHwc->updateCallCnt = 0;
            mHwc->LastModeSwitchTimeStamp = mHwc->LastUpdateTimeStamp;
            //ALOGI("[DYNAMIC_RECOMP] GLES_2_HWC by BW check");
            return GLES_2_HWC;
        }
    }

    /*
     * There will be at least one composition call per one minute (because of time update)
     * To minimize the analysis overhead, just analyze it once in a second
     */
    TimeStampDiff = systemTime(SYSTEM_TIME_MONOTONIC) - mHwc->LastModeSwitchTimeStamp;

    /* check fps every 250ms from LastModeSwitchTimeStamp*/
    if (TimeStampDiff < (VSYNC_INTERVAL * 15)) {
        return 0;
    }
    mHwc->LastModeSwitchTimeStamp = mHwc->LastUpdateTimeStamp;
    Temp = (VSYNC_INTERVAL * 60) / TimeStampDiff;
    updateFps = (int)(mHwc->updateCallCnt * Temp + 0.5);
    mHwc->updateCallCnt = 0;
    /*
     * FPS estimation.
     * If FPS is lower than HWC_FPS_TH, try to switch the mode to GLES
     */
    if (updateFps < HWC_FPS_TH) {
        if (mHwc->CompModeSwitch != HWC_2_GLES) {
            mHwc->CompModeSwitch = HWC_2_GLES;
            //ALOGI("[DYNAMIC_RECOMP] HWC_2_GLES by low FPS(%d)", updateFps);
            return HWC_2_GLES;
        } else {
            return 0;
        }
    } else {
         if (mHwc->CompModeSwitch == HWC_2_GLES) {
            mHwc->CompModeSwitch = GLES_2_HWC;
            //ALOGI("[DYNAMIC_RECOMP] GLES_2_HWC by high FPS(%d)", updateFps);
            return GLES_2_HWC;
        } else {
            return 0;
        }
    }

    return 0;
}

int32_t ExynosOverlayDisplay::getDisplayAttributes(const uint32_t attribute)
{
    switch(attribute) {
    case HWC_DISPLAY_VSYNC_PERIOD:
        return this->mVsyncPeriod;

    case HWC_DISPLAY_WIDTH:
        return this->mXres;

    case HWC_DISPLAY_HEIGHT:
        return this->mYres;

    case HWC_DISPLAY_DPI_X:
        return this->mXdpi;

    case HWC_DISPLAY_DPI_Y:
        return this->mYdpi;

    default:
        ALOGE("unknown display attribute %u", attribute);
        return -EINVAL;
    }
}

void ExynosOverlayDisplay::skipStaticLayers(hwc_display_contents_1_t* contents)
{
    static int init_flag = 0;
    int last_ovly_lay_idx = -1;

    mVirtualOverlayFlag = 0;
    mLastOverlayWindowIndex = -1;

    if (!mHwc->hwc_ctrl.skip_static_layer_mode)
        return;

    if (mBypassSkipStaticLayer)
        return;

    if (contents->flags & HWC_GEOMETRY_CHANGED) {
        init_flag = 0;
        return;
    }

    for (size_t i = 0; i < NUM_HW_WINDOWS; i++) {
        if (mPostData.overlay_map[i] != -1) {
            last_ovly_lay_idx = mPostData.overlay_map[i];
            mLastOverlayWindowIndex = i;
        }
    }

    if ((last_ovly_lay_idx == -1) || !mFbNeeded ||
        ((mLastFb - mFirstFb + 1) > NUM_VIRT_OVER)) {
        init_flag = 0;
        return;
    }
    mLastOverlayLayerIndex = last_ovly_lay_idx;

    if (init_flag == 1) {
        for (size_t i = mFirstFb; i <= mLastFb; i++) {
            hwc_layer_1_t &layer = contents->hwLayers[i];
            if (!layer.handle || (layer.flags & HWC_SKIP_LAYER) || (mLastLayerHandles[i - mFirstFb] !=  layer.handle)) {
                init_flag = 0;
                return;
            }
        }

        mVirtualOverlayFlag = 1;
        for (size_t i = 0; i < contents->numHwLayers-1; i++) {
            hwc_layer_1_t &layer = contents->hwLayers[i];
            if (layer.compositionType == HWC_FRAMEBUFFER)
                layer.compositionType = HWC_OVERLAY;
        }
        mLastFbWindow = mPostData.fb_window;
        return;
    }

    init_flag = 1;
    for (size_t i = 0; i < NUM_VIRT_OVER; i++)
        mLastLayerHandles[i] = 0;

    for (size_t i = mFirstFb; i <= mLastFb; i++) {
        hwc_layer_1_t &layer = contents->hwLayers[i];
        mLastLayerHandles[i - mFirstFb] = layer.handle;
    }

    return;
}

void ExynosOverlayDisplay::forceYuvLayersToFb(hwc_display_contents_1_t *contents)
{
}

void ExynosOverlayDisplay::handleOffscreenRendering(hwc_layer_1_t &layer)
{
}

void ExynosOverlayDisplay::determineYuvOverlay(hwc_display_contents_1_t *contents)
{
    mPopupPlayYuvContents = false;
    mForceOverlayLayerIndex = -1;
    mHasDrmSurface = false;
    mYuvLayers = 0;
    mHasCropSurface = false;
    for (size_t i = 0; i < contents->numHwLayers; i++) {
        hwc_layer_1_t &layer = contents->hwLayers[i];
        if (layer.handle) {
            private_handle_t *handle = private_handle_t::dynamicCast(layer.handle);
            if (getDrmMode(handle->flags) != NO_DRM) {
                this->mHasDrmSurface = true;
                mForceOverlayLayerIndex = i;
            }

            /* check yuv surface */
            if (mMPPs[0]->formatRequiresGsc(handle->format) && isOverlaySupported(contents->hwLayers[i], i)) {
                this->mYuvLayers++;
                mForceOverlayLayerIndex = i;
            }

            handleOffscreenRendering(layer);
        }
    }
    mPopupPlayYuvContents = !!((mYuvLayers == 1) && (mForceOverlayLayerIndex > 0));

#ifdef USE_FB_PHY_LINEAR
    if ((!mHasDrmSurface) &&
#ifdef G2D_COMPOSITION
            (mG2dBypassCount > 0) &&
#endif
            (contents->flags & HWC_GEOMETRY_CHANGED))
        mForceFb = true;
#endif
}

void ExynosOverlayDisplay::determineSupportedOverlays(hwc_display_contents_1_t *contents)
{
    bool videoLayer = false;

    mFbNeeded = false;
    mFirstFb = mLastFb = 0;

    for (size_t i = 0; i < NUM_HW_WINDOWS; i++)
        mPostData.overlay_map[i] = -1;

    // find unsupported overlays
    for (size_t i = 0; i < contents->numHwLayers; i++) {
        hwc_layer_1_t &layer = contents->hwLayers[i];

        if (layer.compositionType == HWC_FRAMEBUFFER_TARGET) {
            ALOGD("\tlayer %u: framebuffer target", i);
            continue;
        }

        if (layer.compositionType == HWC_BACKGROUND && !mForceFb) {
            ALOGD("\tlayer %u: background supported", i);
            //dumpLayer(&contents->hwLayers[i]);
            continue;
        }

        if (layer.handle) {
            private_handle_t *handle = private_handle_t::dynamicCast(layer.handle);

            //mHwc->totPixels += WIDTH(layer.displayFrame) * HEIGHT(layer.displayFrame);

            if (isOverlaySupported(contents->hwLayers[i], i) && !mForceFb) {
                ALOGD("\tlayer %u: overlay supported", i);
                layer.compositionType = HWC_OVERLAY;
                dumpLayer(&contents->hwLayers[i]);
                continue;
            }
        }

        if (!mFbNeeded) {
            mFirstFb = i;
            mFbNeeded = true;
        }
        mLastFb = i;
        layer.compositionType = HWC_FRAMEBUFFER;

        dumpLayer(&contents->hwLayers[i]);
    }

    mFirstFb = min(mFirstFb, (size_t)NUM_HW_WINDOWS-1);

    // can't composite overlays sandwiched between framebuffers
    if (mFbNeeded) {
        for (size_t i = mFirstFb; i < mLastFb; i++) {
            ALOGD("%s layer %d sandwiched between FB, set to FB too", __FUNCTION__, i);
            contents->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        }
    }
}

void ExynosOverlayDisplay::determineBandwidthSupport(hwc_display_contents_1_t *contents)
{
    // Incrementally try to add our supported layers to hardware windows.
    // If adding a layer would violate a hardware constraint, force it
    // into the framebuffer and try again.  (Revisiting the entire list is
    // necessary because adding a layer to the framebuffer can cause other
    // windows to retroactively violate constraints.)
    bool changed;
    this->mBypassSkipStaticLayer = false;

    uint32_t pixel_used[MAX_NUM_FIMD_DMA_CH];
    do {
        android::Vector<hwc_rect> rects[MAX_NUM_FIMD_DMA_CH];
        android::Vector<hwc_rect> overlaps[MAX_NUM_FIMD_DMA_CH];
        int dma_ch_idx;
        uint32_t win_idx = 0;
        size_t rgb_windows_left, other_windows_left = NUM_OTHER_HW_WINDOWS;
        memset(&pixel_used[0], 0, sizeof(pixel_used));
        mGscUsed = false;

        if (mFbNeeded) {
            hwc_rect_t fb_rect;

            fb_rect.top = fb_rect.left = 0;
            fb_rect.right = this->mXres - 1;
            fb_rect.bottom = this->mYres - 1;

            dma_ch_idx = FIMD_DMA_CH_IDX[mFirstFb];
            pixel_used[dma_ch_idx] = (uint32_t) (this->mXres * this->mYres);
            win_idx = (win_idx == mFirstFb) ? (win_idx + 1) : win_idx;
            rgb_windows_left = NUM_RGB_HW_WINDOWS - 1;

            rects[dma_ch_idx].push_back(fb_rect);
        } else {
            rgb_windows_left = NUM_RGB_HW_WINDOWS;
        }

        changed = false;
        mGscLayers = 0;
        mCurrentGscIndex = 0;
        for (size_t i = 0; i < contents->numHwLayers; i++) {
            hwc_layer_1_t &layer = contents->hwLayers[i];

            if ((layer.flags & HWC_SKIP_LAYER) || layer.compositionType == HWC_FRAMEBUFFER_TARGET)
                continue;

            private_handle_t *handle = private_handle_t::dynamicCast(layer.handle);

            // we've already accounted for the framebuffer above
            if (layer.compositionType == HWC_FRAMEBUFFER)
                continue;

            // only layer 0 can be HWC_BACKGROUND, so we can
            // unconditionally allow it without extra checks
            if (layer.compositionType == HWC_BACKGROUND) {
                rgb_windows_left--;
                continue;
            }
            dma_ch_idx = FIMD_DMA_CH_IDX[win_idx];

            size_t pixels_needed = WIDTH(layer.displayFrame) * HEIGHT(layer.displayFrame);

            bool can_compose_rgb = rgb_windows_left && (win_idx < NUM_HW_WINDOWS) &&
                            ((pixel_used[dma_ch_idx] + pixels_needed) <=
                            (uint32_t)this->mDmaChannelMaxBandwidth[dma_ch_idx]);
            int gsc_index = getMPPForUHD(layer);

            bool gsc_required = mMPPs[gsc_index]->isProcessingRequired(layer, handle->format);
            ALOGD("%s: can_compose_rgb(%d) gsc_required(%d)", __FUNCTION__, can_compose_rgb, gsc_required);

            if (gsc_required) {
                //TODO: handle can_compose_other
                /*if (mGscLayers >= MAX_VIDEO_LAYERS)
                    can_compose = can_compose && !mGscUsed;
                if (mHwc->hwc_ctrl.num_of_video_ovly <= mGscLayers)
                    can_compose = false;*/
            }

            // hwc_rect_t right and bottom values are normally exclusive;
            // the intersection logic is simpler if we make them inclusive
            hwc_rect_t visible_rect = layer.displayFrame;
            visible_rect.right--; visible_rect.bottom--;

            if (can_compose_rgb) {
                switch (this->mDmaChannelMaxOverlapCount[dma_ch_idx]) {
                case 1: // It means, no layer overlap is allowed
                    for (size_t j = 0; j < rects[dma_ch_idx].size(); j++)
                         if (intersect(visible_rect, rects[dma_ch_idx].itemAt(j)))
                            can_compose_rgb = false;
                    break;
                case 2: //It means, upto 2 layer overlap is allowed.
                    for (size_t j = 0; j < overlaps[dma_ch_idx].size(); j++)
                        if (intersect(visible_rect, overlaps[dma_ch_idx].itemAt(j)))
                            can_compose_rgb = false;
                    break;
                default:
                    break;
                }
                if (!can_compose_rgb)
                    this->mBypassSkipStaticLayer = true;
            }

            ALOGD("%s: AFTER RECT HANDLING: can_compose_rgb(%d)", __FUNCTION__, can_compose_rgb);

            if (!can_compose_rgb) {
                layer.compositionType = HWC_FRAMEBUFFER;
                if (!mFbNeeded) {
                    mFirstFb = mLastFb = i;
                    mFbNeeded = true;
                } else {
                    mFirstFb = min(i, mFirstFb);
                    mLastFb = max(i, mLastFb);
                }
                changed = true;
                mFirstFb = min(mFirstFb, (size_t)NUM_HW_WINDOWS-1);
                break;
            }

            for (size_t j = 0; j < rects[dma_ch_idx].size(); j++) {
                const hwc_rect_t &other_rect = rects[dma_ch_idx].itemAt(j);
                if (intersect(visible_rect, other_rect))
                    overlaps[dma_ch_idx].push_back(intersection(visible_rect, other_rect));
            }

            rects[dma_ch_idx].push_back(visible_rect);
            pixel_used[dma_ch_idx] += pixels_needed;

            win_idx++;
            win_idx = (win_idx == mFirstFb) ? (win_idx + 1) : win_idx;
            win_idx = min(win_idx, (uint32_t) (NUM_HW_WINDOWS - 1));

            if (can_compose_rgb && !gsc_required)
                rgb_windows_left--;

            // TODO: handle windows_left_other
            //windows_left--;

            if (gsc_required) {
                mGscUsed = true;
                mGscLayers++;
            }
        }

        if (changed)
            for (size_t i = mFirstFb; i < mLastFb; i++)
                contents->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        handleTotalBandwidthOverload(contents);
    } while(changed);
}

void ExynosOverlayDisplay::assignWindows(hwc_display_contents_1_t *contents)
{
    unsigned int nextWindow = 0;

    //ALOGD("%s", __FUNCTION__);

    for (size_t i = 0; i < contents->numHwLayers; i++) {
        hwc_layer_1_t &layer = contents->hwLayers[i];

        if (mFbNeeded && i == mFirstFb) {
            //ALOGD("%s: assigning framebuffer to window %u\n", __FUNCTION__, nextWindow);
            mPostData.fb_window = nextWindow;
            nextWindow++;
            continue;
        }

        if (layer.compositionType != HWC_FRAMEBUFFER &&
                layer.compositionType != HWC_FRAMEBUFFER_TARGET) {
            //ALOGD("assigning layer %u to window %u", i, nextWindow);

            this->mPostData.overlay_map[nextWindow] = i;

            if (layer.compositionType == HWC_OVERLAY) {
                private_handle_t *handle =
                        private_handle_t::dynamicCast(layer.handle);

                layer.hints = HWC_HINT_CLEAR_FB; //TODO - check whether it's necessary

                if (mMPPs[0]->isProcessingRequired(layer, handle->format)) {
                    ALOGD("%s processing required, format(0x%x)", __FUNCTION__, handle->format);
                    //TODO
                    //this->mPostData.decon_dma_type[nextWindow] = getDeconDMAType(?);
                } else {
                    if (isFormatRgb(handle->format)) {
                        this->mPostData.decon_dma_type[i] = getRGBDeconDMAType(this->mPostData.numRgbOvly);
                        ALOGD("%s: rgb format decon_dma_type[%d]=%d", __FUNCTION__, nextWindow, this->mPostData.decon_dma_type[i]);
                        this->mPostData.numRgbOvly++;
                    } else {
                        ALOGD("%s window %u processing not required, format is not RGB (0x%x)", __FUNCTION__, nextWindow, handle->format);
                    }
                }
            }
            nextWindow++;
        }
    }

    ALOGD("%s: Assigned %u windows", __FUNCTION__, nextWindow);
}

bool ExynosOverlayDisplay::assignGscLayer(hwc_layer_1_t &layer, int index, int nextWindow)
{
    private_handle_t *handle = private_handle_t::dynamicCast(layer.handle);
    size_t gscIndex = 0;
    int down_ratio = mMPPs[0]->getDownscaleRatio(this->mXres, this->mYres);
    bool ret = true;
    if (nextWindow == 0 &&
            mMPPs[0]->isProcessingSupported(layer, handle->format, true, down_ratio)) {
        gscIndex = mCurrentGscIndex;
        ALOGV("\tusing gscaler %u in LOCAL-PATH", AVAILABLE_GSC_UNITS[gscIndex]);
        mPostData.gsc_map[nextWindow].mode = exynos5_gsc_map_t::GSC_LOCAL;
        mMPPs[gscIndex]->setMode(exynos5_gsc_map_t::GSC_LOCAL);
        mPostData.gsc_map[nextWindow].idx = FIMD_GSC_IDX;
        if (mOtfMode == OTF_OFF || mOtfMode == SEC_M2M)
            mOtfMode = OTF_RUNNING;
        mMPPs[0]->mNeedReqbufs = false;
    } else {
        gscIndex = getMPPForUHD(layer);
        if (gscIndex == FIMD_GSC_IDX) {
            gscIndex = mCurrentGscIndex;
            ret = true;
        } else {
            ret = false;
        }
        ALOGV("\tusing gscaler %u in M2M", AVAILABLE_GSC_UNITS[gscIndex]);
        mPostData.gsc_map[nextWindow].mode = exynos5_gsc_map_t::GSC_M2M;
        mMPPs[gscIndex]->setMode(exynos5_gsc_map_t::GSC_M2M);
        mPostData.gsc_map[nextWindow].idx = gscIndex;
        if (mOtfMode == OTF_RUNNING) {
            ALOGV("change from OTF to M2M");
            mOtfMode = OTF_TO_M2M;
            mMPPs[gscIndex]->setMode(exynos5_gsc_map_t::GSC_LOCAL);
            mMPPs[FIMD_GSC_SEC_IDX]->setMode(exynos5_gsc_map_t::GSC_M2M);
            mPostData.gsc_map[nextWindow].idx = FIMD_GSC_SEC_IDX;
        }
        if (mOtfMode == SEC_M2M) {
            mMPPs[gscIndex]->setMode(exynos5_gsc_map_t::GSC_NONE);
            mMPPs[FIMD_GSC_SEC_IDX]->setMode(exynos5_gsc_map_t::GSC_M2M);
            mPostData.gsc_map[nextWindow].idx = FIMD_GSC_SEC_IDX;
        }
    }
    ALOGV("\tusing gscaler %u",
            AVAILABLE_GSC_UNITS[gscIndex]);
    return ret;
}

int ExynosOverlayDisplay::waitForRenderFinish(buffer_handle_t *handle, int buffers)
{
    return 0;
}

int ExynosOverlayDisplay::postGscM2M(hwc_layer_1_t &layer, struct decon_win_config *config, int win_map, int index)
{
    exynos5_hwc_post_data_t *pdata = &mPostData;
    int gsc_idx = pdata->gsc_map[index].idx;
    int dst_format = HAL_PIXEL_FORMAT_RGBX_8888;
    private_handle_t *handle = private_handle_t::dynamicCast(layer.handle);

    hwc_frect_t sourceCrop = { 0, 0,
            (float)WIDTH(layer.displayFrame), (float)HEIGHT(layer.displayFrame) };
    if (mHwc->mS3DMode == S3D_MODE_READY || mHwc->mS3DMode == S3D_MODE_RUNNING) {
        int S3DFormat = getS3DFormat(mHwc->mHdmiPreset);
        if (S3DFormat == S3D_SBS)
            mMPPs[gsc_idx]->mS3DMode = S3D_SBS;
        else if (S3DFormat == S3D_TB)
            mMPPs[gsc_idx]->mS3DMode = S3D_TB;
    } else {
        mMPPs[gsc_idx]->mS3DMode = S3D_NONE;
    }

    if (isFormatRgb(handle->format))
        waitForRenderFinish(&layer.handle, 1);
    /* OFF_Screen to ON_Screen changes */
    if (getDrmMode(handle->flags) == SECURE_DRM)
        recalculateDisplayFrame(layer, mXres, mYres);

    int err = mMPPs[gsc_idx]->processM2M(layer, dst_format, &sourceCrop);
    if (err < 0) {
        ALOGE("failed to configure gscaler %u for layer %u",
                gsc_idx, index);
        pdata->gsc_map[index].mode = exynos5_gsc_map_t::GSC_NONE;
        return -1;
    }

    buffer_handle_t dst_buf = mMPPs[gsc_idx]->mDstBuffers[mMPPs[gsc_idx]->mCurrentBuf];
    private_handle_t *dst_handle =
            private_handle_t::dynamicCast(dst_buf);
    int fence = mMPPs[gsc_idx]->mDstConfig.releaseFenceFd;
    configureHandle(dst_handle, 0, layer, fence, config[win_map], (enum decon_idma_type)0);
    return 0;
}

int ExynosOverlayDisplay::postGscOtf(hwc_layer_1_t &layer, struct decon_win_config *config, int win_map, int index)
{
    exynos5_hwc_post_data_t *pdata = &mPostData;
    int gsc_idx = pdata->gsc_map[index].idx;
    if (mHwc->mS3DMode == S3D_MODE_READY || mHwc->mS3DMode == S3D_MODE_RUNNING) {
        int S3DFormat = getS3DFormat(mHwc->mHdmiPreset);
        if (S3DFormat == S3D_SBS)
            mMPPs[gsc_idx]->mS3DMode = S3D_SBS;
        else if (S3DFormat == S3D_TB)
            mMPPs[gsc_idx]->mS3DMode = S3D_TB;
    } else {
        mMPPs[gsc_idx]->mS3DMode = S3D_NONE;
    }

    int err = mMPPs[gsc_idx]->processOTF(layer);

    if (err < 0) {
        ALOGE("failed to config_gsc_localout %u input for layer %u", gsc_idx, index);
        return -1;
    }
    configureOtfWindow(layer.displayFrame, layer.blending, layer.planeAlpha,
                        HAL_PIXEL_FORMAT_RGBX_8888, config[win_map]);
    return 0;
}

void ExynosOverlayDisplay::handleStaticLayers(hwc_display_contents_1_t *contents, struct decon_win_config_data &win_data, int tot_ovly_wins)
{
    int win_map = 0;
    if (mLastFbWindow >= NUM_HW_WINDOWS) {
        ALOGE("handleStaticLayers:: invalid mLastFbWindow(%d)", mLastFbWindow);
        return;
    }
    win_map = getDeconWinMap(mLastFbWindow, tot_ovly_wins);
    ALOGV("[USE] SKIP_STATIC_LAYER_COMP, mLastFbWindow(%d), win_map(%d)\n", mLastFbWindow, win_map);

    memcpy(&win_data.config[win_map],
            &mLastConfig[win_map], sizeof(struct decon_win_config));
    win_data.config[win_map].fence_fd = -1;

    for (size_t i = mFirstFb; i <= mLastFb; i++) {
        hwc_layer_1_t &layer = contents->hwLayers[i];
        if (layer.compositionType == HWC_OVERLAY) {
            ALOGV("[SKIP_STATIC_LAYER_COMP] layer.handle: 0x%p, layer.acquireFenceFd: %d\n", layer.handle, layer.acquireFenceFd);
            layer.releaseFenceFd = layer.acquireFenceFd;
        }
    }
}

int ExynosOverlayDisplay::getMPPForUHD(hwc_layer_1_t &layer)
{
    return FIMD_GSC_IDX;
}

void ExynosOverlayDisplay::cleanupGscs()
{
    if (mMPPs[FIMD_GSC_IDX]->isM2M()) {
        mMPPs[FIMD_GSC_IDX]->cleanupM2M();
        mMPPs[FIMD_GSC_IDX]->setMode(exynos5_gsc_map_t::GSC_NONE);
    } else if (mMPPs[FIMD_GSC_IDX]->isOTF()) {
        mMPPs[FIMD_GSC_IDX]->cleanupOTF();
        if (mOtfMode == OTF_TO_M2M)
            mOtfMode = SEC_M2M;
        else
            mOtfMode = OTF_OFF;
    } else if (mMPPs[FIMD_GSC_SEC_IDX]->isM2M()) {
        mMPPs[FIMD_GSC_SEC_IDX]->cleanupM2M();
        mMPPs[FIMD_GSC_SEC_IDX]->setMode(exynos5_gsc_map_t::GSC_NONE);
    }
    for (int i = FIMD_GSC_SEC_IDX + 1; i < mNumMPPs; i++) {
        if (mMPPs[i]->isM2M()) {
            mMPPs[i]->cleanupM2M();
            mMPPs[i]->setMode(exynos5_gsc_map_t::GSC_NONE);
        }
    }
}

void ExynosOverlayDisplay::freeMPP()
{
    for (int i = 0; i < mNumMPPs; i++)
        mMPPs[i]->free();
}

void ExynosOverlayDisplay::handleTotalBandwidthOverload(hwc_display_contents_1_t *contents)
{
}
