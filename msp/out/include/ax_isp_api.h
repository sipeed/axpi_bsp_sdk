/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_ISP_API_H__
#define __AX_ISP_API_H__

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_isp_common.h"
#include "ax_isp_iq_api.h"
#include "ax_isp_3a_plus.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CHIP_NAME    "AX620"

#define SDK_VER_PRIX "_ISP_V"

#ifndef ISP_VERSION_MAJOR
#define ISP_VERSION_MAJOR 0
#endif

#ifndef ISP_VERSION_MINOR
#define ISP_VERSION_MINOR 1
#endif

#ifndef ISP_VERSION_MINOR2
#define ISP_VERSION_MINOR2 0
#endif


#define __MAKE_VERSION(a,x,y) #a"."#x"."#y
#define MAKE_VERSION(a,x,y) __MAKE_VERSION(a,x,y)
#define ISP_VERSION  CHIP_NAME SDK_VER_PRIX MAKE_VERSION(ISP_VERSION_MAJOR,ISP_VERSION_MINOR,ISP_VERSION_MINOR2)

#define AX_ISP_VERSION_MAX_SIZE (64)
#define AX_ISP_BUILD_TIME_MAX_SIZE (32)


/**********************************************************************************
 *                                   AE
 * input (AX_ISP_AE_INPUT_INFO_T) --> ae_alg --> output (AX_ISP_AE_RESULT_T)
 **********************************************************************************/
#define AX_HDR_RATIO_CHN_NUM    (3)

typedef enum {
    AX_HDR_CHN_L = 0,   /* long frame */
    AX_HDR_CHN_M,       /* middle frame */
    AX_HDR_CHN_S,       /* short frame */
    AX_HDR_CHN_VS,      /* very short frame */
} AX_HDR_CHN_E;

typedef enum {
    AX_HDR_RATIO_CHN_L_M = 0,   /* long frame / middle frame */
    AX_HDR_RATIO_CHN_M_S,       /* middle frame / short frame */
    AX_HDR_RATIO_CHN_S_VS,      /* short frame / very short frame */
} AX_HDR_RATIO_CHN_E;

typedef enum {
    AX_IRQ_TYPE_ITP_FEOF = 0,
} AX_IRQ_TYPE_E;

typedef struct {
    AX_U32 nSnsId;
    AX_SNS_HDR_MODE_E eSnsMode;
    AX_BAYER_PATTERN_E eBayerPattern;
    AX_U32 nFrameRate;
} AX_ISP_AE_INITATTR_T;

typedef struct {
    AX_ISP_AE_STAT_INFO_T sAeStat;
} AX_ISP_AE_INPUT_INFO_T;

typedef struct {
    AX_U32 nIntTime[AX_HDR_CHN_NUM];            /* ExposeTime(us). Accuracy: U32 Range: [0x0, 0xFFFFFFFF] */

    AX_U32 nAgain[AX_HDR_CHN_NUM];              /* Total Again value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]
                                                 * Total Again = Sensor Register Again x HCG Ratio
                                                 * LCG Mode: HCG Ratio = 1.0
                                                 * HCG Mode: HCG Ratio = Refer to Sensor Spec */

    AX_U32 nDgain[AX_HDR_CHN_NUM];              /* Sensor Dgain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]
                                                 * Not Used, should be set to 0x400. AX Platform Use ISP DGain */

    AX_U32 nIspGain;                            /* ISP Dgain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF] */

    AX_U32 nTotalGain[AX_HDR_CHN_NUM];          /* Total Gain value. Accuracy: U22.10 Range: [0x400, 0xFFFFFFFF]
                                                 * Total Gain value = SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain */

    AX_U32 nHcgLcgMode;                         /* 0:HCG 1:LCG 2:Not Support */

    AX_U32 nHcgLcgRatio;                        /* Accuracy: U10.10 Range: [0x400, 0x2800] */

    AX_U32 nHdrRatio[AX_HDR_RATIO_CHN_NUM];     /* Accuracy: U7.10 Range: [0x400, 0x1FC00] */

    AX_U32 nLux;                                /* Accuracy: U22.10 Range: [0, 0xFFFFFFFF]
                                                 * fLux = (MeanLuma*LuxK) / (AGain*Dgain*IspGain)
                                                 * where LuxK is a calibrated factor */

    AX_U32 nMeanLuma;                           /* Mean Luma of the Frame. Accuracy: U8.10 Range:[0, 0x3FC00] */

    // AX_BOOL bNeedExcu;                       /* true:need caller to config sensor */
} AX_ISP_AE_RESULT_T;

