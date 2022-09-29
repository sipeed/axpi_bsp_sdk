/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __SAMPLE_ISP_AE_H__
#define __SAMPLE_ISP_AE_H__

#include <stdio.h>
#include <stdarg.h>

#include "ax_vin_api.h"
#include "ax_base_type.h"
#include "ax_interpreter_external_api.h"
#include "ax_sys_api.h"
#include "ax_mipi_api.h"
#include "common_vin.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define RET_NOTSUPP             2   //!< feature not supported
#define RET_SUCCESS             0   //!< this has to be 0, if clauses rely on it
#define AE_MASTER_PIPE_ID       0
#define AE_SLAVE_PIPE_ID        1
#define AE_ISPGAIN_MEMORY       5
#define AE_EV_MEMORY            5
#define AE_RATIO_MEMORY         5
#define AE_SETPOINT_MAX_NUM     10
#define MAX_AE_BLOCK_NUM        4096
#define AE_SENSOR_MAX_GAIN_STEP 20000
#define AE_DAMP_MAX_NUM         10
#define AE_ROUTE_CURVE_NUM      (16)


/* Mean Value of Each RAW Channel of Each Grid */
#define AX_MEAN_LUMA_ZONE_ROW           (32)
#define AX_MEAN_LUMA_ZONE_COLUMN        (32)
#define AX_MEAN_LUMA_ZONE_GRID_SIZE     (4096)
#define AX_MEAN_LUMA_ZONE_CHN           (4)

#define MASK(width)                     ((1 << (width)) - 1) /* 2^w - 1 */
#ifndef ABS
    #define ABS(a)                      (((a) < 0) ? -(a) : (a))
#endif
#define false                           (0)
#define true                            (1)


typedef enum {
    SAMPLE_SUCCESS = 0x0,
    SAMPLE_FAILED = 0x1,
    SAMPLE_PTR_NULL,
    SAMPLE_NO_MEM,
    SAMPLE_INVALID_ADDRESS,
    SAMPLE_ILLEGAL_PARAMS,
    SAMPLE_SNS_UNREGISTER,
} SAMPLE_ALG_ERR_CODE_E;

typedef enum AxAeFrame_s
{
    AXAE_LONG_FRAME   = 0,
    AXAE_MEDIUM_FRAME = 1,
    AXAE_SHORT_FRAME  = 2,
    AXAE_FRAME_MAX
} AxAeFrame_t;

typedef enum AxAeMultiCamSyncType_s
{
    AXAE_MULTICAM_INDEPEND     = 0,
    AXAE_MULTICAM_MASTER_SLAVE = 1,
    AXAE_MULTICAM_OVERLAP      = 2,
    AXAE_MULTICAM_MAX          = 3,
} AxAeMultiCamSyncType_t;

typedef enum AxAeRouteFlag_s
{
    AXAE_AEROUTE_INTETIME    = 0,
    AXAE_AEROUTE_GAIN        = 1,
    AXAE_AEROUTE_TOTALGAIN   = 2,
    AXAE_AEROUTE_ISPGAIN     = 3,
} AxAeRouteFlag_t;

typedef enum AxAeStrategyMode_s
{
    AXAE_STRATEGY_SHUTTER_PRIOR = 0,
    AXAE_STRATEGY_GAIN_PRIOR    = 1,
    AXAE_STRATEGY_AEROUTE       = 2,
} AxAeStrategyMode_t;

typedef struct {
    AX_F32 fMax;
    AX_F32 fMin;
    AX_F32 fStep;
} AX_ISP_3A_UNIT_T;

typedef struct {
    AX_U32 nMax;
    AX_U32 nMin;
    AX_U32 nStep;
} AX_ISP_3A_SHUTTER_UNIT_T;

typedef enum AX_ISP_3A_ANTI_FLICKER_MODE_S
{
    AX_ISP_ANTI_FLICKER_OFF   = 0,
    AX_ISP_ANTI_FLICKER_ON    = 1,
} AX_ISP_3A_ANTI_FLICKER_MODE_T;

typedef enum AX_ISP_3A_FLICKER_PERIOD_S
{
    AX_ISP_FLICKER_100HZ   = 0,
    AX_ISP_FLICKER_120HZ   = 1,
} AX_ISP_3A_FLICKER_PERIOD_T;

