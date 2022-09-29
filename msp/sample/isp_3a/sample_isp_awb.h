/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __SAMPLE_ISP_AWB_H__
#define __SAMPLE_ISP_AWB_H__
#include <stdio.h>
#include <stdarg.h>

#include "ax_vin_api.h"
#include "ax_base_type.h"
#include "ax_interpreter_external_api.h"
#include "ax_sys_api.h"
#include "ax_mipi_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AWB_MAX_BLK_NUM 4096
#define HIST_BIN_NUM 16
#define AX_PIPE_MAX_NUM 4
#define MASK(width)                     ((1 << (width)) - 1) /* 2^w - 1 */
#ifndef ABS
    #define ABS(a)                      (((a) < 0) ? -(a) : (a))
#endif
#define false                           (0)
#define true                            (1)


/* Static Calibration Info, Most of Which Come from Offline XML. */
typedef struct {

    /* The (R/G, B/G) Coords of the Illums. */
    float aRg;
    float aBg;
    float d50Rg;
    float d50Bg;
    float d65Rg;
    float d65Bg;

    /* Here is the Most Simplest Square Gray Zone, Just for Demo.
       SHOULD be replaced by a Calibrated Advanced Gray Zone (e.g. Planckian Sector). */
    float grayZoneMinRg;
    float grayZoneMaxRg;
    float grayZoneMinBg;
    float grayZoneMaxBg;

    /* CCT Values Used for Simple Gray World Linear CCT Interpolation. */
    float aCct;
    float d50Cct;
    float d65Cct;

    /* The CCT Diff Between A and D65, Used For CCT Linear Interpolation */
    float cctSpan;

    /* Here is the Most Simplest CCT Straight Line, Just for Demo.
       The Straight Line Pass Through Illum A and D65.
       The Vector From A to D65 is used for Linear CCT Approximation. */
    float cctLineVecLen;        /* The Length of the Vector */
    float cctLineNormVecRg;     /* The X Coord of the Normalized Vector */
    float cctLineNormVecBg;     /* The Y Coord of the Normalized Vector */

} SAMPLE_AWB_CALIB_T;


/* Static Tuning Parameters, Most Which Come from Online XML. */
typedef struct {
    float dampRatio;
    float outdoorLuxTh;
    float yClipMin;
    float yClipMax;
} SAMPLE_AWB_TUNING_T;


/* Dynamic Information of the AWB Algorithm, Updated each Frame. */
typedef struct {

    /* Damped Result for the Last Frame:
       Simple Gray World (R/G, B/G) Result and CCT Approx Result. */
    float prevAvgSgwRg;
    float prevAvgSgwBg;
    float prevCct;

    /* Previous Frame Result:
       Simple Gray World (R/G, B/G) Result and CCT Approx Result. */
    float currAvgSgwRg;
    float currAvgSgwBg;
    float currCct;

    /* R, G, B Gains set to the Hardware Register */
    float rGain;
    float grGain;
    float gbGain;
    float bGain;

    /* R/G, B/G, Y Values Calculated from the AWB Statistics Input.
       These values are used by the AWB Algorithm (Such as Simple Gray World). */
    int blkRowNum;
    int blkColNum;
    float blkRg[AWB_MAX_BLK_NUM];
    float blkBg[AWB_MAX_BLK_NUM];
    float blkY[AWB_MAX_BLK_NUM];

    /* Weights of Each Block Determined by the AWB Algorithm. */
    float blkWeight[AWB_MAX_BLK_NUM];

    /*sensor mode*/
    int eSnsMode;



} SAMPLE_AWB_STATUS_T;


/* AWB Algorithm Context of a Sensor Pipe. */
typedef struct {

    /* Static Calibration Info: Basically Come from Offline XML. */
    SAMPLE_AWB_CALIB_T calibInfo;

    /* Static Tuning Parameters: Basically Come from Online XML. */
    SAMPLE_AWB_TUNING_T tuningParam;

    /* Dynamic Information of the AWB Algorithm, Updated each Frame. */
    SAMPLE_AWB_STATUS_T tStatus;

} SAMPLE_AWB_CTX_T;

/* Callback Functions Called by the AX Platform 3A Framework. */
AX_S32 sample_awb_init(AX_U8 pipe, AX_ISP_AWB_INITATTR_T *pAwbInitParam);
AX_S32 sample_awb_run(AX_U8 pipe, AX_ISP_AWB_INPUT_INFO_T *pAwbInputInfo, AX_ISP_AWB_RESULT_T *pAwbResult);
AX_S32 sample_awb_deinit(AX_U8 pipe);


#ifdef __cplusplus
}
#endif

#endif
