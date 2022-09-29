/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __SAMPLE_ISP_LSC_H__
#define __SAMPLE_ISP_LSC_H__
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

#define SAMPLE_ISP_LSC_MESH_SIZE_V              (15)
#define SAMPLE_ISP_LSC_MESH_SIZE_H              (19)
#define SAMPLE_ISP_LSC_COLOR_TEMP_GRP_NUM       (10)
#define SAMPLE_ISP_LSC_MESH_SIZE SAMPLE_ISP_LSC_MESH_SIZE_V * SAMPLE_ISP_LSC_MESH_SIZE_H
#define SAMPLE_ISP_LSC_ILLUM_NAME_LEN 12
#define SAMPLE_ISP_LSC_CHANNEL_NUM 4

typedef struct {
    AX_U32 nRRMeshLut[SAMPLE_ISP_LSC_MESH_SIZE_V][SAMPLE_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x44000] */
    AX_U32 nGRMeshLut[SAMPLE_ISP_LSC_MESH_SIZE_V][SAMPLE_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x44000] */
    AX_U32 nGBMeshLut[SAMPLE_ISP_LSC_MESH_SIZE_V][SAMPLE_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x44000] */
    AX_U32 nBBMeshLut[SAMPLE_ISP_LSC_MESH_SIZE_V][SAMPLE_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x44000] */
} SAMPLE_ISP_LSC_COLOR_MESH_T;

typedef struct {
    AX_F32                          fLumaRatioOut;
    SAMPLE_ISP_LSC_COLOR_MESH_T     tColorMeshTab;
    AX_F32                          fRRLscMatrix[SAMPLE_ISP_LSC_MESH_SIZE];
    AX_F32                          fGRLscMatrix[SAMPLE_ISP_LSC_MESH_SIZE];
    AX_F32                          fGBLscMatrix[SAMPLE_ISP_LSC_MESH_SIZE];
    AX_F32                          fBBLscMatrix[SAMPLE_ISP_LSC_MESH_SIZE];
}SAMPLE_ISP_LSC_STATUS_T;


typedef struct {
    AX_U32 nLumaMeshLut[SAMPLE_ISP_LSC_MESH_SIZE_V][SAMPLE_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x44000] */
} SAMPLE_ISP_LSC_LUMA_MESH_T;

typedef struct {
    AX_U8                               nLumaRatio;         /* Accuacy: U8 Range: [0, 100] */
    SAMPLE_ISP_LSC_LUMA_MESH_T          tLumaMeshTab;       /* Luma Shading mesh table */
    SAMPLE_ISP_LSC_COLOR_MESH_T         tColorMeshTab;      /* Color Shading mesh table */
} SAMPLE_ISP_LSC_MANUAL_T;

typedef struct{
    AX_U8                               nColorTempNum;      /* Accuracy: U8 Range: [0, SAMPLE_ISP_LSC_COLOR_TEMP_GRP_NUM] */
    AX_U32                              nColorTemp[SAMPLE_ISP_LSC_COLOR_TEMP_GRP_NUM];         /* Accuracy: U32.0 Range: [0, 100000] */
    SAMPLE_ISP_LSC_COLOR_MESH_T         tColorMeshTab[SAMPLE_ISP_LSC_COLOR_TEMP_GRP_NUM];      /* Color Shading mesh table */
    SAMPLE_ISP_LSC_LUMA_MESH_T          tLumaMeshTab;       /* Luma Shading mesh table */
} SAMPLE_ISP_LSC_CALIB_T;

typedef struct {
    AX_U8                               nDampRatio;      /*Damp Ratio; Accuacy: U8 Range: [0, 100] */
    AX_U8                               nParamGrpNum;      /* Accuacy: U8 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32                              nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];    /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U20.12 Range: [0, 0xFFFFFFFF] */
    AX_U8                               nLumaRatio[AX_ISP_AUTO_TABLE_MAX_NUM];    /* Accuacy: U8 Range: [0, 100] */
    SAMPLE_ISP_LSC_CALIB_T              tCalibMeshValue;
} SAMPLE_ISP_LSC_AUTO_T;

typedef struct {
    AX_U8                               nLscEn;                  /* Acuracy: U8 Range: [0, 1] */
    AX_U8                               nRefMode;                /* choose ref mode, Accuracy: U8 Range: [0, 1], 0: use lux as ref, 1: use gain as ref */
    AX_U8                               nMeshMode;               /* mesh mode, 1: mirror mode, 0: normal mode, Accuracy: U8 Range: [0, 1] */
    AX_U8                               nAutoMode;               /* for ref auto or manual adjust mode, Accuracy: U8 Range: [0, 1]; 0: manual, 1:auto, default:1 */
    SAMPLE_ISP_LSC_MANUAL_T             tManualParam;
    SAMPLE_ISP_LSC_AUTO_T               tAutoParam;
} SAMPLE_ISP_LSC_PARAM_T;

typedef struct {
    AX_U32                              uPreTotalGain[AX_HDR_CHN_NUM];
    AX_U32                              uPreLux;
    AX_U32                              uPreColorTemp;
    AX_U8                               nUserUpdate;
    AX_BOOL                             bUpdateMeshRegs;
    SAMPLE_ISP_LSC_STATUS_T             tStatus;
    SAMPLE_ISP_LSC_PARAM_T              tLscParam;
} SAMPLE_ISP_LSC_CTX_T;

AX_S32 sample_lsc_init(AX_U8 pipe, AX_ISP_LSC_TABLE_T  *tLscResult);
AX_S32 sample_lsc_deinit(AX_U8 pipe);
AX_S32 sample_lsc_run(AX_U8 pipe, AX_ISP_LSC_INPUT_INFO_T tLscInputInfo, AX_BOOL *bUpdateMeshRegs, AX_ISP_LSC_TABLE_T *tLscResult);

#ifdef __cplusplus
}
#endif

#endif
