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

#include "m2m1shot.h"
#include "ExynosJpegApi.h"

#define MAXIMUM_JPEG_SIZE(n) ((65535 - (n)) * 32768)

#define LOG_TAG "ExynosJpeg"
#define JPEG_ERROR_LOG(fmt,...) ALOGE(fmt,##__VA_ARGS__)

ExynosJpegBase::ExynosJpegBase()
{
    memset(&t_stJpegOutbuf, 0, sizeof(struct BUFFER));
    memset(&t_stJpegConfig, 0, sizeof(struct CONFIG));
    memset(&t_stJpegInbuf, 0, sizeof(struct BUFFER));
    t_bFlagCreate = false;
    t_bFlagCreateInBuf = false;
    t_bFlagCreateOutBuf = false;
    t_bFlagExcute = false;
    t_bFlagSelect = false;
    t_iCacheValue = 0;
    t_iSelectNode = 0; // 0:jpeg2 hx , 1:jpeg2 hx , 2:jpeg hx;
    t_iPlaneNum = 0;
    t_iJpegFd = 0;
}

ExynosJpegBase::~ExynosJpegBase()
{
}

int ExynosJpegBase::create(enum MODE eMode)
{
    if (t_bFlagCreate == true)
        return ERROR_JPEG_DEVICE_ALREADY_CREATE;

    memset(&t_stJpegConfig, 0, sizeof(struct CONFIG));
    memset(&t_stJpegInbuf, 0, sizeof(struct BUFFER));
    memset(&t_stJpegOutbuf, 0, sizeof(struct BUFFER));

    t_stJpegConfig.mode = eMode;
    t_bFlagCreate = true;
    t_bFlagCreateInBuf = false;
    t_bFlagCreateOutBuf = false;
    t_bFlagExcute = false;
    t_bFlagSelect = false;
    t_iCacheValue = 0;
    t_iSelectNode = 0;
    t_iPlaneNum = 0;

    return ERROR_NONE;
}

int ExynosJpegBase::openJpeg(enum MODE eMode)
{
    return openNode(eMode);
}

int ExynosJpegBase::destroy(int iInBufs, int iOutBufs)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_ALREADY_DESTROY;

    close(t_iJpegFd);

    t_iJpegFd = -1;
    t_bFlagCreate = false;
    return ERROR_NONE;
}

int ExynosJpegBase::setSize(int iW, int iH)
{
    int mcu_x_size = 0;

    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (iW > 16368 || iH > 16368) {
        JPEG_ERROR_LOG("%s::Invalid size (%d,%d)", __func__, iW, iH);
        return ERROR_INVALID_IMAGE_SIZE;
    }

    t_stJpegConfig.width = iW;
    t_stJpegConfig.height = iH;

    return ERROR_NONE;
}

int ExynosJpegBase::setJpegConfig(enum MODE eMode, void *pConfig)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (pConfig == NULL)
        return ERROR_JPEG_CONFIG_POINTER_NULL;

    memcpy(&t_stJpegConfig, pConfig, sizeof(struct CONFIG));

    switch (eMode) {
    case MODE_ENCODE:
        switch (t_stJpegConfig.pix.enc_fmt.in_fmt) {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_RGB565X:
        case V4L2_PIX_FMT_BGR32:
        case V4L2_PIX_FMT_RGB32:
            t_iPlaneNum = 1;
            break;
        default:
            JPEG_ERROR_LOG("%s::Invalid input color format(%d) fail\n", __func__, t_stJpegConfig.pix.enc_fmt.in_fmt);
            t_iPlaneNum = 0;
            return ERROR_INVALID_COLOR_FORMAT;
        }
        break;
    case MODE_DECODE:
        switch (t_stJpegConfig.pix.dec_fmt.out_fmt) {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_RGB565X:
        case V4L2_PIX_FMT_BGR32:
        case V4L2_PIX_FMT_RGB32:
            t_iPlaneNum = 1;
            break;
        default:
            JPEG_ERROR_LOG("%s::Invalid input color format(%d) fail\n", __func__, t_stJpegConfig.pix.dec_fmt.out_fmt);
            t_iPlaneNum = 0;
            return ERROR_INVALID_COLOR_FORMAT;
        }
        break;
    default:
        t_iPlaneNum = 0;
        return ERROR_INVALID_JPEG_MODE;
        break;
    }

    return ERROR_NONE;
}

void *ExynosJpegBase::getJpegConfig(void)
{
    if (t_bFlagCreate == false)
        return NULL;

    return &t_stJpegConfig;
}

