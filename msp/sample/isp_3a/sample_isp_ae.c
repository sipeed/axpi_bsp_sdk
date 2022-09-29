/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <inttypes.h>
#include <syslog.h>

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_vin_api.h"
#include "ax_isp_api.h"
#include "ax_mipi_api.h"
#include "ax_sensor_struct.h"
#include "ax_isp_3a_api.h"
#include "common_cam.h"
#include "common_type.h"
#include "sample_isp_ae.h"

#define max(a,b) ( ((a)>(b)) ? (a):(b) )
#define min(a,b) ( ((a)>(b)) ? (b):(a) )

#define AXAE_DIV_0_TO_1(a)             ( (0 == (a)) ? 1 : (a) )
#define AXAE_DIV_0_TO_1_FLOAT(a)       ((((a) < 1E-6) && ((a) > -1E-6)) ? 1 : (a))

static AX_SENSOR_REGISTER_FUNC_T *pSnsHandle[DEF_VIN_PIPE_MAX_NUM];

static SAMPLE_AEC_CTX_T g_tAeAlgCtx[DEF_VIN_PIPE_MAX_NUM];

static SAMPLE_AEC_CTX_T *sample_ae_get_ctx_handle(AX_U32 pipe)
{
    SAMPLE_AEC_CTX_T *pAec_ctx = &g_tAeAlgCtx[pipe];
    return pAec_ctx;
}


///////////////////////////////////////////////////////////////////
/////////////////////////    Sensor APIs   ////////////////////////
///////////////////////////////////////////////////////////////////

/* Analog Gain */
AX_S32 sample_ae_sns_set_again(AX_U8 pipe, AX_SNS_AE_GAIN_CFG_T *ptAnalogGainTbl)
{
    AX_S32 result = 0;

    /*  Other Gain does not use the default assignment of 1X gain */
    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_set_again) {

        result = pSnsHandle[pipe]->pfn_sensor_set_again(pipe, ptAnalogGainTbl);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    return result;
}


/* Digital Gain */
AX_S32 sample_ae_sns_set_dgain(AX_U8 pipe, AX_SNS_AE_GAIN_CFG_T *ptDigitalGainTbl)
{
    AX_S32 result = 0;

    /*  Other Gain does not use the default assignment of 1X gain */
    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_set_dgain) {

        result = pSnsHandle[pipe]->pfn_sensor_set_dgain(pipe, ptDigitalGainTbl);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    return result;
}


/*  shutterLong : Interation Time(Unit:Second) */
AX_S32 sample_ae_sns_set_shutter(AX_U8 pipe, AX_SNS_AE_SHUTTER_CFG_T *ptIntTimeTbl)
{
    AX_S32 result = 0;

    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_set_integration_time) {
        result = pSnsHandle[pipe]->pfn_sensor_set_integration_time(pipe, ptIntTimeTbl);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    return result;
}


AX_S32 sample_ae_sns_set_lcg_hcg(AX_U8       pipe, AX_U32 lcgHcg, AX_F32 *pfSnsGain)
{
    AX_S32 result = 0;

    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_hcglcg_ctrl) {
        result = pSnsHandle[pipe]->pfn_sensor_hcglcg_ctrl(pipe, lcgHcg, pfSnsGain);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    return result;
}


AX_S32 sample_ae_sns_set_param(AX_U8 pipe, AX_SNS_PARAMS_T *pSnsParam)
{
    AX_S32 result = 0;

    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_set_params) {
        result = pSnsHandle[pipe]->pfn_sensor_set_params(pipe, pSnsParam);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    return result;
}


AX_S32 sample_ae_sns_get_param(AX_U8       pipe, AX_SNS_PARAMS_T *pSnsParam)
{
    AX_S32 result = 0;

    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_get_params) {
        result = pSnsHandle[pipe]->pfn_sensor_get_params(pipe, pSnsParam);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    return result;
}


AX_S32 sample_ae_sns_get_hdr_mode(AX_U8        pipe, AX_U32 *pHdrMode)
{
    AX_S32 result = 0;
    AX_SNS_PARAMS_T tSnsParam;
    tSnsParam.sns_dev_attr.eSnsMode = AX_SNS_LINEAR_MODE;

    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_get_params) {
        result = pSnsHandle[pipe]->pfn_sensor_get_params(pipe, &tSnsParam);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_SNS_UNREGISTER;
    }

    *pHdrMode = tSnsParam.sns_dev_attr.eSnsMode;

    return result;
}


AX_S32 sample_ae_sns_get_gain_table(AX_U8 pipe, AX_SNS_AE_GAIN_TABLE_T *pTableParam)
{
    AX_S32 result = 0;

    if (AX_NULL != pSnsHandle[pipe]->pfn_sensor_get_gain_table) {
        result = pSnsHandle[pipe]->pfn_sensor_get_gain_table(pipe, pTableParam);
        if (SAMPLE_SUCCESS != result) {
            return SAMPLE_FAILED;
        }
    } else {
        return SAMPLE_FAILED;
    }

    return result;
}


AX_S32 sample_ae_sns_get_hcg_status_ratio
(
    AX_U8       pipe,
    AX_U32     *phcgEn,
    AX_F32     *phcgRatio,
    AX_U32     *phcgStatus
)
{
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    SAMPLE_AEC_CONFIG_T *pCtxConfig  = &(pAlgCtx->tConfig);

    AX_S32 result = 0;
    AX_SNS_PARAMS_T tSnsParam;
    result = sample_ae_sns_get_param(pipe, &tSnsParam);

    switch (tSnsParam.sns_dev_attr.eSnsHcgLcg) {
    case AX_HCG_MODE:
        *phcgEn = 1;
        *phcgRatio = pCtxConfig->againRatio_lcg2hcg;
        *phcgStatus = AX_HCG_MODE;
        break;
    case AX_LCG_MODE:
        *phcgEn = 1;
        *phcgRatio = pCtxConfig->againRatio_lcg2hcg;
        *phcgStatus = AX_LCG_MODE;
        break;
    case AX_LCG_NOTSUPPORT_MODE:
        *phcgEn = 0;
        *phcgRatio = pCtxConfig->againRatio_lcg2hcg;
        *phcgStatus = AX_LCG_NOTSUPPORT_MODE;
        break;
    default:
        break;
    }

    return result;
}


///////////////////////////////////////////////////////////////////
///////////////////     Data Type Convert      ////////////////////
///////////////////////////////////////////////////////////////////

AX_F32 sample_ae_int_convert_to_float(AX_S32 p, AX_S32 int_bit, AX_S32 frac_bit, AX_BOOL signed_value)
{
    AX_F32 result = 0.0;
    if ((int_bit + frac_bit + signed_value) > 32) {
        printf("%s, invalid parameters\n", __func__);
        return -1.0;
    }
    AX_BOOL neg_flag = AX_FALSE;
    if (signed_value) {
        if (p < 0) {
            p = -p;
            neg_flag = AX_TRUE;
        }
    }

    result = ((AX_F32)p / (AX_F32)(1 << frac_bit));

    if (neg_flag) {
        result = -result;
    }

    return result;
}


AX_U32 sample_ae_float_convert_to_int(AX_F32 value, AX_U32 int_bit, AX_U32 frac_bit, AX_BOOL signed_value)
{
    AX_U32 result = 0;
    if ((int_bit + frac_bit + signed_value) > 32) {
        printf("%s, invalid parameters\n", __func__);
        return -1;
    }

    result = ABS(value) * (AX_U32)(1 << frac_bit);

    AX_U32 sign_bit = 0;
    AX_U32 data_bit = int_bit + frac_bit;
    /* if reg is signed */
    if (signed_value) {
        sign_bit = int_bit + frac_bit;
        if (value < 0) {
            result = ((~result + 1) & MASK(data_bit)) | (1 << sign_bit);
        } else {
            result = result & MASK(data_bit);
        }
    }

    return result;
}


AX_F64 sample_ae_int_convert_to_float_64bit(AX_U64 p, AX_U32 int_bit, AX_U32 frac_bit, AX_BOOL signed_value)
{
    AX_F64 result = 0.0;
    if ((int_bit + frac_bit + signed_value) > 64) {
        printf("%s, invalid parameters\n", __func__);
        return -1.0;
    }
    AX_BOOL neg_flag = AX_FALSE;
    if (signed_value) {
        if (p < 0) {
            p = -p;
            neg_flag = AX_TRUE;
        }
    }

    result = ((AX_F64)p / (AX_F64)(1 << frac_bit));

    if (neg_flag) {
        result = -result;
    }

    return result;
}


