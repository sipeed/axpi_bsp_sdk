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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#include "ax_vin_api.h"
#include "ax_isp_api.h"
#include "ax_mipi_api.h"
#include "ax_sensor_struct.h"
#include "ax_isp_3a_api.h"
#include "common_cam.h"
#include "common_type.h"
#include "sample_isp_awb.h"

static SAMPLE_AWB_CTX_T g_tAwbAlgCtx[AX_PIPE_MAX_NUM];
SAMPLE_AWB_CTX_T* sample_awb_getCtxHandle(AX_U32 pipe)
{
    SAMPLE_AWB_CTX_T *pAwb_ctx = &g_tAwbAlgCtx[pipe];
    return pAwb_ctx;
}

///////////////////////////////////////////////////////////////////
///////////////////     Data Type Convert      ////////////////////
///////////////////////////////////////////////////////////////////

AX_F32 axawb_int_convert_to_float(AX_S32 p, AX_S32 int_bit, AX_S32 frac_bit, AX_BOOL signed_value)
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


AX_U32 axawb_float_convert_to_int(AX_F32 value, AX_U32 int_bit, AX_U32 frac_bit, AX_BOOL signed_value)
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


AX_S32 sample_awb_init
(
    AX_U8                 pipe,
    AX_ISP_AWB_INITATTR_T *pAwbInitParam
){
    /* Step1: Get the AWB Algo Context Handler of the Current Sensor. */
    SAMPLE_AWB_CTX_T *pCtx = sample_awb_getCtxHandle(pipe);

    /* Step2: Most of the Calibration Info Come from Offline XML, or Hard-Code here. */
    SAMPLE_AWB_CALIB_T *pCalibInfo = &(pCtx->calibInfo);

    // Illum (R/G, B/G) Calibration Info.
    pCalibInfo->aRg   = 0.808;
    pCalibInfo->aBg   = 0.317;
    pCalibInfo->d50Rg = 0.594;
    pCalibInfo->d50Bg = 0.587;
    pCalibInfo->d65Rg = 0.466;
    pCalibInfo->d65Bg = 0.607;

    // Simplest Square Gray Zone Borders.
    pCalibInfo->grayZoneMinRg = 0.3;
    pCalibInfo->grayZoneMaxRg = 1.05;
    pCalibInfo->grayZoneMinBg = 0.3;
    pCalibInfo->grayZoneMaxBg = 0.8;

    // CCT Calibration Info.
    pCalibInfo->aCct = 2856.0;
    pCalibInfo->d50Cct = 5000.0;
    pCalibInfo->d65Cct = 6500.0;
    pCalibInfo->cctSpan = pCalibInfo->d65Cct - pCalibInfo->aCct;

    // CCT Straight Line Pass Through Illum A and D65.
    float cctLineVecRg = pCalibInfo->d65Rg - pCalibInfo->aRg;
    float cctLineVecBg = pCalibInfo->d65Bg - pCalibInfo->aBg;
    pCalibInfo->cctLineVecLen = sqrt(cctLineVecRg*cctLineVecRg + cctLineVecBg*cctLineVecBg);
    pCalibInfo->cctLineNormVecRg = cctLineVecRg / pCalibInfo->cctLineVecLen;
    pCalibInfo->cctLineNormVecBg = cctLineVecBg / pCalibInfo->cctLineVecLen;
    printf("CCT Line: VecLen=%.4f, VecRg=%.4f, VecBg=%.4f \n", pCalibInfo->cctLineVecLen,
           pCalibInfo->cctLineNormVecRg, pCalibInfo->cctLineNormVecBg);

    /* Step3: Most of the Tuning Parameters Come from Online XML, or Hard-Code here. */
    SAMPLE_AWB_TUNING_T *pTuningParam = &(pCtx->tuningParam);
    pTuningParam->dampRatio    = 0.9;
    pTuningParam->outdoorLuxTh = 1500.0;
    pTuningParam->yClipMin     = 3.0;
    pTuningParam->yClipMax     = 210.0;

    /* Step4: Initialize Dynamic Information. (Use D50 as the Default Illuminant) */
    SAMPLE_AWB_STATUS_T *pStatus = &(pCtx->tStatus);
    pStatus->prevAvgSgwRg = pCalibInfo->d50Rg;
    pStatus->prevAvgSgwBg = pCalibInfo->d50Bg;
    pStatus->prevCct      = pCalibInfo->d50Cct;
    pStatus->currAvgSgwRg = pCalibInfo->d50Rg;
    pStatus->currAvgSgwBg = pCalibInfo->d50Bg;
    pStatus->prevCct      = pCalibInfo->d50Cct;

    pStatus->rGain  = 1.0 / pCalibInfo->d50Rg;
    pStatus->grGain = 1.0;
    pStatus->gbGain = 1.0;
    pStatus->bGain  = 1.0 / pCalibInfo->d50Bg;
    pStatus->eSnsMode = pAwbInitParam->eSnsMode;

    for (int i = 0; i < pCtx->tStatus.blkColNum * pCtx->tStatus.blkRowNum; i++) {
        pStatus->blkRg[i]     = 0.0;
        pStatus->blkBg[i]     = 0.0;
        pStatus->blkY[i]      = 0.0;
        pStatus->blkWeight[i] = 1.0;
    }
    return 0;
}

