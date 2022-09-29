/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "StageOptionHelper.h"
#include "OptionHelper.h"
#include "IVPSStage.h"
#include "JsonCfgParser.h"
#include "Capture.h"
#include "Md.h"
#include "Od.h"

extern COptionHelper gOptions;
extern IVPS_GROUP_CFG_T g_tIvpsGroupConfig[IVPS_GROUP_NUM];
extern END_POINT_OPTIONS g_tEPOptions[MAX_VENC_CHANNEL_NUM];

CStageOptionHelper::CStageOptionHelper(void)
{
}

CStageOptionHelper::~CStageOptionHelper(void)
{
}

AX_BOOL CStageOptionHelper::Init()
{
    for (const auto& item : g_tEPOptions) {
        AX_U32 nDefaultBitrate = 4096;
        if (item.eEPType == E_END_POINT_VENC) {
            VIDEO_CONFIG_T tVEncConfig;
            if (CConfigParser().GetInstance()->GetVideoCfgByID(item.nInnerIndex, tVEncConfig)) {
                nDefaultBitrate = tVEncConfig.nBitrate;
            }
        }

        AX_U32 nChannelID = item.nChannel;
        AX_U32 nISPChn = CIVPSStage::GetIspChnIndex(nChannelID);
        AX_U32 nIvpsInnerIndex = CIVPSStage::GetIvpsChnIndex(nChannelID);
        VIDEO_ATTR_T videoAttr;
        videoAttr.bit_rate = nDefaultBitrate;
        videoAttr.width = g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][0];
        videoAttr.height = g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][1];
        videoAttr.enable = AX_TRUE;

        m_mapVideo.emplace(nChannelID, videoAttr);
    }

    SENSOR_CONFIG_T tSensorCfg;
    CConfigParser().GetInstance()->GetCameraCfg(tSensorCfg, E_SENSOR_ID_0);

    m_tCamera.nHdrEnable        = AX_SNS_LINEAR_MODE == tSensorCfg.eSensorMode ? 0 : 1;
    m_tCamera.nRotation         = (AX_U8)gOptions.GetRotation();
    m_tCamera.nMirror           = (AX_U8)gOptions.GetMirror();
    m_tCamera.nFlip             = (AX_U8)gOptions.GetFlip();
    m_tCamera.nFramerate        = tSensorCfg.nFrameRate;
    m_tCamera.nDaynightMode     = 0;
    m_tCamera.nNrMode           = 1;
    m_tCamera.nEISSupport       = gOptions.IsEISSupport() ? 1 : 0;
    m_tCamera.nEISEnable        = gOptions.IsEnableEIS() ? 1 : 0;

    InitAiAttr();

    InitHotBalanceAttr();

    m_bSnapshotOpen = AX_FALSE;

    return AX_TRUE;
}