typedef struct {
    AX_S32(*pfnAe_Init)(AX_U8 pipe, AX_ISP_AE_INITATTR_T *pAeInitParam);
    AX_S32(*pfnAe_Run)(AX_U8 pipe, AX_ISP_AE_INPUT_INFO_T *pAeInputInfo, AX_ISP_AE_RESULT_T *pAeResult);
    AX_S32(*pfnAe_Exit)(AX_U8 pipe);
} AX_ISP_AE_REGFUNCS_T;

/**********************************************************************************
 *                                  AWB
 * input (AX_ISP_AWB_INPUT_INFO_T) --> awb_alg --> output (AX_ISP_AWB_RESULT_T)
 **********************************************************************************/
typedef struct {
    AX_U32 nSnsId;
    AX_SNS_HDR_MODE_E eSnsMode;
    AX_BAYER_PATTERN_E eBayerPattern;
    AX_U32 nFrameRate;
} AX_ISP_AWB_INITATTR_T;

typedef struct {
    AX_U32 nLux;        /* from AE */
    AX_U32 nMeanLuma;   /* from AE */
    AX_ISP_AWB_STAT_INFO_T tAwbStat;
} AX_ISP_AWB_INPUT_INFO_T;

typedef struct {
    AX_U32 nGrGain;         /* = gain * 256.0f, gain:[1.00f, 15.0f) */
    AX_U32 nGbGain;         /* = gain * 256.0f, gain:[1.00f, 15.0f) */
    AX_U32 nRgain;          /* = gain * 256.0f, gain:[1.00f, 15.0f) */
    AX_U32 nBgain;          /* = gain * 256.0f, gain:[1.00f, 15.0f) */
    AX_U32 nColorTemp;      /* = Accuracy:U32.0, CCT Range:[1000, 20000]  Color Temperature */
    AX_U32 nSatDiscnt;     /* Saturation Used for CCM or LSC Interpolation
                             *e.g. 100 for 100%
                             * If Not Used, Just return 100 */
} AX_ISP_AWB_RESULT_T;

typedef struct {
    AX_S32(*pfnAwb_Init)(AX_U8 pipe, AX_ISP_AWB_INITATTR_T *pAwbInitParam);
    AX_S32(*pfnAwb_Run)(AX_U8 pipe, AX_ISP_AWB_INPUT_INFO_T *pAwbInputInfo, AX_ISP_AWB_RESULT_T *pAwbResult);
    AX_S32(*pfnAwb_Exit)(AX_U8 pipe);
} AX_ISP_AWB_REGFUNCS_T;

