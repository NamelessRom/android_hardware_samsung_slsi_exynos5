/*
**
** Copyright 2013, Samsung Electronics Co. LTD
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef EXYNOS_CAMERA_PIPE_JPEG_H
#define EXYNOS_CAMERA_PIPE_JPEG_H

#include "ExynosCameraPipe.h"
/* #include "csc.h" */
#include "ExynosJpegEncoderForCamera.h"

#define CSC_MEMORY_TYPE         CSC_MEMORY_DMABUF /* (CSC_MEMORY_USERPTR) */

namespace android {

typedef ExynosCameraList<ExynosCameraFrame *> frame_queue_t;

class ExynosCameraPipeJpeg : protected virtual ExynosCameraPipe {
public:
    ExynosCameraPipeJpeg(
        int cameraId,
        ExynosCameraParameters *obj_param,
        bool isReprocessing,
        int32_t *nodenums);

    virtual ~ExynosCameraPipeJpeg();

    virtual status_t        create(int32_t *sensorIds);
    virtual status_t        destroy(void);

    virtual status_t        start(void);
    virtual status_t        stop(void);
    virtual status_t        startThread(void);

protected:
    virtual status_t        m_run(void);

protected:
    virtual bool            m_mainThreadFunc(void);

private:
    void                   *m_csc;
    ExynosJpegEncoderForCamera m_jpegEnc;
};

}; /* namespace android */

#endif
