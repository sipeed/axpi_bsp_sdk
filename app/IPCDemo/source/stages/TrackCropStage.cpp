/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "TrackCropStage.h"
#include "OptionHelper.h"
#include "ax_ivps_api.h"
#include "CommonUtils.h"

#include <thread>

#define CROP "TRACK_CROP"

extern COptionHelper gOptions;

CTrackCropStage::CTrackCropStage(AX_U8 nChannel /*= 0*/, AX_U8 nInnerIndex /*= 0*/)
 : CStage(CROP)
 , m_pWebServer(nullptr)
 , m_nChannel(nChannel)
 , m_nCfgIndex(nInnerIndex)
{

}

CTrackCropStage::CTrackCropStage(AX_VOID)
 : CStage(CROP)
 , m_pWebServer(nullptr)
 , m_nChannel(0)
 , m_nCfgIndex(0)
{

}

CTrackCropStage::~CTrackCropStage(AX_VOID)
{

}

AX_VOID CTrackCropStage::SetChannel(AX_U8 nChannel, AX_U8 nInnerIndex)
{
    m_nChannel = nChannel;
    m_nCfgIndex = nInnerIndex;
}

AX_VOID CTrackCropStage::SetWebServer(CWebServer* webServer)
{
    m_pWebServer = webServer;
}

AX_BOOL CTrackCropStage::ProcessFrame(CMediaFrame* pFrame)
{
    return AX_TRUE;
}

AX_BOOL CTrackCropStage::Init()
{
    return CStage::Init();
}

AX_VOID CTrackCropStage::DeInit()
{
    CStage::DeInit();
}

AX_VOID CTrackCropStage::DoTracking(AX_VOID* data, AX_U32 size, JpegDataInfo* pJpegInfo /*= nullptr*/)
{
    if (m_pWebServer && data && size > 0) {
        m_pWebServer->SendCaptureData(m_nChannel, data, size, 0, AX_TRUE, pJpegInfo);
    }
}

