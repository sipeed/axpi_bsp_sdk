/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_ISP_3A_STRUCT_H_
#define _AX_ISP_3A_STRUCT_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif


////////////////////////////////////////////////////////////////////////////////////
//  AWB ALG Param
////////////////////////////////////////////////////////////////////////////////////
#define AX_ISP_AWB_GRID_NUM_MAX             (4096)
#define AX_ISP_AWB_ILLUM_NAME_LEN_MAX       (32)
#define AX_ISP_AWB_POLY_NUM_MAX             (32)
#define AX_ISP_AWB_POLY_PNT_NUM_MAX         (64)
#define AX_ISP_AWB_ILLUM_NUM_MAX            (16)
#define AX_ISP_AWB_EXT_ILLUM_NUM_MAX        (32)
#define AX_ISP_AWB_ALL_ILLUM_NUM_MAX        (AX_ISP_AWB_ILLUM_NUM_MAX + AX_ISP_AWB_EXT_ILLUM_NUM_MAX)
#define AX_ISP_AWB_CTRL_PNT_PART_NUM_MAX    (32)
#define AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX     (AX_ISP_AWB_CTRL_PNT_PART_NUM_MAX * AX_ISP_AWB_ILLUM_NUM_MAX)
#define AX_ISP_AWB_DOMINANT_ZONE_NUM        (4)
#define AX_ISP_AWB_PLANCKIAN_ZONE_NUM       (24)
#define AX_ISP_AWB_SPATIAL_SEG_MAX_NUM      (32)
#define AX_ISP_AWB_PREFER_CCT_MAX_NUM       (32)
#define AX_ISP_AWB_LUX_TYPE_NUM             (8)
#define AX_ISP_AWB_ZONE_MAX_NUM             (40)
#define AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM      (32)
#define AX_ISP_AWB_MIXLIGHT_CCT_MAX_NUM     (32)
#define AX_ISP_AWB_GRID_WEIGHT_ROW_MAX      (27)
#define AX_ISP_AWB_GRID_WEIGHT_COLUMN_MAX   (36)

typedef struct {
    AX_U32 nRg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nBg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
} AX_ISP_IQ_AWB_PNT_T;

typedef struct {
    AX_S32 nK;      /* Accuracy:S21.10 Range:[-2147483647, 2147483647 (2*1024*1024*1024)] */
    AX_S32 nB;      /* Accuracy:S21.10 Range:[-2147483647, 2147483647 (2*1024*1024*1024)] */
} AX_ISP_IQ_AWB_LINE_KB_T;

typedef struct {
    /* Illum Calib Info */
    AX_CHAR  szName[AX_ISP_AWB_ILLUM_NAME_LEN_MAX];
    AX_U32   nCct;      /* Accuracy:U14 Range:[0, 16000] */
    AX_U32   nRadius;   /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_ISP_IQ_AWB_PNT_T tCoord;
} AX_ISP_IQ_AWB_ILLUM_T;


typedef struct {
    AX_U32 nMinX;   /*  Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nMaxX;   /*  Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nMinY;   /*  Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nMaxY;   /*  Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nPntCnt; /*  Accuracy:U8 Range:[0, AX_ISP_AWB_POLY_PNT_NUM_MAX-1] */
    AX_ISP_IQ_AWB_PNT_T tPntArray[AX_ISP_AWB_POLY_PNT_NUM_MAX];
} AX_ISP_IQ_AWB_POLY_T;

typedef struct {
    AX_U8  nDominantEnable;         /* Accuracy:U1 Range:[0, 1] */
    AX_U32 nDomMinCctThresh;        /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nDomMaxCctThresh;        /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nDom2AllRatioThresh;     /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2MinorRatioThresh;   /* Accuracy:U10.10 Range:[0, 1,048,575(1024*1024-1)] */
    AX_U32 nMinorWeight;            /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nSmoothPercent;          /* Accuracy:U7.10 Range:[0, 131071 (128*1024)] */
} AX_ISP_3A_AWB_DOMINANT_T;

