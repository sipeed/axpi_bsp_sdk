/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "AXFramedSource.h"
#include "global.h"
#include "OptionHelper.h"
#include <GroupsockHelper.hh>

#define LIVE "LIVE"

extern COptionHelper gOptions;

AXFramedSource* AXFramedSource::createNew(UsageEnvironment& env) {
    return new AXFramedSource(env);
}

EventTriggerId AXFramedSource::eventTriggerId = 0;

unsigned AXFramedSource::referenceCount = 0;

AXFramedSource::AXFramedSource(UsageEnvironment& env)
: FramedSource(env) {
    if (referenceCount == 0) {
        // Any global initialization of the device would be done here:
        //%%% TO BE WRITTEN %%%
    }
    ++referenceCount;

    m_nTriggerID = envir().taskScheduler().createEventTrigger(deliverFrame);

#ifndef RTSP_SERVER_APPLY_QUEUE
    printf("Initializer(): referenceCount=%d, TriggerID=%d\n", referenceCount, m_nTriggerID);
    m_pRingBuf = new CAXRingBuffer(700000, 2, "RTSP");
#endif
}

AXFramedSource::~AXFramedSource() {
    --referenceCount;

    envir().taskScheduler().deleteEventTrigger(m_nTriggerID);
    eventTriggerId = 0;

#ifndef RTSP_SERVER_APPLY_QUEUE
    printf("Deinitializer(): referenceCount=%d, TriggerID=%d\n", referenceCount, m_nTriggerID);
    delete(m_pRingBuf);
#endif
}

void AXFramedSource::AddFrameBuff(AX_U8 nChn, const AX_U8* pBuf, AX_U32 nLen, AX_U64 nPts/*=0*/, AX_BOOL bIFrame/*=AX_FALSE*/) {
#ifdef RTSP_SERVER_APPLY_QUEUE
    FrameInfo_T tFrame;
    tFrame.pBuf = (AX_U8 *)malloc(nSize);
    memcpy(tFrame.pBuf, pBuf,nSize);
    tFrame.nSize = nSize;
    m_qFrame.push(tFrame);
#else
    CAXRingElement ele((AX_U8*)pBuf, nLen, nChn, nPts, bIFrame);
    m_pRingBuf->Put(ele);
#endif
    envir().taskScheduler().triggerEvent(m_nTriggerID, this);
}

void AXFramedSource::doGetNextFrame() {
    _deliverFrame();
}

void AXFramedSource::deliverFrame(void* clientData) {
    ((AXFramedSource*)clientData)->_deliverFrame();
}

void AXFramedSource::_deliverFrame() {
    if (!isCurrentlyAwaitingData()) {
        return; // we're not ready for the data yet
    }

#ifdef RTSP_SERVER_APPLY_QUEUE
    if (m_qFrame.size() == 0) {
        return;
    }
    if (m_qFrame.size() > 100) {
        printf("q size = %lu\n", m_qFrame.size());
    }

    FrameInfo_T tFrame = m_qFrame.front();
    m_qFrame.pop();

    u_int8_t* newFrameDataStart = (u_int8_t*)tFrame.pBuf; //%%% TO BE WRITTEN %%%
    unsigned newFrameSize = static_cast<unsigned int>(tFrame.nSize); //%%% TO BE WRITTEN %%%
#else
    CAXRingElement *element = m_pRingBuf->Get();
    if (!element) {
        return;
    }
    u_int8_t* newFrameDataStart = element->pBuf;
    AX_U32 newFrameSize = element->nSize;

    if (0 == newFrameSize || nullptr == newFrameDataStart) {
        return;
    }
 #endif
    // Deliver the data here:
    if (newFrameSize > fMaxSize) {
        LOG_M_W(LIVE, "Exceeding max frame size: newFrameSize:%d > fMaxSize:%d", newFrameSize, fMaxSize);
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = newFrameSize - fMaxSize;
    } else {
        fFrameSize = newFrameSize;
    }
    gettimeofday(&fPresentationTime, NULL); // If you have a more accurate time - e.g., from an encoder - then use that instead.
    // If the device is *not* a 'live source' (e.g., it comes instead from a file or buffer), then set "fDurationInMicroseconds" here.
    memmove(fTo, newFrameDataStart, fFrameSize);

#ifdef RTSP_SERVER_APPLY_QUEUE
    free(tFrame.pBuf);
#endif

    m_pRingBuf->Pop();

    // After delivering the data, inform the reader that it is now available:
    FramedSource::afterGetting(this);
}

unsigned AXFramedSource::maxFrameSize() const {
  // By default, this source has no maximum frame size.
  return 700000;
}