/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef __AX_NPU_COMMON_H__
#define __AX_NPU_COMMON_H__

#include "./ax_base_type.h"
#include "./ax_global_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AX_ID_NPU_SMOD_NPU_CV                       0x11
#define AX_ERR_NPU_CV_SUCCESS                       0x00
#define AX_ERR_NPU_CV_INVALID_PARAM                 AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x81)
#define AX_ERR_NPU_CV_UNSUPPORT_FUNCTION            AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x82)
#define AX_ERR_NPU_CV_NOT_INIT                      AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x83)
#define AX_ERR_NPU_CV_INVALID_SEMANTIC_HANDLE       AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x84)
#define AX_ERR_NPU_CV_INFERENCE_FAIL                AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x85)
#define AX_ERR_NPU_CV_INFERENCE_CANCELLED           AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x86)
#define AX_ERR_NPU_CV_INVALID_HDR_MODE              AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x87)
#define AX_ERR_NPU_CV_TASK_NOT_EXIST                AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x88)
#define AX_ERR_NPU_CV_TASK_RUNNING                  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_NPU_CV, 0x89)

typedef enum _AX_NPU_CV_ErrorCode {
    AX_NPU_CV_SUCCESS = AX_ERR_NPU_CV_SUCCESS,
    AX_NPU_CV_INVALID_PARAM = AX_ERR_NPU_CV_INVALID_PARAM,
    AX_NPU_CV_UNSUPPORT_FUNCTION = AX_ERR_NPU_CV_UNSUPPORT_FUNCTION,
    AX_NPU_CV_NOT_INIT = AX_ERR_NPU_CV_NOT_INIT,
    AX_NPU_CV_INVALID_SEMANTIC_HANDLE = AX_ERR_NPU_CV_INVALID_SEMANTIC_HANDLE,
    AX_NPU_CV_INFERENCE_FAIL = AX_ERR_NPU_CV_INFERENCE_FAIL,
    AX_NPU_CV_INFERENCE_CANCELLED = AX_ERR_NPU_CV_INFERENCE_CANCELLED,
    AX_NPU_CV_INVALID_HDR_MODE = AX_ERR_NPU_CV_INVALID_HDR_MODE,
    AX_NPU_CV_TASK_NOT_EXIST = AX_ERR_NPU_CV_TASK_NOT_EXIST,
    AX_NPU_CV_TASK_RUNNING = AX_ERR_NPU_CV_TASK_RUNNING
} AX_NPU_CV_ErrorCode;

typedef enum _AX_NPU_CV_FrameDataType {
    AX_NPU_CV_FDT_UNKNOWN = 0,
    AX_NPU_CV_FDT_RAW10 = 1,
    AX_NPU_CV_FDT_RAW12 = 2,
    AX_NPU_CV_FDT_RAW16 = 3,
    AX_NPU_CV_FDT_NV12 = 4,
    AX_NPU_CV_FDT_NV21 = 5,
    AX_NPU_CV_FDT_RGB = 6,
    AX_NPU_CV_FDT_BGR = 7,
    AX_NPU_CV_FDT_RGBA = 8,
    AX_NPU_CV_FDT_GRAY = 9,
    AX_NPU_CV_FDT_YUV444 = 10,
    AX_NPU_CV_FDT_UV = 11,
    AX_NPU_CV_FDT_YUV422 = 12,
    AX_NPU_CV_FDT_BAYER_RGGB = 13,
    AX_NPU_CV_FDT_BAYER_GBRG = 14,
    AX_NPU_CV_FDT_BAYER_GRBG = 15,
    AX_NPU_CV_FDT_BAYER_BGGR = 16,
    AX_NPU_CV_FDT_UYVY = 17,
    AX_NPU_CV_FDT_YUYV = 18,
    AX_NPU_CV_FDT_YUV420_LEGACY = 19,
    AX_NPU_CV_FDT_LAB = 20,
} AX_NPU_CV_FrameDataType;

typedef struct _AX_NPU_CV_Stride {
    AX_U32 nH, nW, nC; // Currently only supports the stride of width
} AX_NPU_CV_Stride;

typedef struct _AX_NPU_CV_Image {
    AX_U8* pVir;
    AX_U64 pPhy;
    AX_U32 nSize;
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_NPU_CV_FrameDataType eDtype;
    AX_NPU_CV_Stride tStride; // If unset, will be the same as the image shape.
} AX_NPU_CV_Image;

typedef struct _AX_NPU_CV_Box {
    AX_F32 fX, fY, fW, fH;
} AX_NPU_CV_Box;

typedef struct _AX_NPU_CV_Point {
    AX_F32 fX, fY;
} AX_NPU_CV_Point;

typedef union _AX_NPU_CV_Color {
    AX_U8 nRGBColorValue[3]; // nRGBColorValue[0] for R, nRGBColorValue[1] for G, nRGBColorValue[2] for B
    AX_U8 nYUVColorValue[2]; // nYUVColorValue[0] for Y;  nYUVColorValue[1] for UV or VU
    AX_U8 nGrayColorValue[1];
} AX_NPU_CV_Color;

typedef enum _AX_NPU_CV_DataType {
    AX_NPU_CV_DT_UNKNOWN = 0,
    AX_NPU_CV_DT_UINT8 = 1,
    AX_NPU_CV_DT_INT8 = 2,
    AX_NPU_CV_DT_UINT16 = 3,
    AX_NPU_CV_DT_INT16 = 4,
    AX_NPU_CV_DT_UINT32 = 5,
    AX_NPU_CV_DT_INT32 = 6,
    AX_NPU_CV_DT_UINT64 = 7,
    AX_NPU_CV_DT_INT64 = 8,
    AX_NPU_CV_DT_FLOAT32 = 9,
    AX_NPU_CV_DT_FLOAT64 = 10,
} AX_NPU_CV_DataType;
typedef struct _AX_NPU_CV_Matrix2D {
    AX_U8* pVir;
    AX_U64 pPhy;
    AX_U32 nRows;
    AX_U32 nColumns;
    AX_NPU_CV_DataType eDataType;
} AX_NPU_CV_Matrix2D;


typedef enum _AX_NPU_CV_Interp {
    AX_NPU_CV_BILINEAR = 0,
    AX_NPU_CV_NEAREST = 1
} AX_NPU_CV_Interp;

static const AX_S32 PHY_MEM_ALIGN_SIZE = 128;

#ifdef __cplusplus
}
#endif

#endif