typedef struct {
    /* Gray Zone Common Info */
    AX_ISP_IQ_AWB_PNT_T tCenterPnt;
    AX_U32 nCenterPntRadius;    /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */

    AX_U32 nLowCut;     /* Accuracy:U8 Range:[0, 64] */
    AX_U32 nHighCut;    /* Accuracy:U8 Range:[0, 64] */
    AX_U32 nCctMax;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctMin;     /* Accuracy:U14 Range:[0, 16000] */

    AX_U32 nPartCtrlPntNum;     /* Accuracy:U8 Range:[0, 15] */
    AX_U32 nCtrlPntNum;         /* Accuracy:U8 Range:[0, 128] */
    AX_U32 nCtrlSegKbNum;       /* Accuracy:U8 Range:[0, 127] Should Always be nCtrlPntNum-1 */

    AX_U32                  nCctList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];      /* Accuracy:U14 Range:[0, 16000] */
    AX_ISP_IQ_AWB_LINE_KB_T tChordKB;
    AX_ISP_IQ_AWB_PNT_T     tChordPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_PNT_T     tArcPointList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_LINE_KB_T tRadiusLineList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];

    /* Gray Zone Borders */
    AX_ISP_IQ_AWB_PNT_T     tInLeftBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_PNT_T     tInRightBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];

    AX_ISP_IQ_AWB_PNT_T     tOutLeftBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];
    AX_ISP_IQ_AWB_PNT_T     tOutRightBorderPntList[AX_ISP_AWB_CTRL_PNT_ALL_NUM_MAX];

    /* Illum Info */
    AX_U32 nIllumNum;       /* Accuracy:U8 Range:[0, 64] */
    AX_ISP_IQ_AWB_ILLUM_T tIllumList[AX_ISP_AWB_ILLUM_NUM_MAX];
    AX_U32 nExtIllumNum;    /* Accuracy:U8 Range:[0, 16] */
    AX_ISP_IQ_AWB_ILLUM_T tExtIllumList[AX_ISP_AWB_EXT_ILLUM_NUM_MAX];

    /* Poly Info */
    AX_U32 nPolyNum;        /* Accuracy:U8 Range:[0, 64] */
    AX_ISP_IQ_AWB_POLY_T  tPolyList[AX_ISP_AWB_POLY_NUM_MAX];


    /*   Tuning Params  */

    /* Common Settings */
    AX_U8  nMode;            /* Accuracy:U6 Range:[0, 2] INVALID=0, MANUAL=1, AUTO=2 */
    AX_U32 nIndex;           /* Accuracy:U8 Range:[0, 64] */
    AX_U32 nDampRatio;       /* Accuracy:U1.20 Range:[0, 1048576 (1024*1024)] */
    AX_U32 nToleranceRg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nToleranceBg;     /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */

    /* Lux Type Threshold */
    AX_U32 nLuxVeryDarkStart;   /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxVeryDarkEnd;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxDarkStart;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxDarkEnd;         /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxIndoorStart;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxIndoorEnd;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransInStart;    /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransInEnd;      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransOutStart;   /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTransOutEnd;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxOutdoorStart;    /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxOutdoorEnd;      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxBrightStart;     /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxBrightEnd;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxVeryBrightStart; /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */

    /* Gray Zone CCT Split Threshold */
    AX_U32 nCctMinInner;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctMaxInner;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctMinOuter;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctMaxOuter;     /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctSplitHtoA;    /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctSplitAtoF;    /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctSplitFtoD5;   /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctSplitD5toD6;  /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nCctSplitD6toS;   /* Accuracy:U14 Range:[0, 16000] */

    /* Grid Weight Params */
    AX_U8  nGridWeightEnable;         /* Accuracy: U8    Range: [0x0, 0x1] */
    AX_U8  nGridWeightRow;            /* Accuracy: U8    Range: [0x1, 0x1B] */
    AX_U8  nGridWeightColumn;         /* Accuracy: U8    Range: [0x1, 0x24] */
    AX_U16 nGridWeightTable[AX_ISP_AWB_GRID_WEIGHT_ROW_MAX][AX_ISP_AWB_GRID_WEIGHT_COLUMN_MAX]; /* Accuracy: U1.10 Range: [0x0, 0x400] */

    /* Lux Weights of Gray Zones and Extra Illuminations */
    AX_U32 nGrayZoneLuxWeight[AX_ISP_AWB_PLANCKIAN_ZONE_NUM][AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nExtIlllumLuxWeight[AX_ISP_AWB_EXT_ILLUM_NUM_MAX][AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U10 Range:[0, 1000] */

    /* Luma Weight*/
    AX_U8  nLumaWeightNum;       /* Accuracy:U6 Range:[0, 32]*/
    AX_U32 nLumaSplitList[AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM];   /* Accuracy:U8.10 Range:[0, 262143 (256 *1024)] */
    AX_U32 nLumaWeightList[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM];   /* Accuracy:U1.10 Range:[0, 1024] */

    /* Mix Light*/
    AX_U8  bMixLightEn;         /* Accuracy:U1 Range:[0, 1] */ // 1: Enable Mix Light Weight & CCM Saturation Discount,  0: Disable
    AX_U32 nMixLightProba_0_CctStd[AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U14 Range:[0, 9999] */   // Proba=0,   if CCT Std below this Thresh
    AX_U32 nMixLightProba_100_CctStd[AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U14 Range:[0, 9999] */ // Proba=100, if CCT Std above this Thresh
    AX_U32 nMixLightProba_100_SatDiscnt[AX_ISP_AWB_LUX_TYPE_NUM];  /* Accuracy:U8 Range:[0, 100] */ // CCM Saturation Discount When Proba=100
    AX_U32 nMixLightKneeNum;                                    /* Accuracy:U6 Range:[0, 32]*/   // Weight LUT: Size
    AX_U32 nMixLightKneeCctList[AX_ISP_AWB_MIXLIGHT_CCT_MAX_NUM];  /* Accuracy:U14 Range:[0, 16000] */ // Weight LUT: Key
    AX_U32 nMixLightKneeWtList[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_MIXLIGHT_CCT_MAX_NUM]; /* Accuracy:U1.10 Range:[0, 1024] */ // Weight LUT: Value

    /* Dominant Params */
    AX_ISP_3A_AWB_DOMINANT_T tDomParamList[AX_ISP_AWB_DOMINANT_ZONE_NUM];

    AX_U32 nTmpoStabTriggerAvgBlkWt; /* Accuracy:U10 Range:[0, 1000] */

    /* Planckian Locus Project in High Lux Scene */
    AX_U8  nPlanckianLocusProjEn;           /* Accuracy:U6 Range:[0, 1] Enable or Disable*/
    AX_U32 nPlanckianLocusNotProjLux;       /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nPlanckianLocusFullProjLux;      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */

    /* Spatial Predictor Params */
    AX_U32 nSpatialSegmetNum;                                   /* Accuracy:U8  Range:[0, 64] */
    AX_U32 nSpatialStartLux[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];    /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nSpatialEndLux[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];      /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nSpatialRg[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];          /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */
    AX_U32 nSpatialBg[AX_ISP_AWB_SPATIAL_SEG_MAX_NUM];          /* Accuracy:U4.20 Range:[0, 16777215 (16*1024*1024)] */

    /* Fusion Params */
    AX_U32 nFusionGrayZoneConfid_0_AvgBlkWeight;        /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nFusionGrayZoneConfid_100_AvgBlkWeight;      /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nFusionSpatialConfid_0_Lux;                  /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nFusionSpatialConfid_100_Lux;                /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nFusionWeightGrayZone;                       /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nFusionWeightSpatial;                        /* Accuracy:U1.10 Range:[0, 1024] */

    /* Preference Params */
    AX_U32 nPreferCctNum;        /* Accuracy:U8 Range:[0, 64] */
    AX_U32 nPreferSrcCctList[AX_ISP_AWB_PREFER_CCT_MAX_NUM];                             /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nPreferDstCct[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_PREFER_CCT_MAX_NUM];        /* Accuracy:U14 Range:[0, 16000] */
    AX_S32 nPreferGrShift[AX_ISP_AWB_LUX_TYPE_NUM][AX_ISP_AWB_PREFER_CCT_MAX_NUM];       /* Accuracy:S4.20 Range:[-16777215, 16777215 (16*1024*1024)] */

    /* Multi Camera Sync Params */
    AX_U32 nMultiCamSyncMode;   /* Accuracy:U2 Range:[0, 3] */
} AX_ISP_IQ_AWB_ALG_CONFIG_T;


typedef struct {
    AX_U32 nblkRowNum;      /* Accuracy:U7 Range:[0, 54] */
    AX_U32 nblkColNum;      /* Accuracy:U7 Range:[0, 72] */
    AX_ISP_IQ_AWB_PNT_T tStats[AX_ISP_AWB_GRID_NUM_MAX];

    AX_U32 nCct;            /* Accuracy:U14 Range:[0, 16000] */
    AX_U32 nLux;            /* Accuracy:U22.10 Range:[0, 4294967295 (4*1024*1024*1024)] */
    AX_U32 nLuxTypeInd;     /* Accuracy:U8 Range:[0, 64] */
    AX_U32 nCctStd;         /* Accuracy:U14 Range:[0, 9999] */
    AX_U32 nMixLightProba; /* Accuracy:U10 Range:[0, 1000] Current Mix Light Probability  */
    AX_U32 nSatDiscount;   /*  Accuracy:U8 Range:[0, 100] Current Saturation Discount  */

    /* Stat Info of each Planckian Zone and Extra Illumination.
     *   Element[0~23]: Stat Info of each Planckian Zone.
     *   Element[23~ ]: Stat Info of each Extra Illumination.
     */
    AX_U32 nGrayZoneCnt[AX_ISP_AWB_ZONE_MAX_NUM];               /* Accuracy:U12 Range:[0, 4096] */
    AX_U32 nGrayZoneLuxWeight[AX_ISP_AWB_ZONE_MAX_NUM];         /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nGrayZoneLuxWeightSum[AX_ISP_AWB_ZONE_MAX_NUM];      /* Accuracy:U32 Range:[0, 4294967295] */
    AX_U32 nGrayZoneFinalWeightSum[AX_ISP_AWB_ZONE_MAX_NUM];    /* Accuracy:U32 Range:[0, 4294967295] */
    AX_U32 nGrayZoneLumaSum[AX_ISP_AWB_ZONE_MAX_NUM];           /* Accuracy:U32 Range:[0, 4294967295] */

    /* Fusion Status */
    AX_U32 nGrayZoneBlkWeightAvg;   /* Accuracy:U10 Range:[0, 1000] */
    AX_U32 nGrayZoneConfid;         /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nSpatialConfid;          /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nGrayZoneFusionRatio;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nSpatialFusionRatio;     /* Accuracy:U1.10 Range:[0, 1024] */
    AX_ISP_IQ_AWB_PNT_T tGrayZonePoint;
    AX_ISP_IQ_AWB_PNT_T tSpatialPoint;
    AX_ISP_IQ_AWB_PNT_T tFusionPoint;

    /* Dominant Status */
    AX_U32 nDomCntH;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U32 nDomCntA;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U32 nDomCntF;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U32 nDomCntD;            /* Accuracy:U13 Range:[0, 4096] */
    AX_U32 nDom2MinorRatioH;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2MinorRatioA;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2MinorRatioF;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2MinorRatioD;    /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2AllRatioH;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2AllRatioA;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2AllRatioF;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDom2AllRatioD;      /* Accuracy:U1.10 Range:[0, 1024] */
    AX_U32 nDomActiveH;         /* Accuracy:U1 Range:[0, 1] */
    AX_U32 nDomActiveA;         /* Accuracy:U1 Range:[0, 1] */
    AX_U32 nDomActiveF;         /* Accuracy:U1 Range:[0, 1] */
    AX_U32 nDomActiveD;         /* Accuracy:U1 Range:[0, 1] */

    /* Luma Counter */
    AX_U32 nLumaWeight[AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM+1];  /* Accuracy:U1.10 Range:[0, 1024] Current Luma Weight*/
    AX_U32 nLumaCount[AX_ISP_AWB_LUMA_WEIGHT_MAX_NUM+1];   /* Accuracy:U12 Range:[0, 4000] Current Luma Count*/
} AX_ISP_IQ_AWB_ALG_STATUS_T;


typedef struct {
    AX_U32 nGainR;    /* Accuracy:U4.10 Range:[1024, 16383] */
    AX_U32 nGainGr;   /* Accuracy:U4.10 Range:[1024, 16383] */
    AX_U32 nGainGb;   /* Accuracy:U4.10 Range:[1024, 16383] */
    AX_U32 nGainB;    /* Accuracy:U4.10 Range:[1024, 16383] */
} AX_ISP_IQ_AWB_GAIN_T;


typedef struct {
    AX_U8 nEnable;
    AX_ISP_IQ_AWB_GAIN_T       tManualParam;
    AX_ISP_IQ_AWB_ALG_CONFIG_T tAutoParam;
} AX_ISP_IQ_AWB_PARAM_T;


typedef struct {
    AX_ISP_IQ_AWB_GAIN_T       tGainStatus;
    AX_ISP_IQ_AWB_ALG_STATUS_T tAlgoStatus;
} AX_ISP_IQ_AWB_STATUS_T;

////////////////////////////////////////////////////////////////////////////////////
//  AE ALG Param
////////////////////////////////////////////////////////////////////////////////////
#define AX_ISP_AE_LUX_MAX_PAIR_NUM          (10)
#define AX_TFLICKER_100HZ                   (1000000.0/100.0 )                             /* predefined flicker period value for ECM module, uint:us */
#define AX_TFLICKER_120HZ                   (1000000.0/120.0)                              /* predefined flicker period value for ECM module, uint:us */
#define AX_ISP_AE_GRID_WEIGHT_ROW_MAX       (48)
#define AX_ISP_AE_GRID_WEIGHT_COLUMN_MAX    (64)

typedef struct{
    AX_U8  nEnable;                /* 0: Antiflicker off; 1: antiflicker on */
    AX_U8  nFlickerPeriod;         /* 0: 100HZ, 1: 120HZ */
    AX_U32 nAntiFlickerTolerance;  /* Uints: us. Accuracy:U32 Range:[0x0, 0x208d] */
    AX_U8  nOverExpMode;           /* 0: ANTI PRIOR,1: LUMA PRIOR */
    AX_U8  nUnderExpMode;          /* 0: ANTI PRIOR,1: LUMA PRIOR */
} AX_ISP_IQ_AE_ANTIFLICKER_PARAMS_T;

typedef struct{
    AX_U32 nIntergrationTime;    /* Uints: us. Accuracy:U32
                                  * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                                  * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */
    AX_U32 nGain;                /* Accuracy: U22.10  Range: [nTotalGainMin, nTotalGainMax] */
    AX_U8 nIncrementPriority;    /* 0: Exp Time 1: Gain */
} AX_ISP_IQ_AE_ROUTE_CURVE_NODE_T;

#define AE_ISP_ROUTE_MAX_NODES 16
#define AX_AE_TABLE_NAME_LENGTH_MAX 32
typedef struct{
    AX_CHAR sTableName[AX_AE_TABLE_NAME_LENGTH_MAX];
    AX_U8 nRouteCurveNum;    /* Accuracy: U8 Range: [0x1, 0x10] */
    AX_ISP_IQ_AE_ROUTE_CURVE_NODE_T  tRouteCurveList[AE_ISP_ROUTE_MAX_NODES];
} AX_ISP_IQ_AE_ROUTE_TABLE_T;

#define AX_AE_TABLE_NUM_MAX 8
typedef struct{
    AX_U8 nTableNum;     /* Accuracy: U8 Range: [0x1, 0x8] */
    AX_U8 nUsedTableId;  /* Accuracy: U8 Range: [0x0, 0x7] */
    AX_ISP_IQ_AE_ROUTE_TABLE_T  tRouteTable[AX_AE_TABLE_NUM_MAX];
} AX_ISP_IQ_AE_ROUTE_PARAM_T;

#define AE_ISP_SETPOINT_MAX_NUM 10
typedef struct{
    AX_U8      nSize;                                 /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U32     nRefList[AE_ISP_SETPOINT_MAX_NUM];     /* <gain value/lux value> if nSetPointMode = 1, use gain range; if nSetPointMode = 2,use lux range;
                                                       * Accuracy: U22.10
                                                       * gain range：[nTotalGainMin, nTotalGainMax]
                                                       * lux range：[0x0, 0x3D090000]*/
    AX_U32     nSetPointList[AE_ISP_SETPOINT_MAX_NUM]; /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
} AX_ISP_IQ_AE_SETPOINT_CURVE_T;

typedef struct {
    AX_U32 nMinRatio; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nMaxRatio; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nShortNonSatAreaPercent; /* Accuracy: U32  Range: [0x0, 0x2710] */
    AX_U32 nShortSatLuma;           /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nTolerance;              /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U8  nConvergeCntFrameNum;    /* Accuracy: U8 Range: [0x0, 0xA] */
    AX_U32 nDampRatio;              /* Accuracy: U0.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_HDR_RATIO_STRATEGY_PARAM_T;

typedef struct {
    AX_U8  nHdrMode;       /* 1: Dynamic mode 0: fixed mode*/
    AX_ISP_IQ_AE_HDR_RATIO_STRATEGY_PARAM_T tRatioStrategyParam;
    AX_U32 nFixedHdrRatio; /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
} AX_ISP_IQ_AE_HDR_RATIO_T;

typedef struct
{
    AX_U32 nBigStepFactor;              /* Accuracy: U4.20  Range: [0x0, 0xA00000]  */
    AX_U32 nSmallStepFactor;            /* Accuracy: U4.20  Range: [0x0, 0xA00000]  */
    AX_U32 nLumaDiffOverThresh;         /* Accuracy: U8.10  Range: [0x0, 0x3FC00]   */
    AX_U32 nLumaDiffUnderThresh;        /* Accuracy: U8.10  Range: [0x0, 0x3FC00]   */
    AX_U32 nLumaSpeedThresh;            /* Accuracy: U8.10  Range: [0x0, 0x3FC00]   */
    AX_U32 nSpeedDownFactor;            /* Accuracy: U4.20  Range: [0x0, 0xA00000]  */
    AX_U32 nMinUserPwmDuty;             /* Accuracy: U7.10  Range: [0x0, 0x19000]   */
    AX_U32 nMaxUserPwmDuty;             /* Accuracy: U7.10  Range: [0x0, 0x19000]   */
    AX_U32 nOpenPwmDuty;                /* Accuracy: U7.10  Range: [0x0, 0x19000]   */
    AX_U32 nConvergeLumaDiffTolerance;  /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U32 nConvergeFrameCntThresh;     /* Accuracy: U32 Range: [0x0, 0x64] */
} AX_ISP_IQ_AE_DCIRIS_PARAMS_T;

#define AX_ISP_AE_SPARSE_SLOW_SHUTTER_MAX_NUM 5
typedef struct
{
    AX_U32 nNodeNum;  /* Accuracy: U32  Range: [0x0, 0x5]*/
    AX_U32 nFpsList[AX_ISP_AE_SPARSE_SLOW_SHUTTER_MAX_NUM];  /* Accuracy: U8.10  Range: [nSnsSlowShutterModeFpsMin, nSnsSlowShutterModeFpsMax] */
} AX_ISP_IQ_AE_SPARSE_MODE_PARAM_T;

typedef struct
{
    AX_U8 nFrameRateMode;             /* 0: FIX FRAME RATE MODE; 1: SLOW SHUTTER MODE */
    AX_U8 nFpsIncreaseDelayFrame;     /* Accuracy: U8 Range: [0x0, 0xA] */
} AX_ISP_IQ_AE_SLOW_SHUTTER_PARAM_T;

typedef struct
{
    AX_U32  nIrisType;           /* Accuracy: U32 0: FIXED TYPE; 1： DC-IRIS  */
    AX_ISP_IQ_AE_DCIRIS_PARAMS_T  tDcIrisParam;
} AX_ISP_IQ_AE_IRIS_PARAMS_T;

typedef struct
{
    AX_U32 nToFastLumaThOver;   /* Accuracy: U4.10 Range: [0x0, 0x2800]*/
    AX_U32 nToFastLumaThUnder;  /* Accuracy: U4.10 Range: [0x0, 0x2800]*/
    AX_U32 nToSlowLumaThOver;   /* Accuracy: U4.10 Range: [0x0, 0x2800]*/
    AX_U32 nToSlowLumaThUnder;  /* Accuracy: U4.10 Range: [0x0, 0x2800]*/
    AX_U32 nToSlowFrameTh;      /* Accuracy: U32 Range: [0x0, 0xA]*/
    AX_U32 nToConvergedFrameTh; /* Accuracy: U32 Range: [0x0, 0xA]*/
}AX_ISP_IQ_AE_STATE_MACHINE_T;

#define AX_ISP_AE_SPEED_KNEE_MAX_NUM (16)
typedef struct
{
    AX_U32 nFastOverKneeCnt;                                           /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nFastOverLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];        /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U32 nFastOverStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];      /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nFastOverSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM]; /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nFastOverSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];            /* Accuracy: U32 Range: [0x0, 0xA] */

    AX_U32 nFastUnderKneeCnt;                                           /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nFastUnderLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];            /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U32 nFastUnderStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];      /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nFastUnderSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM]; /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nFastUnderSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];            /* Accuracy: U32 Range: [0x0, 0xA] */

    AX_U32 nSlowOverKneeCnt;                                             /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nSlowOverLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];              /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U32 nSlowOverStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];        /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nSlowOverSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];   /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nSlowOverSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];              /* Accuracy: U32 Range: [0x0, 0xA] */

    AX_U32 nSlowUnderKneeCnt;                                            /* Accuracy: U32 Range: [0x1, 0x10] */
    AX_U32 nSlowUnderLumaDiffList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];             /* Accuracy: U9.10 Range: [0x0, 0x4 0000] */
    AX_U32 nSlowUnderStepFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];       /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nSlowUnderSpeedDownFactorList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];  /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nSlowUnderSkipList[AX_ISP_AE_SPEED_KNEE_MAX_NUM];             /* Accuracy: U32 Range: [0x0, 0xA] */
}AX_ISP_IQ_AE_CONVERGE_SPEED_T;


