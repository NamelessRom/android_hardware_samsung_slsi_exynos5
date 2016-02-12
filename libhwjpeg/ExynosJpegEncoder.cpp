/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <sys/poll.h>
#include <cutils/log.h>
#include <utils/Log.h>

#include "ExynosJpegApi.h"

#define LOG_TAG "ExynosJpeg"
#define JPEG_ERROR_LOG(fmt,...) ALOGE(fmt,##__VA_ARGS__)

#define NUM_JPEG_ENC_IN_PLANES (1)
#define NUM_JPEG_ENC_OUT_PLANES (1)
#define NUM_JPEG_ENC_IN_BUFS (1)
#define NUM_JPEG_ENC_OUT_BUFS (1)

ExynosJpegEncoder::ExynosJpegEncoder()
{
    t_iJpegFd = -1;
    t_bFlagCreate = false;
}

ExynosJpegEncoder::~ExynosJpegEncoder()
{
    if (t_bFlagCreate == true)
        this->destroy();
}

int ExynosJpegEncoder::create(void)
{
    return ExynosJpegBase::create(MODE_ENCODE);
}

int ExynosJpegEncoder::destroy(void)
{
    return ExynosJpegBase::destroy(NUM_JPEG_ENC_IN_BUFS, NUM_JPEG_ENC_OUT_BUFS);
}

int ExynosJpegEncoder::setJpegConfig(void *pConfig)
{
    return ExynosJpegBase::setJpegConfig(MODE_ENCODE, pConfig);
}

int ExynosJpegEncoder::checkInBufType(void)
{
    return checkBufType(&t_stJpegInbuf);
}

int ExynosJpegEncoder::checkOutBufType(void)
{
    return checkBufType(&t_stJpegOutbuf);
}

int ExynosJpegEncoder::getInBuf(char **pcBuf, int *piInputSize, int iSize)
{
    return getBuf(t_bFlagCreateInBuf, &t_stJpegInbuf, pcBuf, piInputSize, iSize, t_iPlaneNum);
}

int ExynosJpegEncoder::getOutBuf(char **pcBuf, int *piOutputSize)
{
    return getBuf(t_bFlagCreateOutBuf, &t_stJpegOutbuf, pcBuf, piOutputSize,
                    NUM_JPEG_ENC_OUT_PLANES, NUM_JPEG_ENC_OUT_PLANES);
}

int ExynosJpegEncoder::setInBuf(char **pcBuf, int *iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegInbuf, pcBuf, iSize, t_iPlaneNum);

    if (iRet == ERROR_NONE)
        t_bFlagCreateInBuf = true;

    return iRet;
}

int  ExynosJpegEncoder::setOutBuf(char *pcBuf, int iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegOutbuf, &pcBuf, &iSize, NUM_JPEG_ENC_OUT_PLANES);

    if (iRet == ERROR_NONE)
        t_bFlagCreateOutBuf = true;

    return iRet;
}

int ExynosJpegEncoder::getInBuf(int *piBuf, int *piInputSize, int iSize)
{
    return getBuf(t_bFlagCreateInBuf, &t_stJpegInbuf, piBuf, piInputSize, iSize, t_iPlaneNum);
}

int ExynosJpegEncoder::getOutBuf(int *piBuf, int *piOutputSize)
{
    return getBuf(t_bFlagCreateOutBuf, &t_stJpegOutbuf, piBuf, piOutputSize,
                    NUM_JPEG_ENC_OUT_PLANES, NUM_JPEG_ENC_OUT_PLANES);
}

int ExynosJpegEncoder::setInBuf(int *piBuf, int *iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegInbuf, piBuf, iSize, t_iPlaneNum);

    if (iRet == ERROR_NONE)
        t_bFlagCreateInBuf = true;

    return iRet;
}

int ExynosJpegEncoder::setOutBuf(int piBuf, int iSize)
{
    int iRet = ERROR_NONE;
    iRet = setBuf(&t_stJpegOutbuf, &piBuf, &iSize, NUM_JPEG_ENC_OUT_PLANES);

    if (iRet == ERROR_NONE)
        t_bFlagCreateOutBuf = true;

    return iRet;
}

int ExynosJpegEncoder::getSize(int *piW, int *piH)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (t_stJpegConfig.width == 0 && t_stJpegConfig.height == 0)
        return ERROR_SIZE_NOT_SET_YET;

    *piW = t_stJpegConfig.width;
    *piH = t_stJpegConfig.height;

    return ERROR_NONE;
}

int ExynosJpegEncoder::getColorFormat(void)
{
    return t_stJpegConfig.pix.enc_fmt.in_fmt;
}

int ExynosJpegEncoder::setColorFormat(int iV4l2ColorFormat)
{
    return ExynosJpegBase::setColorFormat(MODE_ENCODE, iV4l2ColorFormat);
}

int ExynosJpegEncoder::setJpegFormat(int iV4l2JpegFormat)
{
    return ExynosJpegBase::setJpegFormat(MODE_ENCODE, iV4l2JpegFormat);
}

int ExynosJpegEncoder::setColorBufSize(int *piBufSize, int iSize)
{
    return ExynosJpegBase::setColorBufSize(MODE_ENCODE, piBufSize, iSize);
}

int ExynosJpegEncoder::updateConfig(void)
{
    return ExynosJpegBase::updateConfig(MODE_ENCODE,
                    NUM_JPEG_ENC_IN_BUFS, NUM_JPEG_ENC_OUT_BUFS,
                    NUM_JPEG_ENC_IN_PLANES, NUM_JPEG_ENC_OUT_PLANES);
}

int ExynosJpegEncoder::setQuality(int iV4l2Quality)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (iV4l2Quality <= 0)
        t_stJpegConfig.enc_qual = 1;
    else if (iV4l2Quality > 100)
        t_stJpegConfig.enc_qual = 100;
    else
        t_stJpegConfig.enc_qual = iV4l2Quality;

    JPEG_ERROR_LOG("%s: iV4l2Quality(%d) enc_qual(%d)", __func__, iV4l2Quality, t_stJpegConfig.enc_qual);

    t_bFlagSelect = false;

    return ERROR_NONE;
}

int ExynosJpegEncoder::getJpegSize(void)
{
    if (t_bFlagCreate == false)
        return 0;

    int iSize = -1;
    iSize = t_stJpegConfig.sizeJpeg & ~(t_stJpegConfig.sizeJpeg >> 31);

    JPEG_ERROR_LOG("%s: sizeJpeg=%d iSize=%d", __func__, t_stJpegConfig.sizeJpeg, iSize);

    if (iSize < 0) {
        JPEG_ERROR_LOG("%s::Fail to JPEG output buffer!!\n", __func__);
        return 0;
    }

    return iSize;
}

int ExynosJpegEncoder::encode(void)
{
    return ExynosJpegBase::execute(t_iPlaneNum, NUM_JPEG_ENC_OUT_PLANES);
}

int ExynosJpegEncoder::setQuality(unsigned char const *qtable)
{
    if (qtable == NULL)
        return ERROR_BUFFR_IS_NULL;

    JPEG_ERROR_LOG("%s: qtable=%s", __func__, qtable);

    memcpy(&t_qtable, qtable, NUM_QTABLE_VALUES * 2); //kernel: jpeg_1shotdev_prepare_operation()
    t_bFlagSelect = true;

    return ERROR_NONE;
}
