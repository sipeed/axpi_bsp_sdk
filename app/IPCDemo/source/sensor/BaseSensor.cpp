/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "BaseSensor.h"
#include "OS04a10.h"
// #include "IMX334.h"
#include "GC4653.h"
// #include "OS08a20.h"
// #include "SC1345.h"
// #include "SC530AI.h"
#include "JsonCfgParser.h"
#include "OptionHelper.h"
#include <dlfcn.h>

#define SENSOR "SENSOR"

extern COptionHelper gOptions;

CBaseSensor::CBaseSensor(SENSOR_CONFIG_T tSensorConfig)
{
    m_nSensorID = 0;
    m_nDeviceID = 0;
    m_nPipeID = 0;
    m_pSnsLib = nullptr;
    m_pSnsObj = nullptr;
    m_eImgFormatSDR = AX_FORMAT_BAYER_RAW_10BPP;
    m_eImgFormatHDR = AX_FORMAT_BAYER_RAW_10BPP;

    FillValues(tSensorConfig);
}

CBaseSensor::~CBaseSensor(AX_VOID)
{

}

CBaseSensor *CBaseSensor::NewInstance(AX_U8 nSensorIndex)
{
    if (nSensorIndex >= E_SENSOR_ID_MAX) {
        LOG_M_E(SENSOR, "Sensor index out of range.");
        return nullptr;
    }

    SENSOR_CONFIG_T tSensorCfg;
    if (!CConfigParser().GetInstance()->GetCameraCfg(tSensorCfg, (SENSOR_ID_E)nSensorIndex)) {
        LOG_M_W(SENSOR, "Load camera configure for sensor %d failed.", nSensorIndex);
        return nullptr;
    }

    switch (tSensorCfg.eSensorType) {
        case E_SNS_TYPE_OS04A10: {
            return new COS04a10(tSensorCfg);
        }
        // case E_SNS_TYPE_IMX334: {
        //     return new CIMX334(tSensorCfg);
        // }
        case E_SNS_TYPE_GC4653: {
            return new CGC4653(tSensorCfg);
        }
        // case E_SNS_TYPE_OS08A20: {
        //     return new COS08a20(tSensorCfg);
        // }
        // case E_SNS_TYPE_SC1345: {
        //     return new CSC1345(tSensorCfg);
        // }
        // case E_SNS_TYPE_SC530AI: {
        //     return new CSC530AI(tSensorCfg);
        // }
        default: {
            LOG_M_E(SENSOR, "unkown senser type %d", tSensorCfg.eSensorType);
            return nullptr;
        }
    }
}

AX_VOID CBaseSensor::FillValues(SENSOR_CONFIG_T tSensorConfig)
{
    m_tSnsInfo.eSensorType    = tSensorConfig.eSensorType;
    m_tSnsInfo.eSensorMode    = tSensorConfig.eSensorMode;
    m_tSnsInfo.nFrameRate     = tSensorConfig.nFrameRate;
    m_tSnsInfo.eRunMode       = tSensorConfig.eRunMode;
    m_tSnsInfo.bSupportTuning = tSensorConfig.bTuning;
    m_tSnsInfo.nTuningPort    = tSensorConfig.nTuningPort;
}

