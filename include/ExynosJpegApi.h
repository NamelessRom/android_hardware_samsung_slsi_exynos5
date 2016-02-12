/*
 * Copyright Samsung Electronics Co.,LTD.
 * Copyright (C) 2012~2013 The Android Open Source Project
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

#ifndef __EXYNOS_JPEG_BASE_H__
#define __EXYNOS_JPEG_BASE_H__

#include <linux/videodev2.h>
#include "videodev2_exynos_media.h"

#define JPEG_CACHE_OFF (0)
#define JPEG_CACHE_ON (1)

#define JPEG_BUF_TYPE_USER_PTR (1)
#define JPEG_BUF_TYPE_DMA_BUF (2)

#define NUM_QTABLE_VALUES (64)

class ExynosJpegBase {
public:
    #define JPEG_MAX_PLANE_CNT          (3)
    ExynosJpegBase();
    virtual ~ExynosJpegBase();

    enum ERROR_JPEG_HAL {
        ERROR_JPEG_DEVICE_ALREADY_CREATE = -0x100,
        ERROR_INVALID_JPEG_MODE,
        ERROR_CANNOT_OPEN_JPEG_DEVICE,
        ERROR_JPEG_DEVICE_ALREADY_CLOSED,
        ERROR_JPEG_DEVICE_ALREADY_DESTROY,
        ERROR_JPEG_DEVICE_NOT_CREATE_YET,
        ERROR_INVALID_COLOR_FORMAT,
        ERROR_INVALID_JPEG_FORMAT,
        ERROR_INVALID_IMAGE_SIZE,
        ERROR_JPEG_CONFIG_POINTER_NULL,
        ERROR_INVALID_JPEG_CONFIG,
        ERROR_IN_BUFFER_CREATE_FAIL,
        ERROR_OUT_BUFFER_CREATE_FAIL,
        ERROR_EXCUTE_FAIL,
        ERROR_JPEG_SIZE_TOO_SMALL,
        ERROR_CANNOT_CHANGE_CACHE_SETTING,
        ERROR_SIZE_NOT_SET_YET,
        ERROR_BUFFR_IS_NULL,
        ERROR_BUFFER_TOO_SMALL,
        ERROR_GET_SIZE_FAIL,
        ERROR_BUF_NOT_SET_YET,
        ERROR_REQBUF_FAIL,
        ERROR_INVALID_V4l2_BUF_TYPE = -0x80,
        ERROR_INVALID_SELECT,
        ERROR_MMAP_FAILED,
        ERROR_FAIL,
        ERROR_NONE = 0
    };

    enum MODE {
        MODE_ENCODE = 0,
        MODE_DECODE
    };

    struct BUFFER{
        int     numOfPlanes;
        int     i_addr[JPEG_MAX_PLANE_CNT];
        char    *c_addr[JPEG_MAX_PLANE_CNT];
        int     size[JPEG_MAX_PLANE_CNT];
    };

    struct BUF_INFO{
        int                 numOfPlanes;
        enum v4l2_memory    memory;
        enum v4l2_buf_type  buf_type;
        int                 reserved[4];
    };

    struct PIX_FMT{
        int in_fmt;
        int out_fmt;
        int reserved[4];
    };

    struct CONFIG{
        int               mode;
        int               enc_qual;
        int               width;
        int               height;
        int               scaled_width;
        int               scaled_height;
        int               numOfPlanes;
        int               sizeJpeg;
        union {
            PIX_FMT enc_fmt;
            PIX_FMT dec_fmt;
        } pix;
        int              reserved[8];
    };

    int setSize(int iW, int iH);
    int setCache(int iValue);
    void *getJpegConfig(void);
    int selectJpegHW(int iSel);
    int ckeckJpegSelct(enum MODE eMode);

protected:
    bool t_bFlagCreate;
    bool t_bFlagCreateInBuf;
    bool t_bFlagCreateOutBuf;
    bool t_bFlagExcute;
    bool t_bFlagSelect;
    int t_iCacheValue;
    int t_iSelectNode;
    int t_iPlaneNum;
    int t_iJpegFd;

    struct CONFIG t_stJpegConfig;
    struct BUFFER t_stJpegInbuf;
    struct BUFFER t_stJpegOutbuf;

    char t_qtable[NUM_QTABLE_VALUES * 2];

    int create(enum MODE eMode);
    int openJpeg(enum MODE eMode);
    int openNode(enum MODE eMode);
    int destroy(int iInBufs, int iOutBufs);
    int setJpegConfig(enum MODE eMode, void *pConfig);
    int setColorFormat(enum MODE eMode, int iV4l2ColorFormat);
    int setJpegFormat(enum MODE eMode, int iV4l2JpegFormat);
    int setColorBufSize(enum MODE eMode, int *piBufSize, int iSize);
    int setColorBufSize(int iFormat, int *piBufSize, int iSize, int width, int height);

    int checkBufType(struct BUFFER *pstBuf);
    int getBufType(struct BUFFER *pstBuf);

    int getBuf(bool bCreateBuf, struct BUFFER *pstBuf, int *piBuf, int *iBufSize, int iSize, int iPlaneNum);
    int setBuf(struct BUFFER *pstBuf, int *piBuf, int *iSize, int iPlaneNum);

    int getBuf(bool bCreateBuf, struct BUFFER *pstBuf, char **pcBuf, int *iBufSize, int iSize, int iPlaneNum);
    int setBuf(struct BUFFER *pstBuf, char **pcBuf, int *iSize, int iPlaneNum);
    int updateConfig(enum MODE eMode, int iInBufs, int iOutBufs, int iInBufPlanes, int iOutBufPlanes);
    int execute(int iInBufPlanes, int iOutBufPlanes);
};

/*
 * ExynosJpegEncoder class
 */