AX_BOOL CStageOptionHelper::InitAiAttr()
{
    AI_CONFIG_T tAiConfig;

    memset(&tAiConfig, 0x00, sizeof(tAiConfig));
    if (CConfigParser().GetInstance()->GetAiCfg(0, tAiConfig)) {
        m_tAiAttr.tConfig = tAiConfig;
    }
    else {
        m_tAiAttr.tConfig.nDetectFps = DETECT_DEFAULT_FRAMERATE_CTRL;
        m_tAiAttr.tConfig.nAiFps = DETECT_DEFAULT_AI_FRAMERATE_CTRL;
        m_tAiAttr.tConfig.nIvesFps = DETECT_DEFAULT_IVES_FRAMERATE_CTRL;
        m_tAiAttr.tConfig.nWidth = DETECT_DEFAULT_WIDTH;
        m_tAiAttr.tConfig.nHeight = DETECT_DEFAULT_HEIGHT;
    }

    m_tAiAttr.nEnable = gOptions.IsActivedDetect();
    m_tAiAttr.eDetectModel = E_AI_DETECT_MODEL_TYPE_FACEHUMAN;
    m_tAiAttr.tPushStrgy.eMode = E_AI_DETECT_PUSH_MODE_TYPE_BEST;
    m_tAiAttr.tPushStrgy.nInterval = 2000;
    m_tAiAttr.tPushStrgy.nCount = 1;
    m_tAiAttr.tPushStrgy.nPushSameFrame = 1;

    m_tAiAttr.tEvents.tMD.nEnable = gOptions.IsActivedMotionDetect();
    AX_U8 mdThrd;
    AX_U8 mdConfidence;
    CMD::GetInstance()->GetDefaultThresholdY(mdThrd, mdConfidence);
    m_tAiAttr.tEvents.tMD.nThrsHoldY = mdThrd;
    m_tAiAttr.tEvents.tMD.nConfidence = mdConfidence;

    m_tAiAttr.tEvents.tOD.nEnable = gOptions.IsActivedOcclusionDetect();
    AX_U8 odThrd;
    AX_U8 odConfidence;
    COD::GetInstance()->GetDefaultThresholdY(odThrd, odConfidence);
    m_tAiAttr.tEvents.tOD.nThrsHoldY = odThrd;
    m_tAiAttr.tEvents.tOD.nConfidence = odConfidence;

    m_tAiAttr.nDetectOnly         = 0;
    if (E_AI_DETECT_MODEL_TYPE_FACEHUMAN == m_tAiAttr.eDetectModel) {
        m_tAiAttr.tHumanFaceSetting.tFace.nEnable       = AI_TRACK_FACE_ENABLE;
        m_tAiAttr.tHumanFaceSetting.tFace.nDrawRect     = AI_DRAW_RECT_FACE_ENABLE;
        m_tAiAttr.tHumanFaceSetting.tBody.nEnable       = AI_TRACK_BODY_ENABLE;
        m_tAiAttr.tHumanFaceSetting.tBody.nDrawRect     = AI_DRAW_RECT_BODY_ENABLE;
        m_tAiAttr.tHumanFaceSetting.nEnableFI           = gOptions.IsActivedSearch();
    } else if (E_AI_DETECT_MODEL_TYPE_HVCFP == m_tAiAttr.eDetectModel) {
        m_tAiAttr.tHvcfpSetting.tFace.nEnable           = AI_TRACK_FACE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tFace.nDrawRect         = AI_DRAW_RECT_FACE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tBody.nEnable           = AI_TRACK_BODY_ENABLE;
        m_tAiAttr.tHvcfpSetting.tBody.nDrawRect         = AI_DRAW_RECT_BODY_ENABLE;
        m_tAiAttr.tHvcfpSetting.tVechicle.nEnable       = AI_TRACK_VEHICLE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tVechicle.nDrawRect     = AI_DRAW_RECT_VEHICLE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tCycle.nEnable          = AI_TRACK_CYCLE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tCycle.nDrawRect        = AI_DRAW_RECT_CYCLE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tPlate.nEnable          = AI_TRACK_PLATE_ENABLE;
        m_tAiAttr.tHvcfpSetting.tPlate.nDrawRect        = AI_DRAW_RECT_PLATE_ENABLE;
        m_tAiAttr.tHvcfpSetting.nEnablePI               = 1;
    }

    return AX_TRUE;
}

CAMERA_ATTR_T CStageOptionHelper::GetCamera()
{
    std::lock_guard<std::mutex> lck(m_mtxOption);
    return m_tCamera;
}

AX_VOID CStageOptionHelper::SetCamera(const CAMERA_ATTR_T& tCamera)
{
    std::lock_guard<std::mutex> lck(m_mtxOption);
    m_tCamera.nHdrEnable            = tCamera.nHdrEnable;
    m_tCamera.nRotation             = tCamera.nRotation;
    m_tCamera.nMirror               = tCamera.nMirror;
    m_tCamera.nFlip                 = tCamera.nFlip;
    m_tCamera.nFramerate            = tCamera.nFramerate;
    m_tCamera.nDaynightMode         = tCamera.nDaynightMode;
    m_tCamera.nNrMode               = tCamera.nNrMode;
    m_tCamera.nEISSupport           = tCamera.nEISSupport;
    m_tCamera.nEISEnable            = tCamera.nEISEnable;
}

AX_VOID CStageOptionHelper::SetVideo(AX_U32 nChan, const VIDEO_ATTR_T& tVideo)
{
    std::lock_guard<std::mutex> lck(m_mtxOption);
    m_mapVideo[nChan] = tVideo;
}

