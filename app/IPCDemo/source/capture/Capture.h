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
#include "global.h"
#include "Condition.hpp"
#include "Singleton.h"
#include "MediaFrame.h"

using namespace ax;

class CCapture : public CSingleton<CCapture> {
    friend class CSingleton<CCapture>;

private:
    CCapture(AX_VOID);
    virtual ~CCapture(AX_VOID);

    AX_BOOL Init(AX_VOID) override {
        return AX_TRUE;
    }

public:
    AX_BOOL ProcessFrame(AX_U8 nChn, CMediaFrame* pFrame);
    AX_BOOL CaptureFrame(AX_U8 nChn, AX_VIDEO_FRAME_S &Frame);
    AX_BOOL FreeCaptureFrame(AX_U8 nChn, AX_VIDEO_FRAME_S &Frame);
    AX_BOOL CapturePicture(AX_U32 nChn, AX_VOID **ppBuf, AX_U32 *pBufSize, AX_U32 nQpLevel);
    AX_BOOL FreeCapturePicture(AX_U32 nChn, AX_VOID *pBuf);
    AX_BOOL GetCaptureStat(AX_U8 nChn);

private:
    CMutex m_mutexStat;
    CMutex m_mutexCapture;
    CCondition m_cvCapture;

    AX_BOOL m_bCapture{AX_FALSE};
    AX_U8 m_nCaptureChn{0};
    CMediaFrame *m_ptCaptureFrame{nullptr};

    AX_VENC_STREAM_S m_stVencStream{0};
};
