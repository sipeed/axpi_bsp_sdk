/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "global.h"
#include "MediaFrame.h"
#include "YuvHandler.h"
#include "BmpOSD.h"

class CStage
{
public:
    CStage(const string& strName);
    virtual ~CStage();

public:
    virtual CStage* BindNextStage(CStage* pNext);
    virtual CStage* GetNextStage();

    virtual AX_BOOL Init();
    virtual AX_VOID DeInit();

    virtual AX_BOOL Start(AX_BOOL bThreadStart = AX_TRUE);
    virtual AX_VOID Stop();

    virtual AX_BOOL IsDataPrepared();
    virtual AX_BOOL EnqueueFrame(CMediaFrame* pFrame);
    virtual AX_BOOL ProcessFrame(CMediaFrame* pFrame);
    virtual AX_VOID ProcessFrameThreadFunc();

    virtual string  GetStageName() { return m_strStageName; };

protected:
    AX_VOID DrawTimeRect(CMediaFrame * pFrame);

private:
    AX_BOOL LoadFont(AX_VOID);

public:
    queue<CMediaFrame*> m_qFrame;
    mutex               m_mtxFrameQueue;
    condition_variable  m_cvFrameCome;
    AX_BOOL             m_bProcessFrameWorking;

protected:
    string      m_strStageName;
    CStage*     m_pNextStage;
    thread*     m_pProcFrameThread;
    CBmpOSD     m_font;

};