VIDEO_ATTR_T CStageOptionHelper::GetVideo(AX_U32 nChan)
{
    std::lock_guard<std::mutex> lck(m_mtxOption);
    return m_mapVideo[nChan];
}

AX_BOOL CStageOptionHelper::GetCameraStr(AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    std::lock_guard<std::mutex> lck(m_mtxOption);

    AX_S32 nCount = snprintf(pOutBuf, nSize,
            "{sns_work_mode: %d, rotation: %d, mirror: %s, flip: %s, framerate: %d, daynight: %d, nr_mode: %s, eis_support: %s, eis: %s}",
            m_tCamera.nHdrEnable,
            m_tCamera.nRotation,
            m_tCamera.nMirror ? "true" : "false",
            m_tCamera.nFlip ? "true" : "false",
            m_tCamera.nFramerate,
            m_tCamera.nDaynightMode,
            m_tCamera.nNrMode ? "true" : "false",
            m_tCamera.nEISSupport ? "true" : "false",
            m_tCamera.nEISEnable ? "true" : "false");

    return nCount > 0 ? AX_TRUE : AX_FALSE;
}

std::string CStageOptionHelper::GetFramerateOptStr()
{
    std::lock_guard<std::mutex> lck(m_mtxOption);
    const AX_U8 nSensorSize = 6;
    static const std::string arySensorFramerateOpts[nSensorSize] = {
        "[25, 30]", // os04a10
        "[30]",     // imx334
        "[25, 30]", // GC4653
        "[25, 30]", // os08a20
        "[25, 30]", // sc1345
        "[25, 30]", // sc530ai
    };
    AX_U8 nSensorID = gOptions.GetSensorID();
    return ((nSensorID < nSensorSize) ? arySensorFramerateOpts[nSensorID] : "");
}

AI_ATTR_T CStageOptionHelper::GetAiAttr()
{
    std::lock_guard<std::mutex> lck(m_mtxAi);
    return m_tAiAttr;
}

AX_VOID CStageOptionHelper::SetAiAttr(const AI_ATTR_T& tAttr)
{
    std::lock_guard<std::mutex> lck(m_mtxAi);
    m_tAiAttr = tAttr;
}

AX_BOOL CStageOptionHelper::GetAiInfoStr(AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    std::lock_guard<std::mutex> lck(m_mtxAi);

    AX_CHAR szPushStrgy[128] = {0};
    AX_CHAR szDetModelAttr[512] = {0};
    AX_CHAR szEvent[256] = {0};

    if (!GetPushStrgyStr(szPushStrgy, 128) || !GetDetectModelAttrStr(szDetModelAttr, 512) || !GetEventsStr(szEvent, 256)) {
        return AX_FALSE;
    }

    AX_S32 nCount = snprintf(pOutBuf, nSize, "{ai_enable: %s, \
        detect_model: %s, \
        detect_fps: %d, \
        push_strategy: {%s}, \
        detect_only: %s, \
        %s: {%s}, \
        events: {%s}}",
        ADAPTER_INT2BOOLSTR(m_tAiAttr.nEnable),
        GetDetectModelStr().c_str(),
        m_tAiAttr.tConfig.nAiFps,
        szPushStrgy,
        ADAPTER_INT2BOOLSTR(m_tAiAttr.nDetectOnly),
        GetDetectModelStr().c_str(),
        szDetModelAttr,
        szEvent);

    return nCount > 0 ? AX_TRUE : AX_FALSE;
}

std::string CStageOptionHelper::GetDetectModelStr()
{
    if (E_AI_DETECT_MODEL_TYPE_FACEHUMAN == m_tAiAttr.eDetectModel) {
        return "facehuman";
    } else if (E_AI_DETECT_MODEL_TYPE_HVCFP == m_tAiAttr.eDetectModel) {
        return "hvcfp";
    } else {
        return "";
    }
}

std::string CStageOptionHelper::GetPushModeStr()
{
    switch (m_tAiAttr.tPushStrgy.eMode) {
        case E_AI_DETECT_PUSH_MODE_TYPE_FAST:
            return "FAST";
        case E_AI_DETECT_PUSH_MODE_TYPE_INTERVAL:
            return "INTERVAL";
        case E_AI_DETECT_PUSH_MODE_TYPE_BEST:
            return "BEST_FRAME";
        default:
            return "BEST_FRAME";
    }
}

