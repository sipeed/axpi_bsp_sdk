/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_ISP_IQ_API_H__
#define __AX_ISP_IQ_API_H__

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_isp_common.h"
#include "ax_isp_debug.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AX_ISP_AUTO_TABLE_MAX_NUM           (16)
#define AX_ISP_GAIN_GRP_NUM                 (24)
#define AX_ISP_EXPOSE_TIME_GRP_NUM          (10)

/************************************************************************************
 *  BLC IQ Param: SBL + GBL
 ************************************************************************************/
#define AX_ISP_BLC_SBL_WIN_NUM              (4)
#define AX_ISP_BLC_GBL_IIR_SIZE             (8)

typedef struct {
    AX_U32 nSblRValue[AX_ISP_BLC_SBL_WIN_NUM];     /* Accuracy: U8.12 Range: [0, 0xFFFFF] */
    AX_U32 nSblGrValue[AX_ISP_BLC_SBL_WIN_NUM];    /* Accuracy: U8.12 Range: [0, 0xFFFFF] */
    AX_U32 nSblGbValue[AX_ISP_BLC_SBL_WIN_NUM];    /* Accuracy: U8.12 Range: [0, 0xFFFFF] */
    AX_U32 nSblBValue[AX_ISP_BLC_SBL_WIN_NUM];     /* Accuracy: U8.12 Range: [0, 0xFFFFF] */
} AX_ISP_IQ_BLC_MANUAL_T;

typedef struct {
    AX_U8  nGainGrpNum;                    /* Gain dimension num. Accuracy: U8.0 Range: [0, AX_ISP_GAIN_GRP_NUM] */
    AX_U8  nExposeTimeGrpNum;              /* ExposeTime dimension num. Accuracy: U8.0 Range: [0, AX_ISP_EXPOSE_TIME_GRP_NUM] */
    AX_U32 nGain[AX_ISP_GAIN_GRP_NUM];     /* Again value for sbl tunning. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF] */
    AX_U32 nExposeTime[AX_ISP_EXPOSE_TIME_GRP_NUM];    /* ExposeTime value for sbl tunning. Accuracy: U32 Range: [0x0, 0xFFFFFFFF] */
    AX_U32 nAutoSblRValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];    /* offline sbl tunning value for R channel.  Accuracy: U8.12 Range: [0, 0xFFFFF] */
    AX_U32 nAutoSblGrValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];   /* offline sbl tunning value for Gr channel. Accuracy: U8.12 Range: [0, 0xFFFFF] */
    AX_U32 nAutoSblGbValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];   /* offline sbl tunning value for Gb channel. Accuracy: U8.12 Range: [0, 0xFFFFF] */
    AX_U32 nAutoSblBValue[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];    /* offline sbl tunning value for B channel.  Accuracy: U8.12 Range: [0, 0xFFFFF] */
} AX_ISP_IQ_BLC_AUTO_TABLE_T;

typedef struct {
    AX_ISP_IQ_BLC_AUTO_TABLE_T      tHcgAutoTable;
    AX_ISP_IQ_BLC_AUTO_TABLE_T      tLcgAutoTable;
} AX_ISP_IQ_BLC_AUTO_T;

typedef struct {
    AX_U8                           nBlcEnable;     /* sbl correction enable */
    AX_U8                           nAutoMode;      /* 0: manual, 1: auto */
    AX_ISP_IQ_BLC_MANUAL_T          tManualParam;
    AX_ISP_IQ_BLC_AUTO_T            tAutoParam;
} AX_ISP_IQ_BLC_PARAM_T;

typedef struct {
    AX_U8                           nGblEnable; /* GBL enable */
    AX_U8                           nBlcDetSel; /* 0: use det0 win0 result 1: use det1 win0 result */
    AX_U8                           nGblIirRate[AX_ISP_BLC_GBL_IIR_SIZE]; /* Accuracy: U1.6  Range: [0, 0x7F] */
    AX_U16                          nGblIirTh[AX_ISP_BLC_GBL_IIR_SIZE];   /* Accuracy: U4.12 Range: [0, 0xFFFF] */
} AX_ISP_IQ_GBL_PARAM_T;

/************************************************************************************
 *  FPN IQ Param
 ************************************************************************************/
typedef struct {
    AX_S32 nRangeMax;               /* max threshold of saturation for VOB pixel. Accuracy: S8.8 Range: [-65535, 65535] */
    AX_S32 nRangeMin;               /* min threshold of saturation for VOB pixel. Accuracy: S8.8 Range: [-65535, 65535] */
    AX_U32 nIirRate;                /* smooth ratio. Accuracy: U1.8 Range: [0, 0x1FF] */
} AX_ISP_IQ_FPN_CTRL_T;

typedef struct {
    AX_U8                           nFpnEnable; /* FPN enable */
    AX_ISP_IQ_FPN_CTRL_T            tCtrlParam; /* FPN control parameters */
} AX_ISP_IQ_FPN_PARAM_T;

/************************************************************************************
 *  DarkSharding IQ Param
 ************************************************************************************/
#define AX_ISP_BLC_MESH_SIZE_H              (24)
#define AX_ISP_BLC_MESH_SIZE_V              (24)

typedef struct {
    AX_U16 nMeshLut[AX_ISP_BLC_MESH_SIZE_H][AX_ISP_BLC_MESH_SIZE_V];    /* mesh lut for the specific Again and ExposeTime. Accuracy: U8.8 Range: [0, 0xFFFF] */
} AX_ISP_IQ_DS_MESH_LUT_T;

typedef struct {
    AX_U8  nGainGrpNum;                               /* Gain dimension num. Accuracy: U8.0 Range: [0, AX_ISP_GAIN_GRP_NUM] */
    AX_U8  nExposeTimeGrpNum;                         /* ExposeTime dimension num. Accuracy: U8.0 Range: [0, AX_ISP_EXPOSE_TIME_GRP_NUM] */
    AX_U32 nGain[AX_ISP_GAIN_GRP_NUM];                /* Again value for sbl tunning. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF] */
    AX_U32 nExposeTime[AX_ISP_EXPOSE_TIME_GRP_NUM];   /* ExposeTime value for sbl tunning. Accuracy: U32 Range: [0x0, 0xFFFFFFFF] */
    AX_ISP_IQ_DS_MESH_LUT_T tMeshTab[AX_ISP_GAIN_GRP_NUM][AX_ISP_EXPOSE_TIME_GRP_NUM];    /* Dark Shading mesh table */
} AX_ISP_IQ_DS_AUTO_TABLE_T;

typedef struct {
    AX_ISP_IQ_DS_AUTO_TABLE_T       tHcgAutoTable;
    AX_ISP_IQ_DS_AUTO_TABLE_T       tLcgAutoTable;
} AX_ISP_IQ_DS_AUTO_T;

typedef struct {
    AX_ISP_IQ_DS_MESH_LUT_T         tMeshTab;       /* Dark Shading mesh table */
} AX_ISP_IQ_DS_MANUAL_T;

typedef struct {
    AX_U8  nDsEnable;                               /* Dark Shading enable */
    AX_U8  nAutoMode;                               /* 0: manual, 1: auto */
    AX_ISP_IQ_DS_MANUAL_T           tManualParam;
    AX_ISP_IQ_DS_AUTO_T             tAutoParam;
} AX_ISP_IQ_DS_PARAM_T;

/************************************************************************************
 *  DPC IQ Param
 ************************************************************************************/
#define AX_ISP_DPC_NOISE_PARAM_NUM          (4)
#define AX_ISP_DPC_SBPC_BUFFER_MAX          (8192)
#define AX_ISP_DPC_QUICK_DET_TH_NUM         (3)
#define AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM  (3)

typedef struct {
    AX_U8  nDetM1;                                                /* detect bad pixel. Accuracy: U4.4 Range: [0, 255] */
    AX_U8  nDetM2;                                                /* detect bad pixel. Accuracy: U4.4 Range: [0, 255] */
    AX_U8  nInterpolateM3;                                        /* Interpolation for bad pixel. Accuracy: U1.7 Range: [0, 255] */
    AX_U8  nInterpolateM4;                                        /* Interpolation for bad pixel. Accuracy: U1.7 Range: [0, 128] */
    AX_U16 nMarginU[AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM];          /* prevent excessive saturation. Accuracy: U8.6 Range: [0, 16320] */
    AX_U16 nMarginL[AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM];          /* prevent excessive saturation. Accuracy: U8.6 Range: [0, 16320] */
    AX_U8  nColorLimitRatio[AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM];  /* prevent excessive saturation. Accuracy: U1.7 Range: [0, 128] */
    AX_U16 nQuickDetThreshold[AX_ISP_DPC_QUICK_DET_TH_NUM];       /* Accuracy: U8.6 Range: [0, 0x3FFF] */
} AX_ISP_IQ_DPC_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum;                               /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];         /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nDetM1[AX_ISP_AUTO_TABLE_MAX_NUM];                                                /* detect bad pixel. Accuracy: U4.4 Range: [0, 255] */
    AX_U8  nDetM2[AX_ISP_AUTO_TABLE_MAX_NUM];                                                /* detect bad pixel. Accuracy: U4.4 Range: [0, 255] */
    AX_U8  nInterpolateM3[AX_ISP_AUTO_TABLE_MAX_NUM];                                        /* Interpolation for bad pixel. Accuracy: U1.7 Range: [0, 255] */
    AX_U8  nInterpolateM4[AX_ISP_AUTO_TABLE_MAX_NUM];                                        /* Interpolation for bad pixel. Accuracy: U1.7 Range: [0, 128] */
    AX_U16 nMarginU[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM];          /* prevent excessive saturation. Accuracy: U8.6 Range: [0, 16320] */
    AX_U16 nMarginL[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM];          /* prevent excessive saturation. Accuracy: U8.6 Range: [0, 16320] */
    AX_U8  nColorLimitRatio[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DPC_DYNAMIC_STATIC_PDAF_NUM];  /* prevent excessive saturation. Accuracy: U1.7 Range: [0, 128] */
    AX_U16 nQuickDetThreshold[AX_ISP_AUTO_TABLE_MAX_NUM][AX_ISP_DPC_QUICK_DET_TH_NUM];       /* Accuracy: U8.6 Range: [0, 0x3FFF] */
} AX_ISP_IQ_DPC_AUTO_T;

