/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#include "OS04a10.h"
#include <dlfcn.h>
#include "ax_sys_api.h"
#include "ax_isp_3a_api.h"
#include "OptionHelper.h"
#include "StageOptionHelper.h"
#include "ConfigParser.h"

#define OS04A10 "OS04A10"

extern COptionHelper gOptions;

COS04a10::COS04a10(SENSOR_CONFIG_T tSensorConfig)
    : CBaseSensor(tSensorConfig)
{
}

COS04a10::~COS04a10(AX_VOID)
{
}

AX_VOID COS04a10::InitSnsAttr()
{
    /* Referenced by AX_VIN_SetSnsAttr */
    m_tSnsAttr.nWidth               = 2688;
    m_tSnsAttr.nHeight              = 1520;
    m_tSnsAttr.nFrameRate           = m_tSnsInfo.nFrameRate;
    m_tSnsAttr.eSnsMode             = m_tSnsInfo.eSensorMode;
    if (AX_SNS_HDR_2X_MODE == m_tSnsAttr.eSnsMode) {
        m_tSnsAttr.eRawType         = AX_RT_RAW10;
    } else {
        m_tSnsAttr.eRawType         = AX_RT_RAW10;
    }

    m_tSnsAttr.eSnsHcgLcg           = AX_HCG_MODE;
    m_tSnsAttr.eBayerPattern        = AX_BP_RGGB;
    m_tSnsAttr.bTestPatternEnable   = AX_FALSE;
    m_tSnsAttr.eMasterSlaveSel      = AX_SNS_MASTER;
    m_tSnsClkAttr.nSnsClkIdx = 0;
    m_tSnsClkAttr.eSnsClkRate = AX_SNS_CLK_24M;
}

AX_VOID COS04a10::InitMipiRxAttr()
{
    /* Referenced by AX_MIPI_RX_SetAttr */
    m_tMipiRxAttr.eLaneNum      = AX_MIPI_LANE_4;
    m_tMipiRxAttr.eDataRate     = AX_MIPI_DATA_RATE_80M;
    m_tMipiRxAttr.ePhySel       = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3;
    m_tMipiRxAttr.nLaneMap[0]   = 0x00;
    m_tMipiRxAttr.nLaneMap[1]   = 0x01;
    m_tMipiRxAttr.nLaneMap[2]   = 0x03;
    m_tMipiRxAttr.nLaneMap[3]   = 0x04;
    m_tMipiRxAttr.nLaneMap[4]   = 0x02; /* clock lane */
}

AX_VOID COS04a10::InitDevAttr()
{
    /* Referenced by AX_VIN_SetDevAttr */
    m_tDevAttr.eSnsType         = AX_SNS_TYPE_MIPI;
    m_tDevAttr.tDevImgRgn.nStartX = 0;
    m_tDevAttr.tDevImgRgn.nStartY = 0;
    m_tDevAttr.tDevImgRgn.nWidth  = 2688;
    m_tDevAttr.tDevImgRgn.nHeight = 1520;
    m_tDevAttr.bDolSplit        = AX_FALSE;
    m_tDevAttr.bHMirror         = AX_FALSE;
    m_tDevAttr.eBayerPattern    = AX_BP_RGGB;
    m_tDevAttr.eSkipFrame       = AX_SNS_SKIP_FRAME_NO_SKIP;
    m_tDevAttr.eSnsGainMode     = AX_SNS_GAIN_MODE_LCG;
    m_tDevAttr.eSnsMode         = m_tSnsInfo.eSensorMode;
    if (AX_SNS_HDR_2X_MODE == m_tDevAttr.eSnsMode) {
        m_tDevAttr.ePixelFmt    = AX_FORMAT_BAYER_RAW_10BPP;
    } else {
        m_tDevAttr.ePixelFmt    = AX_FORMAT_BAYER_RAW_10BPP;
    }
    //  m_tDevAttr.eSnsOutputMode   = AX_SNS_NORMAL;
}

