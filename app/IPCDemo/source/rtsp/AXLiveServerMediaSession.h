/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AXLIVESEVERMEDIASESSION_H__
#define __AXLIVESEVERMEDIASESSION_H__

#include "liveMedia.hh"
#include "OnDemandServerMediaSubsession.hh"
#include "AXFramedSource.h"
#include "global.h"
#include <queue>
#include <pthread.h>

class AXLiveServerMediaSession: public OnDemandServerMediaSubsession {
public:
    static AXLiveServerMediaSession* createNew(UsageEnvironment& env, bool reuseFirstSource, bool isH264=true);
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();
    void SendNalu(AX_U8 nChn, const AX_U8* pBuf, AX_U32 nLen, AX_U64 nPts=0, AX_BOOL bIFrame=AX_FALSE);
protected:
    AXLiveServerMediaSession(UsageEnvironment& env, bool reuseFirstSource, bool isH264);
    virtual ~AXLiveServerMediaSession(void);
    void setDoneFlag() { fDoneFlag = ~0; }


protected:
    virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
    virtual void closeStreamSource(FramedSource* inputSource);
    virtual char const* sdpLines();

private:
    bool m_bH264;
    char* fAuxSDPLine;
    char fDoneFlag;
    RTPSink* fDummySink;
    AXFramedSource * m_pSource;
    pthread_spinlock_t m_tLock;
};


#endif /*__AXLIVESEVERMEDIASESSION_H__*/