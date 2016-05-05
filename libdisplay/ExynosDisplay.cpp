#include "ExynosDisplay.h"
#include "ExynosMPPModule.h"

ExynosDisplay::ExynosDisplay(int numGSCs)
{
}

ExynosDisplay::~ExynosDisplay()
{
}

int ExynosDisplay::getDeconWinMap(int overlayIndex, int totalOverlays)
{
    if (totalOverlays <= 3)
        return WINMAP0[overlayIndex];

    if (totalOverlays == 4)
        return WINMAP1[overlayIndex];

    return overlayIndex;
}

enum decon_idma_type RGB_DMATYPE[] = {IDMA_G0, IDMA_G1, IDMA_G2, IDMA_G3};

enum decon_idma_type ExynosDisplay::getDeconDMAType(ExynosMPPModule *m)
{
    int result = -1;

    if (m->mGscId) {
        if (m->mGscId == 2) {
            result = m->mIndex + IDMA_VGR0;
        } else if (m->mGscId != 10 || m->mIndex > 3) {
            result = -1;
        } else {
            //GSC Id 10 is used for RGB
            result = RGB_DMATYPE[m->mIndex];
        }
    } else {
        result = m->mIndex + IDMA_VG0;
    }

    return (enum decon_idma_type) result;
}

enum decon_idma_type ExynosDisplay::getRGBDeconDMAType(int rgbLayerIdx)
{
    return RGB_DMATYPE[rgbLayerIdx];
}

int ExynosDisplay::prepare(hwc_display_contents_1_t *contents)
{
    ALOGD("%s: returning 0", __FUNCTION__);
    return 0;
}

int ExynosDisplay::set(hwc_display_contents_1_t *contents)
{
    return 0;
}

void ExynosDisplay::freeMPP()
{
}