typedef struct
{
    int              nDampOverListSize;
    float            fDampOverLumaDevList[AE_DAMP_MAX_NUM];
    float            fDampOverRatioList[AE_DAMP_MAX_NUM];
    int              nDampUnderListSize;
    float            fDampUnderLumaDevList[AE_DAMP_MAX_NUM];
    float            fDampUnderRatioList[AE_DAMP_MAX_NUM];
} SAMPLE_AE_DAMP_INFO_T;

typedef struct{
    AX_U8  enable;                /* 0: Antiflicker off; 1: antiflicker on */
    AX_U8  flickerPeriod;         /* 0: 100HZ, 1: 120HZ */
    AX_U32 antiFlickerTolerance;  /* Uints: us */
    AX_U8  overExpMode;           /* 0: ANTI PRIOR,1: LUMA PRIOR */
    AX_U8  underExpMode;          /* 0: ANTI PRIOR,1: LUMA PRIOR */
} SAMPLE_AE_ANTIFLICKER_PARAMS_T;

/* Static Calibration Info or Tuning Parameters. */
typedef struct {
    AX_F32 targetLuma;                   /* AE target meanValue */
    AX_F32 tolerance;                    /* Upper limit of luma deviation between meanLuma and targetLuma */

    AX_U32 luxK;                         /* Calibrated parameter used to get lux */

    /* Upper and lower limit of exposure, gain ,integrationTime  and sensor status  used in the AE Algorithm. */
    AX_U8  uGainMode;      /* aGain_only; aGain_dGain_both; aGain_dGain_combined */
    AX_U8  uLcghcgEn;      /* if support LCG/HCG switch */
    AX_F32 fHcglcgratio;   /* sensor LCG/HCG ratio */
    AX_F32 minGain;        /* min gain = minUserDgain * minIspGain * minUserAgain*/
    AX_F32 maxGain;        /* max gain = maxUserDgain * maxIspGain * maxUserAgain * againRatio_lcg2hcg*/
    AX_F32 maxIspGain;     /*  max ispgain set by user in AE algo */
    AX_F32 minIspGain;     /*  min ispgain set by user in AE algo */
    AX_F32 maxUserDgain;   /*  max sensor dgain set by user in AE algo */
    AX_F32 minUserDgain;   /*  min sensor dgain set by user in AE algo */
    AX_F32 minUserAgain;   /*  min sensor again set by user in AE algo */
    AX_U32 maxUserShutter; /*  max sensor integration time set by user in AE algo */
    AX_U32 minUserShutter; /*  min sensor integration time set by user in AE algo */

    AX_F32 againRatio_lcg2hcg;    /* ratio uesd for switching from LCG to HCG,
                                   * HCG = LCG * againRatio_hcg2lcg */
    AX_F32 againRatio_hcg2lcg;    /* ratio uesd for switching from HCG to LCG,
                                    LCG = HCG / againRatio_hcg2lcg */
    AX_F32 againTh_lcg2hcg;       /* Threshold from LCG to HCG, in lcg mode,
                                    if again > againTh_lcg2hcg, will switch to HCG mode */
    AX_F32 againTh_hcg2lcg;       /* Threshold from HCG to LCG, in hcg mode,
                                    if again < againTh_hcg2lcg, will switch to LCG mode */
    AX_F32 maxUserAgain;          /* Threshold for ispgain & dgain, in hcg mode,
                                   * if again > maxUserAgain ,will start to use ispgain */

    /*Exp param(gain&shutter)hardware limit*/
    AX_ISP_3A_SHUTTER_UNIT_T IntegrationTimeLimit; /* long frame sensor integration time limit */
    AX_ISP_3A_UNIT_T AgainLimit;                   /* long frame sensor again limit */
    AX_ISP_3A_UNIT_T DgainLimit;                   /* long frame sensor dgain limit */
    AX_ISP_3A_UNIT_T IspDgainLimit;                /* long frame system ispdgain limit */
    AX_F64 fExpValLimitMax; /* fExpValLimitMax sensor and sys EV max limit
                             * fExpValLimitMax = IntegrationTimeLimit.nMax * AgainLimit.fMax * >DgainLimit.fMax * \
                                                 IspDgainLimit.fMax * fHcglcgratio */
    AX_F64 fExpValLimitMin; /* fExpValLimitMin sensor and sys EV min limit
                             * fExpValLimitMix = IntegrationTimeLimit.nMin * AgainLimit.fMin * >DgainLimit.fMin * IspDgainLimit.fMin */
    AX_ISP_3A_UNIT_T sGain;                      /* middle frame sensor again limit */
    AX_ISP_3A_UNIT_T sDgain;                     /* middle frame sensor dgain limit */
    AX_ISP_3A_UNIT_T sVsgain;                    /* short frame sensor dgain limit */
    AX_ISP_3A_UNIT_T sVsdgain;                   /* short frame sensor dgain limit */
    AX_ISP_3A_SHUTTER_UNIT_T sIntegrationTime;   /* middle frame sensor integration time limit */
    AX_ISP_3A_SHUTTER_UNIT_T sVsintegrationTime; /* short frame sensor integration time limit */
    AX_F32 fHwMinHdrRatio;    /* SensorR hdrratio min limit */
    AX_F32 fHwMaxHdrRatio;    /* SensorR hdrratio max limit */
    AX_F32 fTotalGainMin;     /* fTotalGainMin = AgainLimit.fMin * DgainLimit.fMin * IspDgainLimit.fMin */
    AX_F32 fTotalGainMax;     /* fTotalGainMax = AgainLimit.fMax * DgainLimit.fMax * IspDgainLimit.fMax * nSnsHcgLcgRatio */
    AX_F32 fSnsTotalGainMin;  /* fSnsTotalGainMin = AgainLimit.fMin * DgainLimit.fMin */
    AX_F32 fSnsTotalGainMax;  /* fSnsTotalGainMax = AgainLimit.fMax * DgainLimit.fMax* nSnsHcgLcgRatio */

    AX_F32 ispGain;           /* ispgain set to the system */

    /* flicker period */
    AX_U32 ecmTflicker;                           /* flicker period */
    SAMPLE_AE_ANTIFLICKER_PARAMS_T antiFlickerParam;

    /* hdr dynamic strategy */
    AX_F32 hdrRatio;                         /* fixed hdr ratio */

    /* Converge param */
    AX_S32 nCoverFrameThres;             /* Converge status threhold */

    /* Parameter used for time smooth */
    AX_F32 AxPreExposureValue[5];        /* Previous Frame Result */
    AX_U8  snsDelayFrame;                /* sys delay frame */
    AX_U8  ratioCountNum;                /* multi clip frame count Num*/
    AX_F32 ratioClipUpperLimit;          /* multi clip ratio upper limit */
    AX_F32 ratioClipLowerLimit;          /* multi clip ratio lower limit */
    SAMPLE_AE_DAMP_INFO_T dampInfo;  /* damp ratio */

    AX_SNS_HDR_MODE_E eSnsMode;          /* HDR mode  */

    /* again table */
    AX_S32 againTableSize;
    AX_F32 againTable[AE_SENSOR_MAX_GAIN_STEP];

} SAMPLE_AEC_CONFIG_T;

