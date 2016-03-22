#include "MppFactory.h"
#include "LibMpp.h"

#define LOG_TAG "libmpp"

LibMpp *MppFactory::CreateMpp(int id, int mode, int outputMode, int drm)
{
    ALOGD("%s(%d)\n", __func__, __LINE__);
#ifdef ENABLE_GSCALER
    return reinterpret_cast<LibMpp *>(exynos_gsc_create_exclusive(id,
					    mode, outputMode, drm));
#else
    return reinterpret_cast<LibMpp *>(exynos_fimc_create_exclusive(id,
					    mode, outputMode, drm));
#endif
}