AI_DETECT_PUSH_MODE_TYPE_E CStageOptionHelper::TransPushMode(const AX_CHAR* pszPushMode)
{
    if (nullptr == pszPushMode) {
        return E_AI_DETECT_PUSH_MODE_TYPE_BEST;
    }

    if (strcmp(pszPushMode, "FAST") == 0) {
        return E_AI_DETECT_PUSH_MODE_TYPE_FAST;
    } else if (strcmp(pszPushMode, "INTERVAL") == 0) {
        return E_AI_DETECT_PUSH_MODE_TYPE_INTERVAL;
    } else if (strcmp(pszPushMode, "BEST_FRAME") == 0) {
        return E_AI_DETECT_PUSH_MODE_TYPE_BEST;
    } else {
        return E_AI_DETECT_PUSH_MODE_TYPE_BEST;
    }
}

AX_BOOL CStageOptionHelper::GetDetectModelAttrStr(AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    AX_S32 nCount = 0;
    if (E_AI_DETECT_MODEL_TYPE_FACEHUMAN == m_tAiAttr.eDetectModel) {
        nCount = snprintf(pOutBuf, nSize, "face_detect: {enable: %s, draw_rect: %s}, \
            body_detect: {enable: %s, draw_rect: %s}, \
            face_identify: {enable: %s}",
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHumanFaceSetting.tFace.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHumanFaceSetting.tFace.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHumanFaceSetting.tBody.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHumanFaceSetting.tBody.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHumanFaceSetting.nEnableFI));
    } else if (E_AI_DETECT_MODEL_TYPE_HVCFP == m_tAiAttr.eDetectModel) {
        nCount = snprintf(pOutBuf, nSize, "face_detect: {enable: %s, draw_rect: %s}, \
            body_detect: {enable: %s, draw_rect: %s}, \
            vechicle_detect: {enable: %s, draw_rect: %s}, \
            cycle_detect: {enable: %s, draw_rect: %s}, \
            plate_detect: {enable: %s, draw_rect: %s}, \
            plate_identify: {enable: %s}",
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tFace.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tFace.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tBody.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tBody.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tVechicle.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tVechicle.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tCycle.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tCycle.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tPlate.nEnable),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.tPlate.nDrawRect),
            ADAPTER_INT2BOOLSTR(m_tAiAttr.tHvcfpSetting.nEnablePI));
    }

    return nCount > 0 ? AX_TRUE : AX_FALSE;
}

AX_BOOL CStageOptionHelper::GetPushStrgyStr(AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    AX_S32 nCount = snprintf(pOutBuf, nSize, "push_mode: %s, push_interval:%d, push_count: %d, push_same_frame: %s",
        GetPushModeStr().c_str(),
        m_tAiAttr.tPushStrgy.nInterval,
        m_tAiAttr.tPushStrgy.nCount,
        ADAPTER_INT2BOOLSTR(m_tAiAttr.tPushStrgy.nPushSameFrame));

    return nCount > 0 ? AX_TRUE : AX_FALSE;
}

AX_BOOL CStageOptionHelper::GetEventsStr(AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    AX_S32 nCount = snprintf(pOutBuf, nSize, "motion_detect: {enable: %s, threshold_y: %d, confidence: %d}, \
        occlusion_detect: {enable: %s, threshold_y: %d, confidence: %d}",
        ADAPTER_INT2BOOLSTR(m_tAiAttr.tEvents.tMD.nEnable),
        m_tAiAttr.tEvents.tMD.nThrsHoldY,
        m_tAiAttr.tEvents.tMD.nConfidence,
        ADAPTER_INT2BOOLSTR(m_tAiAttr.tEvents.tOD.nEnable),
        m_tAiAttr.tEvents.tOD.nThrsHoldY,
        m_tAiAttr.tEvents.tOD.nConfidence);

    return nCount > 0 ? AX_TRUE : AX_FALSE;
}