typedef struct {
    AX_U32 nSbpcLength;                                     /* Accuracy: U32 Range: [0, AX_ISP_DPC_SBPC_BUFFER_MAX] */
    AX_U32 nSbpcBuffer[AX_ISP_DPC_SBPC_BUFFER_MAX];         /* Accuracy: U32 Range: [0, 0xFFFFFFFF] */
    AX_S32 nShotNoiseCoeffsA[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nShotNoiseCoeffsB[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nReadNoiseCoeffsA[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nReadNoiseCoeffsB[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32 nReadNoiseCoeffsC[AX_ISP_DPC_NOISE_PARAM_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
} AX_ISP_DPC_NOISE_SBPC_T;

typedef struct {
    AX_ISP_DPC_NOISE_SBPC_T         tHcgTable;
    AX_ISP_DPC_NOISE_SBPC_T         tLcgTable;
} AX_ISP_DPC_TABLE_T;

typedef struct {
    AX_U8  nDpcEnable;                                  /* dpc enable */
    AX_U8  nSbpcEnable;                                 /* sbpc enable */
    AX_U8  nQuickDetEnable;                             /* QuickDet enable */
    AX_ISP_DPC_TABLE_T              tDpcParam;
    AX_U8  nAutoMode;      /* for ref auto or manual adjust mode, Range: [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;       /* choose ref mode, Range: [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_ISP_IQ_DPC_MANUAL_T          tManualParam;
    AX_ISP_IQ_DPC_AUTO_T            tAutoParam;
} AX_ISP_IQ_DPC_PARAM_T;

/************************************************************************************
 *  WNR IQ Param
 ************************************************************************************/
#define AX_ISP_WNR_BYAER_NUM                (4)
#define AX_ISP_WNR_STRENGTH_LUT_SIZE        (128)
#define AX_ISP_WNR_WEIGHT_SIZE              (16)
#define AX_ISP_WNR_AUTO_TABLE_MAX_NUM       (9)

typedef struct {
    AX_S32         nShotNoiseCoeffsA[AX_ISP_WNR_BYAER_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32         nShotNoiseCoeffsB[AX_ISP_WNR_BYAER_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32         nReadNoiseCoeffsA[AX_ISP_WNR_BYAER_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32         nReadNoiseCoeffsB[AX_ISP_WNR_BYAER_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
    AX_S32         nReadNoiseCoeffsC[AX_ISP_WNR_BYAER_NUM];   /* Accuracy: S0.31 Range: [-0x7FFFFFFF, 0x7FFFFFFF] */
} AX_ISP_WNR_NOISE_T;

typedef struct {
    AX_ISP_WNR_NOISE_T         tHcgTable;
    AX_ISP_WNR_NOISE_T         tLcgTable;
} AX_ISP_WNR_NOISE_TABLE_T;

typedef struct {
    AX_U16         nRatio[AX_ISP_WNR_BYAER_NUM];  /* Ratio. Accuracy: U8.8 Range: [1, 65535] */
    AX_U16         nShrinkageStrength;            /*2D Shrinkage Strength. Accuracy: U8.8 Range: [1, 65535] */
    AX_U8          nShrinkageWeight[AX_ISP_WNR_WEIGHT_SIZE][AX_ISP_WNR_WEIGHT_SIZE]; /*2D Shrinkage Weight. Accuracy: U1.7 Range: [0, 128] */
    AX_U16         nBlendStrength;                /*3D  Blend Strength. Accuracy: U8.8 Range: [1, 65535] */
    AX_U8          nBlendWeight[AX_ISP_WNR_WEIGHT_SIZE][AX_ISP_WNR_WEIGHT_SIZE];/*3D Blend Weight. Accuracy: U1.7 Range: [0, 128] */
    AX_U16         nDeghostStrength;              /*3D Deghost Strength. Accuracy: U8.8 Range: [1, 65535] */
    AX_U16         nStrengthLut[AX_ISP_WNR_STRENGTH_LUT_SIZE]; /*3D Strength Lut. Accuracy: U8.8 Range: [1, 65535] */
} AX_ISP_IQ_WNR_2D_3DNR_PARAM_T;

typedef struct {
    AX_U8                          nWnrMode;   /* Wnr mode. Accuracy: U2 Range: 1,2,3 */
    AX_ISP_IQ_WNR_2D_3DNR_PARAM_T  tWnrParam;
} AX_ISP_IQ_WNR_MANUAL_T;

typedef struct {
    AX_U8                          nSubGrpNum; /* Accuracy: U8.0 Range: [0, AX_ISP_WNR_AUTO_TABLE_MAX_NUM] */
    AX_U8                          nWnrMode;   /* Wnr mode. Accuracy: U2 Range: 1,2,3 */
    AX_U32                         nIsoThresholdValue[AX_ISP_WNR_AUTO_TABLE_MAX_NUM];  /* choose ref value. Accuracy: U16 Value Range: [1, 65535] */
    AX_ISP_IQ_WNR_2D_3DNR_PARAM_T  tWnrParam[AX_ISP_WNR_AUTO_TABLE_MAX_NUM];
} AX_ISP_IQ_WNR_AUTO_PARAM_T;

typedef struct {
    AX_U8                          nGrpNum;    /*Group number. Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_ISP_IQ_WNR_AUTO_PARAM_T     tSubAutoParam[AX_ISP_AUTO_TABLE_MAX_NUM];
} AX_ISP_IQ_WNR_AUTO_T;

typedef struct {
    AX_U8                           nWnrEn;    /* Wnr enable. Accuracy: U1 Range: [0, 1] */
    AX_U8                           nAutoMode; /* for ref auto or manual adjust mode. Accuracy: U1 Range: [0, 1] */
    AX_ISP_WNR_NOISE_TABLE_T        tWnrNoiseParam;
    AX_ISP_IQ_WNR_MANUAL_T          tManualParam;
    AX_ISP_IQ_WNR_AUTO_T            tAutoParam;
} AX_ISP_IQ_WNR_PARAM_T;

/************************************************************************************
 *  CAC IQ Param
 ************************************************************************************/
#define AX_ISP_CAC_FOCUS_LENG               (2)
#define AX_ISP_CAC_COLOR_TEMP               (4)
#define AX_ISP_CAC_LUT_SIZE                 (32)

typedef struct {
    AX_U32 nScaler[AX_ISP_CAC_LUT_SIZE]; /* pixel position shift lut. Accuracy: U1.28 Range: [214748365, 322122547] */
} AX_ISP_IQ_CAC_LUT_T;

typedef struct {
    AX_U32 nRXCoord;              /* ellipse center x_coord. Accuracy: U12.8 Range: [0x8000, 0x100000)*/
    AX_U32 nRYCoord;              /* ellipse center y_coord. Accuracy: U12.8 Range: [0x8000, 0x100000)*/
    AX_S32 nRAParam;              /* ellipse parameter A. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nRBParam;              /* ellipse parameter B. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nRCParam;              /* ellipse parameter C. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_U32 nRMaxRadInv;           /* inv of bound value in lut. Accuracy: U3.18 Range: [52429, 2097152) */
    AX_ISP_IQ_CAC_LUT_T tRScaler; /* pixel position shift lut. Accuracy: U1.28 Range: [214748365, 322122547] */
    AX_U32 nBXCoord;              /* ellipse center x_coord. Accuracy: U12.8 Range: [0x8000, 0x100000)*/
    AX_U32 nBYCoord;              /* ellipse center y_coord. Accuracy: U12.8 Range: [0x8000, 0x100000)*/
    AX_S32 nBAParam;              /* ellipse parameter A. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nBBParam;              /* ellipse parameter B. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nBCParam;              /* ellipse parameter C. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_U32 nBMaxRadInv;           /* inv of bound value in lut. Accuracy: U3.18 Range: [52429, 2097152) */
    AX_ISP_IQ_CAC_LUT_T tBScaler; /* pixel position shift lut. Accuracy: U1.28 Range: [214748365, 322122547] */
    AX_U8  nAParamLeftShift;      /* left bit shift for ellipse parameter A. Accuracy: U5 Range: [0x10, 0x18] */
    AX_U8  nBParamLeftShift;      /* left bit shift for ellipse parameter B. Accuracy: U5 Range: [0x10, 0x18] */
    AX_U8  nCParamLeftShift;      /* left bit shift for ellipse parameter C. Accuracy: U5 Range: [0x10, 0x18] */
} AX_ISP_IQ_CAC_MANUAL_T;

typedef struct {
    AX_U8  nFocusGrpNum;                                                     /* Accuacy: U8 Range: [0, AX_ISP_CAC_FOCUS_LENG] */
    AX_U8  nColorGrpNum;                                                     /* Accuacy: U8 Range: [0, AX_ISP_CAC_COLOR_TEMP] */
    AX_U8  nFocusVal[AX_ISP_CAC_FOCUS_LENG];                                 /* The normalized value by focus value. Accuacy: U8 Range: [0, 100] */
    AX_U32 nColorTemp[AX_ISP_CAC_COLOR_TEMP];                                /* Accuracy: U32.0 Range: [0, 100000] */
    AX_U32 nRXCoord[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse center x_coord. Accuracy: U12.8 Range: [0x8000, 0x100000) */
    AX_U32 nRYCoord[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse center y_coord. Accuracy: U12.8 Range: [0x8000, 0x100000) */
    AX_S32 nRAParam[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse parameter A. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nRBParam[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse parameter B. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nRCParam[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse parameter C. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_U32 nRMaxRadInv[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];        /* inv of bound value in lut. Accuracy: U3.18 Range: [52429, 2097152)*/
    AX_ISP_IQ_CAC_LUT_T tRLut[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP]; /* pixel position shift lut. Accuracy: U1.28 Range: [214748365, 322122547] */
    AX_U32 nBXCoord[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse center x_coord. Accuracy: U12.8 Range: [0x8000, 0x100000) */
    AX_U32 nBYCoord[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse center y_coord. Accuracy: U12.8 Range: [0x8000, 0x100000) */
    AX_S32 nBAParam[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse parameter A. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nBBParam[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse parameter B. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_S32 nBCParam[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];           /* ellipse parameter C. Accuracy: S0.22 Range: (-4194304, 4194304) */
    AX_U32 nBMaxRadInv[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];        /* inv of bound value in lut. Accuracy: U3.18 Range: [52429, 2097152) */
    AX_ISP_IQ_CAC_LUT_T tBLut[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP]; /* pixel position shift lut. Accuracy: U1.28 Range: [214748365, 322122547] */
    AX_U8  nAParamLeftShift[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];   /* left bit shift for ellipse parameter A. Accuracy: U5 Range: [0x10, 0x18] */
    AX_U8  nBParamLeftShift[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];   /* left bit shift for ellipse parameter B. Accuracy: U5 Range: [0x10, 0x18] */
    AX_U8  nCParamLeftShift[AX_ISP_CAC_FOCUS_LENG][AX_ISP_CAC_COLOR_TEMP];   /* left bit shift for ellipse parameter C. Accuracy: U5 Range: [0x10, 0x18] */
} AX_ISP_IQ_CAC_AUTO_T;

typedef struct {
    AX_U8                           nCacEnable;  /* Accuracy: U8 Range: [0, 1] */
    AX_U8                           nAutoMode;   /* Accuracy: U8 Range: [0, 1]; 0: manual, 1: auto */
    AX_ISP_IQ_CAC_MANUAL_T          tManualCtrl;
    AX_ISP_IQ_CAC_AUTO_T            tAutoParam;
} AX_ISP_IQ_CAC_PARAM_T;

/************************************************************************************
 *  LSC IQ Param
 ************************************************************************************/
#define AX_ISP_LSC_MESH_SIZE_V              (15)
#define AX_ISP_LSC_MESH_SIZE_H              (19)
#define AX_ISP_LSC_COLOR_TEMP_GRP_NUM       (10)

typedef struct AX_ISP_IQ_LSC_COLOR_MESH_T {
    AX_U32 nRRMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32 nGRMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32 nGBMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
    AX_U32 nBBMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
} AX_ISP_IQ_LSC_COLOR_MESH_T;

typedef struct AX_ISP_IQ_LSC_LUMA_MESH_T {
    AX_U32 nLumaMeshLut[AX_ISP_LSC_MESH_SIZE_V][AX_ISP_LSC_MESH_SIZE_H]; /* Accuacy U4.14 Range: [0x4000, 0x3FFFF] */
} AX_ISP_IQ_LSC_LUMA_MESH_T;

typedef struct {
    AX_U8                           nLumaRatio;         /* Accuacy: U8 Range: [0, 100] */
    AX_ISP_IQ_LSC_LUMA_MESH_T       tLumaMeshTab;       /* Luma Shading mesh table */
    AX_ISP_IQ_LSC_COLOR_MESH_T      tColorMeshTab;      /* Color Shading mesh table */
} AX_ISP_IQ_LSC_MANUAL_T;

typedef struct {
    AX_U8      nParamGrpNum;      /*Luma Grp Num; Accuacy: U8 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32     nRefValStart[AX_ISP_AUTO_TABLE_MAX_NUM];  /*Ref Gain Start: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF];Ref Lux Start: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32     nRefValEnd[AX_ISP_AUTO_TABLE_MAX_NUM];    /*Ref Gain End: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF];Ref Lux End: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8      nLumaRatio[AX_ISP_AUTO_TABLE_MAX_NUM];    /*Luma Ratio; Accuacy: U8 Range: [0, 100] */
    AX_ISP_IQ_LSC_LUMA_MESH_T       tLumaMeshTab;        /*Calib Luma Shading mesh table */
} AX_ISP_IQ_LSC_LUMA_PARAM_T;

typedef struct {
    AX_U8      nColTempNum;      /*Calib Color Temp Num; Accuracy: U8 Range: [0, AX_ISP_LSC_COLOR_TEMP_GRP_NUM] */
    AX_U32     nRefColorTempStart[AX_ISP_LSC_COLOR_TEMP_GRP_NUM];  /*Ref CCT Start; Accuracy: U32.0 Range: [0, 100000]*/
    AX_U32     nRefColorTempEnd[AX_ISP_LSC_COLOR_TEMP_GRP_NUM];    /*Ref CCT End; Accuracy: U32.0 Range: [0, 100000]*/
    AX_U32     nColorTemp[AX_ISP_LSC_COLOR_TEMP_GRP_NUM];          /*Calib CCT; Accuracy: U32.0 Range: [0, 100000] */
    AX_ISP_IQ_LSC_COLOR_MESH_T      tColorMeshTab[AX_ISP_LSC_COLOR_TEMP_GRP_NUM];      /*Calib Color Shading mesh table */
} AX_ISP_IQ_LSC_CT_PARAM_T;

typedef struct {
    AX_U8                           nDampRatio;    /*Damp Ratio; Accuacy: U8 Range: [0, 100] */
    AX_ISP_IQ_LSC_LUMA_PARAM_T      tLumaParam;    /*Luma Params*/
    AX_ISP_IQ_LSC_CT_PARAM_T        tColTempParam; /*Color Temp Params*/
} AX_ISP_IQ_LSC_AUTO_T;

typedef struct {
    AX_U8  nLscEn;                  /* Acuracy: U8 Range: [0, 1] */
    AX_U8  nRefMode;                /* choose ref mode, Accuracy: U8 Range: [0, 1], 0: use lux as ref, 1: use gain as ref */
    AX_U8  nMeshMode;               /* mesh mode, 1: mirror mode, 0: normal mode, Accuracy: U8 Range: [0, 1] */
    AX_U8  nAutoMode;               /* for ref auto or manual adjust mode, Accuracy: U8 Range: [0, 1]; 0: manual, 1:auto, default:1 */
    AX_ISP_IQ_LSC_MANUAL_T          tManualParam;
    AX_ISP_IQ_LSC_AUTO_T            tAutoParam;
} AX_ISP_IQ_LSC_PARAM_T;

#define AX_LTM_SCURVE_MAX_LEN (1025)
#define AX_LTM_HISTOGRAM_WEIGHT_MAX_LEN (67)
#define AX_LTM_HISTOGRAM_WEIGHT_NUM (16)
#define AX_RLTM_HIST_REGION_NUM (4)
/************************************************************************************
 *  RLTM IQ Param
 ************************************************************************************/
typedef struct {
    AX_U16 nHistogramWeight[AX_LTM_HISTOGRAM_WEIGHT_MAX_LEN]; /* histogram bin weights. Accuracy: U16 Range: [0, 65535], default 1*/
} AX_ISP_IQ_RLTM_HW_T;

typedef struct {
    AX_U8  nLocalFactor;        /* Accuracy: U1.7 Range: [0, 128], default 90 */
    AX_U8  nHighlightSup;       /* highlight suppress. Accuracy: U5.3 Range: [0, 255], default 50 */
    AX_U16 nKMax;               /* limit brightness. Accuracy: U8.8 Range: [256, 65535], default 1024 */
    AX_U8  nPreGamma;           /* for gamma lut. Accuracy: U3.5 Range: [32, 255], default 32 */
    AX_U8  nPostGamma;          /* for invgamma lut. Accuracy: U3.5 Range: [32, 255], default 64 */
    AX_U8  nDynamicRangeUpper;  /* for dynamic range upper, not dependence on effect_strength. Accuracy: U1.7 Range: [90, 128], default 128 */
    AX_U8  nDynamicRangeLower;  /* for dynamic range lower, not dependence on effect_strength. Accuracy: U1.7 Range: [0, 40], default 0 */
    AX_U16 nExtraDgain;         /* for invgamma lut. Accuracy: U4.4 Range: [16, 255], default 16 */
    AX_U16 nWinSize;            /* for hist. Accuracy: U16.0 Range: [64, 128, 256, 512, 1024], default 256 */
    AX_U8  nRltmStrength;       /* Accuracy: U1.7 Range: [0, 128], default 64 */
    AX_U8  nLog10Offset;        /* log10 offset. Accuracy: U3.5 Range: [0, 211], default 0 */
    AX_U8  nContrastStrength;   /* contrast strength. Accuracy: U1.7 Range: [0, 255], default 42 */
    AX_U16 nBaseGain;           /* base gain. Accuracy: U10.6 Range: [1, 65535], default 64 */
    AX_U8  nDitherMode;         /* 0: no-dither, 1: before pre-gamma, 2: after pre-gamma 0. Accuracy: U2.0 Range: [0, 2], default 0 */
    AX_U16 nDitherScaler;       /* for dither strength. Accuracy: U10.6 Range: [0, 65535], default 64 */
    AX_U8  nGtmSwEn;            /* gtm software switch. Accuracy: U1.0 Range: [0, 1], default 0 */
    AX_U16 nGtmSwDgain;         /* gtm dgain for software gtm curve. Accuracy: U8.8 Range: [256, 65535], default 256 */
    AX_U16 nSCurveList[AX_LTM_SCURVE_MAX_LEN]; /* s curve lut. Accuracy: U1.15 Range: [0, 32768]*/
    AX_ISP_IQ_RLTM_HW_T tHWeight[AX_LTM_HISTOGRAM_WEIGHT_NUM];
} AX_ISP_IQ_RLTM_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum;                                 /* Accuracy: U8 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM]  */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];           /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nLocalFactor[AX_ISP_AUTO_TABLE_MAX_NUM];      /* Accuracy: U1.7 Range: [0, 128], default 90 */
    AX_U8  nHighlightSup[AX_ISP_AUTO_TABLE_MAX_NUM];     /* highlight suppress. Accuracy: U5.3 Range: [0, 255], default 50 */
    AX_U16 nKMax[AX_ISP_AUTO_TABLE_MAX_NUM];             /* limit brightness. Accuracy: U8.8 Range: [256, 65535], default 1024 */
    AX_U8  nPreGamma[AX_ISP_AUTO_TABLE_MAX_NUM];         /* for gamma lut. Accuracy: U3.5 Range: [32, 255], default 32 */
    AX_U8  nPostGamma[AX_ISP_AUTO_TABLE_MAX_NUM];        /* for invgamma lut. Accuracy: U3.5 Range: [32, 255], default 64 */
    AX_U8  nDynamicRangeUpper[AX_ISP_AUTO_TABLE_MAX_NUM];/* for dynamic range upper, not dependence on effect_strength. Accuracy: U1.7 Range: [90, 128], default 128 */
    AX_U8  nDynamicRangeLower[AX_ISP_AUTO_TABLE_MAX_NUM];/* for dynamic range lower, not dependence on effect_strength. Accuracy: U1.7 Range: [0, 40], default 0 */
    AX_U16 nExtraDgain[AX_ISP_AUTO_TABLE_MAX_NUM];       /* for invgamma lut. Accuracy: U4.4 Range: [16, 255], default 16 */
    AX_U16 nWinSize[AX_ISP_AUTO_TABLE_MAX_NUM];          /* for hist. Accuracy: U16.0 Range: [64, 128, 256, 512, 1024], default 256 */
    AX_U8  nRltmStrength[AX_ISP_AUTO_TABLE_MAX_NUM];     /* Accuracy: U1.7 Range: [0, 128], default 64 */
    AX_U8  nLog10Offset[AX_ISP_AUTO_TABLE_MAX_NUM];      /* log10 offset. Accuracy: U3.5 Range: [0, 211], default 0 */
    AX_U8  nContrastStrength[AX_ISP_AUTO_TABLE_MAX_NUM]; /* contrast strength. Accuracy: U1.7 Range: [0, 255], default 42 */
    AX_U16 nBaseGain[AX_ISP_AUTO_TABLE_MAX_NUM];         /* base gain. Accuracy: U10.6 Range: [1, 65535], default 64 */
    AX_U8  nDitherMode[AX_ISP_AUTO_TABLE_MAX_NUM];       /* 0: no-dither, 1: before pre-gamma, 2: after pre-gamma 0. Accuracy: U2.0 Range: [0, 2], default 0 */
    AX_U16 nDitherScaler[AX_ISP_AUTO_TABLE_MAX_NUM];     /* for dither strength. Accuracy: U10.6 Range: [0, 65535], default 64 */
    AX_U8  nGtmSwEn[AX_ISP_AUTO_TABLE_MAX_NUM];          /* gtm software switch. Accuracy: U1.0 Range: [0, 1], default 0 */
    AX_U16 nGtmSwDgain[AX_ISP_AUTO_TABLE_MAX_NUM];       /* gtm dgain for software gtm curve. Accuracy: U8.8 Range: [256, 65535], default 256 */
    AX_U16 nSCurveList[AX_ISP_AUTO_TABLE_MAX_NUM][AX_LTM_SCURVE_MAX_LEN]; /* s curve lut. Accuracy: U1.15 Range: [0, 32768]*/
    AX_ISP_IQ_RLTM_HW_T tHWeight[AX_ISP_AUTO_TABLE_MAX_NUM][AX_LTM_HISTOGRAM_WEIGHT_NUM];
} AX_ISP_IQ_RLTM_AUTO_T;

typedef struct {
    AX_U16 nTop;         /* Accuracy: U16 Range: [0, 8192] */
    AX_U16 nBottom;    /* Accuracy: U16 Range: [0, 8192] */
    AX_U16 nLeft;         /* Accuracy: U16 Range: [0, 8192] */
    AX_U16 nRight;       /* Accuracy: U16 Range: [0, 8192] */
} AX_ISP_IQ_RLTM_ROI_T;

typedef struct {
    AX_U8 nMode;                           /* rltm base&advance mode. Accuracy:U8 Range: [0, 1] */
    AX_U8 nRegionNum;                  /* valide region number. Accuracy:U8 Range: [0, 4] */
    AX_U8 nHWNum;                      /* hist weight number. Accuracy:U8 Range: [1, 16] */
    AX_U8 nLow[AX_RLTM_HIST_REGION_NUM];               /* Lower limit of brightness range. Accuracy: U8 Range: [0, 66] */
    AX_U8 nHigh[AX_RLTM_HIST_REGION_NUM];              /* Upper limit of brightness range. Accuracy: U8 Range: [0, 66] */
    AX_U8 nThreshold[AX_RLTM_HIST_REGION_NUM];     /* Accuracy: U1.7 Range: [0, 129] */
    AX_U8 nFlagHistId[AX_LTM_HISTOGRAM_WEIGHT_NUM][AX_LTM_HISTOGRAM_WEIGHT_NUM]; /* Accuracy: U8 Range: [0, 1] */
    AX_ISP_IQ_RLTM_ROI_T tRoi;
    AX_ISP_IQ_RLTM_HW_T tHWeight;
} AX_ISP_IQ_RLTM_MHW_T;

typedef struct {
    AX_U8  nRltmEn;        /* rltm en -- module control */
    AX_U8  nMultiCamSyncMode;  /* 0：INDEPEND MODE; 1： MASTER SLAVE MODE; 2: OVERLAP MODE */
    AX_U8  nAutoMode;      /* for ref auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;       /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_ISP_IQ_RLTM_MHW_T         tMutiHW;
    AX_ISP_IQ_RLTM_MANUAL_T         tManualParam;
    AX_ISP_IQ_RLTM_AUTO_T           tAutoParam;
} AX_ISP_IQ_RLTM_PARAM_T;

/************************************************************************************
 *  Dehaze IQ Param
 ************************************************************************************/
typedef struct {
    AX_U8  nEffectStrength;                               /* for effect enhence. Accuracy: U7.0 Range: [0,128], default 0 */
} AX_ISP_IQ_DEHAZE_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum;                                  /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];            /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nEffectStrength[AX_ISP_AUTO_TABLE_MAX_NUM];    /* for effect enhence. Accuracy: U7.0 Range: [0,128], default 0 */
} AX_ISP_IQ_DEHAZE_AUTO_T;

typedef struct {
    AX_U8  nDehazeEn;   /* dehaze enable */
    AX_U8  nAutoMode;   /* for lux auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;    /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref, must be follow rltm'suRefMode */
    AX_ISP_IQ_DEHAZE_MANUAL_T       tManualParam;
    AX_ISP_IQ_DEHAZE_AUTO_T         tAutoParam;
} AX_ISP_IQ_DEHAZE_PARAM_T;

/************************************************************************************
 *  Demosaic IQ Param
 ************************************************************************************/
#define AX_ISP_DEMOSAIC_GAMMA_LUT_SIZE      (8)

typedef struct {
    AX_U8  nGradfRatio;    /* gradient filter frequency adjustment ratio. Accuracy: U1.4 Range: [0, 0x10], default 12 */
    AX_U8  nStrength;      /* edge direction estimation strength. Accuracy: U2.6 Range: [0, 255] */
} AX_ISP_IQ_DEMOSAIC_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum;                              /* Accuracy: U8 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];        /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nGradfRatio[AX_ISP_AUTO_TABLE_MAX_NUM];    /* gradient filter frequency adjustment ratio. Accuracy: U1.4 Range: [0, 0x10], default 12  */
    AX_U8  nStrength[AX_ISP_AUTO_TABLE_MAX_NUM];      /* edge direction estimation strength. Accuracy: U2.6 Range: [0, 255] */
} AX_ISP_IQ_DEMOSAIC_AUTO_T;

typedef struct {
    AX_U8  nDemosaicEn;      /* Demosaic module enable,  Range: [0, 1], 0: Disable, 1: Enable */
    AX_U8  nMode;            /* U4, 0:diff ratio,1:copy to gray,2:copy to green,3:RGGB2RGB,4:divider ratio. set 4 at gain residual mode. default 4 */
    AX_U16 nGammaLut[AX_ISP_DEMOSAIC_GAMMA_LUT_SIZE];    /* set linear at gain residual mode. Accuracy: U8.6 Range: [0, 0x3FFF] */
    AX_U8  nClipLevel;       /* Accuracy: U0.4 Range: [0, 0xF], default 8 */
    AX_U8  nRbclipEnable;    /* 0: disable rbclip, 1:enable rbclip. set 1 at gain residual mode. */
    AX_U8  nFcsEnable;       /* enable false color suppression, Range: [0, 1], 0: Disable, 1: Enable, default 0 */
    AX_U8  nAutoMode;        /* for ref auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;         /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_ISP_IQ_DEMOSAIC_MANUAL_T     tManualParam;
    AX_ISP_IQ_DEMOSAIC_AUTO_T       tAutoParam;
} AX_ISP_IQ_DEMOSAIC_PARAM_T;

/************************************************************************************
*  CLC IQ Param
************************************************************************************/
#define AX_ISP_CLC_LUMA_RATIO_SIZE          (3)
#define AX_ISP_CLC_ANGLE_SIZE               (16)
#define AX_ISP_CLC_CCM_H_SIZE               (2)
#define AX_ISP_CLC_CCM_V_SIZE               (3)
#define AX_ISP_CLC_MAX_GROUP_SIZE           (64)
#define AX_ISP_CLC_MATRIX_COLOR_TEMP_SIZE   (12)
#define AX_ISP_CLC_MATRIX_SATURAT_SIZE      (5)

typedef struct {
    AX_S8  nSat;  /* S1.6 [-64, 64] */
    AX_S16 nHue;  /* s5.6 [-1920, 1920] */
} AX_ISP_IQ_CLC_HS_CCM_T;

typedef struct {
    AX_S8  nSat[AX_ISP_CLC_ANGLE_SIZE];  /* S1.6 [-32, 32] */
    AX_S16 nHue[AX_ISP_CLC_ANGLE_SIZE];  /* s5.6 [-640, 640] */
} AX_ISP_IQ_CLC_HS_XCM_T;

typedef struct {
    AX_S16 nMatrix[AX_ISP_CLC_CCM_V_SIZE][AX_ISP_CLC_CCM_H_SIZE];    /* Accuracy: S3.8 Range: [-2047, 2047] */
} AX_ISP_IQ_CLC_CCM_T;

typedef struct {
    AX_ISP_IQ_CLC_CCM_T      tXcm[AX_ISP_CLC_ANGLE_SIZE];
} AX_ISP_IQ_CLC_XCM_T;

typedef struct {
    AX_ISP_IQ_CLC_HS_CCM_T tHsCcm;
    AX_ISP_IQ_CLC_HS_XCM_T tHsXcm;
} AX_ISP_IQ_CLC_HS_T;

typedef struct {
    AX_U16 nCtrlLevelCcm;                        /* U1.8 [0, 256] */
    AX_U16 nCtrlLevelXcm[AX_ISP_CLC_ANGLE_SIZE]; /* U1.8 [0, 256] */
} AX_ISP_IQ_CLC_CTRL_LEVEL_T;

typedef struct {
    AX_ISP_IQ_CLC_CCM_T           tClcCcm;
    AX_ISP_IQ_CLC_HS_T            tClcHs;
    AX_ISP_IQ_CLC_CTRL_LEVEL_T    tClcCtrlLevel;
} AX_ISP_IQ_CLC_MANUAL_T;

typedef struct {
    AX_U32                      nColorTemp;      /* Accuracy: U32.0 Range: [0, 100000] */
    AX_U8                       nSaturationNum;  /* Accuracy: U8 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM]  */
    AX_U32                      nRefVal[AX_ISP_CLC_MATRIX_SATURAT_SIZE];    /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32                      nSaturation[AX_ISP_CLC_MATRIX_SATURAT_SIZE];/* Accuracy: U8.10 Range: [0, 204800] */
    AX_ISP_IQ_CLC_HS_T          tClcHsList[AX_ISP_CLC_MATRIX_SATURAT_SIZE];
    AX_ISP_IQ_CLC_CTRL_LEVEL_T  tClcCtrlLevelList[AX_ISP_CLC_MATRIX_SATURAT_SIZE];
    AX_ISP_IQ_CLC_CCM_T         tClcCcmList[AX_ISP_CLC_MATRIX_SATURAT_SIZE];
} AX_ISP_IQ_CLC_COL_TEMP_PARAM_T;

typedef struct {
    AX_U8                            nNumCt; /* Accuracy: U8 Range: [0, AX_ISP_CLC_MATRIX_COLOR_TEMP_SIZE] */
    AX_ISP_IQ_CLC_COL_TEMP_PARAM_T   tColTempParam[AX_ISP_CLC_MATRIX_COLOR_TEMP_SIZE];  /* CLC 3*3 clib Matrix*/
} AX_ISP_IQ_CLC_AUTO_T;

typedef struct {
    AX_U32                      nColorTemp;      /* Accuracy: U32.0 Range: [0, 100000] */
    AX_ISP_IQ_CLC_HS_T          tClcHs;
    AX_ISP_IQ_CLC_CTRL_LEVEL_T  tClcCtrlLevel;
    AX_ISP_IQ_CLC_CCM_T         tClcCcm;
} AX_ISP_IQ_CLC_SPC_PARAM_T;

typedef struct {
    AX_U8                            nNumSpc; /* Accuracy: U8 Range: [0, AX_ISP_CLC_MATRIX_COLOR_TEMP_SIZE] */
    AX_ISP_IQ_CLC_SPC_PARAM_T        tSpcParam[AX_ISP_CLC_MATRIX_COLOR_TEMP_SIZE];  /* CLC 3*3 clib Matrix*/
} AX_ISP_IQ_CLC_SPC_T;

typedef struct {
    AX_U8  nCLcEn;              /* Clc lut enable,  Range: [0, 1], 0: Disable, 1: Enable*/
    AX_U8  nClcMode;            /* mode, Range: [0, 1], 0: basic mode 1: advanced mode*/
    AX_U8  nAutoMode;           /* for ref auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1 */
    AX_U8  nRefMode;            /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_U8  nLightSourceMode;    /* U8.0 0-16 0: interpolation mode, 1-16 select spc light source*/
    AX_U8  nSatHueCtrlLuma[AX_ISP_CLC_LUMA_RATIO_SIZE]; /* u1.7  [0, 128] */
    AX_ISP_IQ_CLC_MANUAL_T   tManualParam;
    AX_ISP_IQ_CLC_AUTO_T     tAutoParam;
    AX_ISP_IQ_CLC_SPC_T      tSpcLightSource;
} AX_ISP_IQ_CLC_PARAM_T;

/************************************************************************************
 *  Pfr IQ Param
 ************************************************************************************/
#define AX_ISP_PFR_AUTO_SIZE             (16)
#define AX_ISP_PFR_MATRIX_H_SIZE         (3)
#define AX_ISP_PFR_MATRIX_V_SIZE         (3)
#define AX_ISP_PFR_LUMARATIO_SIZE        (3)
#define AX_ISP_PFR_LUMALUT_SIZE          (8)
#define AX_ISP_PFR_ANGLERATIOLUT_SIZE    (16)
#define AX_ISP_PFR_MAX_PATH_SIZE         (256)

typedef struct {
    AX_U8  nLuxConfidenceLevel;   /* Accuracy: u1.7 Range: [0, 128] */
    AX_U8  nCctConfidenceLevel;   /* Accuracy: u1.7 Range: [0, 128] */
    AX_S16 nMaskMatrix[AX_ISP_PFR_MATRIX_V_SIZE][AX_ISP_PFR_MATRIX_H_SIZE];  /* read only Accuracy: S3.8 Range: [-2047, 2047] */
} AX_ISP_IQ_PFR_MANUAL_T;

typedef struct {
    AX_U8  nLuxNum;                                     /* Accuracy: U8.0 Range: [0, 16]; */
    AX_U8  nCctNum;                                     /* Accuracy: U8.0 Range: [0, 16]; */
    AX_U32 nRefLuxOrGain[AX_ISP_PFR_AUTO_SIZE];         /* Accuracy: U22.10 RefMode == 0: Range: [0x000, 0xFFFFFFFF], 1: [0x400, 0xFFFFFFFF]; */
    AX_U32 nRefCct[AX_ISP_PFR_AUTO_SIZE];               /* Accuracy: U32.0 Range: [0, 100000] */
    AX_U8  nLuxConfidenceLevel[AX_ISP_PFR_AUTO_SIZE];   /* Accuracy: u1.7 Range: [0, 128] */
    AX_U8  nCctConfidenceLevel[AX_ISP_PFR_AUTO_SIZE];   /* Accuracy: u1.7 Range: [0, 128] */
} AX_ISP_IQ_PFR_AUTO_T;

typedef struct {
    AX_CHAR szWbtModelName[AX_ISP_PFR_MAX_PATH_SIZE];  /* wbt model path, absolute path */
    AX_CHAR szModelName[AX_ISP_PFR_MAX_PATH_SIZE];     /* model path, absolute path */
} AX_ISP_NPU_PFR_PARAM_T;

typedef struct {
    AX_U8  nPfrEn;
    AX_U8  nRefMode;                                      /* choose ref mode, Accuracy: U8.0 Range: [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_U8  nProcessMode;                                  /* Accuracy: U8.0 Range: [0, 1, 2], 0: manual, 1:auto, 2: debugmode default:1 */
    AX_U8  nDepurpleStrength;                             /* Accuracy: u1.7 Range: [0, 128] */
    AX_U8  nLumaRatio[AX_ISP_PFR_LUMARATIO_SIZE];         /* Accuracy: u1.7 Range: [0, 128] */
    AX_U8  nLumaLut[AX_ISP_PFR_LUMALUT_SIZE];             /* Accuracy: u1.7 Range: [0, 128] */
    AX_U8  nAngleRatioLut[AX_ISP_PFR_ANGLERATIOLUT_SIZE]; /* Accraucy: u1.7 Range: [0, 128] */
    AX_S16 nCcmMatrix[AX_ISP_PFR_MATRIX_V_SIZE][AX_ISP_PFR_MATRIX_H_SIZE];  /* read only Accuracy: S3.8 Range: [-2047, 2047] */
    AX_ISP_IQ_PFR_MANUAL_T   tManualParam;
    AX_ISP_IQ_PFR_AUTO_T     tAutoParam;
    AX_ISP_NPU_PFR_PARAM_T   tNpuPfrParam;
} AX_ISP_IQ_PFR_PARAM_T;

/************************************************************************************
 *  Gamma IQ Param
 ************************************************************************************/
#define AX_ISP_GAMMA_LUT_SIZE               (65)
#define AX_ISP_GAMMA_CURVE_MAX_NUM          (3)
typedef struct {
    AX_U16                           nGammaLut[AX_ISP_GAMMA_LUT_SIZE];     /* Accuracy: U8.6 Range: [0, 0x3FFF] */
} AX_ISP_IQ_GAMMA_LUT_T;

typedef struct {
    AX_ISP_IQ_GAMMA_LUT_T            tGammaLut;   /* Gamma lut */
} AX_ISP_IQ_GAMMA_MANUAL_T;

typedef struct {
    AX_U8                            nParamGrpNum;            /* Accuracy: U8.0 Range: [0, AX_ISP_GAMMA_CURVE_MAX_NUM] ,must be 3*/
    AX_U32                           nRefValStart[AX_ISP_GAMMA_CURVE_MAX_NUM];  /*Ref Gain Start: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF];Ref Lux Start: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U32                           nRefValEnd[AX_ISP_GAMMA_CURVE_MAX_NUM];    /*Ref Gain End: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF];Ref Lux End: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_ISP_IQ_GAMMA_LUT_T            tGammaLut[AX_ISP_GAMMA_CURVE_MAX_NUM];     /* Gamma lut */
} AX_ISP_IQ_GAMMA_AUTO_T;

typedef struct {
    AX_U8                            nGammaEn;    /* Gamma module enable,  Range: [0, 1], 0: Disable, 1: Enable*/
    AX_U8                            nAutoMode;   /* for ref auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:0 */
    AX_U8                            nRefMode;    /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref */
    AX_U8                            nLutMode;    /* mode, Range: [0, 1], 0: linear, 1: exponential, default:0*/
    AX_ISP_IQ_GAMMA_MANUAL_T         tManualParam;
    AX_ISP_IQ_GAMMA_AUTO_T           tAutoParam;
} AX_ISP_IQ_GAMMA_PARAM_T;

/************************************************************************************
 *  CSC IQ Param
 ************************************************************************************/
#define AX_ISP_YUV_CSC0_MATRIX_SIZE         (3)
#define AX_ISP_YUV_CSC0_DECIMATION_NUM      (7)

typedef struct  {
    AX_S16 nMatrix[AX_ISP_YUV_CSC0_MATRIX_SIZE][AX_ISP_YUV_CSC0_MATRIX_SIZE]; /* color matrix. Accuracy: S1.8 Range: [-512, 511] */
    AX_S8  nDeciH[AX_ISP_YUV_CSC0_DECIMATION_NUM]; /* horizontal decimation filter, nDeciH[0] + ... nDeciH[6] = 32. Accuracy: S1.5 Range: [-64, 63] */
    AX_U8  nUvSeqSel; /* U/V sequence select. Accuracy: U1 Range: [0, 1] */
} AX_ISP_IQ_YUV_CSC0_PARAM_T;

typedef struct  {
    AX_U8  nDeciV0; /* vertical decimation filter, nDeciV0 + nDeciV1 = 2. Accuracy: U1.1 Range: [0, 2] */
    AX_U8  nDeciV1; /* vertical decimation filter, nDeciV0 + nDeciV1 = 2. Accuracy: U1.1 Range: [0, 2] */
} AX_ISP_IQ_YUV_CSC1_PARAM_T;

/************************************************************************************
 *  Sharpen IQ Param
 ************************************************************************************/
#define AX_ISP_SHP_LIMIT_SIZE           (2)
#define AX_ISP_SHP_OS_LIMIT_SIZE        (2)
#define AX_ISP_SHP_GAIN_SIZE            (2)
#define AX_ISP_SHP_HPF_LUT_SIZE_H       (3)
#define AX_ISP_SHP_HPF_LUT_SIZE_V       (3)

typedef struct {
    AX_U8  nSlope;  /* sharpen edge slop. Accuracy: U1.7 Range: [0, 128] */
    AX_S8  nOffset; /* sharpen edge offset. Accuracy: S1.6 Range: [-128, 127] */
    AX_U8  nLimit;  /* sharpen edge limit. Accuracy: U0.8 Range: [0, 255] */
} AX_ISP_IQ_SHP_ED_T;

typedef struct {
    AX_U8  nShpGain[AX_ISP_SHP_GAIN_SIZE];        /* sharpen gain. Accuracy: U4.4 Range: [0, 255] */
    AX_S16 nShpLimit[AX_ISP_SHP_LIMIT_SIZE];      /* sharpen limit. Accuracy: S8.2 Range: [-1024, 1023] */
    AX_S8  nShpOsLimit[AX_ISP_SHP_OS_LIMIT_SIZE]; /* sharpen over shot limit. Accuracy: S5.2 Range: [-128, 127] */
    AX_U8  nShpOsGain; /* sharpen over shot gain. Accuracy: U1.3 Range: [0, 8] */
} AX_ISP_IQ_SHP_GAIN_T;

typedef struct {
    AX_S16 nShpHpfLut[AX_ISP_SHP_HPF_LUT_SIZE_H][AX_ISP_SHP_HPF_LUT_SIZE_V]; /* high pass filter lut. Accuracy: S1.8 Range: [-512, 511] */
} AX_ISP_IQ_SHP_HPF_LUT;

typedef struct {
    AX_U8  nShpCoring;           /* sharp coring. Accuracy: U4.4 Range: [0, 255] */
    AX_ISP_IQ_SHP_ED_T tShpEd;
    AX_ISP_IQ_SHP_GAIN_T tGain;
    AX_ISP_IQ_SHP_HPF_LUT tHpfLut;
} AX_ISP_IQ_SHARPEN_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum; /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];    /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nShpCoring[AX_ISP_AUTO_TABLE_MAX_NUM]; /* sharp coring. Accuracy: U4.4 Range: [0, 255] */
    AX_ISP_IQ_SHP_ED_T    tShpEd[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_SHP_GAIN_T  tGain[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_SHP_HPF_LUT tHpfLut[AX_ISP_AUTO_TABLE_MAX_NUM];
} AX_ISP_IQ_SHARPEN_AUTO_T;

typedef struct {
    AX_U8  nShpEn;    /* sharpen on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nAutoMode; /* for lux auto or manual adjust mode. Accuracy: U1 Range: [0, 1] */
    AX_U8  nRefMode;  /* choose ref mode. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_SHARPEN_MANUAL_T  tManualParam;
    AX_ISP_IQ_SHARPEN_AUTO_T    tAutoParam;
} AX_ISP_IQ_SHARPEN_PARAM_T;

/************************************************************************************
 *  YNR IQ Param: Luma Noise Reduction : YNR + DBPC
 ************************************************************************************/
#define AX_ISP_YNR_INV_LUT_SIZE         (4)
#define AX_ISP_DBPC_ED_LUT_SIZE         (4)
#define AX_ISP_YNR_SF_LUT_SIZE          (4)

typedef struct {
    AX_U8  nYnrSfLut[AX_ISP_YNR_SF_LUT_SIZE];         /* ynr sf lut. Accuracy: U0.8 Range: [0, 255] */
} AX_ISP_IQ_YNR_SF_LUT_T;

typedef struct {
    AX_U16 nYnrInvNrLut[AX_ISP_YNR_INV_LUT_SIZE];     /* ynr 1/noise lut. Accuracy: U1.10 Range: [0, 2047] */
} AX_ISP_IQ_YNR_INV_NOISE_LUT_T;

typedef struct {
    AX_U16 nDbpcEdOffsetLut[AX_ISP_DBPC_ED_LUT_SIZE]; /* dbpc edge level offset lut. Accuracy: U8.2 Range: [0, 1023] */
} AX_ISP_IQ_DBPC_ED_LUT_T;

typedef struct {
    AX_ISP_IQ_YNR_SF_LUT_T        tSfLut;
    AX_ISP_IQ_YNR_INV_NOISE_LUT_T tNrLut;
    AX_U8  nDbpcEdSlope; /* dbpc edge level coefficient. Accuracy: U4.4 Range: [0, 255] */
    AX_ISP_IQ_DBPC_ED_LUT_T       tDbpcEdLut;
} AX_ISP_IQ_LUMA_NR_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum; /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM]; /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_ISP_IQ_YNR_SF_LUT_T        tSfLut[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_YNR_INV_NOISE_LUT_T tNrLut[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_U8  nDbpcEdSlope[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_DBPC_ED_LUT_T       tDbpcEdLut[AX_ISP_AUTO_TABLE_MAX_NUM];
} AX_ISP_IQ_LUMA_NR_AUTO_T;

typedef struct {
    AX_U8  nYnrEn;    /* ynr on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nDbpcEn;   /* dbpc on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nAutoMode; /* for lux auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8  nRefMode;  /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_LUMA_NR_MANUAL_T tManualParam;
    AX_ISP_IQ_LUMA_NR_AUTO_T   tAutoParam;
} AX_ISP_IQ_LUMA_NR_PARAM_T;

/************************************************************************************
 *  CNR IQ Param: Chroma Noise Reduction : CNR + CCMP
 ************************************************************************************/
#define AX_ISP_CNR_INV_LUT_SIZE         (4)
#define AX_ISP_CCMP_Y_LUT_SIZE          (8)
#define AX_ISP_CCMP_SAT_LUT_SIZE        (8)

typedef struct {
    AX_U16 nCnrInvNrLut[AX_ISP_CNR_INV_LUT_SIZE]; /* cnr 1/noise lut. Accuracy: U1.10 Range: [0, 2047] */
} AX_ISP_IQ_CNR_INV_NOISE_LUT_T;

typedef struct {
    AX_U16 nCcmpYLut[AX_ISP_CCMP_Y_LUT_SIZE];     /* ccmp y lut. Accuracy: U1.9 Range: [0, 1023] */
} AX_ISP_IQ_CCMP_Y_LUT_T;

typedef struct {
    AX_U16 nCcmpSatLut[AX_ISP_CCMP_SAT_LUT_SIZE]; /* ccmp sat lut. Accuracy: U1.9 Range: [0, 1023] */
} AX_ISP_IQ_CCMP_SAT_LUT_T;

typedef struct {
    AX_U8  nCnrLevel; /* cnr level. Accuracy: U1.4 Range: [0, 16] */
    AX_ISP_IQ_CNR_INV_NOISE_LUT_T tCnrLut;
    AX_ISP_IQ_CCMP_Y_LUT_T        tYLut;
    AX_ISP_IQ_CCMP_SAT_LUT_T      tSatLut;
} AX_ISP_IQ_CHROMA_NR_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum; /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM]; /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8  nCnrLevel[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_CNR_INV_NOISE_LUT_T tCnrLut[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_CCMP_Y_LUT_T        tYLut[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_CCMP_SAT_LUT_T      tSatLut[AX_ISP_AUTO_TABLE_MAX_NUM];
} AX_ISP_IQ_CHROMA_NR_AUTO_T;

typedef struct {
    AX_U8  nCnrEn;     /* cnr on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nCcmpEn;    /* ccmp on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nAutoMode;  /* for lux auto or manual adjust mode, [0,1], 0: manual, 1:auto, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8  nRefMode;   /* choose ref mode, [0,1], 0:use lux as ref, 1:use gain as ref. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_CHROMA_NR_MANUAL_T tManualParam;
    AX_ISP_IQ_CHROMA_NR_AUTO_T   tAutoParam;
} AX_ISP_IQ_CHROMA_NR_PARAM_T;

/************************************************************************************
 *  CSET IQ Param: CHROMA CONTROL
 ************************************************************************************/
#define AX_ISP_CSET_COLOR_SIZE          (2)
#define AX_ISP_CSET_MASK_CENTER_UV_SIZE (2)
#define AX_ISP_CSET_MASK_SIZE           (3)

typedef struct {
    AX_S16 nCsetColor[AX_ISP_CSET_COLOR_SIZE]; /* target color. Accuracy: S7.2 Range: [-512, 511] */
} AX_ISP_IQ_CSET_COLOR_T;

typedef struct {
    AX_U16 nCsetCenterY; /* color mask center Y. Accuracy: U8.2 Range: [0, 1023] */
    AX_S16 nCsetCenterUv[AX_ISP_CSET_MASK_CENTER_UV_SIZE]; /* cset color mask center [0]:U, [1]:V. Accuracy: S7.2 Range: [-512, 511] */
} AX_ISP_IQ_CSET_CENTER_T;

typedef struct {
    AX_U16 nSetRadius[AX_ISP_CSET_MASK_SIZE]; /* cset color mask radius [0]:Y, [1]:U, [2]:V. Accuracy: U7.2 Range: [0, 511] */
} AX_ISP_IQ_CSET_RADIUS_T;

typedef struct {
    AX_U8  nCsetGrad[AX_ISP_CSET_MASK_SIZE]; /* cset color mask transition gradient. Accuracy: U4 Range: [0, 15] */
} AX_ISP_IQ_CSET_GRAD_T;

typedef struct {
    AX_ISP_IQ_CSET_COLOR_T  tColor;
    AX_ISP_IQ_CSET_CENTER_T tCenter;
    AX_ISP_IQ_CSET_RADIUS_T tRadius;
    AX_ISP_IQ_CSET_GRAD_T   tGrad;
} AX_ISP_IQ_CSET_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum; /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM]; /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_ISP_IQ_CSET_COLOR_T  tColor[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_CSET_CENTER_T tCenter[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_CSET_RADIUS_T tRadius[AX_ISP_AUTO_TABLE_MAX_NUM];
    AX_ISP_IQ_CSET_GRAD_T   tGrad[AX_ISP_AUTO_TABLE_MAX_NUM];
} AX_ISP_IQ_CSET_AUTO_T;

typedef struct {
    AX_U8  nCsetEn;     /* cset on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nAutoMode;   /* for lux auto or manual adjust mode. Accuracy: U1 Range: [0, 1] */
    AX_U8  nRefMode;    /* choose ref mode. Accuracy: U1 Range: [0, 1] */
    AX_U8  nCsetIoFlag; /* cset color mask in/out flag. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_CSET_MANUAL_T tManualParam;
    AX_ISP_IQ_CSET_AUTO_T   tAutoParam;
} AX_ISP_IQ_CSET_PARAM_T;

/************************************************************************************
 *  YCPROC IQ Param: COLOR PROCESS
 ************************************************************************************/
typedef struct {
    AX_U16 nBrightness; /* adjust brightness. Accuracy: U6.10 Range: [0, 65535] */
    AX_U16 nContrast;   /* adjust contrast. Accuracy: U6.10 Range: [0, 65535] */
    AX_U16 nSaturation; /* adjust saturation. Accuracy: U4.12 Range: [0, 65535] */
    AX_U16 nHue;        /* adjust hue. Accuracy: U1.15 Range: [0, 65535] */
} AX_ISP_IQ_YCPROC_MANUAL_T;

typedef struct {
    AX_U8  nParamGrpNum; /* Accuracy: U8.0 Range: [0, AX_ISP_AUTO_TABLE_MAX_NUM] */
    AX_U32 nRefVal[AX_ISP_AUTO_TABLE_MAX_NUM];     /* Gain: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Lux: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U16 nBrightness[AX_ISP_AUTO_TABLE_MAX_NUM]; /* adjust brightness. Accuracy: U6.10 Range: [0, 65535] */
    AX_U16 nContrast[AX_ISP_AUTO_TABLE_MAX_NUM];   /* adjust contrast. Accuracy: U6.10 Range: [0, 65535] */
    AX_U16 nSaturation[AX_ISP_AUTO_TABLE_MAX_NUM]; /* adjust saturation. Accuracy: U4.12 Range: [0, 65535] */
    AX_U16 nHue[AX_ISP_AUTO_TABLE_MAX_NUM];        /* adjust hue. Accuracy: U1.15 Range: [0, 65535] */
} AX_ISP_IQ_YCPROC_AUTO_T;

typedef struct {
    AX_U8  nCprocEn;   /* ycproc on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nAutoMode;  /* for lux auto or manual adjust mode. Accuracy: U1 Range: [0, 1] */
    AX_U8  nRefMode;   /* choose ref mode. Accuracy: U1 Range: [0, 1] */
    AX_ISP_IQ_YCPROC_MANUAL_T tManualParam;
    AX_ISP_IQ_YCPROC_AUTO_T   tAutoParam;
} AX_ISP_IQ_YCPROC_PARAM_T;

/************************************************************************************
 *  YCRT IQ Param
 ************************************************************************************/
#define AX_ISP_YCRT_SIZE                (2)

typedef struct {
    AX_U8  nYcrtInputRange[AX_ISP_YCRT_SIZE];  /* ycrt matrix. Accuracy: U8.0 Range: [0, 255] */
    AX_U8  nYcrtOutputRange[AX_ISP_YCRT_SIZE]; /* ycrt matrix. Accuracy: U8.0 Range: [0, 255] */
} AX_Y_YCRT_RANGE_T;

typedef struct {
    AX_U8  nYcrtInputRange[AX_ISP_YCRT_SIZE];  /* ycrt matrix. Accuracy: U8.0 Range: [0, 255] */
    AX_U8  nYcrtOutputRange[AX_ISP_YCRT_SIZE]; /* ycrt matrix. Accuracy: U8.0 Range: [0, 255] */
} AX_CBCR_YCRT_RANGE_T;

typedef struct {
    AX_U8  nYcrtEn;     /* ycrt on-off. Accuracy: U1 Range: [0, 1] */
    AX_U8  nClipEn;     /* clip. Accuracy: U1 Range: [0, 1] */
    AX_U8  nCompressEn; /* compress. Accuracy: U1 Range: [0, 1] */
    AX_U16 nYrt[AX_ISP_YCRT_SIZE]; /* Video Default: [4, 1016]; Jpeg Default: [0, 1020]; Accuracy: U8.2 Range: [0, 1020] */
    AX_S16 nCrt[AX_ISP_YCRT_SIZE]; /* Video Default: [-508, 504]; Jpeg Default: [-512, 508]; Accuracy: S7.2 Range: [-512, 508] */
    AX_Y_YCRT_RANGE_T    tYRange;
    AX_CBCR_YCRT_RANGE_T tCbCrRange;
} AX_ISP_IQ_YCRT_PARAM_T;

/************************************************************************************
 *  3A Stat Config
 ************************************************************************************/
typedef struct {
    AX_U16 nRoiOffsetH;       /* horiOffset, must be even, Accuracy: U13.0, Range: [0, 5132] */
    AX_U16 nRoiOffsetV;       /* vertOffset, must be even, Accuracy: U13.0, Range: [0, 3984]*/
    AX_U16 nRoiRegionNumH;    /* must be even, Accuracy: U6.0, Range: [2, 64] */
    AX_U16 nRoiRegionNumV;    /* Accuracy: U6.0, Range: [1, 48] */
    AX_U16 nRoiRegionW;       /* regionW, must be even, Accuracy: U10.0, Range: [16, 5132], nRoiOffsetH + (nRoiRegionNumH * nRoiRegionW) <= hsize */
    AX_U16 nRoiRegionH;       /* regionH, must be even, Accuracy: U10.0, Range: [16, 3984], nRoiOffsetV + (nRoiRegionNumV * nRoiRegionH) <= vsize */
} AX_ISP_IQ_AE_STAT_ROI_T;

typedef struct {
    AX_U16 nRoiOffsetH;       /* horiOffset, must be even, Accuracy: U13.0, Range: [0, 5132] */
    AX_U16 nRoiOffsetV;       /* vertOffset, must be even, Accuracy: U13.0, Range: [0, 3984]*/
    AX_U16 nRoiRegionNumH;    /* Accuracy: U7.0, Range: [1, 72] */
    AX_U16 nRoiRegionNumV;    /* Accuracy: U6.0, Range: [1, 54] */
    AX_U16 nRoiRegionW;       /* regionW, must be even, Accuracy: U10.0, Range: [16, 5132], nRoiOffsetH + (nRoiRegionNumH * nRoiRegionW) <= hsize */
    AX_U16 nRoiRegionH;       /* regionH, must be even, Accuracy: U10.0, Range: [16, 3984], nRoiOffsetV + (nRoiRegionNumV * nRoiRegionH) <= vsize */
} AX_ISP_IQ_AWB_STAT_ROI_T;

typedef struct {
    AX_U16 nRThr;           /* AE RThr. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nBThr;           /* AE BThr. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nGrThr;          /* AE GrThr. Accuracy: U8.2 Range: [0, 1023] */
    AX_U16 nGbThr;          /* AE GbThr. Accuracy: U8.2 Range: [0, 1023] */
} AX_ISP_IQ_AE_STAT_THR_T;

typedef struct {
    AX_U16 nRThr;           /* AWB RThr. Accuracy: U8.6 Range: [0, 16383] */
    AX_U16 nBThr;           /* AWB BThr. Accuracy: U8.6 Range: [0, 16383] */
    AX_U16 nGrThr;          /* AWB GrThr. Accuracy: U8.6 Range: [0, 16383] */
    AX_U16 nGbThr;          /* AWB GbThr. Accuracy: U8.6 Range: [0, 16383] */
} AX_ISP_IQ_AWB_STAT_THR_T;

/************************************************************************************
 *  AE Stat Config
 ************************************************************************************/
#define DEF_AE_HIST_STAT_ROI_MAX_NUM    (2)

typedef struct {
    AX_U16 nRoiOffsetH;       /* horiOffset, must be even, Accuracy: U13.0, Range: [0, 5132] */
    AX_U16 nRoiOffsetV;       /* vertOffset, must be even, Accuracy: U12.0, Range: [0, 3984] */
    AX_U16 nRoiWidth;         /* RoiWidth, Accuracy: U12.0, Range: [0, 5132], nRoiOffsetH + nRoiWidth <= hsize */
    AX_U16 nRoiHeight;        /* RoiHeight, Accuracy: U12.0, Range: [0, 3984], nRoiOffsetV + nRoiHeight <= vsize  */
} AX_ISP_IQ_AE_HIST_ROI_T;

typedef struct {
    AX_U8  nEnable;           /* AE Enable. Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_AE_STAT_ROI_T tGridRoi;
    AX_ISP_IQ_AE_STAT_THR_T tSatThr;
    AX_ISP_IQ_AE_HIST_ROI_T tHistRoi[DEF_AE_HIST_STAT_ROI_MAX_NUM];
} AX_ISP_IQ_AE_STAT_PARAM_T;

/************************************************************************************
 *  AE Stat Info
 ************************************************************************************/
/* AE Grid & Hist */
#define AX_AE_GRID_ROW              (48)
#define AX_AE_GRID_COL              (64)
#define AX_AE_GRID_CHN              (4)
#define AX_AE_HIST_LOG_BIN          (16)
#define AX_AE_HIST_LINEAR_BIN       (256)
#define AX_AE_HIST_CHN              (4)

typedef struct {
    AX_U32 nBin[AX_AE_HIST_CHN];
} AX_AE_HIST_BIN_T;

typedef struct {
    AX_U8  nValid;
    AX_AE_HIST_BIN_T nLinearHist[ AX_AE_HIST_LINEAR_BIN];
    AX_AE_HIST_BIN_T nLogHist[AX_AE_HIST_LOG_BIN];
} AX_AE_HIST_STAT_T;

typedef struct {
    AX_U32 nGridSum[AX_AE_GRID_CHN];
    AX_U16 nGridNum[AX_AE_GRID_CHN];
} AX_AE_GRID_STATS;

typedef struct {
    AX_U8  nValid;
    AX_U8  nZoneRowSize;
    AX_U8  nZoneColSize;
    AX_U8  nChnNum;
    AX_AE_GRID_STATS  tGridStats[AX_AE_GRID_ROW * AX_AE_GRID_COL];
} AX_AE_GRID_STAT_T;

typedef struct {
    AX_U32 nSeqNum;                 /* frame seq num */
    AX_U64 nTimestamp;              /* frame timestamp */
    AX_U32 nSkipNum;                /* Algorithm running interval */
    AX_AE_GRID_STAT_T tAeGridStat[AX_HDR_CHN_NUM];
    AX_AE_HIST_STAT_T tAeHistStat[AX_HDR_CHN_NUM];
} AX_ISP_AE_STAT_INFO_T;

/************************************************************************************
 *  WB Gain Info
 ************************************************************************************/
typedef struct {
    AX_U16 nRGain;      /* WBC RGain. Accuracy: U4.8 Range: [0, 0xFFF] */
    AX_U16 nGrGain;     /* WBC RGain. Accuracy: U4.8 Range: [0, 0xFFF] */
    AX_U16 nGbGain;     /* WBC RGain. Accuracy: U4.8 Range: [0, 0xFFF] */
    AX_U16 nBGain;      /* WBC RGain. Accuracy: U4.8 Range: [0, 0xFFF] */
} AX_ISP_IQ_WB_GAIN_PARAM_T;

/************************************************************************************
 *  AWB Stat Config
 ************************************************************************************/
typedef struct {
    AX_ISP_IQ_AWB_STAT_THR_T tLowThr;
    AX_ISP_IQ_AWB_STAT_THR_T tHighThr;
} AX_ISP_IQ_RGB_THR_T;

typedef struct {
    AX_U8  nEnable;      /* AWB Enable. Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_AWB_STAT_ROI_T tGridRoi;
    AX_ISP_IQ_AWB_STAT_THR_T tSatThr;
    AX_ISP_IQ_RGB_THR_T tRgbThr;
} AX_ISP_IQ_AWB_STAT_PARAM_T;

/************************************************************************************
 *  AWB Stat Info
 ************************************************************************************/
#define AX_AWB_GRID_ROW       (54)
#define AX_AWB_GRID_COL       (72)
#define AX_AWB_GRID_CHN       (4)

typedef struct {
    AX_U32 nUnSatGridSum[AX_AWB_GRID_CHN];
    AX_U16 nUnSatGridNum[AX_AWB_GRID_CHN];
    AX_U32 nSatGridSum[AX_AWB_GRID_CHN];
    AX_U16 nSatGridNum[AX_AWB_GRID_CHN];
} AX_AWB_GRID_STATS;

typedef struct {
    AX_U8  nValid;
    AX_U8  nZoneRowSize;
    AX_U8  nZoneColSize;
    AX_AWB_GRID_STATS tAwbGridStats[AX_AWB_GRID_ROW * AX_AWB_GRID_COL];
} AX_AWB_GRID_STATS_INFO_T;

typedef struct {
    AX_U32 nSeqNum;             /* frame seq num */
    AX_U64 nTimestamp;          /* frame timestamp */
    AX_U32 nSkipNum;            /* Algorithm running interval */
    AX_AWB_GRID_STATS_INFO_T tAwbGridStats[AX_HDR_CHN_NUM];
} AX_ISP_AWB_STAT_INFO_T;

/************************************************************************************
 *  AF Stat Config
 ************************************************************************************/
#define AX_ISP_AF_GAMMA_LUT_NUM             (33)
#define AX_ISP_AF_WEIGHT_LUT_NUM            (16)
#define AX_ISP_AF_CORING_LUT_NUM            (16)
#define AX_ISP_AF_IIR_COEF_NUM              (10)
#define AX_ISP_AF_FIR_COEF_NUM              (13)
#define AX_ISP_AF_IIR_REF_LIST_SIZE         (32)

typedef struct {
    AX_U8  nYSel;    /* Accuracy: U1.0, Range: [0, 1], 0:Disable Y, Use Green Channle.  1:Use RGB to Y */
    AX_U8  nGrgbSel; /* Accuracy: U1.0, Range: [0, 1], 0: Use Gr,  1:Use Gb */
    AX_U16 nCoeffR;  /* Accuracy: U0.12, Range: [0, 4095] */
    AX_U16 nCoeffG;  /* Accuracy: U0.12, Range: [0, 4095] */
    AX_U16 nCoeffB;  /* Accuracy: U0.12, Range: [0, 4095], nCoeffR + nCoeffG + nCoeffB = 4095*/
} AX_ISP_IQ_AF_BAYER2Y_PARAM_T;

typedef struct {
    AX_U8  nGammaEnable;    /* Accuracy: U1.0, Range: [0, 1], 0:Disable Gamma,   1:Enable.  */
    AX_U16 nGammaLut[AX_ISP_AF_GAMMA_LUT_NUM];  /* Accuracy: U8.6, Range: [0, 16383] */
} AX_ISP_IQ_AF_GAMMA_PARAM_T;

typedef struct {
    AX_U8  nScaleEnable;   /* Accuracy: U1.0, Range: [0, 1], 0:Disable Downsample,   1:Enable Downsample.  */
    AX_U8  nScaleFactor;   /* Accuracy: U3.0, Range: [1, 3], Downsample Ratio.   */
    AX_U16 nWeightLut[AX_ISP_AF_WEIGHT_LUT_NUM];   /* Accuracy: U0.12, Range: [0, 4095] */
} AX_ISP_IQ_AF_DOWNSCALE_PARAM_T;

typedef struct {
    AX_U32 nCoringThr;      /* Accuracy: U8.10, Range:[0, 2^18-1], suggest 18 numbers: {2^0, 2^1, ..., 2^17} */
    AX_U16 nCoringGain;     /* Accuracy: U5.7, Range:[0, 4095] */
    AX_U8  nCoringLut[AX_ISP_AF_CORING_LUT_NUM];   /* Accuracy: U5.0, Range[0, 31], nCoringLut[i] <= nCoringLut[i+1] */
} AX_ISP_IQ_AF_CORING_PARAM_T;

typedef struct {
    AX_U16 nRoiOffsetH;       /* Accuracy: U13.0, Range: [32, 5132], horiOffset, must be even */
    AX_U16 nRoiOffsetV;       /* Accuracy: U13.0, Range: [16, 3984], vertOffset, must be even */
    AX_U16 nRoiRegionNumH;    /* Accuracy: U6.0, Range: [1, 21], horiRegionNum, (nRoiRegionNumH * nRoiRegionW) % 4 == 0 */
    AX_U16 nRoiRegionNumV;    /* Accuracy: U6.0, Range: [1, 64], vertRegionNum, nRoiRegionNumH * nRoiRegionNumV <= 180 */
    AX_U16 nRoiRegionW;       /* Accuracy: U10.0, Range: [1, 5131], nRoiOffsetH + nRoiRegionNumH * nRoiRegionW <= hsize*/
    AX_U16 nRoiRegionH;       /* Accuracy: U10.0, Range: [1, 3983], nRoiOffsetV + nRoiRegionNumV * nRoiRegionH <= vsize*/
} AX_ISP_IQ_AF_ROI_PARAM_T;

typedef struct {
    AX_U8  nFirEnable;                       /* Accuracy: U1.0 Range: [0, 1] 0:Disable FIR,  1:Enable FIR */
    AX_U32 nViirRefId;                       /* Accuracy: U6.0, Range:[0, 31] */
    AX_U32 nH1IirRefId;                      /* Accuracy: U6.0, Range:[0, 31] */
    AX_U32 nH2IirRefId;                      /* Accuracy: U6.0, Range:[0, 31]*/
} AX_ISP_IQ_AF_FLT_PARAM_T;

typedef struct {
    AX_U8  nAfEnable;    /* AF Enable. Accuracy: U1.0 Range: [0, 1] */
    AX_ISP_IQ_AF_BAYER2Y_PARAM_T   tAfBayer2Y;
    AX_ISP_IQ_AF_GAMMA_PARAM_T     tAfGamma;
    AX_ISP_IQ_AF_DOWNSCALE_PARAM_T tAfScaler;
    AX_ISP_IQ_AF_FLT_PARAM_T       tAfFilter;
    AX_ISP_IQ_AF_CORING_PARAM_T    tAfCoring;
    AX_ISP_IQ_AF_ROI_PARAM_T       tAfRoi;
} AX_ISP_IQ_AF_STAT_PARAM_T;

/* Bandpass Filter for Reference, with the Coefficients and Bandpass Info. */
typedef struct {
    AX_U32 nStartFreq;   /* Accuracy:U1.20 Range:[1, 0x200000] */
    AX_U32 nEndFreq;     /* Accuracy:U1.20 Range:[1, 0x200000] */
    AX_S32 nIirCoefList[AX_ISP_AF_IIR_COEF_NUM]; /* Accuracy: S2.14, Range:[-65535, 65535]. */
} AX_ISP_IQ_AF_IIR_REF_T;

/* Frequently Used Bandpass Filter List for Reference.  */
typedef struct {
    AX_U32 nViirRefNum;  /* Accuracy: U6.0, Range:[1, 32] */
    AX_U32 nH1IirRefNum; /* Accuracy: U6.0, Range:[1, 32] */
    AX_U32 nH2IirRefNum; /* Accuracy: U6.0, Range:[1, 32] */
    AX_ISP_IQ_AF_IIR_REF_T tVIirRefList[AX_ISP_AF_IIR_REF_LIST_SIZE];
    AX_ISP_IQ_AF_IIR_REF_T tH1IirRefList[AX_ISP_AF_IIR_REF_LIST_SIZE];
    AX_ISP_IQ_AF_IIR_REF_T tH2IirRefList[AX_ISP_AF_IIR_REF_LIST_SIZE];
} AX_ISP_IQ_AF_IIR_REF_LIST_T;

/************************************************************************************
 *  AF Stat Info
 ************************************************************************************/
#define AX_AF_ROI_OUTPUT_LINES               (180)
#define AX_AF_ROI_OUTPUT_LINE_BYTES          (16)

typedef struct {
    AX_U32 nPixCount;
    AX_U64 nPixSum;
    AX_U64 nSharpness;
} AX_ISP_AF_GRID_STATS;

typedef struct {
    AX_U8  nValid;
    AX_ISP_AF_GRID_STATS tAfRoiV[AX_AF_ROI_OUTPUT_LINES];
    AX_ISP_AF_GRID_STATS tAfRoiH1[AX_AF_ROI_OUTPUT_LINES];
    AX_ISP_AF_GRID_STATS tAfRoiH2[AX_AF_ROI_OUTPUT_LINES];
} AX_ISP_AF_STATS;

typedef struct {
    AX_U32 nSeqNum;
    AX_ISP_AF_STATS tAfStats[AX_HDR_CHN_NUM];
} AX_ISP_AF_STAT_INFO_T;

/************************************************************************************
 *  NPU IQ Param
 ************************************************************************************/
#define AX_ISP_MAX_PATH_SIZE                (256)
#define AX_ISP_NPU_ISO_MODEL_MAX_NUM        (16)
#define AX_ISP_NPU_REF_VALUE_MAX_NUM        (16)
#define AX_ISP_NPU_MODEL_MAX_NUM            (AX_ISP_NPU_ISO_MODEL_MAX_NUM * 2)

typedef struct {
    AX_CHAR                      szWbtModelName[AX_ISP_MAX_PATH_SIZE];  /* wbt model path, absolute path */
    AX_U8                        nHcgMode;         /* model param, based on the real param of model. Accuracy: U2 Range: [0, 3] 0:LCG,1:HCG,2:LCG NOT SURPPORT*/
    AX_U32                       nIsoThresholdMin; /* Accuracy: U16 Range: [1, 65535] <= */
    AX_U32                       nIsoThresholdMax; /* Accuracy: U16 Range: [1, 65535] > */
    AX_CHAR                      szModelName[AX_ISP_MAX_PATH_SIZE];  /* model path, relative path */
    AX_U8                        n2DLevel; /* selected 2d noise level */
    AX_U8                        n3DLevel; /* selected 3d noise level */
    AX_U8                        nRefGrpNum; /* Ref Grp Num; Accuacy: U8 Range: [0, AX_ISP_NPU_REF_VALUE_MAX_NUM] */
    AX_U32                       nRefValue[AX_ISP_NPU_REF_VALUE_MAX_NUM]; /* Ref Gain End: Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]; Ref Lux End: Accuracy: U22.10 Range: [0, 0xFFFFFFFF] */
    AX_U8                        nSpatialMR[AX_ISP_NPU_REF_VALUE_MAX_NUM]; /* Spatial MR NR Strength, default:128. Accuracy: U1.7 Range: [0 ~ 192] */
    AX_U8                        nSpatialBR[AX_ISP_NPU_REF_VALUE_MAX_NUM]; /* Spatial BR NR Strength, default:128. Accuracy: U1.7 Range: [0 ~ 192] */
    AX_U8                        nTemporalMR[AX_ISP_NPU_REF_VALUE_MAX_NUM]; /* Temporal MR NR Strength, default:128. Accuracy: U1.7 Range: [0 ~ 256) */
    AX_U8                        nTemporalBR[AX_ISP_NPU_REF_VALUE_MAX_NUM]; /* Temporal BR NR Strength, default:64. Accuracy: U1.7 Range: [0 ~ 256) */
    AX_S16                       nBiasIn;  /* for NR model bias param get/set, default:0. Accuracy: S7.8 Range: [-black_level ~ +black_level] */
    AX_S16                       nBiasOut; /* for NR model bias param get/set, default:0. Accuracy: S7.8 Range: [-black_level ~ +black_level] */
} AX_ISP_NPU_MODEL_PARAM_T;

typedef struct {
    AX_U8  nAutoModelNum;        /* total number of models. Accuracy: U8.0 Range: [0, AX_ISP_NPU_MODEL_MAX_NUM] */
    AX_ISP_NPU_MODEL_PARAM_T     tAutoModelTable[AX_ISP_NPU_MODEL_MAX_NUM];
} AX_ISP_NPU_AUTO_PARAM_T;

typedef struct {
    AX_CHAR                      szWbtModelName[AX_ISP_MAX_PATH_SIZE];  /* wbt model path, absolute path */
    AX_CHAR                      szModelName[AX_ISP_MAX_PATH_SIZE];     /* model path, relative path */
    AX_U8                        n2DLevel; /* selected 2d noise level */
    AX_U8                        n3DLevel; /* selected 3d noise level */
    AX_U8                        nSpatialMR; /* Spatial MR NR Strength, default:128. Accuracy: U1.7 Range: [0 ~ 192] */
    AX_U8                        nSpatialBR; /* Spatial BR NR Strength, default:128. Accuracy: U1.7 Range: [0 ~ 192] */
    AX_U8                        nTemporalMR; /* Temporal MR NR Strength, default:128. Accuracy: U1.7 Range: [0 ~ 256) */
    AX_U8                        nTemporalBR; /* Temporal BR NR Strength, default:64. Accuracy: U1.7 Range: [0 ~ 256) */
    AX_S16                       nBiasIn;  /* for NR model bias param get/set, default:0. Accuracy: S7.8 Range: [-black_level ~ +black_level] */
    AX_S16                       nBiasOut; /* for NR model bias param get/set, default:0. Accuracy: S7.8 Range: [-black_level ~ +black_level] */
} AX_ISP_NPU_MANUAL_PARAM_T;

typedef struct {
    AX_U8  nDummyModelNum;       /* total number of models. Accuracy: U8.0 Range: [0, AX_ISP_NPU_MODEL_MAX_NUM] */
    AX_ISP_NPU_MODEL_PARAM_T     tDummyModelTable[AX_ISP_NPU_MODEL_MAX_NUM];
} AX_ISP_NPU_DUMMY_PARAM_T;

typedef struct {
    AX_U8    bNrEnable;     /* for NR mode, 0: dummy mode, 1:nr mode, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8    bAutoMode;     /* for NR auto or manual adjust mode, 0: manual, 1:auto, default:1. Accuracy: U1 Range: [0, 1] */
    AX_U8    bUpdateTable;  /* for NR model table switch enable mode, 0: disable, 1:enable, default:0. Accuracy: U1 Range: [0, 1] */
    AX_U8    nHdrMode;      /* for NR model hdr mode get/set, 1: sdr, 2:hdr_2x, 3:hdr_3x, default:1. Accuracy: U2 Range: [1, 2, 3] */
    AX_U8    bRefMode;      /* choose ref mode, Range: [0, 1], 0:use lux as ref, 1:use gain as ref */
    AX_ISP_NPU_DUMMY_PARAM_T     tDummyParam;   /* for NR dummy mode, just be setting when bUpdateTable = 1 */
    AX_ISP_NPU_MANUAL_PARAM_T    tManualParam;  /* manual model, must be one of the tModelTable model */
    AX_ISP_NPU_AUTO_PARAM_T      tAutoParam;    /* auto mode param, just be setting when bUpdateTable = 1 */
} AX_ISP_IQ_NPU_PARAM_T;

typedef struct {
    AX_CHAR                     szWbtModelName[AX_ISP_MAX_PATH_SIZE];  /* wbt model path, absolute path */
    AX_CHAR                     szModelName[AX_ISP_MAX_PATH_SIZE];     /* model path, absolute path */
} AX_ISP_NPU_EIS_PARAM_T;

typedef struct {
    AX_U8                       bEisEnable;     /* for EIS mode enable, 0: disable, 1:enable, default:0. Accuracy: U1 Range: [0, 1] */
    AX_U8                       nEisDelayNum;   /* for EIS delay num, defalut 2. Accuracy: U8 Range: [0, 4] */
    AX_U8                       nCropRatioH;    /* for EIS crop ratio, defalut 0. Accuracy: U0.8 Range: [0, 128) */
    AX_U8                       nCropRatioW;    /* for EIS crop ratio, defalut 0. Accuracy: U0.8 Range: [0, 128) */
    AX_ISP_NPU_EIS_PARAM_T      tEisNpuParam;   /* EIS NPU model param */
} AX_ISP_IQ_EIS_PARAM_T;

/* Only for RGBIR(Dual Vision) mode */
#define AX_ISP_NPU_WARP_MAX_NUM           (3)
typedef struct {
    AX_U8                       bWarpEnable;
    AX_S32                      nWarpParam[AX_ISP_NPU_WARP_MAX_NUM][AX_ISP_NPU_WARP_MAX_NUM];
} AX_ISP_IQ_NPU_RGBIR_PARAM_T;
/* Only for RGBIR(Dual Vision) mode */


#define AX_ISP_NPU_CAP_MAX_NUM            (16 * 2)

typedef struct {
    AX_BOOL                     bSupportCondLevel;
    AX_BOOL                     bSupportDyStrength;
    AX_BOOL                     bSupportBias;
    AX_U8                       nCondMinLevel2D; /* 可控降噪模型 2D 力度最小档位 */
    AX_U8                       nCondMaxLevel2D; /* 可控降噪模型 2D 力度最大档位 */
    AX_U8                       nCondMinLevel3D; /* 可控降噪模型 3D 力度最小档位 */
    AX_U8                       nCondMaxLevel3D; /* 可控降噪模型 3D 力度最大档位 */
    AX_S16                      nBiasMinValue;
    AX_S16                      nBiasMaxValue;
    AX_CHAR                     szModelName[AX_ISP_MAX_PATH_SIZE];
} AX_ISP_NPU_CAP_T;

typedef struct {
    AX_U8               nValidNum;
    AX_ISP_NPU_CAP_T    tNpuCapList[AX_ISP_NPU_CAP_MAX_NUM];
} AX_ISP_IQ_NPU_CAP_TABLE_T;

/************************************************************************************
 *  ISP IQ API
 ************************************************************************************/
AX_S32 AX_ISP_IQ_SetBlcParam(AX_U8 pipe, AX_ISP_IQ_BLC_PARAM_T *pIspBlcParam);
AX_S32 AX_ISP_IQ_GetBlcParam(AX_U8 pipe, AX_ISP_IQ_BLC_PARAM_T *pIspBlcParam);

AX_S32 AX_ISP_IQ_SetFpnParam(AX_U8 pipe, AX_ISP_IQ_FPN_PARAM_T  *pIspFpnParam);
AX_S32 AX_ISP_IQ_GetFpnParam(AX_U8 pipe, AX_ISP_IQ_FPN_PARAM_T  *pIspFpnParam);

AX_S32 AX_ISP_IQ_SetGblParam(AX_U8 pipe, AX_ISP_IQ_GBL_PARAM_T  *pIspGblParam);
AX_S32 AX_ISP_IQ_GetGblParam(AX_U8 pipe, AX_ISP_IQ_GBL_PARAM_T  *pIspGblParam);

AX_S32 AX_ISP_IQ_SetDarkShadingParam(AX_U8 pipe, AX_ISP_IQ_DS_PARAM_T  *pIspDsParam);
AX_S32 AX_ISP_IQ_GetDarkShadingParam(AX_U8 pipe, AX_ISP_IQ_DS_PARAM_T  *pIspDsParam);

AX_S32 AX_ISP_IQ_SetDpcParam(AX_U8 pipe, AX_ISP_IQ_DPC_PARAM_T *pIspDpcParam);
AX_S32 AX_ISP_IQ_GetDpcParam(AX_U8 pipe, AX_ISP_IQ_DPC_PARAM_T *pIspDpcParam);

AX_S32 AX_ISP_IQ_SetWnrParam(AX_U8 pipe, AX_ISP_IQ_WNR_PARAM_T *pIspWnrParam);
AX_S32 AX_ISP_IQ_GetWnrParam(AX_U8 pipe, AX_ISP_IQ_WNR_PARAM_T *pIspWnrParam);

AX_S32 AX_ISP_IQ_SetCacParam(AX_U8 pipe, AX_ISP_IQ_CAC_PARAM_T  *pIspCacParam);
AX_S32 AX_ISP_IQ_GetCacParam(AX_U8 pipe, AX_ISP_IQ_CAC_PARAM_T  *pIspCacParam);

AX_S32 AX_ISP_IQ_SetLscParam(AX_U8 pipe, AX_ISP_IQ_LSC_PARAM_T *pIspLscParam);
AX_S32 AX_ISP_IQ_GetLscParam(AX_U8 pipe, AX_ISP_IQ_LSC_PARAM_T *pIspLscParam);

AX_S32 AX_ISP_IQ_SetWbGainParam(AX_U8 pipe, AX_ISP_IQ_WB_GAIN_PARAM_T *pIspWbGainParam);
AX_S32 AX_ISP_IQ_GetWbGainParam(AX_U8 pipe, AX_ISP_IQ_WB_GAIN_PARAM_T *pIspWbGainParam);

AX_S32 AX_ISP_IQ_SetRltmParam(AX_U8 pipe, AX_ISP_IQ_RLTM_PARAM_T *pIspRltmParam);
AX_S32 AX_ISP_IQ_GetRltmParam(AX_U8 pipe, AX_ISP_IQ_RLTM_PARAM_T *pIspRltmParam);

AX_S32 AX_ISP_IQ_SetDehazeParam(AX_U8 pipe, AX_ISP_IQ_DEHAZE_PARAM_T *pIspDehazeParam);
AX_S32 AX_ISP_IQ_GetDehazeParam(AX_U8 pipe, AX_ISP_IQ_DEHAZE_PARAM_T *pIspDehazeParam);

AX_S32 AX_ISP_IQ_SetDemosaicParam(AX_U8 pipe, AX_ISP_IQ_DEMOSAIC_PARAM_T *pIspDemosaicParam);
AX_S32 AX_ISP_IQ_GetDemosaicParam(AX_U8 pipe, AX_ISP_IQ_DEMOSAIC_PARAM_T *pIspDemosaicParam);

AX_S32 AX_ISP_IQ_SetPfrParam(AX_U8 pipe, AX_ISP_IQ_PFR_PARAM_T *pIspPfrParam);
AX_S32 AX_ISP_IQ_GetPfrParam(AX_U8 pipe, AX_ISP_IQ_PFR_PARAM_T *pIspPfrParam);
AX_S32 AX_ISP_IQ_SetClcParam(AX_U8 pipe, AX_ISP_IQ_CLC_PARAM_T *pIspClcParam);
AX_S32 AX_ISP_IQ_GetClcParam(AX_U8 pipe, AX_ISP_IQ_CLC_PARAM_T *pIspClcParam);

AX_S32 AX_ISP_IQ_SetGammaParam(AX_U8 pipe, AX_ISP_IQ_GAMMA_PARAM_T *pIspGammaParam);
AX_S32 AX_ISP_IQ_GetGammaParam(AX_U8 pipe, AX_ISP_IQ_GAMMA_PARAM_T *pIspGammaParam);

AX_S32 AX_ISP_IQ_SetCsc0Param(AX_U8 pipe, AX_ISP_IQ_YUV_CSC0_PARAM_T *pIspCsc0Param);
AX_S32 AX_ISP_IQ_GetCsc0Param(AX_U8 pipe, AX_ISP_IQ_YUV_CSC0_PARAM_T *pIspCsc0Param);

AX_S32 AX_ISP_IQ_SetCsc1Param(AX_U8 pipe, AX_ISP_IQ_YUV_CSC1_PARAM_T *pIspCsc1Param);
AX_S32 AX_ISP_IQ_GetCsc1Param(AX_U8 pipe, AX_ISP_IQ_YUV_CSC1_PARAM_T *pIspCsc1Param);

AX_S32 AX_ISP_IQ_SetShpParam(AX_U8 pipe, AX_ISP_IQ_SHARPEN_PARAM_T *pIspShpParam);
AX_S32 AX_ISP_IQ_GetShpParam(AX_U8 pipe, AX_ISP_IQ_SHARPEN_PARAM_T *pIspShpParam);

AX_S32 AX_ISP_IQ_SetLumaNrParam(AX_U8 pipe, AX_ISP_IQ_LUMA_NR_PARAM_T *pIspLumaNrParam);
AX_S32 AX_ISP_IQ_GetLumaNrParam(AX_U8 pipe, AX_ISP_IQ_LUMA_NR_PARAM_T *pIspLumaNrParam);

AX_S32 AX_ISP_IQ_SetChromaNrParam(AX_U8 pipe, AX_ISP_IQ_CHROMA_NR_PARAM_T *pIspChromaNrParam);
AX_S32 AX_ISP_IQ_GetChromaNrParam(AX_U8 pipe, AX_ISP_IQ_CHROMA_NR_PARAM_T *pIspChromaNrParam);

AX_S32 AX_ISP_IQ_SetCsetParam(AX_U8 pipe, AX_ISP_IQ_CSET_PARAM_T *pIspCsetParam);
AX_S32 AX_ISP_IQ_GetCsetParam(AX_U8 pipe, AX_ISP_IQ_CSET_PARAM_T *pIspCsetParam);

AX_S32 AX_ISP_IQ_SetYcprocParam(AX_U8 pipe, AX_ISP_IQ_YCPROC_PARAM_T *pIspYcprocParam);
AX_S32 AX_ISP_IQ_GetYcprocParam(AX_U8 pipe, AX_ISP_IQ_YCPROC_PARAM_T *pIspYcprocParam);

AX_S32 AX_ISP_IQ_SetYcrtParam(AX_U8 pipe, AX_ISP_IQ_YCRT_PARAM_T *pIspYcrtParam);
AX_S32 AX_ISP_IQ_GetYcrtParam(AX_U8 pipe, AX_ISP_IQ_YCRT_PARAM_T *pIspYcrtParam);

AX_S32 AX_ISP_IQ_SetAeStatParam(AX_U8 pipe, AX_ISP_IQ_AE_STAT_PARAM_T *pAeStatParam);
AX_S32 AX_ISP_IQ_GetAeStatParam(AX_U8 pipe, AX_ISP_IQ_AE_STAT_PARAM_T *pAeStatParam);

AX_S32 AX_ISP_IQ_SetAwbStatParam(AX_U8 pipe, AX_ISP_IQ_AWB_STAT_PARAM_T *pAwbStatParam);
AX_S32 AX_ISP_IQ_GetAwbStatParam(AX_U8 pipe, AX_ISP_IQ_AWB_STAT_PARAM_T *pAwbStatParam);

AX_S32 AX_ISP_IQ_SetAfStatParam(AX_U8 pipe, AX_ISP_IQ_AF_STAT_PARAM_T *pAfStatParam);
AX_S32 AX_ISP_IQ_GetAfStatParam(AX_U8 pipe, AX_ISP_IQ_AF_STAT_PARAM_T *pAfStatParam);
AX_S32 AX_ISP_IQ_SetAFIirRefList(AX_U8 pipe, AX_ISP_IQ_AF_IIR_REF_LIST_T *pIirRefList);
AX_S32 AX_ISP_IQ_GetAFIirRefList(AX_U8 pipe, AX_ISP_IQ_AF_IIR_REF_LIST_T *pIirRefList);

AX_S32 AX_ISP_IQ_GetAEStatistics(AX_U8 pipe, AX_ISP_AE_STAT_INFO_T *pAeStat);
AX_S32 AX_ISP_IQ_GetAWB0Statistics(AX_U8 pipe, AX_ISP_AWB_STAT_INFO_T *pAwbStat);
AX_S32 AX_ISP_IQ_GetAWB1Statistics(AX_U8 pipe, AX_ISP_AWB_STAT_INFO_T *pAwbStat);
AX_S32 AX_ISP_IQ_GetAFStatistics(AX_U8 pipe, AX_ISP_AF_STAT_INFO_T *pAfStat);

AX_S32 AX_ISP_IQ_SetNpuParam(AX_U8 pipe, AX_ISP_IQ_NPU_PARAM_T *pIspNpuParam);
AX_S32 AX_ISP_IQ_GetNpuParam(AX_U8 pipe, AX_ISP_IQ_NPU_PARAM_T *pIspNpuParam);

AX_S32 AX_ISP_IQ_SetEisParam(AX_U8 pipe, AX_ISP_IQ_EIS_PARAM_T *pIspEisParam);
AX_S32 AX_ISP_IQ_GetEisParam(AX_U8 pipe, AX_ISP_IQ_EIS_PARAM_T *pIspEisParam);

AX_S32 AX_ISP_IQ_SetRGBIRParam(AX_U8 pipe, AX_ISP_IQ_NPU_RGBIR_PARAM_T *pIspRgbIRParam);
AX_S32 AX_ISP_IQ_GetRGBIRParam(AX_U8 pipe, AX_ISP_IQ_NPU_RGBIR_PARAM_T *pIspRgbIRParam);

AX_S32 AX_ISP_IQ_GetNpuCapability(AX_U8 pipe, AX_ISP_IQ_NPU_CAP_TABLE_T *pIspNpuCapTbl);

#ifdef __cplusplus
}
#endif

#endif  //_AX_ISP_IQ_API_H_
