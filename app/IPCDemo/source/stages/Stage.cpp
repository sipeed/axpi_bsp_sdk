/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "Stage.h"

#define STAGE "STAGE"

CStage::CStage(const string& strName)
{
   m_strStageName = strName;
   m_pNextStage = nullptr;
   m_bProcessFrameWorking = AX_FALSE;
   m_pProcFrameThread =  nullptr;
}

CStage::~CStage()
{

}

CStage* CStage::BindNextStage(CStage* pNext)
{
    if (!pNext) {
        return nullptr;
    }
    m_pNextStage = pNext;
    return pNext;
}

CStage* CStage::GetNextStage()
{
    return m_pNextStage;
}

AX_BOOL CStage::Init()
{
    LoadFont();

    return AX_TRUE;
}

AX_VOID CStage::DeInit()
{

}

AX_BOOL CStage::Start(AX_BOOL bThreadStart /* = AX_TRUE */)
{
    LOG_M(m_strStageName.c_str(), "CStage::Start +++");
    if (!Init()) {
        return AX_FALSE;
    }

    if (bThreadStart) {
        m_bProcessFrameWorking = AX_TRUE;
        m_pProcFrameThread = new thread(&CStage::ProcessFrameThreadFunc,this);
    }

    LOG_M(m_strStageName.c_str(), "CStage::Start ---");
    return AX_TRUE;
}

AX_VOID CStage::Stop()
{
    LOG_M(m_strStageName.c_str(), "CStage::Stop +++");

    DeInit();

    {
        std::unique_lock<std::mutex> lck(m_mtxFrameQueue);
        m_bProcessFrameWorking = AX_FALSE;
        m_cvFrameCome.notify_one();
    }

    if (m_pProcFrameThread) {
        m_pProcFrameThread->join();
        delete m_pProcFrameThread;
        m_pProcFrameThread = nullptr;
    }
    LOG_M(m_strStageName.c_str(), "CStage::Stop ---");
}

AX_BOOL CStage::EnqueueFrame(CMediaFrame* pFrame)
{
    std::unique_lock<std::mutex> lck(m_mtxFrameQueue);
    if (m_bProcessFrameWorking) {
        m_qFrame.push(pFrame);
        m_cvFrameCome.notify_one();
        return AX_TRUE;
    }
    return AX_FALSE;
}

AX_BOOL CStage::IsDataPrepared()
{
    return m_qFrame.empty() ? AX_FALSE : AX_TRUE;
}

AX_BOOL CStage::ProcessFrame(CMediaFrame* pFrame)
{
    return AX_TRUE;
}

AX_VOID CStage::ProcessFrameThreadFunc()
{
    LOG_M(m_strStageName.c_str(), "+++");

    char szThreadName[64] = {0};
    sprintf(&szThreadName[0], "IPC_STG_%s", m_strStageName.c_str());

    prctl(PR_SET_NAME, szThreadName);

    CMediaFrame* pFrame = nullptr;

    while(m_bProcessFrameWorking) {
        pFrame = nullptr;
        {
            std::unique_lock<std::mutex> lck(m_mtxFrameQueue);
            m_cvFrameCome.wait(lck, [this]() {
                 return (!m_qFrame.empty() || !m_bProcessFrameWorking);
            });

            if (!m_bProcessFrameWorking) {
                break;
            }
            pFrame = m_qFrame.front();
            m_qFrame.pop();
        }

        if (ProcessFrame(pFrame)) {
            if (GetNextStage()) {
                if (GetNextStage()->EnqueueFrame(pFrame)) {
                   continue;
                }
            }
        }
        pFrame->FreeMem();
    }

    LOG_M(m_strStageName.c_str(), "---");
}

AX_BOOL CStage::LoadFont(AX_VOID)
{
    AX_U16 u16W = 0;
    AX_U16 u16H = 0;
    AX_U32 u32Size = 0;
    if (!m_font.LoadBmp((AX_CHAR *)"./res/font.bmp", u16W, u16H, u32Size)) {
        LOG_M_E(STAGE, "load font.bmp failed");
        return AX_FALSE;
    }

    return AX_TRUE;
}


AX_VOID CStage::DrawTimeRect(CMediaFrame * pFrame)
{
    if (!pFrame) {
        return;
    }

    CYuvHandler YUV((const AX_U8 *)pFrame->tFrame.tFrameInfo.stVFrame.u64VirAddr[0], pFrame->tFrame.tFrameInfo.stVFrame.u32Width, pFrame->tFrame.tFrameInfo.stVFrame.u32Height, AX_YUV420_SEMIPLANAR, 0);

    auto OSDTime = [&](AX_U16 x, AX_U16 y) {
        OSD_CONFIG_T stOSDCfg;
        stOSDCfg.osdType = E_OSD_TYPE_TIME;
        stOSDCfg.overlain = true;
        stOSDCfg.osdPosX = x;
        stOSDCfg.osdPosY = y;
        stOSDCfg.osdFontSize = 10;
        stOSDCfg.osdTimeStampFmt = E_OSD_TIME_FMT_24H;

        int nFontScale = (pFrame->tFrame.tFrameInfo.stVFrame.u32Width + 720) / 720 * 2;

        AX_CHAR szText[MAX_OSD_STRING_LEN] = {0};
        if (CTimeUtils::GenerateTimeStamp(&stOSDCfg, szText, MAX_OSD_STRING_LEN)) {
            m_font.FillString(szText, x, y, &YUV, pFrame->tFrame.tFrameInfo.stVFrame.u32Width, pFrame->tFrame.tFrameInfo.stVFrame.u32Height, nFontScale);
        }
    };

    OSDTime(4, 4);
}