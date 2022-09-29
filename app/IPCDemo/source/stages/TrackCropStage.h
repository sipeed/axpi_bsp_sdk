/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _TRACK_CROP_STAGE_H_
#define _TRACK_CROP_STAGE_H_

#include "global.h"
#include "Stage.h"
#include "WebServer.h"

#define MAX_IMG_NUM (20)

class CTrackCropStage : public CStage
{
public:
    CTrackCropStage(AX_U8 nChannel, AX_U8 nInnerIndex);
    CTrackCropStage(AX_VOID);
    virtual ~CTrackCropStage(AX_VOID);

    virtual AX_BOOL Init();
    virtual AX_VOID DeInit();
    virtual AX_BOOL ProcessFrame(CMediaFrame* pFrame);

    AX_VOID SetChannel(AX_U8 nChannel, AX_U8 nInnerIndex);
    AX_VOID SetWebServer(CWebServer* webServer);
    AX_VOID DoTracking(AX_VOID* data, AX_U32 size, JpegDataInfo* pJpegInfo = nullptr);

private:
    CWebServer* m_pWebServer;
    AX_U8 m_nChannel;
    AX_U8 m_nCfgIndex;
};

#endif // _TRACK_CROP_STAGE_H_