typedef struct
{
    AX_ISP_IQ_AE_STATE_MACHINE_T tStateMachineParam;
    AX_ISP_IQ_AE_CONVERGE_SPEED_T tConvergeSpeedParam;
}AX_ISP_IQ_AE_TIME_SMOOTH_PARAM_T;

#define AX_ISP_AE_LUMA_WEIGHT_MAX_NUM 64
typedef struct
{
    AX_U8 nEnable;          /* 0: disable luma weight 1: enable luma weight */
    AX_U32 nLumaWeightNum;  /* Accuracy: U32  Range: [0x0, 0x40]*/
    AX_U32 nLumaSplitList[AX_ISP_AE_LUMA_WEIGHT_MAX_NUM];  /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nWeightList[AX_ISP_AE_LUMA_WEIGHT_MAX_NUM]; /* Accuracy: U1.10 Range: [0x0, 0x400] */
} AX_ISP_IQ_AE_LUMA_WEIGHT_PARAM_T;

typedef struct {
    AX_U32 nSetPoint;            /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */
    AX_U32 nTolerance;           /* Accuracy: U7.20  Range: [0x0, 0x6400000] */
    AX_U32 nAgainLcg2HcgTh;      /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nAgainHcg2LcgTh;      /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nAgainLcg2HcgRatio;   /* Accuracy: U10.10 Range: [0x400, 0x2800] */
    AX_U32 nAgainHcg2LcgRatio;   /* Accuracy: U10.10 Range: [0x400, 0x2800] */
    AX_U32 nLuxk;                /* Accuracy: U24    Range: [0x0, 0x989680] */

    AX_U32 nMaxIspGain;     /* Accuracy:U22.10 Range:[tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U32 nMinIspGain;     /* Accuracy:U22.10 Range:[tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U32 nMaxUserDgain;   /* Accuracy:U22.10 Range:[tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U32 nMinUserDgain;   /* Accuracy:U22.10 Range:[tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U32 nMaxUserAgain;   /* Accuracy:U22.10 Range:[tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nMinUserAgain;   /* Accuracy:U22.10 Range:[tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nMaxShutter;     /* Uints: us. Accuracy:U32
                             * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                             * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */
    AX_U32 nMinShutter;     /* Uints: us. Accuracy:U32 Range:[tSnsShutterLimit.nMin, tSnsShutterLimit.nMax] */

    AX_U8  nPositionWeightMode;  /* Accuracy: U8    Range: [0x0, 0x2] 0:Close  1:GridWeightMode  2:RoiWeightMode   */
    AX_U32 nRoiStartX;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nRoiStartY;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nRoiWidth;            /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nRoiHeight;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nWeightRoi;           /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U32 nWeightBackgnd;       /* Accuracy: U1.10 Range: [0x0, 0x400] */
    AX_U8  nGridWeightRow;       /* Accuracy: U8    Range: [0x1, 0x30] */
    AX_U8  nGridWeightColumn;    /* Accuracy: U8    Range: [0x1, 0x40] */
    AX_U16 nGridWeightTable[AX_ISP_AE_GRID_WEIGHT_ROW_MAX][AX_ISP_AE_GRID_WEIGHT_COLUMN_MAX];  /* Accuracy: U1.10 Range: [0x0, 0x400] */

    AX_ISP_IQ_AE_ANTIFLICKER_PARAMS_T tAntiFlickerParam;
    AX_U32 nSetPointMode;     /* 0：fixed； 1： gain; 2: lux */
    AX_U32 nStrategyMode;     /* 0: SHUTTER_PRIOR; 1:GAIN_PRIOR; 2:AE ROUTE */
    AX_ISP_IQ_AE_ROUTE_PARAM_T    tAeRouteParam;
    AX_ISP_IQ_AE_SETPOINT_CURVE_T tAeSetPointCurve;
    AX_ISP_IQ_AE_HDR_RATIO_T      tAeHdrRatio;
    AX_U32 nMultiCamSyncMode;  /* 0：INDEPEND MODE； 1： MASTER SLAVE MODE; 2: OVERLAP MODE */
    AX_U32 nMultiCamSyncRatio; /* Accuracy: U7.20  Range: [0x0, 0x8000000] */
    AX_U8 nSnsDelayFrame;         /* Accuracy: U8 Range: [0x0, 0x5] */
    AX_ISP_IQ_AE_SLOW_SHUTTER_PARAM_T tSlowShutterParam;
    AX_ISP_IQ_AE_IRIS_PARAMS_T  tIrisParam;
    AX_ISP_IQ_AE_LUMA_WEIGHT_PARAM_T tLumaWeightParam;
    AX_ISP_IQ_AE_TIME_SMOOTH_PARAM_T tTimeSmoothParam;
} AX_ISP_IQ_AE_ALG_CONFIG_T;