AX_BOOL CBaseSensor::InitISP(AX_VOID)
{
    memset(&m_tSnsAttr, 0, sizeof(AX_SNS_ATTR_T));
    memset(&m_tSnsClkAttr, 0, sizeof(SENSOR_CLK_T));
    memset(&m_tDevAttr, 0, sizeof(AX_DEV_ATTR_T));
    memset(&m_tPipeAttr, 0, sizeof(AX_PIPE_ATTR_T));
    memset(&m_tMipiRxAttr, 0, sizeof(AX_MIPI_RX_ATTR_S));
    memset(&m_tChnAttr, 0, sizeof(AX_VIN_CHN_ATTR_T));

    InitNPU();
    InitMipiRxAttr();
    InitSnsAttr();
    InitDevAttr();
    InitPipeAttr();
    InitChnAttr();
    if (gOptions.IsEISSupport()) { //ALIGN_UP for GDC
        for (AX_U8 i = 0; i < MAX_ISP_CHANNEL_NUM; i++) {
            m_tChnAttr.tChnAttr[i].nWidthStride = ALIGN_UP(m_tChnAttr.tChnAttr[i].nWidthStride, GDC_STRIDE_ALIGNMENT);
        }
    }
    if (gOptions.IsEnableEISEffectComp() && (MAX_ISP_CHANNEL_NUM > 1)) {
        // last chn ivps linked will use TDP(no eis) and scale down 1/4 size of camere resolution for comparison
        m_tChnAttr.tChnAttr[MAX_ISP_CHANNEL_NUM - 1].nWidth       = m_tChnAttr.tChnAttr[0].nWidth / 4;
        m_tChnAttr.tChnAttr[MAX_ISP_CHANNEL_NUM - 1].nHeight      = m_tChnAttr.tChnAttr[0].nHeight / 4;
        m_tChnAttr.tChnAttr[MAX_ISP_CHANNEL_NUM - 1].nWidthStride = m_tChnAttr.tChnAttr[0].nWidth / 4;
        LOG_M(SENSOR, "EIS Effect comparison is on, reset chn: %d (w, h, s) to (%d, %d, %d)",
                       MAX_ISP_CHANNEL_NUM - 1, m_tChnAttr.tChnAttr[MAX_ISP_CHANNEL_NUM - 1].nWidth,
                       m_tChnAttr.tChnAttr[MAX_ISP_CHANNEL_NUM - 1].nHeight, m_tChnAttr.tChnAttr[MAX_ISP_CHANNEL_NUM - 1].nWidthStride);
    }

    return AX_TRUE;
}

AX_VOID CBaseSensor::InitNPU()
{
    /* NPU */
    AX_NPU_SDK_EX_ATTR_T npuAttr;
    npuAttr.eHardMode = AX_NPU_VIRTUAL_DISABLE;
    if (AX_ISP_PIPELINE_NORMAL == m_tSnsInfo.eRunMode) {
        npuAttr = CBaseSensor::GetNpuAttr(m_tSnsInfo.eSensorType, m_tSnsInfo.eSensorMode);
    }

    AX_S32 nRet = AX_NPU_SDK_EX_Init_with_attr(&npuAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_NPU_SDK_EX_Init_with_attr(%d) failed, ret=0x%x.", npuAttr.eHardMode, nRet);
    }
}

AX_BOOL CBaseSensor::Init(AX_U8 nSensorID, AX_U8 nDevID, AX_U8 nPipeID, AX_POOL_FLOORPLAN_T *stVbConf, AX_U8 &nCount)
{
    LOG_M(SENSOR, "[%d][%d][%d] +++", nSensorID, nDevID, nPipeID);
    m_nSensorID = nSensorID;
    m_nDeviceID = nDevID;
    m_nPipeID   = nPipeID;

    if (!InitISP()) {
        LOG_M_E(SENSOR, "Sensor %d init failed.", nSensorID);
        return AX_FALSE;
    }

    CalcBufSize(nPipeID, stVbConf, nCount, 10, 5);

    LOG_M(SENSOR, "[%d][%d][%d] ---", nSensorID, nDevID, nPipeID);
    return AX_TRUE;
}