/* Convert AWB Hardware Statistics Info to AWB Algo Struct. */
AX_S32 sample_awb_prepare_algo_input
(
    const AX_ISP_AWB_STAT_INFO_T  *pAwbStat,
    SAMPLE_AWB_CTX_T              *pCtx
)
{

    float r  = 0.0f;
    float gr = 0.0f;
    float gb = 0.0f;
    float g  = 0.0f;
    float b  = 0.0f;
    float y  = 0.0f;

    pCtx->tStatus.blkColNum = pAwbStat->tAwbGridStats[0].nZoneColSize;
    pCtx->tStatus.blkRowNum = pAwbStat->tAwbGridStats[0].nZoneRowSize;

    for (int i = 0; i < pCtx->tStatus.blkRowNum ; i++) {
        for(int j = 0; j < pCtx->tStatus.blkColNum; j++) {

        // Note: Unused Now. Just Leave them here for reference, in case for the Rollback when the RLTM WBC Doesn't Works Well.
        // Calculate the R, G, B, Y Values of Each AWB Statistics Block.
        // The AWB Statistics Hardware Collects Info after WB Correction.
        // So a Gain-Revert Operation is Needed to Get the Info before WB Correction.
        // Once AX RLTM WBC Driver is Developed, Gain-Revert Operation will be removed.
        // float preRgain  = pCtx->tStatus.rGain;
        // float preGrgain = pCtx->tStatus.grGain;
        // float preGbgain = pCtx->tStatus.gbGain;
        // float preBgain  = pCtx->tStatus.bGain;
        // r  = (float)pAwbStat->tLumaStatGroup[AX_MEAN_LUMA_STAT_FRAME_CHN_BLEND].aStatistics[i][0] / preRgain;
        // gr = (float)pAwbStat->tLumaStatGroup[AX_MEAN_LUMA_STAT_FRAME_CHN_BLEND].aStatistics[i][1] / preGrgain;
        // gb = (float)pAwbStat->tLumaStatGroup[AX_MEAN_LUMA_STAT_FRAME_CHN_BLEND].aStatistics[i][2] / preGbgain;
        // b  = (float)pAwbStat->tLumaStatGroup[AX_MEAN_LUMA_STAT_FRAME_CHN_BLEND].aStatistics[i][3] / preBgain;

        // Calculate the R, G, B, Y Values of Each AWB Statistics Block.
            if(pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[0] == 0 ||
                pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[1] == 0 ||
                pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[2] == 0 ||
                pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[3] == 0){

                pCtx->tStatus.blkRg[i*pCtx->tStatus.blkRowNum+j] = 0.0f;
                pCtx->tStatus.blkBg[i*pCtx->tStatus.blkRowNum+j] = 0.0f;
                pCtx->tStatus.blkY[i*pCtx->tStatus.blkRowNum+j] = 0.0f;
                continue;
            }

            r  =  ((pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[0] >> 6) + (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[0] & 0x3F) / 64.0)/
                       (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[0]) ;  //1 Sum >> 6 : chang U30 to U24;
                                                                                                  //2 Sum & 0x3F : get the lower U6 bits
                                                                                                  //3 (Sum & 0x3F) / 64.0 : change U6 to float
                                                                                                  //4 (Sum >> 6 + (Sum & 0x3F) / 64.0): change U30 to U24.*
                                                                                                  //5 (Sum >> 6 + (Sum & 0x3F) / 64.0) / Num : average value  U24.*/U16=U8.*
            b  =  ((pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[1] >> 6) + (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[1] & 0x3F) / 64.0)/
                       (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[1]) ;
            gr =  ((pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[2] >> 6) + (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[2] & 0x3F) / 64.0)/
                       (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[2]) ;
            gb =  ((pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[3] >> 6) + (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridSum[3] & 0x3F) / 64.0)/
                       (pAwbStat->tAwbGridStats[0].tAwbGridStats[i * pCtx->tStatus.blkColNum + j].nUnSatGridNum[3]) ;
            g = 0.5 * (gr + gb);
            y = 0.299 * r + 0.587 * g + 0.114 * b;

            // Save Y, R/G, B/G to the Algo Context for further use.
            pCtx->tStatus.blkY[i] = y;
            if (g < 0.00001) {
                pCtx->tStatus.blkRg[i] = 0.0f;
                pCtx->tStatus.blkBg[i] = 0.0f;
            } else {
                pCtx->tStatus.blkRg[i] = r / g;
                pCtx->tStatus.blkBg[i] = b / g;
            }
        }

    }

    return 0;
}

