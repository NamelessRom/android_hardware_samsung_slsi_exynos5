#include "ExynosDisplay.h"
#include "ExynosMPPModule.h"

ExynosDisplay::ExynosDisplay(int numGSCs)
{
}

ExynosDisplay::~ExynosDisplay()
{
}

const int RGB_DMATYPE[] = {IDMA_G0, IDMA_G1, IDMA_G2, IDMA_G3};

int ExynosDisplay::getDeconWinMap(int overlayIndex, int totalOverlays)
{
    if (totalOverlays <= 3)
        return WINMAP0[overlayIndex];

    if (totalOverlays == 4)
        return WINMAP1[overlayIndex];

    return overlayIndex;
}

int ExynosDisplay::getDeconDMAType(ExynosMPPModule *m)
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

    return result;
}

int ExynosDisplay::prepare(hwc_display_contents_1_t *contents)
{
    return 0;
}

int ExynosDisplay::set(hwc_display_contents_1_t *contents)
{
    return 0;
}

void ExynosDisplay::freeMPP()
{
}
