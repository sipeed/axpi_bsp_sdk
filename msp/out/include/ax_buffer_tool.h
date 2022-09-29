/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_BUFFER_TOOL_H_
#define _AX_BUFFER_TOOL_H_

#include "ax_base_type.h"
#include "ax_comm_codec.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define AX_COMM_ALIGN(value, n) (((value) + (n) - 1) & ~((n) - 1))

static __inline AX_U32 AX_VIN_GetImgBufferSize(AX_U32 uHeight, AX_U32 uWidth, AX_IMG_FORMAT_E eImageFormat, AX_BOOL bCompressEn)
{
    AX_S32 width_beat = 0;
    AX_S32 _buf_size = 0;

    if(((uHeight % 16) != 0) && ((eImageFormat == AX_FORMAT_BAYER_RAW_8BPP) ||
        (eImageFormat == AX_FORMAT_BAYER_RAW_10BPP) || (eImageFormat == AX_FORMAT_BAYER_RAW_12BPP) ||
        (eImageFormat == AX_FORMAT_BAYER_RAW_14BPP) || (eImageFormat == AX_FORMAT_BAYER_RAW_16BPP)))
    {
        uHeight = ((uHeight / 16) + 1) * 16;
    }

    switch (eImageFormat) {
    case AX_FORMAT_BAYER_RAW_8BPP:
        if (((uWidth) % (128 / 8)) != 0) {
            width_beat = ((uWidth) / (128 / 8)) + 1;
        } else {
            width_beat = ((uWidth) / (128 / 8));
        }
        _buf_size = width_beat * 16 * uHeight;   // per beat 16bytes
        break;
    case AX_FORMAT_BAYER_RAW_10BPP:
        if (((uWidth) % (128 / 10)) != 0) {
            width_beat = ((uWidth) / (128 / 10)) + 1;
        } else {
            width_beat = ((uWidth) / (128 / 10));
        }
        _buf_size = width_beat * 16 * uHeight;   // per beat 16bytes
        break;
    case AX_FORMAT_BAYER_RAW_12BPP:
        if (((uWidth) % (128 / 12)) != 0) {
            width_beat = ((uWidth) / (128 / 12)) + 1;
        } else {
            width_beat = ((uWidth) / (128 / 12));
        }
        _buf_size = width_beat * 16 * uHeight;   // per beat 16bytes
        break;
    case AX_FORMAT_BAYER_RAW_14BPP:
        if (((uWidth) % (128 / 14)) != 0) {
            width_beat = ((uWidth) / (128 / 14)) + 1;
        } else {
            width_beat = ((uWidth) / (128 / 14));
        }
        _buf_size = width_beat * 16 * uHeight;   // per beat 16bytes
        break;
    case AX_FORMAT_BAYER_RAW_16BPP:
        if (((uWidth) % (128 / 16)) != 0) {
            width_beat = ((uWidth) / (128 / 16)) + 1;
        } else {
            width_beat = ((uWidth) / (128 / 16));
        }
        _buf_size = width_beat * 16 * uHeight;   // per beat 16bytes
        break;

    case AX_YUV420_SEMIPLANAR_VU:
    case AX_YUV420_SEMIPLANAR:
        _buf_size = uWidth * uHeight * 3 / 2;
        break;
    case AX_YUV422_INTERLEAVED_YUYV:
    case AX_YUV422_INTERLEAVED_UYVY:
        _buf_size = uWidth * uHeight * 2;
        break;
    default:
        _buf_size = uWidth * uHeight * 2;
        break;
    }

    if(bCompressEn) {
        switch (eImageFormat) {
        case AX_FORMAT_BAYER_RAW_8BPP:
        case AX_FORMAT_BAYER_RAW_10BPP:
        case AX_FORMAT_BAYER_RAW_12BPP:
        case AX_FORMAT_BAYER_RAW_14BPP:
        case AX_FORMAT_BAYER_RAW_16BPP:
            _buf_size += uHeight * 32; /* add the header buf for compress data */
            break;
        case AX_YUV420_SEMIPLANAR_VU:
        case AX_YUV420_SEMIPLANAR:
            _buf_size += uHeight * 96; /* add the header buf for compress data */
            break;
        case AX_YUV422_INTERLEAVED_YUYV:
        case AX_YUV422_INTERLEAVED_UYVY:
            _buf_size += 0;
            break;
        default:
            _buf_size += 0;
            break;
        }
    }

    return _buf_size;
}

static __inline AX_U32 AX_VDEC_GetPicBufferSize(AX_U32 uHeight, AX_U32 uWidth, AX_PAYLOAD_TYPE_E enType)
{
    AX_U32 picSizeInMbs = 0;
    AX_U32 picSize = 0;
    AX_U32 dmvMemSize = 0;
    AX_U32 refBuffSize = 0;

    picSizeInMbs = (AX_COMM_ALIGN(uHeight, 16) >> 4) * (AX_COMM_ALIGN(uWidth, 16) >> 4);
    if (PT_H264 == enType) {
        picSize = (AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3) >> 1;

        /* buffer size of dpb pic = picSize + dir_mv_size + tbl_size */
        dmvMemSize = picSizeInMbs * 64;
        refBuffSize = picSize  + dmvMemSize + 32;
    }
    else if ((PT_JPEG == enType) || (PT_MJPEG == enType)) {
        picSize = (AX_COMM_ALIGN(uHeight, 16) * AX_COMM_ALIGN(uWidth, 16) * 3) >> 1;
        refBuffSize = picSize;
    }
    else {
        refBuffSize = 0;
    }

    return refBuffSize;
}

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif //_AX_BUFFER_TOOL_H_
