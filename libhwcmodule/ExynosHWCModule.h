/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ANDROID_EXYNOS_HWC_MODULE_H_
#define ANDROID_EXYNOS_HWC_MODULE_H_
#include <hardware/hwcomposer.h>

#define VSYNC_DEV_PREFIX "/sys/devices/"
#define VSYNC_DEV_MIDDLE "13a10000.sysmmu/13a00000.sysmmu/"
#define VSYNC_DEV_NAME  "13930000.decon_fb/vsync"

#define FIMD_WORD_SIZE_BYTES   16
#define FIMD_BURSTLEN   8
#define FIMD_ADDED_BURSTLEN_BYTES     4
#define FIMD_BW_OVERLAP_CHECK

#define TRY_SECOND_VSYNC_DEV
#ifdef TRY_SECOND_VSYNC_DEV
#define VSYNC_DEV_NAME2  "exynos5-fb.1/vsync"
#define VSYNC_DEV_MIDDLE2  "platform/exynos-sysmmu.30/exynos-sysmmu.11/"
#endif

#define DUAL_VIDEO_OVERLAY_SUPPORT

#ifdef FIMD_BW_OVERLAP_CHECK
const size_t MAX_NUM_FIMD_DMA_CH = 5;
const uint32_t FIMD_DMA_CH_IDX[] = {0, 1, 1, 1, 0};
const uint32_t FIMD_DMA_CH_BW_SET1[MAX_NUM_FIMD_DMA_CH] = {1920 * 1080, 1920 * 1080, 1920 * 1080, 1920 * 1080, 1920 * 1080};
const uint32_t FIMD_DMA_CH_BW_SET2[MAX_NUM_FIMD_DMA_CH] = {2560 * 1440, 2560 * 1600, 2560 * 1600, 2560 * 1600, 0};
const uint32_t FIMD_DMA_CH_OVERLAP_CNT_SET1[MAX_NUM_FIMD_DMA_CH] = {1, 1, 1, 1, 1};
const uint32_t FIMD_DMA_CH_OVERLAP_CNT_SET2[MAX_NUM_FIMD_DMA_CH] = {1, 1, 1, 1, 0};

inline void fimd_bw_overlap_limits_init(int xres, int yres,
            uint32_t *fimd_dma_chan_max_bw, uint32_t *fimd_dma_chan_max_overlap_cnt)
{
    if (xres * yres > 1920 * 1080) {
        for (size_t i = 0; i < MAX_NUM_FIMD_DMA_CH; i++) {
            fimd_dma_chan_max_bw[i] = FIMD_DMA_CH_BW_SET2[i];
            fimd_dma_chan_max_overlap_cnt[i] = FIMD_DMA_CH_OVERLAP_CNT_SET2[i];
        }
    } else {
        for (size_t i = 0; i < MAX_NUM_FIMD_DMA_CH; i++) {
            fimd_dma_chan_max_bw[i] = FIMD_DMA_CH_BW_SET1[i];
            fimd_dma_chan_max_overlap_cnt[i] = FIMD_DMA_CH_OVERLAP_CNT_SET1[i];
        }
    }
}
#endif

const size_t GSC_DST_W_ALIGNMENT_RGB888 = 16;
const size_t GSC_DST_CROP_W_ALIGNMENT_RGB888 = 1;
const size_t GSC_W_ALIGNMENT = 16;
const size_t GSC_H_ALIGNMENT = 16;
const size_t GSC_DST_H_ALIGNMENT_RGB888 = 1;
const size_t FIMD_GSC_IDX = 0;
const size_t FIMD_GSC_SEC_IDX = 1;
const size_t HDMI_GSC_IDX = 2;
#ifdef USES_VIRTUAL_DISPLAY
const size_t WFD_GSC_IDX = 3;
#else
const size_t WFD_GSC_DRM_IDX = 3;
#endif
const int FIMD_GSC_USAGE_IDX[] = {FIMD_GSC_IDX, FIMD_GSC_SEC_IDX};
#ifdef USES_VIRTUAL_DISPLAY
const int AVAILABLE_GSC_UNITS[] = { 0, 2, 1, 1, 5, 4};
#else
const int AVAILABLE_GSC_UNITS[] = { 0, 1, 1, 5 };
#endif

#endif
