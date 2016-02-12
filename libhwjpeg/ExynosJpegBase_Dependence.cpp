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

#define JPEG_M2M_NODE        "/dev/m2m1shot_jpeg"

#define LOG_TAG "ExynosJpeg"
#define JPEG_ERROR_LOG(fmt,...) ALOGE(fmt,##__VA_ARGS__)

int ExynosJpegBase::ckeckJpegSelct(enum MODE eMode)
{
    if (t_bFlagSelect == false) {
        return ERROR_NONE;
    } else {
        switch (eMode) {
        case MODE_ENCODE:
            switch (t_stJpegConfig.pix.enc_fmt.in_fmt) {
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_NV12:
            case V4L2_PIX_FMT_NV21:
            case V4L2_PIX_FMT_RGB565X:
            case V4L2_PIX_FMT_BGR32:
            case V4L2_PIX_FMT_RGB32:
            case V4L2_PIX_FMT_YUV420:
                return ERROR_NONE;
            default:
                JPEG_ERROR_LOG("ERR(%s):JPEG device was not matching with colorformat\n", __func__);
                return ERROR_INVALID_SELECT;
            }
            break;
        case MODE_DECODE:
            switch (t_stJpegConfig.pix.enc_fmt.out_fmt) {
            case V4L2_PIX_FMT_YUYV:
            case V4L2_PIX_FMT_NV12:
            case V4L2_PIX_FMT_NV21:
            case V4L2_PIX_FMT_RGB565X:
            case V4L2_PIX_FMT_BGR32:
            case V4L2_PIX_FMT_RGB32:
            case V4L2_PIX_FMT_YUV420:
                return ERROR_NONE;
            default:
                JPEG_ERROR_LOG("ERR(%s):JPEG device was not matching with colorformat\n", __func__);
                return ERROR_INVALID_SELECT;
            }
        default:
            break;
        }
    }

    return ERROR_NONE;
}

int ExynosJpegBase::selectJpegHW(int iSel)
{
    t_iSelectNode = iSel;

    int iRet = ckeckJpegSelct((enum MODE)t_stJpegConfig.mode);
    t_bFlagSelect = true;

    return iRet;
}

int ExynosJpegBase::openNode(enum MODE eMode)
{
    t_iJpegFd = open(JPEG_M2M_NODE, O_RDWR, 0);
    if (t_iJpegFd < 0) {
        t_iJpegFd = -1;
        JPEG_ERROR_LOG("[%s]: JPEG_NODE open failed\n", __func__);
        return ERROR_CANNOT_OPEN_JPEG_DEVICE;
    }

    if (t_iJpegFd <= 0) {
        t_iJpegFd = -1;
        JPEG_ERROR_LOG("ERR(%s):JPEG device was closed\n", __func__);
        return ERROR_JPEG_DEVICE_ALREADY_CLOSED;
    }

    return ERROR_NONE;
}

int ExynosJpegBase::setColorFormat(enum MODE eMode, int iV4l2ColorFormat)
{
    if (t_bFlagCreate == false)
        return ERROR_JPEG_DEVICE_NOT_CREATE_YET;

    switch (eMode) {
    case MODE_ENCODE:
        switch(iV4l2ColorFormat) {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_RGB565X:
        case V4L2_PIX_FMT_RGB32:
        case V4L2_PIX_FMT_BGR32:
            t_iPlaneNum = 1;
            t_stJpegConfig.pix.enc_fmt.in_fmt = iV4l2ColorFormat;
            break;
        default:
            JPEG_ERROR_LOG("%s::Invalid input color format(%d) fail\n", __func__, iV4l2ColorFormat);
            t_iPlaneNum = 0;
            return ERROR_INVALID_COLOR_FORMAT;
        }
        break;
    case MODE_DECODE:
        switch(iV4l2ColorFormat) {
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_RGB565X:
        case V4L2_PIX_FMT_RGB32:
        case V4L2_PIX_FMT_BGR32:
            t_iPlaneNum = 1;
            t_stJpegConfig.pix.dec_fmt.out_fmt = iV4l2ColorFormat;
            break;
        default:
            JPEG_ERROR_LOG("%s::Invalid input color format(%d) fail\n", __func__, iV4l2ColorFormat);
            t_iPlaneNum = 0;
            return ERROR_INVALID_COLOR_FORMAT;
        }
        break;
    default:
        t_iPlaneNum = 0;
        return ERROR_INVALID_JPEG_MODE;
    }

    int iRet = ckeckJpegSelct(eMode);
    t_bFlagSelect = true;

    return iRet;
}

int ExynosJpegBase::checkBufType(struct BUFFER *pstBuf)
{
    int ret =0;
    if ((intptr_t)pstBuf->c_addr[0] != 0 && (intptr_t)pstBuf->c_addr[0] != -1)
        ret = ret|JPEG_BUF_TYPE_USER_PTR;

    if (pstBuf->i_addr[0] > 0)
        ret = ret|JPEG_BUF_TYPE_DMA_BUF;

    return ret;
}

int ExynosJpegBase::getBufType(struct BUFFER *pstBuf)
{
    if (checkBufType(pstBuf) & JPEG_BUF_TYPE_DMA_BUF)
        return M2M1SHOT_BUFFER_DMABUF;
    else if (checkBufType(pstBuf) & JPEG_BUF_TYPE_USER_PTR)
        return M2M1SHOT_BUFFER_USERPTR;
    else
        return 0;
}