AX_S32 sample_awb_calc_rgb_gains
(
    float rg,
    float bg,
    SAMPLE_AWB_STATUS_T *pStatus
)
{

    float tmpRgain = 1.0 / rg;
    float tmpBgain = 1.0 / bg;

    // Gain Smaller than 1.0 is NOT Allowed !
    float minGain  = 1.0;
    if (tmpRgain < minGain) {
        minGain = tmpRgain;
    }
    if (tmpBgain < minGain) {
        minGain = tmpBgain;
    }

    // Gains Normalization.
    pStatus->rGain  = tmpRgain / minGain;
    pStatus->grGain =   1.0    / minGain;
    pStatus->gbGain =   1.0    / minGain;
    pStatus->bGain  = tmpBgain / minGain;

    return 0;
}

AX_S32 sample_awb_gray_world
(
    SAMPLE_AWB_CTX_T *pCtx
)
{

    /* Step1: Get Handlers of Calib Info, Tuning Param, and Dynamic Status. */
    SAMPLE_AWB_CALIB_T  *pCalibInfo   = &(pCtx->calibInfo);
    SAMPLE_AWB_TUNING_T *pTuningParam = &(pCtx->tuningParam);
    SAMPLE_AWB_STATUS_T *pStatus      = &(pCtx->tStatus);

    /* Step2: Determine the Weight of Each Block. */
    for (int i = 0; i < pCtx->tStatus.blkColNum * pCtx->tStatus.blkRowNum; i++) {

        // Ignore Under-Exposured or Over-Exposured Blocks.
        if ( (pStatus->blkY[i] < pTuningParam->yClipMin) ||
                (pStatus->blkY[i] > pTuningParam->yClipMax) ) {
            pStatus->blkWeight[i] = 0.0;
            continue;
        }

        // Ignore Blocks Outside the Gray Zone.
        if ( (pStatus->blkRg[i] < pCalibInfo->grayZoneMinRg) ||
                (pStatus->blkRg[i] > pCalibInfo->grayZoneMaxRg) ||
                (pStatus->blkBg[i] < pCalibInfo->grayZoneMinBg) ||
                (pStatus->blkBg[i] > pCalibInfo->grayZoneMaxBg) ) {
            pStatus->blkWeight[i] = 0.0;
            continue;
        }

        // Good-Exposured Block inside the Gray Zone.
        pStatus->blkWeight[i] = 1.0;

    }

    /* Step3: Calculate Weighted Average Sum of (R/G, B/G). */
    float sumWeight = 0.0;
    float sumRg     = 0.0;
    float sumBg     = 0.0;
    for (int i = 0; i < pCtx->tStatus.blkColNum * pCtx->tStatus.blkRowNum; i++) {
        sumRg     += pStatus->blkWeight[i] * pStatus->blkRg[i];
        sumBg     += pStatus->blkWeight[i] * pStatus->blkBg[i];
        sumWeight += pStatus->blkWeight[i];
    }

    // Use D50 as the Default Illum if No White Block Found.
    if (fabs(sumWeight) < 0.000001 || sumRg < 0.00001 || sumBg < 0.00001) {
        pStatus->currAvgSgwRg = pCalibInfo->d50Rg;
        pStatus->currAvgSgwBg = pCalibInfo->d50Bg;
    }
    else{
        pStatus->currAvgSgwRg = sumRg / sumWeight;
        pStatus->currAvgSgwBg = sumBg / sumWeight;
    }

    /* Step4: Calculate CCT (Linear Approx) of the Simple Gray World Result. */
    float sgwCctVecRg   = pStatus->currAvgSgwRg - pCalibInfo->aRg;
    float sgwCctVecBg   = pStatus->currAvgSgwBg - pCalibInfo->aBg;

    // Project the Current Point to the CCT Line between A and D65,
    // and Calculate the Length between the Projected Foot Point and A.
    // Dot Product = cos(theta) * |sgwCctVec| * |cctLineNormVec|, where |cctLineNormVec| = 1.
    // So: sgwCctProjLen = cos(theta) * |sgwCctVec| = Dot Product of the 2 Vectors.
    float sgwCctProjLen = sgwCctVecRg * pCalibInfo->cctLineNormVecRg +
        sgwCctVecBg * pCalibInfo->cctLineNormVecBg;
    if (sgwCctProjLen < 0.0) {
        pStatus->currCct = pCalibInfo->aCct;
    } else if (sgwCctProjLen > pCalibInfo->cctLineVecLen) {
        pStatus->currCct = pCalibInfo->d65Cct;
    } else {
        float cct_ratio = sgwCctProjLen / pCalibInfo->cctLineVecLen;
        pStatus->currCct = pCalibInfo->aCct + cct_ratio * pCalibInfo->cctSpan;
    }


    /* Step5: Damping for Smooth Change. */
    float dampedRg  = pStatus->currAvgSgwRg * (1.0 - pTuningParam->dampRatio) +
        pStatus->prevAvgSgwRg * pTuningParam->dampRatio;
    float dampedBg  = pStatus->currAvgSgwBg * (1.0 - pTuningParam->dampRatio) +
        pStatus->prevAvgSgwBg * pTuningParam->dampRatio;
    float dampedCct = pStatus->currCct * (1.0 - pTuningParam->dampRatio) +
        pStatus->prevCct * pTuningParam->dampRatio;

    pStatus->prevAvgSgwRg = dampedRg;
    pStatus->prevAvgSgwBg = dampedBg;
    pStatus->prevCct      = dampedCct;

    // printf("AWB: Curr (R/G, B/G, CCT) = (%.3f, %.3f, %.3f),  Damp (R/G, B/G, CCT) = (%.3f, %.3f, %.3f) \n",
    //       pStatus->currAvgSgwRg, pStatus->currAvgSgwBg, pStatus->currCct,
    //       pStatus->prevAvgSgwRg, pStatus->prevAvgSgwBg, pStatus->prevCct);


    /* Step6: Calculate R, G, B Gains for the Next Frame. */
    sample_awb_calc_rgb_gains(dampedRg, dampedBg, pStatus);


    return 0;
}



