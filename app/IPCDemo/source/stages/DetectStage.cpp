/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "DetectStage.h"
#include "MediaFrame.h"
#include "OptionHelper.h"
#include "PrintHelper.h"
#include "StageOptionHelper.h"
#include "CommonUtils.h"
#include "Od.h"
#include "Md.h"

#define DETECT "DETECT"

extern COptionHelper gOptions;
extern CPrintHelper gPrintHelper;
extern AX_VOID IVPSConfigDetectModeRelated(CDetectStage *pStage);

CDetectStage::CDetectStage(AX_U8 nChannel /*= 0*/, AX_U8 nInnerIndex /*= 0*/)
: CStage(DETECT)
, m_nChannel(nChannel)
, m_nCfgIndex(nInnerIndex)
, m_bReseting(AX_FALSE)
{

}

CDetectStage::CDetectStage(AX_VOID)
: CStage(DETECT)
, m_nChannel(0)
, m_nCfgIndex(0)
, m_bReseting(AX_FALSE)
{

}

CDetectStage::~CDetectStage(AX_VOID)
{

}

AX_VOID CDetectStage::SetChannel(AX_U8 nChannel, AX_U8 nInnerIndex)
{
    m_nChannel = nChannel;
    m_nCfgIndex = nInnerIndex;
}

AX_VOID CDetectStage::BindCropStage(CTrackCropStage* pStage)
{
    m_pTrackCropStage = pStage;
}

AX_VOID CDetectStage::FrameStrategy(AX_U64 nDetectFrameId, AX_BOOL &bAiFrameSkip, AX_BOOL &bIvesFrameSkip)
{
    AX_U32 nFrame_id = (AX_U32)nDetectFrameId;

    AX_S32 nSrcFrameRate = m_tStageInfo.nFrmFps;
    AX_S32 nAlgoFramerate = CStageOptionHelper().GetInstance()->GetAiAttr().tConfig.nAiFps;
    AX_S32 nAlgoIvesFramerate = CStageOptionHelper().GetInstance()->GetAiAttr().tConfig.nIvesFps;

    if (nSrcFrameRate <= nAlgoFramerate) {
        bAiFrameSkip = AX_FALSE;

        // set ives framerate to one
        nAlgoIvesFramerate = 1;

        bIvesFrameSkip = CCommonUtils::FrameSkipCtrl(nSrcFrameRate, nAlgoIvesFramerate, nFrame_id);
    }
    else {
        bAiFrameSkip = CCommonUtils::FrameSkipCtrl(nSrcFrameRate, nAlgoFramerate, nFrame_id);

        // if ai skip, will continue to ives handle
        if (bAiFrameSkip) {
            static AX_U32 nIvesFrame_id = 0;

            ++nIvesFrame_id;

            AX_S32 nSrcAlgoIvesFramerate = nSrcFrameRate - nAlgoFramerate;

            bIvesFrameSkip = CCommonUtils::FrameSkipCtrl(nSrcAlgoIvesFramerate, nAlgoIvesFramerate, nIvesFrame_id);
        }
        else {
            bIvesFrameSkip = AX_TRUE;
        }
    }
}