int ExynosJpegBase::getBuf(bool bCreateBuf, struct BUFFER *pstBuf, char **pcBuf, int *iBufSize, int iSize, int iPlaneNum)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (bCreateBuf == false)
        return ERROR_BUF_NOT_SET_YET;

    if ((pcBuf == NULL) || (iSize == 0))
        return ERROR_BUFFR_IS_NULL;

    if (iSize < iPlaneNum)
        return ERROR_BUFFER_TOO_SMALL;

    if (checkBufType(pstBuf) & JPEG_BUF_TYPE_USER_PTR) {
        for (int i = 0; i < iPlaneNum; i++)
            pcBuf[i] = pstBuf->c_addr[i];
    } else {
        for (int i = 0; i < iPlaneNum; i++)
            pcBuf[i] = NULL;
    }

    for (int i = 0; i < iPlaneNum; i++)
        iBufSize[i] = pstBuf->size[i];

    return ERROR_NONE;
}

int ExynosJpegBase::setBuf(struct BUFFER *pstBuf, char **pcBuf, int *iSize, int iPlaneNum)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (iPlaneNum <= 0)
        return ERROR_BUFFER_TOO_SMALL;

    for (int i = 0; i < iPlaneNum; i++) {
        if (pcBuf[i] == NULL) {
            memset(pstBuf, 0, sizeof(struct BUFFER));
            return ERROR_BUFFR_IS_NULL;
        }
        if (iSize[i] <= 0) {
            memset(pstBuf, 0, sizeof(struct BUFFER));
            return ERROR_BUFFER_TOO_SMALL;
        }
        pstBuf->c_addr[i] = pcBuf[i];
        pstBuf->size[i] = iSize[i];
    }

    pstBuf->numOfPlanes = iPlaneNum;

    return ERROR_NONE;
}

int ExynosJpegBase::getBuf(bool bCreateBuf, struct BUFFER *pstBuf, int *piBuf, int *iBufSize, int iSize, int iPlaneNum)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (bCreateBuf == false)
        return ERROR_BUF_NOT_SET_YET;

    if ((piBuf == NULL) || (iSize == 0))
        return ERROR_BUFFR_IS_NULL;

    if (iSize < iPlaneNum)
        return ERROR_BUFFER_TOO_SMALL;

    if (checkBufType(pstBuf) & JPEG_BUF_TYPE_DMA_BUF) {
        for (int i = 0; i < iPlaneNum; i++)
            piBuf[i] = pstBuf->i_addr[i];
    } else {
        for (int i = 0; i < iPlaneNum; i++)
            piBuf[i] = 0;
    }

    for (int i = 0; i < iPlaneNum; i++)
        iBufSize[i] = pstBuf->size[i];

    return ERROR_NONE;
}

int ExynosJpegBase::setBuf(struct BUFFER *pstBuf, int *piBuf, int *iSize, int iPlaneNum)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    if (iPlaneNum <= 0)
        return ERROR_BUFFER_TOO_SMALL;

    for (int i = 0; i < iPlaneNum; i++) {
        if (piBuf[i] == 0) {
            memset(pstBuf, 0, sizeof(struct BUFFER));
            return ERROR_BUFFR_IS_NULL;
        }
        if (iSize[i] <= 0) {
            memset(pstBuf, 0, sizeof(struct BUFFER));
            return ERROR_BUFFER_TOO_SMALL;
        }
        pstBuf->i_addr[i] = piBuf[i];
        pstBuf->size[i] = iSize[i];
    }

    pstBuf->numOfPlanes = iPlaneNum;

    return ERROR_NONE;
}

int ExynosJpegBase::setCache(int iValue)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    t_iCacheValue = iValue;

    return ERROR_NONE;
}

int ExynosJpegBase::setJpegFormat(enum MODE eMode, int iV4l2JpegFormat)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    switch(iV4l2JpegFormat) {
    case V4L2_PIX_FMT_JPEG_444:
    case V4L2_PIX_FMT_JPEG_422:
    case V4L2_PIX_FMT_JPEG_420:
    case V4L2_PIX_FMT_JPEG_GRAY:
        switch (eMode) {
        case MODE_ENCODE:
            t_stJpegConfig.pix.enc_fmt.out_fmt = iV4l2JpegFormat;
            break;
        case MODE_DECODE:
            t_stJpegConfig.pix.dec_fmt.in_fmt = iV4l2JpegFormat;
            break;
        default:
            return ERROR_INVALID_JPEG_MODE;
        }
        break;
    default:
        return ERROR_INVALID_JPEG_FORMAT;
    }

    return ERROR_NONE;
}

int ExynosJpegBase::setColorBufSize(enum MODE eMode, int *piBufSize, int iSize)
{
    int iFormat;

    switch (eMode) {
    case MODE_ENCODE:
        iFormat = t_stJpegConfig.pix.enc_fmt.in_fmt;
        break;
    case MODE_DECODE:
        iFormat = t_stJpegConfig.pix.dec_fmt.out_fmt;
        break;
    default:
        return ERROR_INVALID_JPEG_MODE;
    }

    return setColorBufSize(iFormat, piBufSize, iSize, t_stJpegConfig.width, t_stJpegConfig.height);
}