typedef struct {
    AX_U32 nIspGain;       /* Accuracy: U22.10 Range: [tIspDgainLimit.nMin, tIspDgainLimit.nMax] */
    AX_U32 nAGain;         /* Accuracy: U22.10 Range: [tSnsAgainLimit.nMin, tSnsAgainLimit.nMax] */
    AX_U32 nDgain;         /* Accuracy: U22.10 Range: [tSnsDgainLimit.nMin, tSnsDgainLimit.nMax] */
    AX_U32 nSnsTotalGain;  /* Accuracy: U22.10 Range: [nSnsTotalGainMin, nSnsTotalGainMax] */
    AX_U8  nHcgLcg;        /* HCG:0, LCG:1 */
    AX_U32 nShutter;       /* Uints: us. Accuracy: U32
                            * if nFrameRateMode = 0, Range: [tSnsShutterLimit.nMin, tSnsShutterLimit.nMax]
                            * if nFrameRateMode = 1, Range: [tSnsSlowShutterModeShutterLimit.nMin, tSnsSlowShutterModeShutterLimit.nMax] */

    AX_U32 nIrisPwmDuty;   /* Accuracy: U7.10  Range: [0x0, 0x19000] */
    AX_U32 nHdrRealRatioLtoS;   /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nHdrRealRatioStoVS;  /* Accuracy: U7.10  Range: [nHdrRatioMin, nHdrRatioMax] */
    AX_U32 nSetPoint;           /* Accuracy: U8.10  Range: [0x0, 0x3FC00] */

    /* Below Unused on Tool, first defined, not nsed */
    AX_U32 nShortAgain;    /* Accuracy: U22.10 Range: [tShortAgainLimit.nMin, tShortAgainLimit.nMax] */
    AX_U32 nShortDgain;    /* Accuracy: U22.10 Range: [tShortDgainLimit.nMin, tShortDgainLimit.nMax] */
    AX_U32 nShortShutter;  /* Uints: us. Accuracy: U32 Range: [tShortShutterLimit.nMin, tShortShutterLimit.nMax] */
    AX_U32 nVsAgain;       /* Accuracy: U22.10 Range: [tVsAgainLimit.nMin, tVsAgainLimit.nMax] */
    AX_U32 nVsDgain;       /* Accuracy: U22.10 Range: [tVsDgainLimit.nMin, tVsDgainLimit.nMax] */
    AX_U32 nVsShutter;     /* Uints: us. Accuracy: U32 Range: [tVsShutterLimit.nMin, tVsShutterLimit.nMax] */
    AX_U32 nHdrRatio;      /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */

    AX_U32 nHdrMaxShutterHwLimit;  /* Accuracy: U32 Range [0x0,0xFFFF FFFF]*/
    AX_U32 nRealMaxShutter;        /* Accuracy: U32 Range [0x0,0xFFFF FFFF]*/
} AX_ISP_IQ_EXP_SETTING_T;