AX_BOOL CStageOptionHelper::InitHotBalanceAttr()
{
    HOTBALANCE_CONFIG_T tConfig;

    if (CConfigParser().GetInstance()->GetHotBalanceCfg(tConfig)) {
        m_tHotBalanceAttr.tConfig = tConfig;
    }
    else {
        m_tHotBalanceAttr.tConfig.bEnable = AX_FALSE;
        m_tHotBalanceAttr.tConfig.fThersholdM = HOTBALANCE_THERSHOLD_MEDIAN_DEFAULT;
        m_tHotBalanceAttr.tConfig.fThersholdL = HOTBALANCE_THERSHOLD_LOW_DEFAULT;
        m_tHotBalanceAttr.tConfig.fGap = HOTBALANCE_THERSHOLD_GAP_DEFAULT;
    }

    return AX_TRUE;
}

HOTBALANCE_ATTR_T CStageOptionHelper::GetHotBalanceAttr()
{
    std::lock_guard<std::mutex> lck(m_mtxHotBalance);
    return m_tHotBalanceAttr;
}

AX_VOID CStageOptionHelper::SetHotBalanceAttr(const HOTBALANCE_ATTR_T& tAttr)
{
    std::lock_guard<std::mutex> lck(m_mtxHotBalance);
    m_tHotBalanceAttr = tAttr;
}

AX_S32 CStageOptionHelper::Snapshot(AX_U32 nChn, AX_U32 nQpLevel, SNAPSHOT_CALLBACK_FUNC callback)
{
    {
        std::lock_guard<std::mutex> lck(m_mtxSnapshot);
        m_bSnapshotOpen = AX_TRUE;
    }

    AX_VOID *pBuf = NULL;
    AX_U32 nBufSize = 0;

    CCapture *pInst = CCapture::GetInstance();

    pInst->CapturePicture(nChn, &pBuf, &nBufSize, nQpLevel);

    if (pBuf && nBufSize > 0) {
        if (callback) {
            callback(nChn, pBuf, nBufSize);
        }

        pInst->FreeCapturePicture(nChn, pBuf);
    }

    {
        std::lock_guard<std::mutex> lck(m_mtxSnapshot);
        m_bSnapshotOpen = AX_FALSE;
    }

    return 0;
};

AX_BOOL CStageOptionHelper::IsSnapshotOpen()
{
    std::lock_guard<std::mutex> lck(m_mtxSnapshot);

    return m_bSnapshotOpen;
};

AX_BOOL CStageOptionHelper::StatVencOutBytes(AX_U32 nVencInner, AX_U32 nBytes)
{
    if (0 == m_mapStatInfo[nVencInner].nStartTick) {
        m_mapStatInfo[nVencInner].nStartTick = CTimeUtils::GetTickCount();
    }

    m_mapStatInfo[nVencInner].nVencOutBytes += nBytes;
    return AX_TRUE;
}

AX_BOOL CStageOptionHelper::GetAssistBitrateStr(AX_U32 nVencInner, AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    AX_U64 nEndTick = CTimeUtils::GetTickCount();
    AX_U64 nGap = nEndTick - m_mapStatInfo[nVencInner].nStartTick;

    AX_F64 fBitrate = m_mapStatInfo[nVencInner].nVencOutBytes / (AX_F64)nGap * 8;

    AX_S32 nCount = snprintf(pOutBuf, nSize, "%.2fkbps", fBitrate);
    if (nCount <= 0) {
        return AX_FALSE;
    }

    m_mapStatInfo[nVencInner].nVencOutBytes = 0;
    m_mapStatInfo[nVencInner].nStartTick = nEndTick;

    return AX_TRUE;
}

AX_BOOL CStageOptionHelper::GetAssistResStr(AX_U32 nUniChn, AX_CHAR* pOutBuf, AX_U32 nSize)
{
    if (nullptr == pOutBuf || 0 == nSize) {
        return AX_FALSE;
    }

    VIDEO_ATTR_T tAttr = GetVideo(nUniChn);
    CAMERA_ATTR_T tCameraAttr = GetCamera();
    if (AX_IVPS_ROTATION_90 == tCameraAttr.nRotation || AX_IVPS_ROTATION_270 == tCameraAttr.nRotation) {
        ::swap(tAttr.width, tAttr.height);
    }

    AX_S32 nCount = snprintf(pOutBuf, nSize, "%dx%d", tAttr.width, tAttr.height);
    return nCount > 0 ? AX_TRUE : AX_FALSE;
}
