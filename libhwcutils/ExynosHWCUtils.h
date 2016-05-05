#ifndef HWC_UTILS_H
#define HWC_UTILS_H

#include "ExynosHWC.h"

#define debug_level 0
//#define debug_level (DBG_PREPARE_LAYERS | DBG_WIN_CONFIG | DBG_OVLY | DBG_BW | DBG_BYPASS | DBG_ASSIGN | DBG_SET)

#define DBG_DUMP_LAYER        1 << 0
#define DBG_PREPARE_LAYERS   (1 >> 1) | DBG_DUMP_LAYER
#define DBG_OVLY              1 << 2
#define DBG_BW                1 << 3
#define DBG_BYPASS            1 << 4
#define DBG_ASSIGN            1 << 5
#define DBG_SET               1 << 6
#define DBG_WIN_CONFIG        1 << 7
#define DBG_COMP_MODE_SWITCH  1 << 8

// dump layers and handles
#define LOG_DUMP(...) \
    do { \
        if (debug_level & DBG_DUMP_LAYER) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// prepare phase
#define LOG_PREP(...) \
    do { \
        if (debug_level & DBG_PREPARE_LAYERS) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// show overlay information
#define LOG_OVLY(...) \
    do { \
        if (debug_level & DBG_OVLY) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// show Bandwith information
#define LOG_BW(...) \
    do { \
        if (debug_level & DBG_BW) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// show info when dropping frames to GLES when geometry changed
#define LOG_BYPASS(...) \
    do { \
        if (debug_level & DBG_BYPASS) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// show assign windows info
#define LOG_ASSIGN(...) \
    do { \
        if (debug_level & DBG_ASSIGN) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// set phase
#define LOG_SET(...) \
    do { \
        if (debug_level & DBG_SET) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// show win config
#define LOG_WCFG(...) \
    do { \
        if (debug_level & DBG_WIN_CONFIG) \
            ALOGD(__VA_ARGS__); \
    } while(0)

// show comp mode switch info
#define LOG_CMSW(...) \
    do { \
        if (debug_level & DBG_COMP_MODE_SWITCH) \
            ALOGD(__VA_ARGS__); \
    } while(0)

class ExynosMPP;

inline int WIDTH(const hwc_rect &rect) { return rect.right - rect.left; }
inline int HEIGHT(const hwc_rect &rect) { return rect.bottom - rect.top; }
inline int WIDTH(const hwc_frect_t &rect) { return (int)(rect.right - rect.left); }
inline int HEIGHT(const hwc_frect_t &rect) { return (int)(rect.bottom - rect.top); }

template<typename T> inline T max(T a, T b) { return (a > b) ? a : b; }
template<typename T> inline T min(T a, T b) { return (a < b) ? a : b; }

template<typename T> void alignCropAndCenter(T &w, T &h,
        hwc_frect_t *crop, size_t alignment)
{
    double aspect = 1.0 * h / w;
    T w_orig = w, h_orig = h;

    w = ALIGN(w, alignment);
    h = round(aspect * w);
    if (crop) {
        crop->left = (w - w_orig) / 2;
        crop->top = (h - h_orig) / 2;
        crop->right = crop->left + w_orig;
        crop->bottom = crop->top + h_orig;
    }
}

inline bool intersect(const hwc_rect &r1, const hwc_rect &r2)
{
    return !(r1.left > r2.right ||
        r1.right < r2.left ||
        r1.top > r2.bottom ||
        r1.bottom < r2.top);
}

inline hwc_rect intersection(const hwc_rect &r1, const hwc_rect &r2)
{
    hwc_rect i;
    i.top = max(r1.top, r2.top);
    i.bottom = min(r1.bottom, r2.bottom);
    i.left = max(r1.left, r2.left);
    i.right = min(r1.right, r2.right);
    return i;
}

inline bool yuvConfigChanged(video_layer_config &c1, video_layer_config &c2)
{
    return c1.x != c2.x ||
            c1.y != c2.y ||
            c1.w != c2.w ||
            c1.h != c2.h ||
            c1.fw != c2.fw ||
            c1.fh != c2.fh ||
            c1.format != c2.format ||
            c1.rot != c2.rot ||
            c1.cacheable != c2.cacheable ||
            c1.drmMode != c2.drmMode ||
            c1.index != c2.index;
}

void dumpHandle(private_handle_t *h);
void dumpLayer(hwc_layer_1_t const *l);
void dumpConfig(decon_win_config &c);
void dumpConfig(int winIdx, decon_win_config &c);
void dumpMPPImage(exynos_mpp_img &c);
bool isDstCropWidthAligned(int dest_w);
bool isTransformed(const hwc_layer_1_t &layer);
bool isRotated(const hwc_layer_1_t &layer);
bool isScaled(const hwc_layer_1_t &layer);
enum decon_pixel_format halFormatToS3CFormat(int format);
bool isFormatSupported(int format);
bool isFormatRgb(int format);
bool isFormatYCrCb(int format);
bool isFormatYUV420(int format);
uint8_t formatToBpp(int format);
bool isXAligned(const hwc_layer_1_t &layer, int format);
int getDrmMode(int flags);
int halFormatToV4L2Format(int format);
enum decon_blending halBlendingToS3CBlending(int32_t blending);
bool isBlendingSupported(int32_t blending);
bool isOffscreen(hwc_layer_1_t &layer, int xres, int yres);
bool isSrcCropFloat(hwc_frect &frect);
size_t visibleWidth(ExynosMPP *processor, hwc_layer_1_t &layer, int format,
        int xres);
bool compareYuvLayerConfig(int videoLayers, uint32_t index,
        hwc_layer_1_t &layer,
        video_layer_config *pre_src_data, video_layer_config *pre_dst_data);
size_t getRequiredPixels(hwc_layer_1_t &layer, int xres, int yres);
void recalculateDisplayFrame(hwc_layer_1_t &layer, int xres, int yres);

int getS3DFormat(int preset);
#endif
