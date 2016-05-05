#include "ExynosPrimaryDisplay.h"
#include "ExynosHWCModule.h"

ExynosPrimaryDisplay::ExynosPrimaryDisplay(int numGSCs, struct exynos5_hwc_composer_device_1_t *pdev) :
    ExynosOverlayDisplay(numGSCs, pdev)
{
}

ExynosPrimaryDisplay::~ExynosPrimaryDisplay()
{
}

int ExynosPrimaryDisplay::getDeconWinMap(int overlayIndex, int totalOverlays)
{
    if (totalOverlays <= 3)
        return WINMAP0[overlayIndex];        

    if (totalOverlays == 4)
        return WINMAP1[overlayIndex];

    return overlayIndex;
}