typedef struct {
    AX_F32 fIspGain;                // ISP D Gain
    AX_F32 fGain;                   // Sensor A Gain
    AX_F32 fDgain;                  // Sensor D Gain
    AX_U32 uHcg_lcg;                // current in HCG or LCG
    AX_U32 fIntegrationTime;        // Shutter
    // Below Unused on Tool, first defined, not used
    AX_F32 fSgain;
    AX_F32 fSdgain;
    AX_F32 fVsgain;
    AX_F32 fVsdgain;
    AX_U32 fSintegrationTime;
    AX_U32 fVsintegrationTime;
} SAMPLE_AEC_MANUAL_T;        // Gains and Shutter


/* Dynamic Information of the AE Algorithm, Updated each Frame. */
typedef struct {

    /* AE Result for the Current Frame. */
    AX_F32 meanLuma;

    AX_F32 lux;

    AX_F32 ispGain;

    // Sensor Integration Time (Unit: us)
    AX_U32 snsShutter[4];

    AX_F32 snsDgain[4];          /* SensorDgain */

    // For UI and Sensor Set / Get APIs
    AX_F32 snsRegAgain[4];       /* SensorRegisterAgain */

    // For UI and Sensor Set / Get APIs
    AX_F32 snsTotalRegGain[4];   /* SensorRegisterAgain * SensorDgain */

    // For Output for Other Modules' Tuning
    AX_F32 snsTotalAgain[4];     /* SensorRegisterAgain * CurrHcgRatio */

    // For Gain and Shutter Split
    AX_F32 snsTotalGain[4];      /* SensorRegisterAgain * SensorDgain * CurrHcgRatio */

    // For Exposure Ratio Calculation
    AX_F32 totalGain[4];         /* SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain */

    // For Exposure Ratio between Current Frame and Last Frame
    AX_F64 totalExposure[4];     /* SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain * Shutter */

    AX_U32 currHcgLcg;           /* current hcglcg mode */
    AX_F32 currHcgRatio;         /* current hcg ratio */

    /* HDR status */
    AX_F32  hdrRealRatioLtoS;            /* Real Hdr Ratio from Sensor Driver */
    AX_F32  hdrRealRatioStoVS;           /* Real Hdr Ratio from Sensor Driver */

    /* Converge status */
    AX_BOOL isConverge;                  /* Total converge status */
    AX_S32  nConverFrameCnt;             /* Current frame converge status */

    /* Parameter used for time smooth */
    AX_F32  AxPreEV[AE_EV_MEMORY];            /* Previous sensor EV Result (after sensor quantization) */
    AX_F32  AxPreAlgoEV[AE_EV_MEMORY];        /* Previous algo EV Result (before sensor quantization) */
    AX_F32  AxPreRatio[AE_RATIO_MEMORY];      /* Previous ratio Result */

    /* Parameter used for isp gain delay */
    AX_F32  AxPreIspGain[AE_ISPGAIN_MEMORY];  /* Previous IspGain Result */

    /* Histogram calculate from the AE Statistics Input. */
    AX_S32  longHist[AX_AE_HIST_LINEAR_BIN];
    AX_S32  longCumulaHist[AX_AE_HIST_LINEAR_BIN];
    AX_S32  shortHist[AX_AE_HIST_LINEAR_BIN];
    AX_S32  shortCumulaHist[AX_AE_HIST_LINEAR_BIN];

    AX_U16  AxStatistics[MAX_AE_BLOCK_NUM][AX_MEAN_LUMA_ZONE_CHN];

    /* AE Grid weight */
    AX_S32 nGridRow;
    AX_S32 nGridCol;

    /* HDR max shutter */
    AX_U32 hdrHwMaxShutter;  /* hard limit: calc from hdrratio*/
    AX_U32 algoMaxShutter;   /* cur max shutter= min(hdrHwMaxShutter, maxUserShutter) */

    AX_U32 maxExposure;      /* maxExposure = maxGain * algoMaxShutter */
    AX_U32 minExposure;      /* minExposure = minGain * minUserShutter */

} SAMPLE_AEC_STATUS_T;


/* AE Algorithm Context of a Sensor Pipe. */
typedef struct {
    AX_U8 bEnable;

    /* Static Calibration Info or Tuning Parameters. */
    SAMPLE_AEC_CONFIG_T tConfig;

    /* Dynamic Information of the AE Algorithm, Updated each Frame. */
    SAMPLE_AEC_STATUS_T tStatus;

    /* Manual Information for exposure control. */
    SAMPLE_AEC_MANUAL_T tManual;

} SAMPLE_AEC_CTX_T;

AX_S32 sample_ae_run(AX_U8 pipe, AX_ISP_AE_INPUT_INFO_T *pAeInputInfo,AX_ISP_AE_RESULT_T *pAeResult);
AX_S32 sample_ae_init(AX_U8 pipe, AX_ISP_AE_INITATTR_T *pParam);
AX_S32 sample_ae_deinit(AX_U8 pipe);
AX_S32 SetSensorFunc(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType);

#ifdef __cplusplus
}
#endif

#endif