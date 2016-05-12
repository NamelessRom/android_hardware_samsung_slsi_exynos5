#ifndef LIBMPP_H_
#define LIBMPP_H_

#include <cutils/log.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <system/graphics.h>

#include "exynos_format.h"
#include "exynos_v4l2.h"
#ifdef ENABLE_GSCALER
#include "exynos_gscaler.h"
#endif
#ifdef ENABLE_FIMC
#include "exynos_fimc.h"
#endif

class LibMpp {
public:
    LibMpp() {
        ALOGD("%s\n", __func__);
    }
    virtual ~LibMpp() {
        ALOGD("%s\n", __func__);
    }
    virtual int ConfigMpp(void *handle, exynos_mpp_img *src,
				   exynos_mpp_img *dst) = 0;
    virtual int RunMpp(void *handle, exynos_mpp_img *src,
				   exynos_mpp_img *dst) = 0;
    virtual int StopMpp(void *handle) = 0;
    virtual void DestroyMpp(void *handle) = 0;
    virtual int SetCSCProperty(void *handle, unsigned int eqAuto,
		   unsigned int fullRange, unsigned int colorspace) = 0;
    virtual int FreeMpp(void *handle) = 0;
};

#endif