typedef struct {
    AX_U32 nMeanLuma;            /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
    AX_U32 nWeightedMeanLuma;    /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
    AX_U32 nLux;         /* Accuracy: U22.10 Range: [0x0, 0x3D090000] */
    AX_U64 nExpVal;      /* Accuracy: U36.10
                          * Range：min = tIspDgainLimit.nMin * tSnsAgainLimit.nMin * tSnsDgainLimit.nMin * tSnsShutterLimit.nMin
                          * Range：max = tIspDgainLimit.nMax * tSnsAgainLimit.nMax * tSnsDgainLimit.nMax * tSnsShutterLimit.nMax * nDcgRatio */
    AX_U32 nFps;         /* Accuracy: U8.10 Range: [0x400,0x19000]*/
} AX_ISP_IQ_AE_ALG_STATUS_T;


typedef struct {
    AX_U8 nEnable;
    AX_ISP_IQ_EXP_SETTING_T   tExpManual;
    AX_ISP_IQ_AE_ALG_CONFIG_T tAeAlgAuto;
} AX_ISP_IQ_AE_PARAM_T;

typedef struct {
    AX_ISP_IQ_AE_ALG_STATUS_T tAlgStatus;
    AX_ISP_IQ_EXP_SETTING_T   tExpStatus;
} AX_ISP_IQ_AE_STATUS_T;


