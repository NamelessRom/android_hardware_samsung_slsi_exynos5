/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/hardware.h>

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>
#include <linux/fb.h>

#include <EGL/egl.h>

#include "gralloc_priv.h"

/*****************************************************************************/

#ifndef _VSYNC_PERIOD
#define _VSYNC_PERIOD 1000000000UL
#endif

struct hwc_context_t {
    hwc_composer_device_1_t device;
    /* our private state goes below here */
    const private_module_t  *gralloc_module;
    framebuffer_device_t    *fb_device;

    const hwc_procs_t       *procs;
};

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "Sample hwcomposer module",
        author: "The Android Open Source Project",
        methods: &hwc_module_methods,
    }
};

/*****************************************************************************/

static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);
}

static int hwc_eventControl(struct hwc_composer_device_1* dev, __unused int dpy,
        int event, int enabled)
{
    int val = 0, rc = 0;
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    switch (event) {
    case HWC_EVENT_VSYNC:
        val = enabled;
        ALOGD("%s: HWC_EVENT_VSYNC, enabled=%d", __FUNCTION__, val);
        return rc;
    }
    return -EINVAL;
}

static int hwc_query(struct hwc_composer_device_1* dev,
        int what, int* value)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    ALOGD("%s", __FUNCTION__);

    switch (what) {
    case HWC_BACKGROUND_LAYER_SUPPORTED:
        // stock blob do support background layer
        value[0] = 1;
        break;
    case HWC_VSYNC_PERIOD:
        // vsync period in nanosecond
        value[0] = _VSYNC_PERIOD / ctx->fb_device->fps;
        break;
    default:
        // unsupported query
        return -EINVAL;
    }
    return 0;
}

static void hwc_registerProcs(struct hwc_composer_device_1* dev,
        hwc_procs_t const* procs)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    ctx->procs = const_cast<hwc_procs_t *>(procs);
}

static int hwc_getDisplayConfigs(struct hwc_composer_device_1* dev, int disp,
    uint32_t* configs, size_t* numConfigs)
{
    ALOGD("%s", __FUNCTION__);

    if (*numConfigs == 0)
        return 0;

    if (disp == HWC_DISPLAY_PRIMARY) {
        configs[0] = 0;
        *numConfigs = 1;
        return 0;
    }

    return -EINVAL;
}

static int hwc_getDisplayAttributes(struct hwc_composer_device_1* dev, int disp,
    __unused uint32_t config, const uint32_t* attributes, int32_t* values)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    int i = 0;

    ALOGD("%s", __FUNCTION__);

    while(attributes[i] != HWC_DISPLAY_NO_ATTRIBUTE) {
        switch(disp) {
        case HWC_DISPLAY_PRIMARY:

            switch(attributes[i]) {
            case HWC_DISPLAY_VSYNC_PERIOD: /* The vsync period in nanoseconds */
                values[i] = _VSYNC_PERIOD / ctx->fb_device->fps;
                break;

            case HWC_DISPLAY_WIDTH: /* The number of pixels in the horizontal and vertical directions. */
                values[i] = ctx->fb_device->width;
                break;

            case HWC_DISPLAY_HEIGHT:
                values[i] = ctx->fb_device->height;
                break;

            case HWC_DISPLAY_DPI_X:
                values[i] = ctx->fb_device->xdpi;
                break;

            case HWC_DISPLAY_DPI_Y:
                values[i] = ctx->fb_device->ydpi;
                break;

            case HWC_DISPLAY_COLOR_TRANSFORM:
                /* Indicates which of the vendor-defined color transforms is provided by
                 * this configuration. */
                values[i] = 0;
                break;

            default:
                ALOGE("%s:unknown display attribute %d", __FUNCTION__, attributes[i]);
                return -EINVAL;
            }
            break;

        default:
            ALOGE("%s:unknown display %d", __FUNCTION__, disp);
            return -EINVAL;
        }

        i++;
    }

    return 0;
}

static int hwc_blank(struct hwc_composer_device_1 *dev, int disp, int blank)
{
    struct hwc_context_t *pdev = (struct hwc_context_t *)dev;

    ALOGD("%s blank=%d", __FUNCTION__, blank);

    switch (disp) {
    case HWC_DISPLAY_PRIMARY: {
        int fb_blank = blank ? FB_BLANK_POWERDOWN : FB_BLANK_UNBLANK;
        int err = ioctl(pdev->gralloc_module->fb_fd, FBIOBLANK, fb_blank);
        if (err < 0) {
            if (errno == EBUSY)
                ALOGI("%sblank ioctl failed (display already %sblanked)",
                        blank ? "" : "un", blank ? "" : "un");
            else
                ALOGE("%sblank ioctl failed: %s", blank ? "" : "un",
                        strerror(errno));
            return -errno;
        }
        break;
    }

    default:
        return -EINVAL;
    }

    return 0;
}

static int hwc_prepare(hwc_composer_device_1_t *dev,
        size_t numDisplays, hwc_display_contents_1_t** displays) {
    if (displays && (displays[0]->flags & HWC_GEOMETRY_CHANGED)) {
        for (size_t i=0 ; i<displays[0]->numHwLayers ; i++) {
            //dump_layer(&list->hwLayers[i]);
            displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        }
    }
    return 0;
}

static int hwc_set(hwc_composer_device_1_t *dev,
        size_t numDisplays, hwc_display_contents_1_t** displays)
{
    //for (size_t i=0 ; i<list->numHwLayers ; i++) {
    //    dump_layer(&list->hwLayers[i]);
    //}

    EGLBoolean sucess = eglSwapBuffers((EGLDisplay)displays[0]->dpy,
            (EGLSurface)displays[0]->sur);
    if (!sucess) {
        return HWC_EGL_ERROR;
    }
    return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    if (ctx) {
        free(ctx);
    }
    return 0;
}

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = -EINVAL;
    struct hwc_context_t *dev;

    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        dev = (hwc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize fb */
        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const struct hw_module_t **)&dev->gralloc_module)) {
            ALOGE("%s: failed to get gralloc hw module", __FUNCTION__);
            goto err_get_module;
        }

        if (framebuffer_open((const hw_module_t *)dev->gralloc_module, &dev->fb_device)) {
            ALOGE("%s: failed to open framebuffer", __FUNCTION__);
            goto err_get_module;
        }

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;

        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
        dev->device.eventControl = hwc_eventControl;
        dev->device.blank = hwc_blank;
        dev->device.query = hwc_query;
        dev->device.registerProcs = hwc_registerProcs;
//        dev->device.dump = exynos5_dump;
        dev->device.getDisplayConfigs = hwc_getDisplayConfigs;
        dev->device.getDisplayAttributes = hwc_getDisplayAttributes;

        *device = &dev->device.common;
        status = 0;
    }

    return status;

err_get_module:
    free(dev);
    return status;
}