int ExynosJpegBase::setColorBufSize(int iFormat, int *piBufSize, int iSize, int width, int height)
{
    int pBufSize[3];

    if(iSize > 3)
        return ERROR_INVALID_IMAGE_SIZE;

    switch (iFormat) {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_RGB565X:
        pBufSize[0] = width * height * 2;
        pBufSize[1] = 0;
        pBufSize[2] = 0;
        break;
    case V4L2_PIX_FMT_RGB32:
    case V4L2_PIX_FMT_BGR32:
        pBufSize[0] = width * height * 4;
        pBufSize[1] = 0;
        pBufSize[2] = 0;
        break;
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
        pBufSize[0] = (width * height * 3) >> 1;
        pBufSize[1] = 0;
        pBufSize[2] = 0;
        break;
    case V4L2_PIX_FMT_YUV444:
        pBufSize[0] = width * height * 3;
        pBufSize[1] = 0;
        pBufSize[2] = 0;
        break;
    default:
        pBufSize[0] = width * height * 4;
        pBufSize[1] = width * height * 4;
        pBufSize[2] = width * height * 4;
        break;
    }

    memcpy(piBufSize, pBufSize, iSize * sizeof(int));

    return ERROR_NONE;
}

int ExynosJpegBase::updateConfig(enum MODE eMode, int iInBufs, int iOutBufs, int iInBufPlanes, int iOutBufPlanes)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    return openJpeg(eMode);
}

int ExynosJpegBase::execute(int iInBufPlanes, int iOutBufPlanes)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    struct m2m1shot shot;
    //struct BUF_INFO stBufInfo;
    int iRet = ERROR_NONE;

    t_bFlagExcute = true;

    shot.fmt_out.fmt = t_stJpegConfig.pix.dec_fmt.in_fmt;
    shot.fmt_out.width = t_stJpegConfig.width;
    shot.fmt_out.height = t_stJpegConfig.height;

    shot.fmt_cap.fmt = t_stJpegConfig.pix.dec_fmt.out_fmt;
    shot.fmt_cap.width = t_stJpegConfig.width;
    shot.fmt_cap.height = t_stJpegConfig.height;    

    shot.buf_out.type = getBufType(&t_stJpegInbuf);
    shot.buf_out.num_planes = iInBufPlanes;

    for (int i = 0; i < iInBufPlanes; i++) {
        if (getBufType(&t_stJpegInbuf) == M2M1SHOT_BUFFER_DMABUF) {
            shot.buf_out.plane[i].fd = t_stJpegInbuf.i_addr[i];
        } else if (getBufType(&t_stJpegInbuf) == M2M1SHOT_BUFFER_USERPTR) {
            shot.buf_out.plane[i].userptr = (unsigned long) t_stJpegInbuf.c_addr[i];
        }
        shot.buf_out.plane[i].len = t_stJpegInbuf.size[i];
    }

    shot.buf_cap.type = getBufType(&t_stJpegOutbuf);
    shot.buf_cap.num_planes = iOutBufPlanes;

    JPEG_ERROR_LOG("%s: out fmt(%d) (%d,%d) planes(%d)", 
             __func__, shot.fmt_out.fmt, shot.fmt_out.width, shot.fmt_out.height, shot.buf_out.num_planes);

    JPEG_ERROR_LOG("%s: cap fmt(%d) (%d,%d) planes(%d)", 
             __func__, shot.fmt_cap.fmt, shot.fmt_cap.width, shot.fmt_cap.height, shot.buf_cap.num_planes);

    JPEG_ERROR_LOG("%s: buf_out.type(%d) buf_cap.type(%d)", __func__, shot.buf_out.type, shot.buf_cap.type);

    for (int i = 0; i < iOutBufPlanes; i++) {
        if (getBufType(&t_stJpegOutbuf) == M2M1SHOT_BUFFER_DMABUF) {
            shot.buf_cap.plane[i].fd = t_stJpegOutbuf.i_addr[i];
        } else if (getBufType(&t_stJpegOutbuf) == M2M1SHOT_BUFFER_USERPTR) {
            shot.buf_cap.plane[i].userptr = (unsigned long) t_stJpegOutbuf.c_addr[i];
        }
        shot.buf_cap.plane[i].len = t_stJpegOutbuf.size[i];
    }

    shot.op.quality_level = t_stJpegConfig.enc_qual;

    if (t_bFlagSelect)
        shot.reserved[1] = 0; //TODO shot.reserved[1] contains the quality table passed by in ()
    else
        shot.reserved[1] = 0;

    iRet = ioctl(t_iJpegFd, M2M1SHOT_IOC_PROCESS, &shot);
    if (iRet < 0) {
        JPEG_ERROR_LOG("[%s:%d]: Error in M2M1SHOT_IOC_PROCESS", __func__, iRet);
        return ERROR_EXCUTE_FAIL;
    } else {
        t_stJpegConfig.sizeJpeg = shot.buf_cap.plane[0].len;
        JPEG_ERROR_LOG("%s: M2M1SHOT_IOC_PROCESS sizeJpeg=%d", __func__, shot.buf_cap.plane[0].len);
    }

    return ERROR_NONE;
}