class ExynosJpegEncoder : public ExynosJpegBase {
public:
    ExynosJpegEncoder();
    virtual ~ExynosJpegEncoder();

    int create(void);
    int destroy(void);

    int setJpegConfig(void* pConfig);

    int checkInBufType(void);
    int checkOutBufType(void);

    int getInBuf(int *piBuf, int *piInputSize, int iSize);
    int getOutBuf(int *piBuf, int *piOutputSize);

    int setInBuf(int *piBuf, int *iSize);
    int setOutBuf(int iBuf, int iSize);

    int getInBuf(char **pcBuf, int *piInputSize, int iSize);
    int getOutBuf(char **pcBuf, int *piOutputSize);

    int setInBuf(char **pcBuf, int *iSize);
    int setOutBuf(char *pcBuf, int iSize);

    int getSize(int *piWidth, int *piHeight);
    int getColorFormat(void);
    int setColorFormat(int iV4l2ColorFormat);
    int setJpegFormat(int iV4l2JpegFormat);
    int setColorBufSize(int *piBufSize, int iSize);
    int updateConfig(void);

    int setQuality(int iQuality);
    int setQuality(unsigned char const *qtable); //kernel: jpeg_1shotdev_prepare_operation()
    int getJpegSize(void);

    int encode(void);
};

/*
 * ExynosJpegDecoder class
 */
class ExynosJpegDecoder : public ExynosJpegBase {
public:
    ExynosJpegDecoder();
    virtual ~ExynosJpegDecoder();

    int create(void);
    int destroy(void);

    int setJpegConfig(void* pConfig);

    int checkInBufType(void);
    int checkOutBufType(void);

    int getInBuf(int *piBuf, int *piInputSize);
    int getOutBuf(int *picBuf, int *piOutputSize, int iSize);

    int setInBuf(int iBuf, int iSize);
    int setOutBuf(int *piBuf, int *iSize);

    int getInBuf(char **pcBuf, int *piInputSize);
    int getOutBuf(char **pcBuf, int *piOutputSize, int iSize);

    int setInBuf(char *pcBuf, int iSize);
    int setOutBuf(char **pcBuf, int *iSize);

    int getSize(int *piWidth, int *piHeight);
    int setColorFormat(int iV4l2ColorFormat);
    int setJpegFormat(int iV4l2JpegFormat);
    int updateConfig(void);

    int setScaledSize(int iW, int iH);
    int setJpegSize(int iJpegSize);

    int decode(void);
};

#endif /* __EXYNOS_JPEG_BASE_H__ */