AX_BOOL CDetectStage::ProcessFrame(CMediaFrame* pFrame)
{
    AX_BOOL bAiFrameSkip = AX_FALSE;
    AX_BOOL bIvesFrameSkip = AX_FALSE;

    ++m_nDetectFrameId;

    FrameStrategy(m_nDetectFrameId, bAiFrameSkip, bIvesFrameSkip);

    auto startTime = std::chrono::steady_clock::now();

    if (!bIvesFrameSkip) {
        m_mtxReset.lock();
        if (!m_bReseting) {
            if (gOptions.IsActivedOcclusionDetect()) {
                COD::GetInstance()->ProcessFrame(pFrame);
            }

            if (gOptions.IsActivedMotionDetect()) {
                CMD::GetInstance()->ProcessFrame(pFrame);
            }
        }
        m_mtxReset.unlock();
    }

    auto endTime = std::chrono::steady_clock::now();
    AX_U32 nElapsed = (AX_U32)(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

    if (!bAiFrameSkip) {
        if (gOptions.IsActivedDetect() && gOptions.IsActivedDetectFromWeb()) {
            AX_S32 nAIDetectGaps = -1;
            AX_S32 nSrcFrameRate = m_tStageInfo.nFrmFps;

            if (nSrcFrameRate > 0) {
                nAIDetectGaps = 1000 / nSrcFrameRate - nElapsed - DETECT_MARGIN_GAP_TIME;
                if (nAIDetectGaps < 0) {
                    nAIDetectGaps = 0;
                }
            }

            if (CDetector::GetInstance()->ProcessDetect(pFrame, nAIDetectGaps)) {
                gPrintHelper.Add(E_PH_MOD_DET, 0, m_nChannel);
            }
        }
    }

    return AX_TRUE;
}

AX_BOOL CDetectStage::Init()
{
    if (gOptions.IsActivedDetect()) {
        if (!CDetector::GetInstance()->Startup()) {
            gOptions.SetDetectActived(AX_FALSE);
            LOG_M_E(DETECT, "NPU detector init failed.");
        }
        else {
            CDetector::GetInstance()->BindCropStage(m_pTrackCropStage);

            IVPSConfigDetectModeRelated(this);
        }
    }

    if (0 == m_tStageInfo.nFrmWidth || 0 == m_tStageInfo.nFrmHeight ||
        0 == m_tStageInfo.nVinFps || 0 == m_tStageInfo.nFrmFps) {
        LOG_M_E(DETECT, "invalid stage frame info, %dx%d vin fps: %d, frm fps: %d", \
                        m_tStageInfo.nFrmWidth, m_tStageInfo.nFrmHeight, m_tStageInfo.nVinFps, m_tStageInfo.nFrmFps);
        return AX_FALSE;
    }

    LOG_M_W(DETECT, "stage frame info, %dx%d vin fps: %d, frm fps: %d", \
                     m_tStageInfo.nFrmWidth, m_tStageInfo.nFrmHeight, m_tStageInfo.nVinFps, m_tStageInfo.nFrmFps);

    const AX_U32 &nFrmW = m_tStageInfo.nFrmWidth;
    const AX_U32 &nFrmH = m_tStageInfo.nFrmHeight;

    //if (gOptions.IsActivedOcclusionDetect()) {
    {
        if (!COD::GetInstance()->Startup(m_tStageInfo.nVinFps, nFrmW, nFrmH)) {
            return AX_FALSE;
        }

        const AX_U32 nAreaW = nFrmW / 2;
        const AX_U32 nAreaH = nFrmH / 2;

        const AX_U32 nX = nFrmW/2 - nAreaW/2;
        const AX_U32 nY = nFrmH/2 - nAreaH/2;
        COD::GetInstance()->AddArea(nX, nY, nAreaW, nAreaH, nFrmW, nFrmH);
    }

    // if (gOptions.IsActivedMotionDetect()) {
    {
        if (!CMD::GetInstance()->Startup(nFrmW, nFrmH)) {
            return AX_FALSE;
        }

        const AX_U32 nAreaW = nFrmW / 2;
        const AX_U32 nAreaH = nFrmH / 2;

        const AX_U32 nX = nFrmW/2 - nAreaW/2;
        const AX_U32 nY = nFrmH/2 - nAreaH/2;
        CMD::GetInstance()->AddArea(nX, nY, nAreaW, nAreaH, nFrmW, nFrmH);
    }

    return CStage::Init();
}

AX_VOID CDetectStage::DeInit()
{
    if (gOptions.IsActivedDetect()) {
        CDetector::GetInstance()->Cleanup();
    }

    CMD::GetInstance()->Cleanup();
    COD::GetInstance()->Cleanup();

    CStage::DeInit();
}

AX_BOOL CDetectStage::Reset(const DETECT_STAGE_INFO_T &tInfo)
{
    m_mtxReset.lock();
    m_bReseting = AX_TRUE;

    CMD::GetInstance()->Cleanup();
    COD::GetInstance()->Cleanup();

    ConfigStageInfo(tInfo);

    const AX_U32 &nFrmW = m_tStageInfo.nFrmWidth;
    const AX_U32 &nFrmH = m_tStageInfo.nFrmHeight;

    //if (gOptions.IsActivedOcclusionDetect()) {
    {
        if (!COD::GetInstance()->Startup(m_tStageInfo.nVinFps, nFrmW, nFrmH)) {
            return AX_FALSE;
        }

        const AX_U32 nAreaW = nFrmW / 2;
        const AX_U32 nAreaH = nFrmH / 2;

        const AX_U32 nX = nFrmW/2 - nAreaW/2;
        const AX_U32 nY = nFrmH/2 - nAreaH/2;
        COD::GetInstance()->AddArea(nX, nY, nAreaW, nAreaH, nFrmW, nFrmH);
    }

    // if (gOptions.IsActivedMotionDetect()) {
    {
        if (!CMD::GetInstance()->Startup(nFrmW, nFrmH)) {
            return AX_FALSE;
        }

        const AX_U32 nAreaW = nFrmW / 2;
        const AX_U32 nAreaH = nFrmH / 2;

        const AX_U32 nX = nFrmW/2 - nAreaW/2;
        const AX_U32 nY = nFrmH/2 - nAreaH/2;
        CMD::GetInstance()->AddArea(nX, nY, nAreaW, nAreaH, nFrmW, nFrmH);
    }


    m_bReseting = AX_FALSE;

    m_mtxReset.unlock();

    return AX_TRUE;
}

AX_VOID CDetectStage::ConfigStageInfo(const DETECT_STAGE_INFO_T &tInfo) {
    m_tStageInfo = tInfo;
    LOG_M_I(DETECT, "frame: %dx%d, vin fps: %d, input fps: %d", \
                    tInfo.nFrmWidth, tInfo.nFrmHeight, tInfo.nVinFps, tInfo.nFrmFps);
}