typedef union {
    struct {
        AX_U64  bitEnableLin                : 1 ;   /* RW; [0] */
        AX_U64  bitEnableDpc                : 1 ;   /* RW; [1] */
        AX_U64  bitEnableBlc                : 1 ;   /* RW; [2] */
        AX_U64  bitEnableFpn                : 1 ;   /* RW; [3] */
        AX_U64  bitEnableDarkShading        : 1 ;   /* RW; [4] */
        AX_U64  bitEnableDrc                : 1 ;   /* RW; [5] */
        AX_U64  bitEnableAWBStatFE          : 1 ;   /* RW; [6] */
        AX_U64  bitEnableAWBStatBE          : 1 ;   /* RW; [7] */
        AX_U64  bitEnableAFStat             : 1 ;   /* RW; [8] */
        AX_U64  bitEnableAEStat             : 1 ;   /* RW; [9] */
        AX_U64  bitEnableNpu                : 1 ;   /* RW; [10] */
        AX_U64  bitEnableWnr                : 1 ;   /* RW; [11] */
        AX_U64  bitEnableCac                : 1 ;   /* RW; [12] */
        AX_U64  bitEnableLsc                : 1 ;   /* RW; [13] */
        AX_U64  bitEnableWbc                : 1 ;   /* RW; [14] */
        AX_U64  bitEnableRLTM               : 1 ;   /* RW; [15] */
        AX_U64  bitEnableDehaze             : 1 ;   /* RW; [16] */
        AX_U64  bitEnableDemosaic           : 1 ;   /* RW; [17] */
        AX_U64  bitEnableClc                : 1 ;   /* RW; [18] */
        AX_U64  bitEnableGamma              : 1 ;   /* RW; [19] */
        AX_U64  bitEnableCsc                : 1 ;   /* RW; [20] */
        AX_U64  bitEnableLumaNR             : 1 ;   /* RW; [21] */
        AX_U64  bitEnableChromaNR           : 1 ;   /* RW; [22] */
        AX_U64  bitEnableSharpen            : 1 ;   /* RW; [23] */
        AX_U64  bitEnableYCrt               : 1 ;   /* RW; [24] */
        AX_U64  bitEnableYCproc             : 1 ;   /* RW; [25] */
        AX_U64  bitEnableCSET               : 1 ;   /* RW; [26] */
        AX_U64  bitEnableSifRdma1           : 1 ;   /* RW; [27] */
        AX_U64  bitEnableDpc1               : 1 ;   /* RW; [28] */
        AX_U64  bitRsv30                    : 35 ;  /* H  ; [29:63] */
    };
    AX_U64  u64Key;
} AX_MODULE_CTRL_T;

typedef struct {
    AX_U16 nSnsDefaultBlackLevel;
} AX_ISP_PUB_ATTR_T;

typedef struct _AX_ISP_VERSION_T_ {
    AX_U32  nIspMajor;
    AX_U32  nIspMinor1;
    AX_U32  nIspMinor2;
    AX_CHAR szBuildTime[AX_ISP_BUILD_TIME_MAX_SIZE];
    AX_CHAR szIspVersion[AX_ISP_VERSION_MAX_SIZE];
} AX_ISP_VERSION_T;


/************************************************************************************
 *  ISP API
 ************************************************************************************/
AX_S32 AX_ISP_SetModuleControl(AX_U8 pipe, AX_MODULE_CTRL_T *pModCtrl);
AX_S32 AX_ISP_GetModuleControl(AX_U8 pipe, AX_MODULE_CTRL_T *pModCtrl);

AX_S32 AX_ISP_RegisterAeLibCallback(AX_U8 pipe, AX_ISP_AE_REGFUNCS_T *pRegisters);
AX_S32 AX_ISP_UnRegisterAeLibCallback(AX_U8 pipe);

AX_S32 AX_ISP_RegisterAwbLibCallback(AX_U8 pipe, AX_ISP_AWB_REGFUNCS_T *pRegisters);
AX_S32 AX_ISP_UnRegisterAwbLibCallback(AX_U8 pipe);

AX_S32 AX_ISP_RegisterLscLibCallback(AX_U8 pipe, void *pRegisters);
AX_S32 AX_ISP_UnRegisterLscLibCallback(AX_U8 pipe);

AX_S32 AX_ISP_LoadBinParams(AX_U8 pipe, const AX_CHAR *pFileName);

AX_S32 AX_ISP_Open(AX_U8 pipe);
AX_S32 AX_ISP_Close(AX_U8 pipe);

AX_S32 AX_ISP_Run(AX_U8 pipe);
AX_S32 AX_ISP_GetIRQTimeOut(AX_U8 pipe, AX_IRQ_TYPE_E nIrqType, AX_U32 nTimeOutMs);

AX_S32 AX_ISP_SetPubAttr(AX_U8 pipe, AX_ISP_PUB_ATTR_T *pIspPubAttr);
AX_S32 AX_ISP_GetPubAttr(AX_U8 pipe, AX_ISP_PUB_ATTR_T *pIspPubAttr);
AX_S32 AX_ISP_GetVersion(AX_U8 pipe, AX_ISP_VERSION_T *pIspVersion);

#ifdef __cplusplus
}
#endif

#endif  //_AX_ISP_API_H_