typedef struct {
    AX_U32 nMax;
    AX_U32 nMin;
    AX_U32 nStep;
} AX_ISP_IQ_AE_UNIT_T;

typedef struct {
    AX_U8  nGainMode;   /* 0: AGain_Only, 1: AGain_DGain_Separate, 2: AGain_DGain_Combined */
    AX_U8  nDcgEn;      /* 1: support LCG/HCG switch, 0: Not Support */
    AX_U32 nDcgRatio;   /* Accuracy: U10.10 Range: [0x400, 0x2800] */
    AX_U64 nExpValLimitMin;  /* Accuracy: U36.10 Range: [0x0, 400000000000], Max/Min gain Depend on Sensor Spec */
    AX_U64 nExpValLimitMax;  /* Accuracy: U36.10 Range: [0x0, 400000000000], Max/Min gain Depend on Sensor Spec */
    AX_U32 nTotalGainMin;    /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_U32 nTotalGainMax;    /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_U32 nHdrRatioMin;     /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */
    AX_U32 nHdrRatioMax;     /* Accuracy: U7.10  Range: [0x400, 0x1FC00] */
    AX_U32 nSnsTotalGainMin;     /* Accuracy: U22.10  Range: [0x400, 0xFFFFFFFF] */
    AX_U32 nSnsTotalGainMax;     /* Accuracy: U22.10  Range: [0x400, 0xFFFFFFFF] */

    AX_ISP_IQ_AE_UNIT_T tIspDgainLimit;          /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]  */
    AX_ISP_IQ_AE_UNIT_T tSnsShutterLimit;        /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */
    AX_ISP_IQ_AE_UNIT_T tSnsAgainLimit;          /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tSnsDgainLimit;          /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */

    AX_ISP_IQ_AE_UNIT_T tSnsSlowShutterModeShutterLimit;   /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */

    /*first defined, temp not used*/
    AX_ISP_IQ_AE_UNIT_T tShortAgainLimit;        /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tShortDgainLimit;        /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tShortShutterLimit;      /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */
    AX_ISP_IQ_AE_UNIT_T tVsAgainLimit;           /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tVsDgainLimit;           /* Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF], Max/Min gain Depend on Sensor Spec */
    AX_ISP_IQ_AE_UNIT_T tVsShutterLimit;         /* Uints: us. Accuracy: U32 Range: [0x0, 0xFFFFFFFF], Max/Min IntTime Depends on FPS */

} AX_ISP_IQ_EXP_HW_LIMIT_T;


typedef struct {
    AX_U64 nExpList[AX_ISP_AE_LUX_MAX_PAIR_NUM];     /* Accuracy: U36.10
                                                      * Range：min = tIspDgainLimit.nMin * tSnsAgainLimit.nMin * tSnsDgainLimit.nMin * tSnsShutterLimit.nMin
                                                      *  max = tIspDgainLimit.nMax * tSnsAgainLimit.nMax * tSnsDgainLimit.nMax * tSnsShutterLimit.nMax * nDcgRatio */
    AX_U32 nLuxList[AX_ISP_AE_LUX_MAX_PAIR_NUM];     /* Accuracy: U22.10 Range: [0x0, 0x3D090000] */
    AX_U32 nLumaList[AX_ISP_AE_LUX_MAX_PAIR_NUM];    /* Accuracy: U8.10  Range: [0x0, 0x3FC00]  */
} AX_ISP_IQ_LUX_K_CALIB_INPUT_T;


#ifdef __cplusplus
}
#endif

#endif //_AX_ISP_3A_STRUCT_H_