AX_S32 sample_awb_gen_api_output
(
    SAMPLE_AWB_STATUS_T  *pStatus,
    AX_ISP_AWB_RESULT_T  *pAwbResult
)
{
    pAwbResult->nRgain  = axawb_float_convert_to_int(pStatus->rGain, 4, 8, false);
    pAwbResult->nGrGain = axawb_float_convert_to_int(pStatus->grGain, 4, 8, false);
    pAwbResult->nGbGain = axawb_float_convert_to_int(pStatus->gbGain, 4, 8, false);
    pAwbResult->nBgain  = axawb_float_convert_to_int(pStatus->bGain, 4, 8, false);
    pAwbResult->nColorTemp  = (AX_U32) (pStatus->prevCct);
    pAwbResult->nSatDiscnt  = 100;
    printf("AWB: Gains (R, Gr, Gb, B) = (%.3f, %.3f, %.3f, %.3f),  CCT = %.2f \n",
           pStatus->rGain, pStatus->grGain, pStatus->gbGain, pStatus->bGain, pStatus->prevCct);
    return 0;
}

AX_S32 sample_awb_run
(
    AX_U8                   pipe,
    AX_ISP_AWB_INPUT_INFO_T *pAwbInputInfo,
    AX_ISP_AWB_RESULT_T     *pAwbResult
)
{
    /* Step1: Get the AWB Algo Context Handler of the Current Sensor. */
    SAMPLE_AWB_CTX_T *pCtx = sample_awb_getCtxHandle(pipe);
    if ((pAwbInputInfo->tAwbStat.nSeqNum % (pAwbInputInfo->tAwbStat.nSkipNum + 1)) != 0)
    {
        return 0;
    }
    /* Step2: Convert AWB Hardware Statistics Info to AWB Algo Struct. */
    sample_awb_prepare_algo_input(&(pAwbInputInfo->tAwbStat), pCtx);

    /* Step3: The Core of the AWB Algo: Calculate R,G,B Gains and CCT Estimate. */
    sample_awb_gray_world(pCtx);

    /* Step4: Convert AWB Algo Result to AWB Output API Struct. */
    sample_awb_gen_api_output(&(pCtx->tStatus) , pAwbResult);

    return 0;
}

AX_S32 sample_awb_deinit(AX_U8 pipe)
{
    return 0;
}