AX_VOID COS04a10::InitPipeAttr()
{
    /* Referenced by AX_VIN_SetPipeAttr */
    m_tPipeAttr.nWidth          = 2688;
    m_tPipeAttr.nHeight         = 1520;
    m_tPipeAttr.eBayerPattern   = AX_BP_RGGB;
    m_tPipeAttr.eSnsMode        = m_tSnsInfo.eSensorMode;
    if (AX_SNS_HDR_2X_MODE == m_tPipeAttr.eSnsMode) {
        m_tPipeAttr.ePixelFmt    = m_eImgFormatHDR;
    } else {
        m_tPipeAttr.ePixelFmt    = m_eImgFormatSDR;
    }
    if (AX_SNS_HDR_2X_MODE == m_tPipeAttr.eSnsMode) {
        m_tPipeAttr.ePipeDataSrc     = AX_PIPE_SOURCE_DEV_ONLINE;
    } else {
        m_tPipeAttr.ePipeDataSrc     = AX_PIPE_SOURCE_DEV_ONLINE;
    }
    m_tPipeAttr.eDataFlowType   = AX_DATAFLOW_TYPE_NORMAL;
    m_tPipeAttr.eDevSource      = AX_DEV_SOURCE_SNS_0;
    m_tPipeAttr.ePreOutput      = AX_PRE_OUTPUT_FULL_MAIN;
}

AX_VOID COS04a10::InitChnAttr()
{
    /* Referenced by AX_VIN_SetChnAttr */
    m_tChnAttr.tChnAttr[0].nWidth = 2688;
    m_tChnAttr.tChnAttr[0].nHeight = 1520;
    m_tChnAttr.tChnAttr[0].eImgFormat = AX_YUV420_SEMIPLANAR;
    m_tChnAttr.tChnAttr[0].bEnable = AX_TRUE;
    m_tChnAttr.tChnAttr[0].nWidthStride = 2688;
    m_tChnAttr.tChnAttr[0].nDepth = YUV_OS04A10_DEPTH;

    if (gOptions.IsActivedDetect()) {
        AI_CONFIG_T tAiConfig;
        memset(&tAiConfig, 0x00, sizeof(tAiConfig));
        if (CConfigParser().GetInstance()->GetAiCfg(0, tAiConfig)) {
            m_tChnAttr.tChnAttr[1].nWidth = tAiConfig.nWidth;
            m_tChnAttr.tChnAttr[1].nHeight = tAiConfig.nHeight;
        } else {
            m_tChnAttr.tChnAttr[1].nWidth = DETECT_DEFAULT_WIDTH;
            m_tChnAttr.tChnAttr[1].nHeight = DETECT_DEFAULT_HEIGHT;
        }
    } else {
        m_tChnAttr.tChnAttr[1].nWidth = 1280;
        m_tChnAttr.tChnAttr[1].nHeight = 720;
    }

    m_tChnAttr.tChnAttr[1].eImgFormat = AX_YUV420_SEMIPLANAR;
    m_tChnAttr.tChnAttr[1].bEnable = AX_TRUE;
    m_tChnAttr.tChnAttr[1].nWidthStride = ALIGN_UP(m_tChnAttr.tChnAttr[1].nWidth, 64);
    m_tChnAttr.tChnAttr[1].nDepth = YUV_OS04A10_DEPTH;

    m_tChnAttr.tChnAttr[2].nWidth = 640;
    m_tChnAttr.tChnAttr[2].nHeight = 360;
    m_tChnAttr.tChnAttr[2].eImgFormat = AX_YUV420_SEMIPLANAR;
    m_tChnAttr.tChnAttr[2].bEnable = AX_TRUE;
    m_tChnAttr.tChnAttr[2].nWidthStride = 640;
    m_tChnAttr.tChnAttr[2].nDepth = YUV_OS04A10_DEPTH;
}

AX_S32 COS04a10::RegisterSensor(AX_U8 nPipe)
{
    AX_S32 nRet = 0;

    m_pSnsLib = dlopen("libsns_os04a10.so", RTLD_LAZY);
    if (NULL == m_pSnsLib) {
        LOG_M_E(OS04A10, "Load sensor lib failed!");
        return -1;
    }

    m_pSnsObj = (AX_SENSOR_REGISTER_FUNC_T *)dlsym(m_pSnsLib, "gSnsos04a10Obj");
    if (NULL == m_pSnsObj) {
        LOG_M_E(OS04A10, "AX_VIN Get Sensor Object Failed!");
        return -1;
    }

    nRet = AX_VIN_RegisterSensor(nPipe, m_pSnsObj); /* pipe 0 */
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP Register Sensor Failed, ret=0x%x.", nRet);
        return nRet;
    }

    AX_SNS_COMMBUS_T tSnsBusInfo = {0};
    memset(&tSnsBusInfo, 0, sizeof(AX_SNS_COMMBUS_T));
    tSnsBusInfo.I2cDev = GetI2cDevNode(nPipe);
    if (NULL != m_pSnsObj->pfn_sensor_set_bus_info) {
        nRet = m_pSnsObj->pfn_sensor_set_bus_info(nPipe, tSnsBusInfo); /* pipe 0 */
        if (0 != nRet) {
            LOG_M_E(OS04A10, "Sensor set bus info Failed, ret=0x%x.", nRet);
            return nRet;
        }
        LOG_M(OS04A10, "set sensor bus idx %d", tSnsBusInfo.I2cDev);
    } else {
        LOG_M(OS04A10, "not support set sensor bus info!");
        return -1;
    }

    return 0;
}

