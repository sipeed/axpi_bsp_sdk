/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_ISP_CONNON_H__
#define __AX_ISP_CONNON_H__

#include "ax_base_type.h"
#include "ax_global_type.h"


#define DEF_VIN_DEV_MAX_NUM             (4)
#define DEF_VIN_PIPE_MAX_NUM            (8)
#define DEF_VOUT_DEV_MAX_NUM            (3)
#define DEF_DEV_CHN_OUTPUT_MAX          (3)
#define AX_HDR_CHN_NUM                  (4)
#define AX_ISP_BAYER_CHN_NUM            (4)
#define AX_VIN_COMPMASK_NUM             (2)
#define AX_VIN_SYNC_CODE_NUM            (4)
#define AX_VIN_LVDS_LANE_NUM            (16)

typedef enum {
    AX_DEV_SINK_INVALID                 = -1,
    AX_DEV_SOURCE_SNS_0                 = 0,        // ife data source from sensor 0
    AX_DEV_SOURCE_SNS_1                 = 1,        // ife data source from sensor 1
    AX_DEV_SOURCE_SNS_2                 = 2,        // ife data source from sensor 2
    AX_DEV_SOURCE_SNS_3                 = 3,        // ife data source from sensor 3
    AX_DEV_SINK_MAX
} AX_DEV_SOURCE_E;


typedef enum {
    AX_SNS_MODE_NONE   = 0,
    AX_SNS_LINEAR_MODE = 1,
    AX_SNS_HDR_2X_MODE = 2,
    AX_SNS_HDR_3X_MODE = 3,
    AX_SNS_HDR_4X_MODE = 4,
    AX_SNS_HDR_MODE_BUTT
} AX_SNS_HDR_MODE_E;

typedef enum {
    AX_SNS_HDR_OUTPUT_MODE_FRAME_BASED = 0,
    AX_SNS_HDR_OUTPUT_MODE_DOL = 1,
    AX_SNS_OUTPUT_MODE_BUTT
} AX_SNS_HDR_OUTPUT_MODE_E;

typedef enum {
    AX_SNS_HDR_FRAME_L = 0,
    AX_SNS_HDR_FRAME_S = 1,
    AX_SNS_HDR_FRAME_VS = 2,
    AX_SNS_HDR_FRAME_MAX
} AX_SNS_HDR_FRAME_E;

typedef enum {
    AX_SNS_TYPE_INVALID                 = -1,
    AX_SNS_TYPE_MIPI                    = 0,        // MIPI sensor(data type : raw)
    AX_SNS_TYPE_MIPI_YUV                = 1,        // MIPI sensor(data type : yuv)
    AX_SNS_TYPE_SUB_LVDS                = 2,        // Sub-LVDS sensor
    AX_SNS_TYPE_DVP                     = 3,        // DVP sensor
    AX_SNS_TYPE_BT601                   = 4,        // BT.601 sensor
    AX_SNS_TYPE_BT656                   = 5,        // BT.656 sensor
    AX_SNS_TYPE_BT1120                  = 6,        // BT.1120 sensor
    AX_SNS_TYPE_TPG                     = 7,        // ISP TPG test pattern
    AX_SNS_TYPE_MAX
} AX_SNS_TYPE_E;

typedef enum {
    AX_RT_RAW8                          = 8,        // raw8, 8-bit per pixel
    AX_RT_RAW10                         = 10,       // raw10, 10-bit per pixel
    AX_RT_RAW12                         = 12,       // raw12, 12-bit per pixel
    AX_RT_RAW14                         = 14,       // raw14, 14-bit per pixel
    AX_RT_RAW16                         = 16,       // raw16, 16-bit per pixel
    AX_RT_YUV422                        = 20,       // yuv422
    AX_RT_YUV420                        = 21,       // yuv420
} AX_RAW_TYPE_E;

typedef enum {
    AX_BP_RGGB                          = 0,        // R Gr Gb B bayer pattern
    AX_BP_GRBG                          = 1,        // Gr R B Gb bayer pattern
    AX_BP_GBRG                          = 2,        // Gb B R Gr byaer pattern
    AX_BP_BGGR                          = 3,        // B Gb Gr R byaer pattern
} AX_BAYER_PATTERN_E;

typedef enum {
    AX_SNS_GAIN_MODE_INVALID            = -1,
    AX_SNS_GAIN_MODE_HCG                = 0,        // sensor gain mode HCG
    AX_SNS_GAIN_MODE_LCG                = 1,        // sensor gain mode LCG
    AX_SNS_GAIN_MODE_MAX
} AX_SNS_GAIN_MODE_E;

typedef enum {
    AX_SNS_SKIP_FRAME_NO_SKIP           = 0,        // no frame skip for sif sensor
    AX_SNS_SKIP_FRAME_1D2               = 1,        // 1/2 frame skip for sif sensor
    AX_SNS_SKIP_FRAME_1D3               = 2,        // 1/3 frame skip for sif sensor
    AX_SNS_SKIP_FRAME_1D4               = 3,        // 1/4 frame skip for sif sensor
} AX_SNS_SKIP_FRAME_E ;

typedef enum {
    AX_SNS_NORMAL                       = 0,
    AX_SNS_DOL_HDR                      = 1,
    AX_SNS_BME_HDR                      = 2,
    AX_SNS_QUAD_BAYER_NO_HDR            = 3,
    AX_SNS_QUAD_BAYER_2_HDR_MODE0       = 4,
    AX_SNS_QUAD_BAYER_2_HDR_MODE1       = 5,
    AX_SNS_QUAD_BAYER_2_HDR_MODE2       = 6,
    AX_SNS_QUAD_BAYER_3_HDR_MODE3       = 7
} AX_SNS_OUTPUT_MODE_E;

typedef struct _AX_WIN_AREA_T_{
    AX_U32                              nStartX;
    AX_U32                              nStartY;
    AX_U32                              nWidth;
    AX_U32                              nHeight;
} AX_WIN_AREA_T;



#endif // __AX_ISP_CONNON_H__