AX_U64 sample_ae_float_convert_to_int_64bit(AX_F64 value, AX_U32 int_bit, AX_U32 frac_bit, AX_BOOL signed_value)
{
    AX_U64 result = 0;
    if ((int_bit + frac_bit + signed_value) > 64) {
        return -1;
    }

    result = ABS(value) * (AX_U64)(1 << frac_bit);

    AX_U32 sign_bit = 0;
    AX_U32 data_bit = int_bit + frac_bit;
    /* if reg is signed */
    if (signed_value) {
        sign_bit = int_bit + frac_bit;
        if (value < 0) {
            result = ((~result + 1) & MASK(data_bit)) | (1 << sign_bit);
        } else {
            result = result & MASK(data_bit);
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////
/////////////////////////   AE Algorithm  /////////////////////////
///////////////////////////////////////////////////////////////////

AX_S32 sample_ae_init_sensor
(
    AX_U8 pipe
)
{
    AX_S32 result = 0;
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    SAMPLE_AEC_CONFIG_T *pCtxConfig  = &(pAlgCtx->tConfig);
    SAMPLE_AEC_STATUS_T *pCtxStatus  = &(pAlgCtx->tStatus);
    SAMPLE_AEC_MANUAL_T *pCtxManual  = &(pAlgCtx->tManual);
    AX_SNS_AE_GAIN_CFG_T tGainTbl = {0};
    AX_SNS_AE_SHUTTER_CFG_T tShutterTbl = {0};

    // set again, dgain, uHcg_lcg, shutter
    pCtxManual->fGain = pCtxStatus->snsRegAgain[AX_HDR_CHN_L];
    pCtxManual->fDgain = pCtxStatus->snsDgain[AX_HDR_CHN_L];
    pCtxManual->uHcg_lcg = pCtxStatus->currHcgLcg;
    pCtxManual->fIntegrationTime = pCtxStatus->snsShutter[AX_HDR_CHN_L];
    // Set Sensor Exposure
    if (pCtxConfig->eSnsMode == AX_SNS_LINEAR_MODE) {

        // Set LCG / HCG
        sample_ae_sns_set_lcg_hcg(pipe, pCtxManual->uHcg_lcg, &(pCtxManual->fGain));

        // Set Gains and Shutter
        tGainTbl.nGain[AXAE_LONG_FRAME] = pCtxManual->fGain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_again(pipe, &tGainTbl);

        tGainTbl.nGain[AXAE_LONG_FRAME] = pCtxManual->fDgain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_dgain(pipe, &tGainTbl);

        tShutterTbl.nIntTime[AXAE_LONG_FRAME] = pCtxManual->fIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_shutter(pipe, &tShutterTbl);

    } else if (pCtxConfig->eSnsMode == AX_SNS_HDR_2X_MODE) {

        // Set LCG / HCG
        sample_ae_sns_set_lcg_hcg(pipe, pCtxManual->uHcg_lcg, &(pCtxManual->fGain));

        // Set Gains and Shutter
        tGainTbl.nGain[AXAE_LONG_FRAME] = pCtxManual->fGain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = pCtxManual->fGain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_again(pipe, &tGainTbl);

        tGainTbl.nGain[AXAE_LONG_FRAME] = pCtxManual->fDgain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = pCtxManual->fDgain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_dgain(pipe, &tGainTbl);

        tShutterTbl.nIntTime[AXAE_LONG_FRAME] = pCtxManual->fIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tShutterTbl.nIntTime[AXAE_MEDIUM_FRAME] = pCtxManual->fIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_shutter(pipe, &tShutterTbl);

    } else if (pCtxConfig->eSnsMode == AX_SNS_HDR_3X_MODE) {

        // Set LCG / HCG
        sample_ae_sns_set_lcg_hcg(pipe, pCtxManual->uHcg_lcg, &(pCtxManual->fGain));

        // Set Gains and Shutter
        tGainTbl.nGain[AXAE_LONG_FRAME] = pCtxManual->fGain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = pCtxManual->fGain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tGainTbl.nGain[AXAE_SHORT_FRAME] = pCtxManual->fGain;
        tGainTbl.nHdrRatio[AXAE_SHORT_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_again(pipe, &tGainTbl);

        tGainTbl.nGain[AXAE_LONG_FRAME] = pCtxManual->fDgain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = pCtxManual->fDgain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tGainTbl.nGain[AXAE_SHORT_FRAME] = pCtxManual->fDgain;
        tGainTbl.nHdrRatio[AXAE_SHORT_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_dgain(pipe, &tGainTbl);

        tShutterTbl.nIntTime[AXAE_LONG_FRAME] = pCtxManual->fIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_LONG_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tShutterTbl.nIntTime[AXAE_MEDIUM_FRAME] = pCtxManual->fIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = pAlgCtx->tConfig.hdrRatio;
        tShutterTbl.nIntTime[AXAE_SHORT_FRAME] = pCtxManual->fIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_SHORT_FRAME] = pAlgCtx->tConfig.hdrRatio;
        sample_ae_sns_set_shutter(pipe, &tShutterTbl);
    }

    return result;
}

/*
 * @idealAgain: again from AE algo, before sensor quantization
 *
 * @pSensorSupportAgain: again after sensor quantization
 * pSensorSupportAgain = the max sensor support Again smaller than idealAgain
 *
 */
AX_F32 sample_ae_get_snsRegAgain_after_sensor_quantization
(
    AX_U8   pipe,
    AX_F32  idealAgain,
    AX_F32  *pSensorSupportAgain
)
{
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    int count = pAlgCtx->tConfig.againTableSize;

    if(idealAgain <= pAlgCtx->tConfig.againTable[0]){
        *pSensorSupportAgain = pAlgCtx->tConfig.againTable[0];;
        return 0;
    }
    if(idealAgain >= pAlgCtx->tConfig.againTable[count-1]){
        *pSensorSupportAgain = pAlgCtx->tConfig.againTable[count-1];
        return 0;
    }

    int matchFlag = 0;
    int low = 0;
    int high = count-1;

    int tmpStep = 0;
    while(low <= high){
        int mid = low + (high - low)/2;
        float midVal = pAlgCtx->tConfig.againTable[mid];
        if(idealAgain < midVal) {
            tmpStep = mid;
            high = mid - 1;
        }else if(midVal < idealAgain) {
            low = mid + 1;
        }else if (midVal == idealAgain) {
            tmpStep = mid;
            matchFlag = 1;
            break;
        }
    }

    if (matchFlag == 1) {
        *pSensorSupportAgain = pAlgCtx->tConfig.againTable[tmpStep];
    }else{
        *pSensorSupportAgain = pAlgCtx->tConfig.againTable[tmpStep -1];
    }

    return 0;
}


static AX_S32 sample_ae_upd_ctx_exp_settings
(
    SAMPLE_AEC_CTX_T *pAlgCtx,
    AX_U8                 pipe
)
{

    AX_S32 result = 0;

    // Get the Current Sensor Status Info.
    AX_SNS_PARAMS_T tSnsParam;
    result = sample_ae_sns_get_param(pipe, &tSnsParam);
    if (0 != result) {
        return -1;
    }
    AX_U8 snsDelayFrame = pAlgCtx->tConfig.snsDelayFrame;
    AX_U32 hcgEn = 0;
    AX_U32 hcgStatus = 0;
    AX_F32 hcgRatio = 1.0f;
    AX_F32 scaleRatio = 1.0f;
    sample_ae_sns_get_hcg_status_ratio( pipe, &hcgEn, &hcgRatio, &hcgStatus);
    if (1 == hcgEn && 0 == hcgStatus) {
        scaleRatio = hcgRatio;
    }

    // Collects All Complicated Things Here, to Make it Easier to Understand.
    AX_U32 snsShutter      = 0.0f;  // Shutter (Sensor Integration Time, Unit:us)
    AX_F32 snsDgain        = 0.0f;  // SensorDgain
    AX_F32 snsRegAgain     = 0.0f;  // SensorRegisterAgain
    AX_F32 snsTotalAgain   = 0.0f;  // SensorRegisterAgain * CurrHcgRatio
    AX_F32 snsTotalRegGain = 0.0f;  // SensorRegisterAgain * SensorDgain
    AX_F32 snsTotalGain    = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio
    AX_F32 totalGain       = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain
    AX_F32 totalExposure   = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain * Shutter

    // All Complicated Gains Relations
    snsDgain        = tSnsParam.sns_ae_param.nCurDGain[AXAE_LONG_FRAME];
    snsRegAgain     = tSnsParam.sns_ae_param.nCurAGain[AXAE_LONG_FRAME];
    snsTotalAgain   = snsRegAgain   * scaleRatio;
    snsTotalRegGain = snsRegAgain   * snsDgain;
    snsTotalGain    = snsTotalAgain * snsDgain;
    totalGain       = snsTotalGain  * pAlgCtx->tStatus.AxPreIspGain[snsDelayFrame];

    // Get Integration Time
    snsShutter = tSnsParam.sns_ae_param.nCurIntegrationTime[AXAE_LONG_FRAME];

    // Total EV = Total Gain * Integration Time
    totalExposure = totalGain * snsShutter;

    // Update Context's Exposure Settings
    pAlgCtx->tStatus.snsDgain[AX_HDR_CHN_L]        = snsDgain;
    pAlgCtx->tStatus.snsRegAgain[AX_HDR_CHN_L]     = snsRegAgain;
    pAlgCtx->tStatus.snsTotalRegGain[AX_HDR_CHN_L] = snsTotalRegGain;
    pAlgCtx->tStatus.snsTotalAgain[AX_HDR_CHN_L]   = snsTotalAgain;
    pAlgCtx->tStatus.snsTotalGain[AX_HDR_CHN_L]    = snsTotalGain;
    pAlgCtx->tStatus.totalGain[AX_HDR_CHN_L]       = totalGain;
    pAlgCtx->tStatus.snsShutter[AX_HDR_CHN_L]      = snsShutter;
    pAlgCtx->tStatus.totalExposure[AX_HDR_CHN_L]   = totalExposure;

    pAlgCtx->tStatus.currHcgLcg   = hcgStatus;
    pAlgCtx->tStatus.currHcgRatio = scaleRatio;

    // Get HDR Mode
    AX_U32 hdrMode = AX_SNS_LINEAR_MODE;
    result = sample_ae_sns_get_hdr_mode(pipe, &hdrMode);
    if (result != 0) {
        printf("Get HDR Mode Failed!");
    }

    // Get hdr max shutter according to hdr ratio
    pAlgCtx->tStatus.hdrHwMaxShutter = tSnsParam.sns_ae_limit.nMaxIntegrationTime[AXAE_LONG_FRAME];
    pAlgCtx->tStatus.algoMaxShutter = min(tSnsParam.sns_ae_limit.nMaxIntegrationTime[AXAE_LONG_FRAME], pAlgCtx->tConfig.maxUserShutter);

    //Update maxExposure and minExposure
    pAlgCtx->tStatus.maxExposure =  pAlgCtx->tConfig.maxGain * pAlgCtx->tStatus.algoMaxShutter;
    pAlgCtx->tStatus.minExposure =  pAlgCtx->tConfig.minGain * pAlgCtx->tConfig.minUserShutter;

    if (hdrMode == AX_SNS_LINEAR_MODE) {
        pAlgCtx->tStatus.hdrRealRatioLtoS  = 1.0;
        pAlgCtx->tStatus.hdrRealRatioStoVS = 1.0;
    }
    else if (hdrMode == AX_SNS_HDR_2X_MODE) {
        AX_U32 shortShutter       = 0.0f;
        AX_F32 shortDgain         = 0.0f;  // SensorDgain
        AX_F32 shortRegAgain      = 0.0f;  // SensorRegisterAgain
        AX_F32 shortTotalAgain    = 0.0f;  // SensorRegisterAgain * CurrHcgRatio
        AX_F32 shortTotalRegGain  = 0.0f;  // SensorRegisterAgain * SensorDgain
        AX_F32 shortSnsTotalGain  = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio
        AX_F32 shortTotalGain     = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain
        AX_F32 shortTotalExposure = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain * Shutter

        shortShutter  = tSnsParam.sns_ae_param.nCurIntegrationTime[AXAE_MEDIUM_FRAME];
        shortDgain    = tSnsParam.sns_ae_param.nCurDGain[AXAE_MEDIUM_FRAME];
        shortRegAgain = tSnsParam.sns_ae_param.nCurAGain[AXAE_MEDIUM_FRAME];

        shortTotalAgain    = shortRegAgain     * scaleRatio;
        shortTotalRegGain  = shortRegAgain     * shortDgain;
        shortSnsTotalGain  = shortTotalAgain   * shortDgain;
        shortTotalGain     = shortSnsTotalGain * pAlgCtx->tStatus.AxPreIspGain[snsDelayFrame];
        shortTotalExposure = shortTotalGain    * shortShutter;

        // Update Context's Exposure Settings
        pAlgCtx->tStatus.snsDgain[AX_HDR_CHN_M] 	   = shortDgain;
        pAlgCtx->tStatus.snsRegAgain[AX_HDR_CHN_M]	   = shortRegAgain;
        pAlgCtx->tStatus.snsTotalRegGain[AX_HDR_CHN_M] = shortTotalRegGain;
        pAlgCtx->tStatus.snsTotalAgain[AX_HDR_CHN_M]   = shortTotalAgain;
        pAlgCtx->tStatus.snsTotalGain[AX_HDR_CHN_M]    = shortSnsTotalGain;
        pAlgCtx->tStatus.totalGain[AX_HDR_CHN_M]	   = shortTotalGain;
        pAlgCtx->tStatus.snsShutter[AX_HDR_CHN_M]	   = shortShutter;
        pAlgCtx->tStatus.totalExposure[AX_HDR_CHN_M]   = shortTotalExposure;

        pAlgCtx->tStatus.hdrRealRatioLtoS  = totalExposure / shortTotalExposure;
        pAlgCtx->tStatus.hdrRealRatioStoVS = 1.0;

    }
    else if (hdrMode == AX_SNS_HDR_3X_MODE) {

        AX_U32 shortShutter       = 0.0f;
        AX_F32 shortDgain         = 0.0f;  // SensorDgain
        AX_F32 shortRegAgain      = 0.0f;  // SensorRegisterAgain
        AX_F32 shortTotalAgain    = 0.0f;  // SensorRegisterAgain * CurrHcgRatio
        AX_F32 shortTotalRegGain  = 0.0f;  // SensorRegisterAgain * SensorDgain
        AX_F32 shortSnsTotalGain  = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio
        AX_F32 shortTotalGain     = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain
        AX_F32 shortTotalExposure = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain * Shutter

        shortShutter  = tSnsParam.sns_ae_param.nCurIntegrationTime[AXAE_MEDIUM_FRAME];
        shortDgain    = tSnsParam.sns_ae_param.nCurDGain[AXAE_MEDIUM_FRAME];
        shortRegAgain = tSnsParam.sns_ae_param.nCurAGain[AXAE_MEDIUM_FRAME];

        shortTotalAgain    = shortRegAgain     * scaleRatio;
        shortTotalRegGain  = shortRegAgain     * shortDgain;
        shortSnsTotalGain  = shortTotalAgain   * shortDgain;
        shortTotalGain	   = shortSnsTotalGain * pAlgCtx->tStatus.AxPreIspGain[snsDelayFrame];
        shortTotalExposure = shortTotalGain    * shortShutter;

        // Update Context's Exposure Settings
        pAlgCtx->tStatus.snsDgain[AX_HDR_CHN_M] 	   = shortDgain;
        pAlgCtx->tStatus.snsRegAgain[AX_HDR_CHN_M]	   = shortRegAgain;
        pAlgCtx->tStatus.snsTotalRegGain[AX_HDR_CHN_M] = shortTotalRegGain;
        pAlgCtx->tStatus.snsTotalAgain[AX_HDR_CHN_M]   = shortTotalAgain;
        pAlgCtx->tStatus.snsTotalGain[AX_HDR_CHN_M]    = shortSnsTotalGain;
        pAlgCtx->tStatus.totalGain[AX_HDR_CHN_M]	   = shortTotalGain;
        pAlgCtx->tStatus.snsShutter[AX_HDR_CHN_M]	   = shortShutter;
        pAlgCtx->tStatus.totalExposure[AX_HDR_CHN_M]   = shortTotalExposure;

        AX_U32 vsShutter       = 0.0f;
        AX_F32 vsDgain         = 0.0f;  // SensorDgain
        AX_F32 vsRegAgain      = 0.0f;  // SensorRegisterAgain
        AX_F32 vsTotalAgain    = 0.0f;  // SensorRegisterAgain * CurrHcgRatio
        AX_F32 vsTotalRegGain  = 0.0f;  // SensorRegisterAgain * SensorDgain
        AX_F32 vsSnsTotalGain  = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio
        AX_F32 vsTotalGain     = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain
        AX_F32 vsTotalExposure = 0.0f;  // SensorRegisterAgain * SensorDgain * CurrHcgRatio * IspGain * Shutter

        vsShutter  = tSnsParam.sns_ae_param.nCurIntegrationTime[AXAE_SHORT_FRAME];
        vsDgain    = tSnsParam.sns_ae_param.nCurDGain[AXAE_SHORT_FRAME];
        vsRegAgain = tSnsParam.sns_ae_param.nCurAGain[AXAE_SHORT_FRAME];

        vsTotalAgain    = vsRegAgain     * scaleRatio;
        vsTotalRegGain  = vsRegAgain     * vsDgain;
        vsSnsTotalGain  = vsTotalAgain   * vsDgain;
        vsTotalGain	    = vsSnsTotalGain * pAlgCtx->tStatus.AxPreIspGain[snsDelayFrame];
        vsTotalExposure = vsTotalGain    * vsShutter;

        // Update Context's Exposure Settings
        pAlgCtx->tStatus.snsDgain[AX_HDR_CHN_S] 	   = vsDgain;
        pAlgCtx->tStatus.snsRegAgain[AX_HDR_CHN_S]	   = vsRegAgain;
        pAlgCtx->tStatus.snsTotalRegGain[AX_HDR_CHN_S] = vsTotalRegGain;
        pAlgCtx->tStatus.snsTotalAgain[AX_HDR_CHN_S]   = vsTotalAgain;
        pAlgCtx->tStatus.snsTotalGain[AX_HDR_CHN_S]    = vsSnsTotalGain;
        pAlgCtx->tStatus.totalGain[AX_HDR_CHN_S]	   = vsTotalGain;
        pAlgCtx->tStatus.snsShutter[AX_HDR_CHN_S]	   = vsShutter;
        pAlgCtx->tStatus.totalExposure[AX_HDR_CHN_S]   = vsTotalExposure;

        pAlgCtx->tStatus.hdrRealRatioLtoS  = totalExposure / shortTotalExposure;
        pAlgCtx->tStatus.hdrRealRatioStoVS = shortTotalExposure / vsTotalExposure;
    }

    return result;
}


AX_F32 sample_ae_ideal_again_to_real_again
(
    AX_U8   pipe,
    AX_F32  *pCurrGain
)
{
    AX_S32 result = 0;
    SAMPLE_AEC_CTX_T    *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    SAMPLE_AEC_CONFIG_T *pCtxConfig = &(pAlgCtx->tConfig);

    int count = pCtxConfig->againTableSize;
    int low = 0;
    int high = count - 1;

    if(*pCurrGain < pCtxConfig->againTable[low]){
        *pCurrGain = pCtxConfig->againTable[low];
        return 0;
    }

    if(*pCurrGain > pCtxConfig->againTable[high]){
        *pCurrGain = pCtxConfig->againTable[high];
        return 0;
    }

    int tmpStep = 0;
    while(low <= high){
        int mid = low + (high - low)/2;
        float midVal = pCtxConfig->againTable[mid];
        if(midVal> *pCurrGain) {
            tmpStep = mid;
            high = mid - 1;
        }else if(midVal< *pCurrGain) {
            low = mid + 1;
        }else if (midVal == *pCurrGain) {
            tmpStep = mid;
            break;
        }
    }
    *pCurrGain = pCtxConfig->againTable[tmpStep];

    return (result);
}


AX_S32 sample_ae_check_data_valid
(
    AX_U8 pipe,
    SAMPLE_AEC_CONFIG_T *pConvertConfig,
    AX_U8 setFlag
)
{
    AX_S32 result = 0;

    // Update Gain Limits
    AX_SNS_PARAMS_T tSnsParam;
    result = sample_ae_sns_get_param(pipe, &tSnsParam);

    //Dont need ae route
    //Dont need dynamic ratio

    //Match input gain to sensor gain.
    result = sample_ae_ideal_again_to_real_again(pipe, &(pConvertConfig->minUserAgain));
    result = sample_ae_ideal_again_to_real_again(pipe, &(pConvertConfig->maxUserAgain));
    if (result != 0)
    {
        printf("In Exposure Control, Input Gain Match Sensor Gain failed!");
        return -1;
    }

    //Match again hcg/lcg th to sensor gain.
    result = sample_ae_ideal_again_to_real_again(pipe, &(pConvertConfig->againTh_hcg2lcg));
    result = sample_ae_ideal_again_to_real_again(pipe, &(pConvertConfig->againTh_lcg2hcg));
    if (result != 0)
    {
        printf("In Exposure Control, HCG/LCG Match Sensor Gain failed!");
        return -1;
    }

    //check again switch ratio: should not be 0
    if(pConvertConfig->againRatio_hcg2lcg == 0) {
        pConvertConfig->againRatio_hcg2lcg = 0.1;
    }
    if(pConvertConfig->againRatio_lcg2hcg == 0) {
        pConvertConfig->againRatio_lcg2hcg = 0.1;
    }

    // check not support sensor lcg2hcg ratio, should be 1
    AX_U32 hcgEn = 0;
    AX_U32 hcgStatus = 0;
    AX_F32 hcgRatio = 1.0f;
    sample_ae_sns_get_hcg_status_ratio(pipe, &hcgEn, &hcgRatio, &hcgStatus);
    if (0 == hcgEn) {
        pConvertConfig->againRatio_hcg2lcg = 1;
        pConvertConfig->againRatio_lcg2hcg = 1;
    }

    AX_F32 lumaDev = 0;
    AX_F32 dampRatio= 0;
    for(int i = 0; i < pConvertConfig->dampInfo.nDampOverListSize - 1; i ++){
        for(int j = 0; j < pConvertConfig->dampInfo.nDampOverListSize-1-i; j ++){
            if(pConvertConfig->dampInfo.fDampOverLumaDevList[j] > pConvertConfig->dampInfo.fDampOverLumaDevList[j+1]){
                lumaDev = pConvertConfig->dampInfo.fDampOverLumaDevList[j+1];
                pConvertConfig->dampInfo.fDampOverLumaDevList[j+1] = pConvertConfig->dampInfo.fDampOverLumaDevList[j];
                pConvertConfig->dampInfo.fDampOverLumaDevList[j] = lumaDev;

                dampRatio = pConvertConfig->dampInfo.fDampOverRatioList[j+1];
                pConvertConfig->dampInfo.fDampOverRatioList[j+1] = pConvertConfig->dampInfo.fDampOverRatioList[j];
                pConvertConfig->dampInfo.fDampOverRatioList[j] = dampRatio;
            }
        }
    }

    for(int i = 0; i < pConvertConfig->dampInfo.nDampUnderListSize - 1; i ++){
        for(int j = 0; j < pConvertConfig->dampInfo.nDampUnderListSize-1-i; j ++){
            if(pConvertConfig->dampInfo.fDampUnderLumaDevList[j] > pConvertConfig->dampInfo.fDampUnderLumaDevList[j+1]){
                lumaDev = pConvertConfig->dampInfo.fDampUnderLumaDevList[j+1];
                pConvertConfig->dampInfo.fDampUnderLumaDevList[j+1] = pConvertConfig->dampInfo.fDampUnderLumaDevList[j];
                pConvertConfig->dampInfo.fDampUnderLumaDevList[j] = lumaDev;

                dampRatio = pConvertConfig->dampInfo.fDampUnderRatioList[j+1];
                pConvertConfig->dampInfo.fDampUnderRatioList[j+1] = pConvertConfig->dampInfo.fDampUnderRatioList[j];
                pConvertConfig->dampInfo.fDampUnderRatioList[j] = dampRatio;
            }
        }
    }

    return result;
}

AX_S32 sample_ae_init_ctx_config
(
    AX_U8 pipe,
    SAMPLE_AEC_CONFIG_T *pCtxConfig
){

    AX_S32 result = 0;
    AX_SNS_AE_GAIN_TABLE_T tTableParam;
    result = sample_ae_sns_get_gain_table(pipe, &tTableParam);
    if (result != 0)
    {
        printf("Get Gain Table Failed!");
    }

    pCtxConfig->againTableSize = tTableParam.nAgainTableSize;
    for (int i = 0; i < pCtxConfig->againTableSize; i++) {
        pCtxConfig->againTable[i] = tTableParam.pAgainTable[i];
    }

    pCtxConfig->ecmTflicker = AX_TFLICKER_100HZ;

    //Dont Init Ae Route Curve Param

    //Dont Init SetPoint Param

    //Init Time Smooth Param
    pCtxConfig->snsDelayFrame       = 0;
    pCtxConfig->ratioCountNum       = 3;
    pCtxConfig->ratioClipUpperLimit = 8;
    pCtxConfig->ratioClipLowerLimit = 0.125;

    pCtxConfig->dampInfo.nDampOverListSize = 2;
    pCtxConfig->dampInfo.fDampOverLumaDevList[0] = 0.1;
    pCtxConfig->dampInfo.fDampOverRatioList[0] = 0.9;
    pCtxConfig->dampInfo.fDampOverLumaDevList[1] = 0.6;
    pCtxConfig->dampInfo.fDampOverRatioList[1] = 0.9;

    pCtxConfig->dampInfo.nDampUnderListSize = 2;
    pCtxConfig->dampInfo.fDampUnderLumaDevList[0] = 0.1;
    pCtxConfig->dampInfo.fDampUnderRatioList[0] = 0.9;
    pCtxConfig->dampInfo.fDampUnderLumaDevList[1] = 0.8;
    pCtxConfig->dampInfo.fDampUnderRatioList[1] = 0.9;

    pCtxConfig->tolerance = 20.000000 / 100.0f;
    pCtxConfig->targetLuma = 40;

    pCtxConfig->againTh_lcg2hcg = 15.5;
    pCtxConfig->againTh_hcg2lcg = 1.18;

    pCtxConfig->againRatio_hcg2lcg = 4.05;
    pCtxConfig->againRatio_lcg2hcg = 4.05;

    pCtxConfig->maxUserAgain = 15.5;
    pCtxConfig->minUserAgain = 1;


    // NO AE sync param;
    pCtxConfig->luxK =179120;

    pCtxConfig->maxIspGain   = 15.99;
    pCtxConfig->minIspGain   = 1;
    pCtxConfig->maxUserDgain = 4;
    pCtxConfig->minUserDgain = 1;
    pCtxConfig->maxUserShutter   = 33000;
    pCtxConfig->minUserShutter   = 1000;

    // No Init gridweight.

    //Initialize Hardware limit: integration，time，and gain limit
    AX_SNS_PARAMS_T tSnsParam;
    result = sample_ae_sns_get_param(pipe, &tSnsParam);

    //Init Hardware limit.
    pCtxConfig->IntegrationTimeLimit.nMax =  tSnsParam.sns_ae_limit.nMaxIntegrationTime[AXAE_LONG_FRAME];
    pCtxConfig->IntegrationTimeLimit.nMin =  tSnsParam.sns_ae_limit.nMinIntegrationTime[AXAE_LONG_FRAME];
    pCtxConfig->IntegrationTimeLimit.nStep = tSnsParam.sns_ae_param.nIntegrationTimeIncrement[AXAE_LONG_FRAME];

    pCtxConfig->AgainLimit.fMax = tSnsParam.sns_ae_limit.nMaxAgain[AXAE_LONG_FRAME];
    pCtxConfig->AgainLimit.fMin = tSnsParam.sns_ae_limit.nMinAgain[AXAE_LONG_FRAME];
    pCtxConfig->AgainLimit.fStep = tSnsParam.sns_ae_param.nAGainIncrement[AXAE_LONG_FRAME];

    pCtxConfig->DgainLimit.fMax = tSnsParam.sns_ae_limit.nMaxDgain[AXAE_LONG_FRAME];
    pCtxConfig->DgainLimit.fMin = tSnsParam.sns_ae_limit.nMinDgain[AXAE_LONG_FRAME];
    pCtxConfig->DgainLimit.fStep = tSnsParam.sns_ae_param.nDGainIncrement[AXAE_LONG_FRAME];

    pCtxConfig->IspDgainLimit.fMax = 64.0f;
    pCtxConfig->IspDgainLimit.fMin = 1.0f;
    pCtxConfig->IspDgainLimit.fStep = tSnsParam.sns_ae_param.nIspDGainIncrement[AXAE_LONG_FRAME];

    pCtxConfig->sIntegrationTime.nMax =  tSnsParam.sns_ae_limit.nMaxIntegrationTime[AXAE_MEDIUM_FRAME];
    pCtxConfig->sIntegrationTime.nMin =  tSnsParam.sns_ae_limit.nMinIntegrationTime[AXAE_MEDIUM_FRAME];
    pCtxConfig->sIntegrationTime.nStep = tSnsParam.sns_ae_param.nIntegrationTimeIncrement[AXAE_MEDIUM_FRAME];

    pCtxConfig->sGain.fMax = tSnsParam.sns_ae_limit.nMaxAgain[AXAE_MEDIUM_FRAME];
    pCtxConfig->sGain.fMin = tSnsParam.sns_ae_limit.nMinAgain[AXAE_MEDIUM_FRAME];
    pCtxConfig->sGain.fStep = tSnsParam.sns_ae_param.nAGainIncrement[AXAE_MEDIUM_FRAME];

    pCtxConfig->sDgain.fMax = tSnsParam.sns_ae_limit.nMaxDgain[AXAE_MEDIUM_FRAME];
    pCtxConfig->sDgain.fMin = tSnsParam.sns_ae_limit.nMinDgain[AXAE_MEDIUM_FRAME];
    pCtxConfig->sDgain.fStep = tSnsParam.sns_ae_param.nDGainIncrement[AXAE_MEDIUM_FRAME];

    pCtxConfig->sVsintegrationTime.nMax =  tSnsParam.sns_ae_limit.nMaxIntegrationTime[AXAE_SHORT_FRAME];
    pCtxConfig->sVsintegrationTime.nMin =  tSnsParam.sns_ae_limit.nMinIntegrationTime[AXAE_SHORT_FRAME];
    pCtxConfig->sVsintegrationTime.nStep = tSnsParam.sns_ae_param.nIntegrationTimeIncrement[AXAE_SHORT_FRAME];

    pCtxConfig->sVsgain.fMax = tSnsParam.sns_ae_limit.nMaxAgain[AXAE_SHORT_FRAME];
    pCtxConfig->sVsgain.fMin = tSnsParam.sns_ae_limit.nMinAgain[AXAE_SHORT_FRAME];
    pCtxConfig->sVsgain.fStep = tSnsParam.sns_ae_param.nAGainIncrement[AXAE_SHORT_FRAME];

    pCtxConfig->sVsdgain.fMax = tSnsParam.sns_ae_limit.nMaxDgain[AXAE_SHORT_FRAME];
    pCtxConfig->sVsdgain.fMin = tSnsParam.sns_ae_limit.nMinDgain[AXAE_SHORT_FRAME];
    pCtxConfig->sVsdgain.fStep = tSnsParam.sns_ae_param.nDGainIncrement[AXAE_SHORT_FRAME];

    pCtxConfig->fHwMinHdrRatio   = tSnsParam.sns_ae_limit.nMinratio;
    pCtxConfig->fHwMaxHdrRatio   = tSnsParam.sns_ae_limit.nMaxratio;
    pCtxConfig->fTotalGainMin    = pCtxConfig->AgainLimit.fMin * pCtxConfig->DgainLimit.fMin * pCtxConfig->IspDgainLimit.fMin;
    pCtxConfig->fTotalGainMax    = pCtxConfig->AgainLimit.fMax * pCtxConfig->DgainLimit.fMax * pCtxConfig->IspDgainLimit.fMax * tSnsParam.sns_ae_param.nSnsHcgLcgRatio;
    pCtxConfig->fSnsTotalGainMin = pCtxConfig->AgainLimit.fMin * pCtxConfig->DgainLimit.fMin;
    pCtxConfig->fSnsTotalGainMax = pCtxConfig->AgainLimit.fMax * pCtxConfig->DgainLimit.fMax* tSnsParam.sns_ae_param.nSnsHcgLcgRatio;

    //gain mode: aGain_only; aGain_dGain_both; aGain_dGain_combined
    pCtxConfig->uGainMode = tSnsParam.sns_ae_param.nGainMode;

    //hcglcg enable and ratio status
    AX_U32 hcgEn = 0;
    AX_U32 hcgStatus = 0;
    AX_F32 hcgRatio = 1.0f;
    sample_ae_sns_get_hcg_status_ratio(pipe, &hcgEn, &hcgRatio, &hcgStatus);
    pCtxConfig->uLcghcgEn = hcgEn;
    pCtxConfig->fHcglcgratio = tSnsParam.sns_ae_param.nSnsHcgLcgRatio;

    //Max Min exposure
    pCtxConfig->fExpValLimitMax = pCtxConfig->IntegrationTimeLimit.nMax *
                                  pCtxConfig->AgainLimit.fMax * pCtxConfig->DgainLimit.fMax *
                                  pCtxConfig->IspDgainLimit.fMax * pCtxConfig->fHcglcgratio;
    pCtxConfig->fExpValLimitMin = pCtxConfig->IntegrationTimeLimit.nMin *
                                  pCtxConfig->AgainLimit.fMin * pCtxConfig->DgainLimit.fMin *
                                  pCtxConfig->IspDgainLimit.fMin;

    //Min gain and exposure
    pCtxConfig->minGain = pCtxConfig->minUserDgain  * pCtxConfig->minIspGain * pCtxConfig->minUserAgain;

    //Max gain and exposure
    pCtxConfig->maxGain = pCtxConfig->maxUserDgain  * pCtxConfig->maxIspGain *
                          pCtxConfig->maxUserAgain * pCtxConfig->againRatio_lcg2hcg;

    pCtxConfig->nCoverFrameThres = 5;

    pCtxConfig->ispGain = 1;

    /* NO hdr ratio dynamic*/
    pCtxConfig->hdrRatio = 16;

    AX_U32 hdrMode = AX_SNS_LINEAR_MODE;
    result = sample_ae_sns_get_hdr_mode(pipe, &hdrMode);
    if (result != 0) {
        printf("Get HDR Mode Failed!");
    }
    pCtxConfig->eSnsMode = hdrMode;

    return result;
}


AX_S32 sample_ae_init_ctx_status
(
    AX_U8 pipe,
    SAMPLE_AEC_CONFIG_T *pAlgConfig,
    SAMPLE_AEC_STATUS_T *pAlgStatus

){
    AX_S32 result = 0;
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);

    pAlgStatus->meanLuma = 40.0f;

    AX_SNS_PARAMS_T tSnsParam;
    result = sample_ae_sns_get_param(pipe, &tSnsParam);
    AX_F32 manualhcgRatio = 0;
    manualhcgRatio = tSnsParam.sns_ae_param.nHcgLcgRatio;

    pAlgStatus->currHcgLcg   = tSnsParam.sns_dev_attr.eSnsHcgLcg;
    pAlgStatus->currHcgRatio = manualhcgRatio;

    pAlgStatus->ispGain = pAlgConfig->ispGain;

    for (AX_S32 i = 0; i < MAX_AE_BLOCK_NUM ; i++) {
        pAlgStatus->AxStatistics[i][0] = 0;
        pAlgStatus->AxStatistics[i][1] = 0;
        pAlgStatus->AxStatistics[i][2] = 0;
        pAlgStatus->AxStatistics[i][3] = 0;
    }

    for (int i = 0; i < AE_ISPGAIN_MEMORY; i++) {
        pAlgStatus->AxPreIspGain[i] = 1.0;
    }

    for (int i = 0; i < AE_ISPGAIN_MEMORY; i++) {
        pAlgStatus->AxPreEV[i] = 30000;
    }

    for (int i = 0; i < AE_ISPGAIN_MEMORY; i++) {
        pAlgStatus->AxPreAlgoEV[i] = 30000;
    }

    for (int i = 0; i < AE_RATIO_MEMORY; i++) {
        pAlgStatus->AxPreRatio[i] = 1;
    }

    result = sample_ae_upd_ctx_exp_settings(pAlgCtx, pipe);

    return result;
}


AX_S32 sample_ae_init(AX_U8 pipe, AX_ISP_AE_INITATTR_T *pParam)
{
    AX_S32 result = 0;
    AX_S32 nRet = 0;

    // Update Gain Limits
    AX_SNS_PARAMS_T tSnsParam = {0};

    nRet = AX_VIN_GetSnsAttr(pipe, &tSnsParam.sns_dev_attr);
    if (0 != nRet) {
        COMM_ISP_PRT("AX_VIN_GetSnsAttr failed, nRet=0x%x.\n", nRet);
          return -1;
    }

    tSnsParam.sns_ae_limit.nMingain = 1.000000;
    tSnsParam.sns_ae_limit.nMaxgain = 991.380005;
    tSnsParam.sns_ae_limit.nMingain = 1.000000;
    tSnsParam.sns_ae_limit.nMingain = 1.000000;
    tSnsParam.sns_ae_limit.nMingain = 1.000000;

    for(int i = 0 ;i<tSnsParam.sns_dev_attr.eSnsMode;i++){
        tSnsParam.sns_ae_limit.nMinAgain[i] = 1.0f;
        tSnsParam.sns_ae_limit.nMaxAgain[i] = 25.5f;
        tSnsParam.sns_ae_limit.nMinDgain[i] = 1.0f;
        tSnsParam.sns_ae_limit.nMaxDgain[i] = 25.5f;
        tSnsParam.sns_ae_limit.nMinIntegrationTime[i] = 20;
        tSnsParam.sns_ae_limit.nMaxIntegrationTime[i] = 33000;
        tSnsParam.sns_ae_param.nCurAGain[i] = 2.0f;
        tSnsParam.sns_ae_param.nCurDGain[i] = 1.0f;
        tSnsParam.sns_ae_param.nCurIspDGain[i] = 1.0f;
        tSnsParam.sns_ae_param.nCurIntegrationTime[i] = 10000;
        tSnsParam.sns_ae_param.nAGainIncrement[i] = 1.0f;
        tSnsParam.sns_ae_param.nDGainIncrement[i] = 1.0f;
        tSnsParam.sns_ae_param.nIspDGainIncrement[i] = 1.0f;
        tSnsParam.sns_ae_param.nIntegrationTimeIncrement[i] = 10;

    }

    tSnsParam.sns_ae_limit.nMinratio = 5.0f;
    tSnsParam.sns_ae_limit.nMaxratio = 16.0f;
    tSnsParam.sns_ae_limit.nInitMinIntegrationTime = 20;
    tSnsParam.sns_ae_limit.nInitMaxIntegrationTime = 33000;

    tSnsParam.sns_ae_param.nGainMode = AX_ADGAIN_SEPARATION;

    tSnsParam.sns_ae_param.nSnsHcgLcgRatio = 4.05f;
    tSnsParam.sns_ae_param.nHcgLcgRatio = 0.1;

    tSnsParam.sns_ae_param.nHcgLcgRatio =4.05;

    if (pipe >= DEF_VIN_PIPE_MAX_NUM) {
        return -1;
    }

    result = pSnsHandle[pipe]->pfn_sensor_set_params(pipe, &tSnsParam);

    /* Step1: Get the AE Algo Context Handler of the Current Sensor. */
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    SAMPLE_AEC_CONFIG_T *pCtxConfig  = &(pAlgCtx->tConfig);
    SAMPLE_AEC_STATUS_T *pCtxStatus  = &(pAlgCtx->tStatus);

    /* Step2: Most of the tuning Info Come from online XML, or Hard-Code here. */
    pAlgCtx->bEnable = 1;

    result = sample_ae_init_ctx_config(pipe, pCtxConfig);
    if (0 != result) {
        printf("SAMPLE AE Init Config Failed!\n");
        return -1;
    }

    //Date valid check
    AX_U8 setFlag = 0;
    result = sample_ae_check_data_valid(pipe, pCtxConfig, setFlag);
    if (result != 0)
    {
        printf("SAMPLE AE Check Data Failed!");
        return -1;
    }

    // set again, dgain, uHcg_lcg, shutter
    pCtxStatus->snsDgain[AX_HDR_CHN_L] = 1;
    pCtxStatus->snsRegAgain[AX_HDR_CHN_L] = 5;
    pCtxStatus->snsShutter[AX_HDR_CHN_L] = 10000;    // 10ms
    pCtxStatus->totalGain[AX_HDR_CHN_L] = 5.0;

    pCtxStatus->currHcgLcg = 1;

    /* Step3: init sensor. */
    result = sample_ae_init_sensor(pipe);
    if (0 != result) {
        printf("SAMPLE AE Init Sensor Failed!\n");
        return -1;
    }

    /* Step4: Initialize Dynamic Information. */
    result = sample_ae_init_ctx_status(pipe, pCtxConfig, pCtxStatus);
    if (0 != result) {
        printf("SAMPLE AE Init Status Failed!\n");
        return -1;
    }

    printf("sample_ae_init done\n");

    return 0;
}


AX_S32 sample_ae_deinit(AX_U8 pipe)
{
    printf("sample_ae_deinit done!\n");
    return 0;
}


static AX_S32 sample_ae_update_luma_and_hist
(
    AX_U8                   pipe,
    const AX_ISP_AE_INPUT_INFO_T *pAeInputInfo,
    SAMPLE_AEC_STATUS_T      *pCtxStatus
)
{
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    AX_S32 result = 0;
    AX_AE_GRID_STATS const *luma_data;
    AX_U32 nZoneRow = 0;
    AX_U32 nZoneCol = 0;
    AX_S32 aeBlockNum = 0;

    for (int i = 0; i < AX_AE_HIST_LINEAR_BIN; i++) {
        pCtxStatus->longHist[i]        = 0;
        pCtxStatus->longCumulaHist[i]  = 0;
        pCtxStatus->shortHist[i]       = 0;
        pCtxStatus->shortCumulaHist[i] = 0;
    }

    /* FIXME: need add hdr support */
    luma_data = pAeInputInfo->sAeStat.tAeGridStat[0].tGridStats;
    nZoneRow = pAeInputInfo->sAeStat.tAeGridStat[0].nZoneRowSize;
    nZoneCol = pAeInputInfo->sAeStat.tAeGridStat[0].nZoneColSize;
    pAlgCtx->tStatus.nGridRow = nZoneRow;
    pAlgCtx->tStatus.nGridCol = nZoneCol;
    aeBlockNum = nZoneRow * nZoneCol;
    if (aeBlockNum > MAX_AE_BLOCK_NUM)
    {
        printf("aeBlockNum exceeds, axae_update_luma_and_hist failed!");
        return -1;
    }

    //Calculate current meanluma:luma = 0.299 * R + 0.2935 * (Gr + Gb) + 0.114 * B.
    double sumLuma = 0.0f;
    for (AX_S32 i = 0; i < nZoneRow; i++) {
        for (AX_S32 j = 0; j < nZoneCol; j++) {
            AX_F32 tempR_sum = (float)(luma_data[i * nZoneCol + j].nGridSum[0] >> 2) +
                               (luma_data[i * nZoneCol + j].nGridSum[0] & 0x03) / 4.0;
            AX_F32 tempB_sum = (float)(luma_data[i * nZoneCol + j].nGridSum[1] >> 2) +
                               (luma_data[i * nZoneCol + j].nGridSum[1] & 0x03) / 4.0;
            AX_F32 tempGr_sum = (float)(luma_data[i * nZoneCol + j].nGridSum[2] >> 2) +
                                (luma_data[i * nZoneCol + j].nGridSum[2] & 0x03) / 4.0;
            AX_F32 tempGb_sum = (float)(luma_data[i * nZoneCol + j].nGridSum[3] >> 2) +
                                (luma_data[i * nZoneCol + j].nGridSum[3] & 0x03) / 4.0;
            AX_F32 tempR  = (float)(tempR_sum / AXAE_DIV_0_TO_1(luma_data[i * nZoneCol + j].nGridNum[0] * 1.0f));
            AX_F32 tempB = (float)(tempB_sum / AXAE_DIV_0_TO_1(luma_data[i * nZoneCol + j].nGridNum[1] * 1.0f));
            AX_F32 tempGr = (float)(tempGr_sum / AXAE_DIV_0_TO_1(luma_data[i * nZoneCol + j].nGridNum[2] * 1.0f));
            AX_F32 tempGb  = (float)(tempGb_sum / AXAE_DIV_0_TO_1(luma_data[i * nZoneCol + j].nGridNum[3] * 1.0f));

            //Ignore over-Exposured pixels and calculate sumLuma.
            if (tempR  > 243)  tempR = 243;
            if (tempGr > 243)  tempGr = 243;
            if (tempGb > 243)  tempGb = 243;
            if (tempB  > 243)  tempB = 243;
            AX_F32 tempY = (0.299 * tempR + 0.2935 * (tempGr + tempGb) + 0.114 * tempB);
            sumLuma += tempY;
        }
    }

    pCtxStatus->meanLuma = sumLuma / (nZoneRow * nZoneCol);


    return result;
}


static AX_S32 sample_ae_get_dampratio_by_refval
(
    const SAMPLE_AE_DAMP_INFO_T *pDampInfo,
    AX_F32                           refVal,
    AX_F32                           *pDampRatio,
    AX_U8                            dampMode    /* 0 : dampOver, 1: dampUnder */
)
{

    int ind = -1;

    if(dampMode == 0){
        if(refVal <= pDampInfo->fDampOverLumaDevList[0])
        {
            *pDampRatio = pDampInfo->fDampOverRatioList[0];
            return 0;
        }

        if(refVal >= pDampInfo->fDampOverLumaDevList[pDampInfo->nDampOverListSize - 1])
        {
            *pDampRatio = pDampInfo->fDampOverRatioList[pDampInfo->nDampOverListSize - 1];
            return 0;
        }

        for(int i = 1; i < pDampInfo->nDampOverListSize; ++i){
            if(refVal <= pDampInfo->fDampOverLumaDevList[i]){
                ind = i;
                break;
            }
        }

        float ratio = (refVal - pDampInfo->fDampOverLumaDevList[ind - 1]) /
                      (pDampInfo->fDampOverLumaDevList[ind] - pDampInfo->fDampOverLumaDevList[ind - 1]);
        *pDampRatio = pDampInfo->fDampOverRatioList[ind] * ratio +
                      pDampInfo->fDampOverRatioList[ind - 1] * (1- ratio);
    }else{
        if(refVal <= pDampInfo->fDampUnderLumaDevList[0])
        {
            *pDampRatio = pDampInfo->fDampUnderRatioList[0];
            return 0;
        }

        if(refVal >= pDampInfo->fDampUnderLumaDevList[pDampInfo->nDampUnderListSize - 1]){
            *pDampRatio = pDampInfo->fDampUnderRatioList[pDampInfo->nDampUnderListSize - 1];
            return 0;
        }

        for(int i = 1; i < pDampInfo->nDampUnderListSize; ++i){
            if(refVal <= pDampInfo->fDampUnderLumaDevList[i]){
                ind = i;
                break;
            }
        }

        float ratio = (refVal - pDampInfo->fDampUnderLumaDevList[ind - 1]) /
                  (pDampInfo->fDampUnderLumaDevList[ind] - pDampInfo->fDampUnderLumaDevList[ind - 1]);
        *pDampRatio = pDampInfo->fDampUnderRatioList[ind] * ratio +
                      pDampInfo->fDampUnderRatioList[ind - 1] * (1- ratio);
    }

    return 0;
}

/*
 * @AxPreEV： Previous sensor EV Result (after sensor quantization)
 *
 * @AxPreAlgoEV：Previous algo EV Result (before sensor quantization)
 *
 */
static AX_S32 sample_ae_calc_ideal_newEV_upd_converge
(
    AX_U8 pipe,
    const SAMPLE_AEC_CONFIG_T *pCtxConfig,
    SAMPLE_AEC_STATUS_T       *pCtxStatus,
    AX_F32                    *pNewEV,
    AX_F32                    curMeanLuma
)
{
    AX_S32 result = 0;

    AX_F32 ratio = 1.0f;

    // Calculate ratio between currnt meanluma and target luma.
    AX_F32 lumaDev = fabs(pCtxConfig->targetLuma - curMeanLuma) / (pCtxConfig->targetLuma + 0.000001);
    AX_F32 tolerance = pCtxConfig->tolerance;
    if (lumaDev > tolerance) {
        ratio =  pCtxConfig->targetLuma / (curMeanLuma  + 0.000001);
        pCtxStatus->nConverFrameCnt = 0;
    } else {
        pCtxStatus->nConverFrameCnt += 1;
    }

    AX_U8 snsDelayFrame = pCtxConfig->snsDelayFrame;
    for (int i = 0; i< snsDelayFrame; i++)
    {
        pCtxStatus->AxPreEV[i] = pCtxStatus->AxPreEV[i+1];
    }
    pCtxStatus->AxPreEV[snsDelayFrame] = pCtxStatus->totalExposure[AX_HDR_CHN_L];

    //Adjust cur exposure to target exposure val.
    AX_F32 newEV = ratio * pCtxStatus->AxPreEV[0];

    if (newEV > pCtxStatus->maxExposure) {
        newEV = pCtxStatus->maxExposure;
    } else if (newEV < pCtxStatus->minExposure) {
        newEV = pCtxStatus->minExposure;
    }

    // Get damp Ratio for time smooth
    AX_U8  dampMode;   /* 0 : dampOver, 1: dampUnder */
    AX_F32 dampRatio;
    AX_F32 lumaDiff;

    if (curMeanLuma > pCtxConfig->targetLuma) {
        dampMode = 0;
        AX_F32 target_U = pCtxConfig->targetLuma * (1 + tolerance);
        lumaDiff = fabs(target_U - curMeanLuma) / (target_U + 0.000001);
    } else {
        dampMode = 1;
        AX_F32 target_L = pCtxConfig->targetLuma * (1 - tolerance);
        lumaDiff = fabs(target_L - curMeanLuma) / (target_L + 0.000001);
    }
    result = sample_ae_get_dampratio_by_refval(&(pCtxConfig->dampInfo), lumaDiff,
                                          &dampRatio, dampMode);

    /*damp with  algo EV Result (before sensor quantization)*/
    newEV = newEV * (1.0f - dampRatio) + (pCtxStatus->AxPreAlgoEV[0] * dampRatio);

    AX_F32 ratioAfterDamp = newEV/(pCtxStatus->AxPreEV[0] + 0.000001);

    //Adjust new EV use muti frame ratio clip strategy
    AX_F32 ratioCountVal = 1;
    AX_S32 ratioCountNum = pCtxConfig->ratioCountNum;  /* need larger than 1 */
    if( pCtxConfig->ratioCountNum < 1){
        ratioCountNum = 1;
    }

    for (int i = 0; i< (ratioCountNum -1); i++)
    {
        pCtxStatus->AxPreRatio[i] = pCtxStatus->AxPreRatio[i+1];
        ratioCountVal *= pCtxStatus->AxPreRatio[i];
    }

    pCtxStatus->AxPreRatio[ratioCountNum - 1] = ratioAfterDamp;
    ratioCountVal *= pCtxStatus->AxPreRatio[ratioCountNum - 1];

    AX_F32 th_U = pCtxConfig->ratioClipUpperLimit;
    if ( ratioCountVal > th_U )
    {
        ratioAfterDamp = fmin(pow(th_U, 1.0f/ratioCountNum), ratioAfterDamp);
    }
    AX_F32 th_L = pCtxConfig->ratioClipLowerLimit;
    if ( ratioCountVal < th_L )
    {
        ratioAfterDamp = fmax(pow(th_L, 1.0f/ratioCountNum), ratioAfterDamp);
    }

    //Recalc newEv
    newEV = ratioAfterDamp * pCtxStatus->AxPreEV[0];

    // Update conerage status.
    if (pCtxStatus->nConverFrameCnt >= pCtxConfig->nCoverFrameThres) {
        pCtxStatus->isConverge = true;
    } else {
        pCtxStatus->isConverge = false;
    }

    //Update the Smoothed New Exposure Value.
    *pNewEV = newEV;

    for (int i = 0; i< snsDelayFrame; i++)
    {
        pCtxStatus->AxPreAlgoEV[i] = pCtxStatus->AxPreAlgoEV[i+1];
    }
    pCtxStatus->AxPreAlgoEV[snsDelayFrame] = newEV;

    return result;
}


/* Ensure sensor shutter meet user setting */
AX_S32 sample_ae_calc_gain_shutter_with_shutter_increment_prior
(
    const SAMPLE_AEC_CONFIG_T *pCtxConfig,
    AX_F32 Alpha,
    AX_F32 maxGain,
    AX_F32 minGain,
    AX_U32 maxShutter,
    AX_U32 minShutter,
    AX_F32 *pSplitTotalGain,
    AX_U32 *pSplitIntegrationTime
)
{
    AX_F32 minshutter_fromgain = Alpha / maxGain;
    AX_F32 maxshutter_fromgain = Alpha / minGain;
    AX_F32 Tint;
    AX_F32 gain;

    // underexposure
    if( maxShutter <= minshutter_fromgain ) {
        Tint = maxShutter;
        gain = Alpha / Tint;
        //clip gain
        if (gain < minGain) {
                gain = minGain;
        }
        if (gain > maxGain ) {
                gain = maxGain;
        }
        *pSplitTotalGain = gain;
        *pSplitIntegrationTime = Tint;
        return 0;
    }

    // overexposure
    if( maxshutter_fromgain <= minShutter ) {
        Tint = minShutter;
        gain = Alpha / Tint;
        //clip gain
        if (gain < minGain) {
                gain = minGain;
        }
        if (gain > maxGain ) {
                gain = maxGain;
        }
        *pSplitTotalGain = gain;
        *pSplitIntegrationTime = Tint;
        return 0;
    }

    AX_F32 min_shutter_th = max(minShutter,minshutter_fromgain);
    AX_F32 max_shutter_th = min(maxShutter,maxshutter_fromgain);

    // judge antiflicker or not
    if ( pCtxConfig->antiFlickerParam.enable == AX_ISP_ANTI_FLICKER_OFF)
    {
        Tint = max_shutter_th;
    }else{
        AX_U32 Tmin_antiflicker = floorf(min_shutter_th / pCtxConfig->ecmTflicker);
        AX_U32 Tmax_antiflicker = floorf(max_shutter_th / pCtxConfig->ecmTflicker);
        AX_U32 diff = Tmax_antiflicker - Tmin_antiflicker;
        if( diff >= 1 ) {
            Tint = floorf(max_shutter_th / pCtxConfig->ecmTflicker) * pCtxConfig->ecmTflicker;
        }else if( Tmin_antiflicker * pCtxConfig->ecmTflicker == min_shutter_th ) {
            Tint = min_shutter_th;
        }else{
            Tint = max_shutter_th;
        }
    }

    //clip gain
    gain = Alpha / Tint;
    if (gain < minGain) {
        gain = minGain;
    }
    if (gain > maxGain ) {
        gain = maxGain;
    }

    *pSplitTotalGain = gain;
    *pSplitIntegrationTime = (AX_U32)Tint;
    return 0;

}

/*
 * @idealNewEV
 *  idealNewEV = SplitTotalGain * SplitIntegrationTime
 *             = SensorDgain * SensorRegisterAgain * CurrHcgRatio * IspGain * Shutter
 *
 * @pSplitTotalGain
 *  SplitTotalGain = SensorDgain * SensorRegisterAgain * CurrHcgRatio * IspGain
 *
 * @maxUserShutter: max shutter set by user
 *
 * @AlgoMaxShutter: current max shutter
 *  AlgoMaxShutter = min(hdrHwMaxShutter,maxUserShutter )
 *
 */

static AX_S32 sample_ae_split_gain_shutter
(
    AX_U8 pipe,
    const SAMPLE_AEC_CONFIG_T *pCtxConfig,
    AX_F32                     idealNewEV,
    AX_F32                     *pSplitTotalGain,
    AX_U32                     *pSplitIntegrationTime,
    AX_U32                      algoMaxShutter

)
{

    AX_S32  result = 0;

    if (pCtxConfig == NULL) {
        printf("pCtxConfig is NULL\n");
        return -1;
    }
    if (pSplitTotalGain == NULL) {
         printf("pSplitTotalGain is NULL\n");
        return -1;
    }
    if (pSplitIntegrationTime == NULL) {
        printf("pSplitIntegrationTime is NULL\n");
        return -1;
    }


    if (idealNewEV < 0)
        return -1;

    if (isnan(idealNewEV)) {
        idealNewEV = 0.00001f;
        printf("####clip the sensor to minimal value\n");
    }

    AX_F32 Alpha = idealNewEV;
    /* Only show shutter prior */
    result = sample_ae_calc_gain_shutter_with_shutter_increment_prior(pCtxConfig,Alpha,pCtxConfig->maxGain,pCtxConfig->minGain,
                                                    algoMaxShutter, pCtxConfig->minUserShutter,
                                                    pSplitTotalGain,pSplitIntegrationTime);

    return (result);
}

/*
 * @SplitGain
 *  SplitGain = SplitTotalGain
 *             = SensorDgain * SensorRegisterAgain * CurrHcgRatio * IspGain
 *
 * @pSnsRegAgain
 *  pSnsRegAgain = SensorRegisterAgain
 *
 * @pSnsDGain
 * pSnsDGain= SensorDgain
 *
 * @pIspGain
 *  pIspGain = IspGain
 *
 * @pSnsTotalRegGain
 *  pSnsTotalRegGain = SensorRegisterAgain * SensorDgain
 *
 * @hcglcg_mode 0: HCG mode; 1: LCG mode
 */
AX_S32 sample_ae_split_sns_isp_gain
(
    AX_U8 pipe,
    SAMPLE_AEC_CTX_T *pAlgCtx,
    AX_F32 SplitGain,
    AX_F32 *pSnsRegAgain,
    AX_F32 *pSnsDGain,
    AX_F32 *pIspGain,
    AX_F32 *pSnsTotalRegGain,
    AX_U32 *hcglcg_mode
)
{
    SAMPLE_AEC_CONFIG_T *pCtxConfig = &(pAlgCtx->tConfig);
    AX_U32 hcgEn = 0;
    AX_F32 hcgRatio = 1.0f;
    AX_F32 scaleRatio = 1.0f;
    sample_ae_sns_get_hcg_status_ratio(pipe, &hcgEn, &hcgRatio, hcglcg_mode);
    if (1 == hcgEn && 0 == *hcglcg_mode) {
        scaleRatio = hcgRatio;
    }

    *pSnsRegAgain = SplitGain / scaleRatio / pCtxConfig->minIspGain / pCtxConfig->minUserDgain;
    *pSnsDGain = pCtxConfig->minUserDgain;
    *pIspGain = pCtxConfig->minIspGain;
    *pSnsTotalRegGain = *pSnsRegAgain * *pSnsDGain;
    AX_F32 SnsTotalGain = SplitGain / scaleRatio;

    //Split SplitGainTrans to again, ispgain and dgain.
    //LCG mode, only use again,and clip again to userminagain
    if (1 == hcgEn && *hcglcg_mode == 1) {
        if (*pSnsRegAgain < pCtxConfig->minUserAgain){
            *pSnsRegAgain = pCtxConfig->minUserAgain;
            *pSnsTotalRegGain = *pSnsRegAgain * *pSnsDGain;
        }
        return 0;
    }

    //Not support mode, can use again, ispgain, dgain
    //When SnsTotalRegGain <= maxUserAgain, only use again, clip again to minuseragain
    if (0 == hcgEn && *pSnsRegAgain <= pCtxConfig->maxUserAgain) {
        if (*pSnsRegAgain < pCtxConfig->minUserAgain){
            *pSnsRegAgain = pCtxConfig->minUserAgain;
            *pSnsTotalRegGain = *pSnsRegAgain * *pSnsDGain;
        }
        return 0;
    }

    //HCG mode , can use again, ispgain, dgain.
    //When SnsTotalRegGain <= pCtxConfig->maxUserAgain, only use again.
    if (*pSnsRegAgain <= pCtxConfig->maxUserAgain) {
        return 0;
    }

    //HCG or Not support mode
    //When SnsTotalRegGain > maxUserAgain, clip pSnsRegAgain to maxUserAgain, and start to use ispgain.

    //When ispgain <= maxIspGain, only use again and ispgain.

    *pIspGain = SnsTotalGain / pCtxConfig->maxUserAgain / pCtxConfig->minUserDgain;
    *pSnsRegAgain = pCtxConfig->maxUserAgain;
    *pSnsTotalRegGain = *pSnsRegAgain * *pSnsDGain;

    //When ispgain < maxIspGain, clip ispgain to minuserispgain.
    if (*pIspGain < pCtxConfig->minIspGain){
        *pIspGain = pCtxConfig->minIspGain;
    }

    if (*pIspGain <= pCtxConfig->maxIspGain)
    {
        *pSnsTotalRegGain = *pSnsRegAgain * *pSnsDGain;
        return 0;
    }

    //When ispgain > maxIspGain, clip ispgain to maxispgain, and start to use dgain.

    *pSnsDGain = SnsTotalGain / pCtxConfig->maxIspGain / pCtxConfig->maxUserAgain ;
    *pIspGain = pCtxConfig->maxIspGain;

    //When dgain < minuserdgain, clip ispgain to minuserdgain.
    if (*pSnsDGain < pCtxConfig->minUserDgain){
        *pSnsDGain = pCtxConfig->minUserDgain;
    }

    //When dgain > maxUserDgain, clip dgain to maxUserDgain.
    if (*pSnsDGain >= pCtxConfig->maxUserDgain) {
        *pSnsDGain = pCtxConfig->maxUserDgain;
    }
    *pSnsTotalRegGain = *pSnsDGain * *pSnsRegAgain;

    return 0;
}


static AX_S32  sample_ae_auto_sns_ctrl
(
    SAMPLE_AEC_CTX_T     *pAlgCtx,
    AX_U8                 pipe,
    AX_F32                SplitGain,
    AX_F32                SplitIntegrationTime,
    AX_F32                suggestHdrRatio
)
{
    //Ensure obtain sensorfig configuration parameters.
    if (pAlgCtx == AX_NULL) {
        printf("pAlgCtx is AX_NULL!\n");
        return -1;
    }

    AX_S32 result                 = 0;
    SAMPLE_AEC_CONFIG_T *pCtxConfig = &(pAlgCtx->tConfig);

    AX_F32 snsRegAgain = 1.0f;
    AX_F32 snsDGain = 1.0f;
    AX_F32 ispGain = 1.0f;
    AX_F32 snsTotalRegGain = 1.0f;
    AX_U32 HcgLcg_Mode = 1;

    result = sample_ae_split_sns_isp_gain(pipe, pAlgCtx, SplitGain, &snsRegAgain, &snsDGain, &ispGain, &snsTotalRegGain,
                                     &HcgLcg_Mode);

    //HCG LCG control judge
    if ((snsRegAgain < pCtxConfig->againTh_lcg2hcg) &&
        (HcgLcg_Mode == AX_LCG_MODE)) {
        HcgLcg_Mode= AX_LCG_MODE;
    } else if ((snsRegAgain >= pCtxConfig->againTh_lcg2hcg) &&
               (HcgLcg_Mode == AX_LCG_MODE)) {
        HcgLcg_Mode = AX_HCG_MODE;
        snsRegAgain = snsRegAgain / pCtxConfig->againRatio_lcg2hcg;
        if (snsRegAgain > pCtxConfig->maxUserAgain){
            ispGain = ispGain * (snsRegAgain / pCtxConfig->maxUserAgain);
            snsRegAgain = pCtxConfig->maxUserAgain;
            if (ispGain > pCtxConfig->maxIspGain){
                snsDGain = snsDGain * (ispGain / pCtxConfig->maxIspGain);
                ispGain = pCtxConfig->maxIspGain;
            }
        }
    } else if ((snsRegAgain <= pCtxConfig->maxUserAgain ) &&
               (snsRegAgain >= pCtxConfig->againTh_hcg2lcg ) &&
               (HcgLcg_Mode == AX_HCG_MODE)) {
        HcgLcg_Mode = AX_HCG_MODE;
    } else if ((snsRegAgain < pCtxConfig->againTh_hcg2lcg) &&
               (HcgLcg_Mode == AX_HCG_MODE)) {
        HcgLcg_Mode = AX_LCG_MODE;
        snsRegAgain = snsRegAgain * pCtxConfig->againRatio_hcg2lcg;
        if (snsRegAgain < pCtxConfig->minUserAgain){
            snsRegAgain = pCtxConfig->minUserAgain;
        }
    } else if ((snsRegAgain > pCtxConfig->maxUserAgain ) &&
               (HcgLcg_Mode == AX_HCG_MODE)) {
        HcgLcg_Mode = AX_HCG_MODE;
    } else if ((snsRegAgain < pCtxConfig->maxUserAgain ) &&
               (HcgLcg_Mode == AX_HCG_MODE)) {
        HcgLcg_Mode = AX_HCG_MODE;
    } else {
        if (HcgLcg_Mode == AX_HCG_MODE || HcgLcg_Mode == AX_LCG_MODE ) {
        }
    }

    //Set ispgain
    AX_U8 snsDelayFrame = pCtxConfig->snsDelayFrame;
    for (int i = 0; i < snsDelayFrame; i++)
    {
        pAlgCtx->tStatus.AxPreIspGain[i] = pAlgCtx->tStatus.AxPreIspGain[i + 1];
    }
    pAlgCtx->tStatus.AxPreIspGain[snsDelayFrame] = ispGain;

    pAlgCtx->tStatus.ispGain = pAlgCtx->tStatus.AxPreIspGain[0];

    /* Because model need dgain to be 1.0 , so set dgain to 1.0*/
    snsDGain = pCtxConfig->minUserDgain;

    /* compensation again quantitative accuracy */

    if( snsRegAgain == pCtxConfig->maxUserAgain && HcgLcg_Mode == AX_HCG_MODE){
        /* When snsRegAgain get again limit， use ispgain compensation again quantitative accuracy. */
    }else{
        /* When snsRegAgain don't get again limit， use dgain compensation again quantitative accuracy. */
        AX_F32 sensorSupportAgain = 0.0f;
        result = sample_ae_get_snsRegAgain_after_sensor_quantization(pipe, snsRegAgain, &sensorSupportAgain);
        /* reset lcg/hcg mode after quantization again*/
        /*case 1: lcg -> hcg (since sensorSupportAgain < snsRegAgain, won't hit this case)*/
        /*case 2: hcg -> lcg */
        if((sensorSupportAgain < pCtxConfig->againTh_hcg2lcg) &&  (HcgLcg_Mode == AX_HCG_MODE)) {
            HcgLcg_Mode = AX_LCG_MODE;
            AX_F32 sensorSupportAgain_lcgMode = 0.0f;
            sensorSupportAgain = sensorSupportAgain * pCtxConfig->againRatio_hcg2lcg;
            if (sensorSupportAgain < pCtxConfig->minUserAgain){
                sensorSupportAgain = pCtxConfig->minUserAgain;
            }
            result = sample_ae_get_snsRegAgain_after_sensor_quantization(pipe, sensorSupportAgain, &sensorSupportAgain_lcgMode);
            /* reset dgain */
            snsDGain = ((snsRegAgain * pCtxConfig->againRatio_hcg2lcg) / sensorSupportAgain_lcgMode) * snsDGain;
            if(snsDGain > pCtxConfig->maxUserDgain){
                snsDGain = pCtxConfig->maxUserDgain;
            }
            snsRegAgain = sensorSupportAgain_lcgMode;
        }else{
            /*case 3: lcg hcg don't switch */
            /* reset dgain */
            snsDGain = (snsRegAgain / sensorSupportAgain) * snsDGain;
            if(snsDGain > pCtxConfig->maxUserDgain){
                snsDGain = pCtxConfig->maxUserDgain;
            }
            /* reset again */
            snsRegAgain = sensorSupportAgain;
        }
    }

    AX_SNS_AE_GAIN_CFG_T tGainTbl = {0};
    AX_SNS_AE_SHUTTER_CFG_T tShutterTbl = {0};
    // Set exposure(again, dgain and integration time).
    if (pCtxConfig->eSnsMode == AX_SNS_LINEAR_MODE) {
        // Set LCG / HCG
        sample_ae_sns_set_lcg_hcg(pipe, HcgLcg_Mode, &snsRegAgain);

        // Set Gains and Shutter
        tGainTbl.nGain[AXAE_LONG_FRAME] = snsRegAgain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_again(pipe, &tGainTbl);

        tGainTbl.nGain[AXAE_LONG_FRAME] = snsDGain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_dgain(pipe, &tGainTbl);

        tShutterTbl.nIntTime[AXAE_LONG_FRAME] = SplitIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_shutter(pipe, &tShutterTbl);
    } else if (pCtxConfig->eSnsMode == AX_SNS_HDR_2X_MODE) {
        // Set LCG / HCG
        sample_ae_sns_set_lcg_hcg(pipe, HcgLcg_Mode, &snsRegAgain);

        // Set Gains and Shutter
        tGainTbl.nGain[AXAE_LONG_FRAME] = snsRegAgain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = snsRegAgain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_again(pipe, &tGainTbl);

        tGainTbl.nGain[AXAE_LONG_FRAME] = snsDGain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = snsDGain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_dgain(pipe, &tGainTbl);

        tShutterTbl.nIntTime[AXAE_LONG_FRAME] = SplitIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        tShutterTbl.nIntTime[AXAE_MEDIUM_FRAME] = SplitIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_shutter(pipe, &tShutterTbl);
    } else if (pCtxConfig->eSnsMode == AX_SNS_HDR_3X_MODE) {

        // Set LCG / HCG
        sample_ae_sns_set_lcg_hcg(pipe, HcgLcg_Mode, &snsRegAgain);

        // Set Gains and Shutter
        tGainTbl.nGain[AXAE_LONG_FRAME] = snsRegAgain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = snsRegAgain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = suggestHdrRatio;
        tGainTbl.nGain[AXAE_SHORT_FRAME] = snsRegAgain;
        tGainTbl.nHdrRatio[AXAE_SHORT_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_again(pipe, &tGainTbl);

        tGainTbl.nGain[AXAE_LONG_FRAME] = snsDGain;
        tGainTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        tGainTbl.nGain[AXAE_MEDIUM_FRAME] = snsDGain;
        tGainTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = suggestHdrRatio;
        tGainTbl.nGain[AXAE_SHORT_FRAME] = snsDGain;
        tGainTbl.nHdrRatio[AXAE_SHORT_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_dgain(pipe, &tGainTbl);

        tShutterTbl.nIntTime[AXAE_LONG_FRAME] = SplitIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_LONG_FRAME] = suggestHdrRatio;
        tShutterTbl.nIntTime[AXAE_MEDIUM_FRAME] = SplitIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_MEDIUM_FRAME] = suggestHdrRatio;
        tShutterTbl.nIntTime[AXAE_SHORT_FRAME] = SplitIntegrationTime;
        tShutterTbl.nHdrRatio[AXAE_SHORT_FRAME] = suggestHdrRatio;
        sample_ae_sns_set_shutter(pipe, &tShutterTbl);
    }

    return result;
}


static AX_S32 sample_ae_upd_ctx_lux
(
    SAMPLE_AEC_CTX_T *pAlgCtx,
    AX_U8                 pipe
)
{
    AX_S32 result = 0;

    pAlgCtx->tStatus.lux =   pAlgCtx->tStatus.meanLuma * pAlgCtx->tConfig.luxK
                             / pAlgCtx->tStatus.totalExposure[AX_HDR_CHN_L];

    return result;
}


AX_S32 sample_ae_gen_api_out_put
(
    AX_U8 pipe,
    SAMPLE_AEC_STATUS_T     *pStatus,
    AX_ISP_AE_RESULT_T      *pAeResult
)
{
    pAeResult->nIntTime[AX_HDR_CHN_L]  = pStatus->snsShutter[AX_HDR_CHN_L];
    pAeResult->nAgain[AX_HDR_CHN_L]    = sample_ae_float_convert_to_int(pStatus->snsTotalAgain[AX_HDR_CHN_L], 22, 10, false);
    pAeResult->nDgain[AX_HDR_CHN_L]    = sample_ae_float_convert_to_int(pStatus->snsDgain[AX_HDR_CHN_L], 22, 10, false);
    pAeResult->nTotalGain[AX_HDR_CHN_L]    = sample_ae_float_convert_to_int(pStatus->totalGain[AX_HDR_CHN_L], 22, 10, false);

    pAeResult->nIntTime[AX_HDR_CHN_M]  = pStatus->snsShutter[AX_HDR_CHN_M];
    pAeResult->nAgain[AX_HDR_CHN_M]    = sample_ae_float_convert_to_int(pStatus->snsTotalAgain[AX_HDR_CHN_M], 22, 10, false);
    pAeResult->nDgain[AX_HDR_CHN_M]    = sample_ae_float_convert_to_int(pStatus->snsDgain[AX_HDR_CHN_M], 22, 10, false);
    pAeResult->nTotalGain[AX_HDR_CHN_M]    = sample_ae_float_convert_to_int(pStatus->totalGain[AX_HDR_CHN_M], 22, 10, false);

    pAeResult->nIntTime[AX_HDR_CHN_S]  = pStatus->snsShutter[AX_HDR_CHN_S];
    pAeResult->nAgain[AX_HDR_CHN_S]    = sample_ae_float_convert_to_int(pStatus->snsTotalAgain[AX_HDR_CHN_S], 22, 10, false);
    pAeResult->nDgain[AX_HDR_CHN_S]    = sample_ae_float_convert_to_int(pStatus->snsDgain[AX_HDR_CHN_S], 22, 10, false);
    pAeResult->nTotalGain[AX_HDR_CHN_S]    = sample_ae_float_convert_to_int(pStatus->totalGain[AX_HDR_CHN_S], 22, 10, false);

    pAeResult->nIntTime[AX_HDR_CHN_VS] = pStatus->snsShutter[AX_HDR_CHN_VS];
    pAeResult->nAgain[AX_HDR_CHN_VS]   = sample_ae_float_convert_to_int(pStatus->snsTotalAgain[AX_HDR_CHN_VS], 22, 10, false);
    pAeResult->nDgain[AX_HDR_CHN_VS]   = sample_ae_float_convert_to_int(pStatus->snsDgain[AX_HDR_CHN_VS] , 22, 10, false);
    pAeResult->nTotalGain[AX_HDR_CHN_VS]   = sample_ae_float_convert_to_int(pStatus->totalGain[AX_HDR_CHN_VS] , 22, 10, false);

    pAeResult->nIspGain     = sample_ae_float_convert_to_int(pStatus->ispGain, 22, 10, false);

    pAeResult->nHcgLcgMode  = (AX_U32) pStatus->currHcgLcg;
    pAeResult->nHcgLcgRatio = sample_ae_float_convert_to_int(pStatus->currHcgRatio , 10, 10, false);

    pAeResult->nHdrRatio[AX_HDR_RATIO_CHN_L_M]  = sample_ae_float_convert_to_int(pStatus->hdrRealRatioLtoS, 7, 10, false);
    pAeResult->nHdrRatio[AX_HDR_RATIO_CHN_M_S]  = sample_ae_float_convert_to_int(pStatus->hdrRealRatioStoVS, 7, 10, false);
    pAeResult->nHdrRatio[AX_HDR_RATIO_CHN_S_VS] = 1024.0;


    pAeResult->nLux        = sample_ae_float_convert_to_int(pStatus->lux, 22, 10, false);
    pAeResult->nMeanLuma   = sample_ae_float_convert_to_int(pStatus->meanLuma, 8, 10, false);

    AX_U32 nHdrMode = 1;
    sample_ae_sns_get_hdr_mode(pipe, &nHdrMode);

    if (nHdrMode == 1){
        printf("AE Result: shutter=%d, Again=%.2f, ispGain=%.2f, lux=%.2f, Luma=%.2f \n",
            pStatus->snsShutter[AX_HDR_CHN_L],  pStatus->snsTotalAgain[AX_HDR_CHN_L],
            pStatus->ispGain, pStatus->lux, pStatus->meanLuma);
    }
    else if(nHdrMode == 2){
        printf("AE Result: shutter=%d, Again=%.2f, ispGain=%.2f, lux=%.2f, Luma=%.2f , realhdrRealRatioLtoS = %0.2f\n",
            pStatus->snsShutter[AX_HDR_CHN_L],  pStatus->snsTotalAgain[AX_HDR_CHN_L],
            pStatus->ispGain, pStatus->lux, pStatus->meanLuma,pStatus->hdrRealRatioLtoS);
    }
    else if(nHdrMode == 3){
        printf("AE Result: shutter=%d, Again=%.2f, ispGain=%.2f, lux=%.2f, Luma=%.2f , realhdrRealRatioLtoS = %0.2f, realhdrRealRatioStoVS = %0.2f,\n",
            pStatus->snsShutter[AX_HDR_CHN_L],  pStatus->snsTotalAgain[AX_HDR_CHN_L],
            pStatus->ispGain, pStatus->lux, pStatus->meanLuma,pStatus->hdrRealRatioLtoS,pStatus->hdrRealRatioStoVS);
    }


    return 0;
}


AX_S32 sample_ae_auto_mode_core
(
    AX_U8  pipe
)
{
    AX_S32 result = 0;
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    SAMPLE_AEC_CONFIG_T *pCtxConfig = &(pAlgCtx->tConfig);
    SAMPLE_AEC_STATUS_T *pCtxStatus = &(pAlgCtx->tStatus);

    AX_F32 idealNewEV;                     // For Step2
    AX_F32 splitGain = 0.0f;               // For Step4
    AX_U32 splitIntegrationTime = 0.0f;    // For Step4

    AX_F32 curMeanLuma = pCtxStatus->meanLuma;
    /* Step1: Calculate the Ideal New Exposure Value and Update Converge Status. */
    result = sample_ae_calc_ideal_newEV_upd_converge(pipe, pCtxConfig, pCtxStatus, &idealNewEV, curMeanLuma);
    if (0 != result) {
        printf("Calculate Ideal New Exposure Value Failed!\n");
        return -1;
    }

    /* Step2: Decide whether the Exposure Parameters Need to be Updated According to Converge Status. */
    if ( pCtxStatus->isConverge == true) {
        return 0;
    }

    /* Step3: Split Ideal NewEV into gain & integration time values. */
    AX_U32 algoMaxShutter = pCtxStatus->algoMaxShutter;
    result = sample_ae_split_gain_shutter(pipe, pCtxConfig, idealNewEV, &splitGain, &splitIntegrationTime, algoMaxShutter);
    if (0 != result) {
        printf("Split Gain Shutter Failed!\n");
        return -1;
    }

    /* Step4: Feed New Gain and Shutter to Sensor, Sensor Driver's Status Info Changes Here. */
    result = sample_ae_auto_sns_ctrl(pAlgCtx, pipe, splitGain, splitIntegrationTime,
                                     pCtxConfig->hdrRatio);

    if (0 != result) {
        printf("Auto Sensor Control Failed!\n");
        return -1;
    }

    return result;
}


AX_S32 sample_ae_run
(
    AX_U8 pipe,
    AX_ISP_AE_INPUT_INFO_T *pAeInputInfo,
    AX_ISP_AE_RESULT_T *pAeResult
){

    AX_S32 result = 0;

    if ((pAeInputInfo->sAeStat.nSeqNum % (pAeInputInfo->sAeStat.nSkipNum + 1)) != 0)
    {
        return result;
    }

    /* Step1: Get the AE Algo Context Handler of the Current Sensor. */
    SAMPLE_AEC_CTX_T *pAlgCtx = sample_ae_get_ctx_handle(pipe);
    SAMPLE_AEC_STATUS_T *pCtxStatus  = &(pAlgCtx->tStatus);

    /* Step2: Calculate Current Mean Luma and Histogram. */
    result = sample_ae_update_luma_and_hist(pipe, pAeInputInfo, pCtxStatus);
    if (0 != result) {
        printf("update luma failed!\n");
        return -1;
    }

   if (pAlgCtx->bEnable) {
        /* Step3: Calculate Gain Shutter and Feed them to Sensor */
        result = sample_ae_auto_mode_core(pipe);
        if (0 != result) {
            printf(" Sample Ae Enable Under Different Mode Failed!\n");
            return -1;
        }

        /* Step3: Update Context's Exposure Settings according to Sensor Driver's Status Info. */
        result = sample_ae_upd_ctx_exp_settings(pAlgCtx, pipe);
        if (0 != result) {
            printf("Sample Update Context Exposure Settings Failed!\n");
            return -1;
        }

    }
    else {
        /* Refer to ax_isp_alg_ae_set_param() for Manual Exposure Operations. */
        printf("Sample Manual\n");
    }

    /* Step4: Update Context's Current Lux according to MeanLuma and Exposure Settings. */
    result = sample_ae_upd_ctx_lux(pAlgCtx, pipe);
    if (0 != result) {
        printf("Sample Update Context Lux Failed!\n");
        return -1;
    }

    /* Step5: Convert AE Algo Result to AE Output API Struct. */
    result = sample_ae_gen_api_out_put(pipe, &(pAlgCtx->tStatus) , pAeResult);
    if (0 != result) {
        printf("Sample Generate AE Outputs Failed!\n");
        return -1;
    }

    return 0;
}

AX_S32 SetSensorFunc(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType)
{
    AX_S32 axRet = 0;
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;

    /* 3a get sensor config */
    ptSnsHdl = COMMON_ISP_GetSnsObj(eSnsType);
    if (NULL == ptSnsHdl) {
        COMM_PRT("AX_ISP Get Sensor Object Failed!\n");
        return -1;
    }
    pSnsHandle[pipe] = ptSnsHdl;
    return axRet;
}