#ifndef EXYNOS_DISPLAY_H
#define EXYNOS_DISPLAY_H

#include "ExynosHWC.h"

const int WINMAP0[] = { 0, 3, 4, 1, 2 };
const int WINMAP1[] = { 0, 1, 3, 4, 2 };

class ExynosMPPModule;

class ExynosDisplay {
    public:
        /* Methods */
        ExynosDisplay(int numGSCs);
        virtual ~ExynosDisplay();

        virtual int getDeconWinMap(int overlayIndex, int totalOverlays);
        virtual enum decon_idma_type getDeconDMAType(ExynosMPPModule *m);
        virtual enum decon_idma_type getRGBDeconDMAType(int rgbLayerIdx);
        virtual int prepare(hwc_display_contents_1_t *contents);
        virtual int set(hwc_display_contents_1_t *contents);
        virtual void freeMPP();

        /* Fields */
        int                     mDisplayFd;
        int32_t                 mXres;
        int32_t                 mYres;

        int32_t                 mXdpi;
        int32_t                 mYdpi;
        int32_t                 mVsyncPeriod;

        int                     mOtfMode;
        bool                    mHasDrmSurface;
        alloc_device_t          *mAllocDevice;
        int                     mNumMPPs;

        struct exynos5_hwc_composer_device_1_t *mHwc;
};

#endif