AX_BOOL CBaseSensor::Open()
{
    LOG_M(SENSOR, "+++");

    LOG_M(SENSOR, "Sensor Attr => w:%d h:%d framerate:%d sensor mode:%d run mode: %d rawType:%d"
          , m_tSnsAttr.nWidth
          , m_tSnsAttr.nHeight
          , m_tSnsAttr.nFrameRate
          , m_tSnsAttr.eSnsMode
          , m_tSnsInfo.eRunMode
          , m_tSnsAttr.eRawType);

    AX_S32 nRet = 0;
    AX_VIN_SNS_DUMP_ATTR_T  tDumpAttr;

    nRet = AX_VIN_Create(m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_Create failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = RegisterSensor(m_nPipeID);
    if (AX_SDK_PASS != nRet) {
        LOG_M_E(SENSOR, "RegisterSensor failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_SetRunMode(m_nPipeID, m_tSnsInfo.eRunMode);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_SetRunMode failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_SetSnsAttr(m_nPipeID, &m_tSnsAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_SetSnsAttr failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_SetDevAttr(m_nPipeID, &m_tDevAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_SetDevAttr failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_MIPI_RX_Reset((AX_MIPI_RX_DEV_E)m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_MIPI_RX_Reset failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_MIPI_RX_SetAttr((AX_MIPI_RX_DEV_E)m_nPipeID, &m_tMipiRxAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_MIPI_RX_SetAttr failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_OpenSnsClk(m_nPipeID, m_tSnsClkAttr.nSnsClkIdx, m_tSnsClkAttr.eSnsClkRate);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_OpenSnsClk failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_SetChnAttr(m_nPipeID, &m_tChnAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_SetChnAttr failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_SetPipeAttr(m_nPipeID, &m_tPipeAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VI_SetPipeAttr failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    AX_VIN_DEV_BIND_PIPE_T tDevBindPipe;
    memset(&tDevBindPipe, 0, sizeof(AX_VIN_DEV_BIND_PIPE_T));
    tDevBindPipe.nNum = 1;
    tDevBindPipe.nPipeId[0] = m_nPipeID;
    nRet = AX_VIN_SetDevBindPipe(m_nPipeID, &tDevBindPipe);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_SetDevBindPipe failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_ISP_Open(m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_ISP_Open failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    //EIS Load bin, call after AX_ISP_Open and before AX_VIN_Start
    if (gOptions.IsEISSupport()) {
        SENSOR_CONFIG_T tSensorCfg;
        CConfigParser().GetInstance()->GetCameraCfg(tSensorCfg, (SENSOR_ID_E)m_nSensorID);
        AX_CHAR *chEISBinPath = (AX_SNS_LINEAR_MODE == m_tSnsAttr.eSnsMode) ? tSensorCfg.aEISSdrBin : tSensorCfg.aEISHdrBin;

        if (access(chEISBinPath, F_OK) != 0) {
            LOG_M_E(SENSOR, "Sns[%d] EIS bin(%s) is not exist.", m_nSensorID, chEISBinPath);
            return AX_FALSE;
        }

        AX_S32 nRet = AX_ISP_LoadBinParams(m_nPipeID, (const AX_CHAR *)chEISBinPath);
        if (0 != nRet) {
            LOG_M_E(SENSOR, "AX_ISP_LoadBinParams (%s) failed, ret=0x%x.", chEISBinPath, nRet);
            return AX_FALSE;
        }
        else {
            LOG_M(SENSOR, "AX_ISP_LoadBinParams (%s) sucess.", chEISBinPath);
        }
    }

    nRet = RegisterAeAlgLib(m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "RegisterAeAlgLib failed, ret=0x%x", nRet);
    }

    nRet = RegisterAwbAlgLib(m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "RegisterAwbAlgLib failed, ret=0x%x.", nRet);
    }

    nRet = AX_VIN_Start(m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_Start failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    nRet = AX_VIN_EnableDev(m_nPipeID);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_VIN_EnableDev failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    AX_PIPE_ATTR_T tPipeAttr = {0};
    AX_VIN_GetPipeAttr(m_nPipeID, &tPipeAttr);
    if (AX_PIPE_SOURCE_DEV_OFFLINE == tPipeAttr.ePipeDataSrc) {
        tDumpAttr.bEnable = AX_TRUE;
        tDumpAttr.nDepth = 2;
        nRet = AX_VIN_SetSnsDumpAttr(m_nPipeID, &tDumpAttr);
        if (0 != nRet) {
            LOG_M_E(SENSOR, " failed, ret=0x%x.\n", nRet);
            return AX_FALSE;
        }
    }

    AX_VIN_DUMP_ATTR_T tVinDumpAttr;
    memset(&tVinDumpAttr, 0x00, sizeof(tVinDumpAttr));
    tVinDumpAttr.bEnable = AX_TRUE;
    tVinDumpAttr.nDepth = (VIN_IFE_DUMP_ENABLE ? 1 : 0); // the depth will affect the block of raw10
    AX_VIN_SetPipeDumpAttr(m_nPipeID, VIN_DUMP_SOURCE_IFE, &tVinDumpAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "set VIN_DUMP_SOURCE_IFE fail, ret=0x%x.\n", nRet);
        return AX_FALSE;
    }

    memset(&tVinDumpAttr, 0x00, sizeof(tVinDumpAttr));
    tVinDumpAttr.bEnable = AX_TRUE;
    tVinDumpAttr.nDepth = (VIN_NPU_DUMP_ENABLE ? 1 : 0); // the depth will affect the block of raw16
    AX_VIN_SetPipeDumpAttr(m_nPipeID, VIN_DUMP_SOURCE_NPU, &tVinDumpAttr);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "set VIN_DUMP_SOURCE_NPU fail, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    // Set EIS IQ Params
    if (gOptions.IsEISSupport()) {
        if (!EnableEIS(gOptions.IsEnableEIS())) {
            LOG_M_E(SENSOR, "Enable EIS failed!");
            return AX_FALSE;
        }
    }

    AX_VIN_StreamOn(m_nPipeID); /* stream on */

    if (m_tSnsInfo.bSupportTuning) {
        nRet = AX_NT_StreamInit(6000);
        if (0 != nRet) {
            LOG_M_E(SENSOR, "AX_NT_StreamInit failed, ret=0x%x.", nRet);
            return AX_FALSE;
        }

        nRet = AX_NT_CtrlInit(m_tSnsInfo.nTuningPort);
        if (0 != nRet) {
            LOG_M_E(SENSOR, "AX_NT_CtrlInit failed, ret=0x%x.", nRet);
            return AX_FALSE;
        } else {
            LOG_M(SENSOR, "Enable tunning on port: %d\n", m_tSnsInfo.nTuningPort);
        }

        AX_NT_SetStreamSource(m_nPipeID);
    }

    LOG_M(SENSOR, "---");
    return AX_TRUE;
}

AX_BOOL CBaseSensor::Close()
{
    AX_VIN_SNS_DUMP_ATTR_T  tDumpAttr;

    LOG_M(SENSOR, "[%d][%d][%d] +++", m_nSensorID, m_nDeviceID, m_nPipeID);

    AX_VIN_StreamOff(m_nPipeID);

    AX_PIPE_ATTR_T tPipeAttr = {0};
    AX_VIN_GetPipeAttr(m_nPipeID, &tPipeAttr);
    if (AX_PIPE_SOURCE_DEV_OFFLINE == tPipeAttr.ePipeDataSrc) {
        tDumpAttr.bEnable = AX_FALSE;
        AX_VIN_SetSnsDumpAttr(m_nPipeID, &tDumpAttr);
    }
    AX_VIN_CloseSnsClk(m_tSnsClkAttr.nSnsClkIdx);
    AX_VIN_DisableDev(m_nPipeID);

    AX_VIN_Stop(m_nPipeID);
    UnRegisterAeAlgLib(m_nPipeID);
    UnRegisterAwbAlgLib(m_nPipeID);
    AX_ISP_Close(m_nPipeID);
    UnRegisterSensor(m_nPipeID);
    AX_VIN_Destory(m_nPipeID);

    if (m_tSnsInfo.bSupportTuning) {
        AX_NT_CtrlDeInit();
        AX_NT_StreamDeInit();
    }

    LOG_M(SENSOR, "[%d][%d][%d] ---", m_nSensorID, m_nDeviceID, m_nPipeID);

    return AX_TRUE;
}

AX_NPU_SDK_EX_ATTR_T CBaseSensor::GetNpuAttr(SNS_TYPE_E eSnsType, AX_SNS_HDR_MODE_E eSnsMode)
{
    AX_NPU_SDK_EX_ATTR_T npuAttr;

    switch (eSnsType) {
    case E_SNS_TYPE_OS04A10:
        if (eSnsMode == AX_SNS_HDR_3X_MODE) {
            npuAttr.eHardMode = AX_NPU_VIRTUAL_DISABLE;
        } else {
            npuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
        }
        break;
    case E_SNS_TYPE_IMX334:
        npuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
        break;
    case E_SNS_TYPE_GC4653:
        npuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
        break;
    case E_SNS_TYPE_OS08A20:
        npuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
        break;
    case E_SNS_TYPE_SC1345:
        npuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
        break;
    case E_SNS_TYPE_SC530AI:
        npuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
        break;
    default:
        npuAttr.eHardMode = AX_NPU_VIRTUAL_DISABLE;
        break;
    }

    return npuAttr;
}

AX_BOOL CBaseSensor::GetSnsAttr(AX_SNS_ATTR_T& attr)
{
    attr = m_tSnsAttr;
    return AX_TRUE;
}

AX_BOOL CBaseSensor::GetChnAttr(AX_VIN_CHN_ATTR_T& attr)
{
    attr = m_tChnAttr;
    return AX_TRUE;
}

AX_VOID CBaseSensor::UpdateSnsMode(AX_SNS_HDR_MODE_E eMode)
{
    m_tSnsInfo.eSensorMode = eMode;
}

AX_VOID CBaseSensor::UpdateFramerate(AX_F32 fFramerate)
{
    m_tSnsInfo.nFrameRate = (AX_U32)fFramerate;

    SetFps(m_nPipeID, fFramerate);
}

AX_BOOL CBaseSensor::EnableEIS(AX_BOOL bEnableEIS)
{
    AX_ISP_IQ_EIS_PARAM_T tEISParam;
    AX_S32 nRet = 0;

    nRet = AX_ISP_IQ_GetEisParam(m_nPipeID, &tEISParam);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_ISP_IQ_GetEisParam failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    tEISParam.bEisEnable    = bEnableEIS;
    tEISParam.nEisDelayNum  = gOptions.GetEISDelayNum();
    tEISParam.nCropRatioW   = gOptions.GetEISCropW();
    tEISParam.nCropRatioH   = gOptions.GetEISCropH();
    nRet = AX_ISP_IQ_SetEisParam(m_nPipeID, &tEISParam);
    if (0 != nRet) {
        LOG_M_E(SENSOR, "AX_ISP_IQ_SetEisParam failed, ret=0x%x.", nRet);
        return AX_FALSE;
    }

    if (tEISParam.bEisEnable) {
        LOG_M(SENSOR, "EIS Model: %s", tEISParam.tEisNpuParam.szModelName);
        LOG_M(SENSOR, "Wbt Model: %s", tEISParam.tEisNpuParam.szWbtModelName);
    }

    return AX_TRUE;
}

AX_BOOL CBaseSensor::BuffPoolInit(AX_POOL_FLOORPLAN_T *stVbConf, AX_U8 nCount)
{
    AX_S32 nRet = 0;

    AX_U8 i = 0, j = 0;
    AX_POOL_CONFIG_T v;

    /* Sort in ascending order */
    for (i = 0; i < nCount - 1; i++) {
        for (j = i + 1; j < nCount; j++) {
            if (stVbConf->CommPool[i].BlkSize > stVbConf->CommPool[j].BlkSize) {
                v.BlkSize = stVbConf->CommPool[i].BlkSize;
                v.BlkCnt = stVbConf->CommPool[i].BlkCnt;
                stVbConf->CommPool[i].BlkSize = stVbConf->CommPool[j].BlkSize;
                stVbConf->CommPool[i].BlkCnt = stVbConf->CommPool[j].BlkCnt;
                stVbConf->CommPool[j].BlkSize = v.BlkSize;
                stVbConf->CommPool[j].BlkCnt = v.BlkCnt;
            }
        }
    }

    /* Merge by size */
    for (i = 1, j = 0; i < nCount; i++) {
        if (stVbConf->CommPool[j].BlkSize != stVbConf->CommPool[i].BlkSize) {
            j += 1;
            stVbConf->CommPool[j].BlkSize = stVbConf->CommPool[i].BlkSize;
            stVbConf->CommPool[j].BlkCnt = stVbConf->CommPool[i].BlkCnt;
            if (i != j) {
                stVbConf->CommPool[i].BlkSize = 0;
                stVbConf->CommPool[i].BlkCnt = 0;
            }
        } else {
            stVbConf->CommPool[j].BlkCnt += stVbConf->CommPool[i].BlkCnt;
            stVbConf->CommPool[i].BlkSize = 0;
            stVbConf->CommPool[i].BlkCnt = 0;
        }
    }
    nCount = j + 1;

    for (AX_U32 i = 0; i < nCount; ++i) {
        LOG_M(SENSOR, "[%d] BlkSize:%lld, BlkCnt:%d", i, stVbConf->CommPool[i].BlkSize, stVbConf->CommPool[i].BlkCnt);
    }

    nRet = AX_POOL_SetConfig(stVbConf);
    if (nRet) {
        LOG_M_E(SENSOR, "AX_POOL_SetConfig fail!Error Code:0x%X", nRet);
        return AX_FALSE;
    } else {
        LOG_M(SENSOR, "AX_POOL_SetConfig success!");
    }

    nRet = AX_POOL_Init();
    if (nRet) {
        LOG_M_E(SENSOR, "AX_POOL_Init fail!!Error Code:0x%X", nRet);
    } else {
        LOG_M(SENSOR, "AX_POOL_Init success!");
    }

    return (nRet == 0) ? AX_TRUE : AX_FALSE;
}

AX_BOOL CBaseSensor::BuffPoolExit()
{
    AX_S32 nRet = AX_POOL_Exit();
    if (nRet) {
        LOG_M_E(SENSOR, "AX_POOL_Exit fail, ret=0x%x.", nRet);
    } else {
        LOG_M(SENSOR, "AX_POOL_Exit success!");
    }
    return (nRet == 0) ? AX_TRUE : AX_FALSE;
}

AX_VOID CBaseSensor::CalcBufSize(AX_U8 nPipe, AX_POOL_FLOORPLAN_T *stVbConf, AX_U8 &nCount, AX_U32 nPreISPBlkCnt, AX_U32 nNpuBlkCnt)
{
    AX_S32 nImgWidth = m_tDevAttr.tDevImgRgn.nWidth;
    AX_S32 nImgHeight = m_tDevAttr.tDevImgRgn.nHeight;
    AX_IMG_FORMAT_E eMaxImgFormat = (m_eImgFormatSDR >= m_eImgFormatHDR ? m_eImgFormatSDR : m_eImgFormatHDR);
    AX_IMG_FORMAT_E eFormat = (AX_ISP_PIPELINE_NONE_NPU == m_tSnsInfo.eRunMode ? AX_FORMAT_BAYER_RAW_14BPP : eMaxImgFormat);
    // fbc header
    if (AX_RAW_BLOCK_COUNT == 0) {
        if (HdrModeSupport(nPipe)) {
            stVbConf->CommPool[nCount].BlkCnt = (nPreISPBlkCnt + 3 * AX_SNS_HDR_2X_MODE);
        }
        else {
            stVbConf->CommPool[nCount].BlkCnt = (nPreISPBlkCnt + 3 * m_tSnsInfo.eSensorMode);
        }
    }
    else {
        if (HdrModeSupport(nPipe)) {
            stVbConf->CommPool[nCount].BlkCnt = AX_RAW_BLOCK_COUNT;
        }
        else {
            stVbConf->CommPool[nCount].BlkCnt = AX_RAW_BLOCK_COUNT_SDR;
        }
    }
    stVbConf->CommPool[nCount].BlkSize   = AX_VIN_GetImgBufferSize(nImgHeight, nImgWidth, eFormat, AX_TRUE);
    stVbConf->CommPool[nCount].MetaSize  = AX_RAW_META_SIZE * 1024;
    memset(stVbConf->CommPool[nCount].PartitionName, 0, sizeof(stVbConf->CommPool[nCount].PartitionName));
    strcpy((char *)stVbConf->CommPool[nCount].PartitionName, "anonymous");
    nCount += 1;

    nImgWidth = m_tPipeAttr.nWidth;
    nImgHeight = m_tPipeAttr.nHeight;
    // fbc header
    stVbConf->CommPool[nCount].BlkCnt    = (AX_NPU_BLOCK_COUNT == 0) ? nNpuBlkCnt : AX_NPU_BLOCK_COUNT;
    stVbConf->CommPool[nCount].BlkSize   = AX_VIN_GetImgBufferSize(nImgHeight, nImgWidth, AX_FORMAT_BAYER_RAW_16BPP, AX_TRUE);
    stVbConf->CommPool[nCount].MetaSize  = AX_NPU_META_SIZE * 1024;
    memset(stVbConf->CommPool[nCount].PartitionName, 0, sizeof(stVbConf->CommPool[nCount].PartitionName));
    strcpy((char *)stVbConf->CommPool[nCount].PartitionName, "anonymous");
    nCount += 1;

    for (AX_U32 nChn = 0; nChn < MAX_ISP_CHANNEL_NUM; nChn++) {
        if (m_tChnAttr.tChnAttr[nChn].bEnable) {
            stVbConf->CommPool[nCount].MetaSize   = AX_YUV_META_SIZE * 1024;
            stVbConf->CommPool[nCount].BlkSize    = AX_VIN_GetImgBufferSize(m_tChnAttr.tChnAttr[nChn].nHeight, m_tChnAttr.tChnAttr[nChn].nWidthStride, m_tChnAttr.tChnAttr[nChn].eImgFormat, AX_TRUE);
            stVbConf->CommPool[nCount].BlkCnt     = m_tChnAttr.tChnAttr[nChn].nDepth + (gOptions.IsLinkMode() ? YUV_BLOCK_COUNT_ADDATION_LINK : YUV_BLOCK_COUNT_ADDATION_NORMAL);
            //for WBT_SDR_WNR
            if ((0 == nChn)/* && (m_tSnsInfo.eSensorMode == AX_SNS_LINEAR_MODE)*/) {
                stVbConf->CommPool[nCount].BlkCnt += YUV_BLOCK_COUNT_ADDATION_WBT_SDR_WNR;
            }

            //DETECTION
            if (gOptions.IsActivedDetect()) {
                //jenc for detection
                if (1 == nChn) {
                    stVbConf->CommPool[nCount].BlkCnt += AX_AI_DETECT_BLOCK_COUNT;
                }
            }
            //for EIS
            if (gOptions.IsEISSupport()) {
                stVbConf->CommPool[nCount].BlkCnt += m_tChnAttr.tChnAttr[nChn].nDepth;
            }
            stVbConf->CommPool[nCount].CacheMode  = POOL_CACHE_MODE_NONCACHE;

            memset(stVbConf->CommPool[nCount].PartitionName, 0, sizeof(stVbConf->CommPool[nCount].PartitionName));
            strcpy((char *)stVbConf->CommPool[nCount].PartitionName, "anonymous");

            nCount += 1;
        }
    }
}

AX_S8 CBaseSensor::GetI2cDevNode(AX_U8 nPipe)
{
    AX_S8 nBusNum = 0;
    AX_U8 board_id = 0;
    FILE *pFile = NULL;
    AX_CHAR id[10] = {0};

    pFile = fopen("/sys/devices/platform/hwinfo/board_id", "r");
    if (pFile) {
        fread(&id[0], 10, 1, pFile);
        fclose(pFile);
    } else {
        LOG_M_E(SENSOR, "Failed to open /sys/devices/platform/hwinfo/board_id!");
    }

    board_id = atoi(id);
    if (0 == strncasecmp("F", id, 1)) {
       board_id = 15;
    }

    LOG_M(SENSOR, "Board id: %d", board_id);

    if (0 == board_id || 1 == board_id) {
        if (0 == nPipe || 1 == nPipe) {
            nBusNum = 0;
        } else {
            nBusNum = 1;
        }
    } else if (2 == board_id || 3 == board_id || 15 == board_id) {
        if (0 == nPipe) {
            nBusNum = 0;
        } else if (1 == nPipe) {
            nBusNum = 1;
        } else {
            nBusNum = 6;
        }
    } else {
        LOG_M_E(SENSOR, "Board id: %d error!", board_id);
        return -1;
    }

    return nBusNum;
}

AX_VOID CBaseSensor::InitSensor(AX_U8 nPipe)
{
    if (m_pSnsObj && m_pSnsObj->pfn_sensor_init) {
        m_pSnsObj->pfn_sensor_init(nPipe);
    }
}

AX_VOID CBaseSensor::ExitSensor(AX_U8 nPipe)
{
    if (m_pSnsObj && m_pSnsObj->pfn_sensor_exit) {
        m_pSnsObj->pfn_sensor_exit(nPipe);
    }
}
