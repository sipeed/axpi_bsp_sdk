/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_ISP_3A_PLUS_H_
#define _AX_ISP_3A_PLUS_H_

#include "ax_isp_api.h"
#include "ax_isp_iq_api.h"

/////////////////////////////////////////////////////////
/* discard this API*/
////////////////////////////////////////////////////////

/* LSC Grid & Hist */
#define AX_LSC_GRID_ROW            (64)
#define AX_LSC_GRID_COL            (48)
#define AX_LSC_GRID_CHN            (4)
#define AX_LSC_HIST_LINEAR_BIN     (256)
#define AX_LSC_HIST_LOG_BIN        (16)

typedef struct {
    AX_U32 uGridSum[AX_LSC_GRID_CHN];
    AX_U16 uGridNum[AX_LSC_GRID_CHN];
} AX_LSC_GRID_STATS;

typedef struct {
    AX_U8  uValid;
    AX_U32 uZoneRowSize;
    AX_U32 uZoneColumnSize;
    AX_U8  uChnNum;
    AX_LSC_GRID_STATS  sGridStats[AX_LSC_GRID_ROW * AX_LSC_GRID_COL];
} AX_LSC_GRID_STAT_T;

/**********************************************************************************
 *                                  CLC
 * input (AX_ISP_CLC_INPUT_INFO_T) --> clc_alg --> output (AX_ISP_CLC_RESULT_T)
 **********************************************************************************/

typedef struct {
    AX_U32 SnsId;
    AX_SNS_HDR_MODE_E  eSnsMode;
    AX_BAYER_PATTERN_E eBayerPattern;
    AX_U32 uFrameRate;
} AX_ISP_CLC_INITATTR_T;

typedef struct {
    AX_U32 uTotalGain[AX_HDR_CHN_NUM];      /* Total Gain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]*/
    AX_U32 uLux;                            /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF]
                                            * fLux = (MeanLuma*LuxK) / (AGain*Dgain*IspGain)
                                            * where LuxK is a calibrated factor */
    AX_U32 uColorTemp;                      /* = CCT * 1000.0f, CCT:[1000, 20000]  Color Temperature */
    AX_U32 uSatDiscnt;                     /* Saturation Used for CLC or LSC Interpolation
                                             * Percetage * 1000.0f, e.g. 100000 for 100%
                                             * If Not Used, Just return 100000 */
} AX_ISP_CLC_INPUT_INFO_T;

typedef struct {
    /* transformation matrices corresponding to 16 different hue */
    AX_ISP_IQ_CLC_XCM_T                  sClcMatrixList;
} AX_ISP_CLC_RESULT_T;

typedef struct {
    AX_S32(*pfnClc_Init)(AX_U8 pipe, AX_ISP_CLC_INITATTR_T *pClcInitParam);
    AX_S32(*pfnClc_Run)(AX_U8 pipe, AX_ISP_CLC_INPUT_INFO_T *pClcInputInfo, AX_ISP_CLC_RESULT_T *pClcResult);
    AX_S32(*pfnClc_Exit)(AX_U8 pipe);
} AX_ISP_CLC_REGFUNCS_T;


/**********************************************************************************
 *                                  LSC
 * input (AX_ISP_LSC_INPUT_INFO_T) --> lsc_alg --> output (AX_ISP_LSC_RESULT_T)
 **********************************************************************************/

typedef struct {
    AX_U32 SnsId;
    AX_SNS_HDR_MODE_E  eSnsMode;
    AX_BAYER_PATTERN_E eBayerPattern;
    AX_U32 uFrameRate;
} AX_ISP_LSC_INITATTR_T;

typedef struct {
    AX_U32 uSeqNum;             /* frame seq num */
    AX_U64 uTimestamp;          /* frame timestamp */
    AX_LSC_GRID_STAT_T sLscGridStat;
} AX_ISP_LSC_STAT_INFO_T;

typedef struct {
    AX_U32 uTotalGain[AX_HDR_CHN_NUM];      /* Total Gain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]*/
    AX_U32 uLux;                            /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF]
                                            * fLux = (MeanLuma*LuxK) / (AGain*Dgain*IspGain)
                                            * where LuxK is a calibrated factor */
    AX_U32 uColorTemp;                      /* = CCT * 1000.0f, CCT:[1000, 20000]  Color Temperature */
    AX_U32 uSaturation;                     /* Saturation Used for CLC or LSC Interpolation
                                             * Percetage * 1000.0f, e.g. 100000 for 100%
                                             * If Not Used, Just return 100000 */
    AX_ISP_LSC_STAT_INFO_T sLscStat;
} AX_ISP_LSC_INPUT_INFO_T;

#define LSC_MESH_SIZE AX_ISP_LSC_MESH_SIZE_V * AX_ISP_LSC_MESH_SIZE_H
typedef struct {
    AX_U32                          rr_gain_lut[LSC_MESH_SIZE];
    AX_U32                          gr_gain_lut[LSC_MESH_SIZE];
    AX_U32                          gb_gain_lut[LSC_MESH_SIZE];
    AX_U32                          bb_gain_lut[LSC_MESH_SIZE];
} AX_ISP_LSC_TABLE_T;

typedef struct {
    AX_S32(*pfnLsc_Init)(AX_U8 pipe, AX_ISP_LSC_TABLE_T  *tLscResult);
    AX_S32(*pfnLsc_Run)(AX_U8 pipe, AX_ISP_LSC_INPUT_INFO_T tLscInputInfo, AX_BOOL *bUpdateMeshRegs, AX_ISP_LSC_TABLE_T *pLscResult);
    AX_S32(*pfnLsc_Exit)(AX_U8 pipe);
} AX_ISP_LSC_REGFUNCS_T;

AX_S32 AX_ISP_IQ_GetLscStatus(AX_U8 pipe, AX_ISP_LSC_TABLE_T *pIspLscStatus);
#endif //_AX_ISP_3A_PLUS_H_
