/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _DETECT_STAGE_H_
#define _DETECT_STAGE_H_

#include "global.h"
#include "Stage.h"
#include "Detector.h"
#include "TrackCropStage.h"

typedef struct axDETECT_STAGE_INFO_T {
    AX_U32 nFrmWidth;
    AX_U32 nFrmHeight;
    AX_U16 nVinFps;
    AX_U16 nFrmFps;
} DETECT_STAGE_INFO_T;

class CDetectStage : public CStage
{
public:
    CDetectStage(AX_U8 nChannel, AX_U8 nInnerIndex);
    CDetectStage(AX_VOID);
    virtual ~CDetectStage(AX_VOID);

    virtual AX_BOOL Init();
    virtual AX_VOID DeInit();
    virtual AX_BOOL ProcessFrame(CMediaFrame *pFrame);
    AX_VOID SetChannel(AX_U8 nChannel, AX_U8 nInnerIndex);
    AX_VOID BindCropStage(CTrackCropStage* pStage);

    AX_VOID FrameStrategy(AX_U64 nDetectFrameId, AX_BOOL &bAiFrameSkip, AX_BOOL &bIvesFrameSkip);
    AX_VOID ConfigStageInfo(const DETECT_STAGE_INFO_T &tInfo);
    AX_BOOL Reset(const DETECT_STAGE_INFO_T &tInfo);

public:
    AX_U8       m_nChannel;
    AX_U8       m_nCfgIndex;

private:
    AX_U64 m_nDetectFrameId{0};
    CTrackCropStage* m_pTrackCropStage = nullptr;
    DETECT_STAGE_INFO_T m_tStageInfo{0};
    AX_BOOL m_bReseting;
    mutex m_mtxReset;
};

#endif // _DETECT_STAGE_H_