AX_S32 COS04a10::UnRegisterSensor(AX_U8 nPipe)
{
    AX_VIN_UnRegisterSensor(nPipe); /* pipe 0 */

    if (m_pSnsLib) {
        dlclose(m_pSnsLib);
        m_pSnsLib = nullptr;
        m_pSnsObj = nullptr;
    }

    return 0;
}

AX_S32 COS04a10::RegisterAeAlgLib(AX_U8 nPipe)
{
    if (!m_pSnsObj) {
        LOG_M_E(OS04A10, "RegisterAeAlgLib: m_pSnsObj is null!");
        return -1;
    }
    AX_S32 nRet = 0;

    AX_ISP_AE_REGFUNCS_T tAeFuncs = {0};

    tAeFuncs.pfnAe_Init = AX_ISP_ALG_AeInit;
    tAeFuncs.pfnAe_Exit = AX_ISP_ALG_AeDeInit;
    tAeFuncs.pfnAe_Run  = AX_ISP_ALG_AeRun;


    /* Register the sensor driven interface TO the AE library */
    nRet = AX_ISP_ALG_AeRegisterSensor(nPipe, m_pSnsObj);
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP_ALG_AeRegisterSensor Failed, ret=0x%x", nRet);
        //return axRet;     /* FIX debug */
    }

    /* Register ae alg */
    nRet = AX_ISP_RegisterAeLibCallback(nPipe, &tAeFuncs);
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP_RegisterAeLibCallback Failed, ret=0x%x", nRet);
        return nRet;
    }

    return 0;
}

AX_S32 COS04a10::UnRegisterAeAlgLib(AX_U8 nPipe)
{
    AX_S32 nRet = 0;

    nRet = AX_ISP_ALG_AeUnRegisterSensor(nPipe);
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP ae un register sensor Failed, ret=0x%x", nRet);
        return nRet;
    }

    nRet = AX_ISP_UnRegisterAeLibCallback(nPipe);
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP Unregister Sensor Failed, ret=0x%x", nRet);
        return nRet;
    }

    return nRet;
}

AX_S32 COS04a10::RegisterAwbAlgLib(AX_U8 nPipe)
{
    AX_S32 nRet = 0;

    AX_ISP_AWB_REGFUNCS_T tAwbFuncs = {0};

    tAwbFuncs.pfnAwb_Init = AX_ISP_ALG_AwbInit;
    tAwbFuncs.pfnAwb_Exit = AX_ISP_ALG_AwbDeInit;
    tAwbFuncs.pfnAwb_Run  = AX_ISP_ALG_AwbRun;

    /* Register awb alg */
    nRet = AX_ISP_RegisterAwbLibCallback(nPipe, &tAwbFuncs);
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP_RegisterAwbLibCallback Failed, ret=0x%x", nRet);
        return nRet;
    }

    return 0;
}

AX_S32 COS04a10::UnRegisterAwbAlgLib(AX_U8 nPipe)
{
    AX_S32 nRet = 0;

    nRet = AX_ISP_UnRegisterAwbLibCallback(nPipe);
    if (nRet) {
        LOG_M_E(OS04A10, "AX_ISP Unregister Sensor Failed, ret=0x%x", nRet);
        return nRet;
    }

    return nRet;
}

AX_S32 COS04a10::SetFps(AX_U8 nPipe, AX_F32 fFrameRate)
{
    if (m_pSnsObj && m_pSnsObj->pfn_sensor_set_fps) {
        AX_S32 nRet = m_pSnsObj->pfn_sensor_set_fps(nPipe, fFrameRate, NULL);

        if (nRet) {
            LOG_M_E(OS04A10, "[%d] Set Sensor Fps  to %.1f failed, ret=0x%x", nPipe, fFrameRate, nRet);
            return nRet;
        }
        else {
            LOG_M(OS04A10, "[%d] Set Sensor Fps to %.1f", nPipe, fFrameRate);
        }

        m_tSnsAttr.nFrameRate = (AX_U32)fFrameRate;
    }

    return 0;
}

AX_BOOL COS04a10::HdrModeSupport(AX_U8 nPipe)
{
    return AX_TRUE;
}
