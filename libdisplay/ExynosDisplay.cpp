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

//enum decon_idma_type RGB_DMATYPE[] = {IDMA_G0, IDMA_G1, IDMA_G2, IDMA_G3};

enum decon_idma_type ExynosDisplay::getDeconDMAType(enum mppLocality loc, int layerIdx)
{
    int result = -1;

    switch(loc) {
    case DECON_MPP_INT:
        result = layerIdx + IDMA_VGR0;
        break;

    case DECON_MPP_EXT:
        result = layerIdx + IDMA_VG0;
        break;

    case DECON_RGB:
        result = mRgbDmaType[layerIdx];
        break;
    }

    return (enum decon_idma_type) result;
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
