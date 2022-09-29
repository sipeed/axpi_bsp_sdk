/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "AXLiveServerMediaSession.h"
#include <GroupsockHelper.hh>


AXLiveServerMediaSession* AXLiveServerMediaSession::createNew(UsageEnvironment& env, bool reuseFirstSource, bool isH264)
{
    return new AXLiveServerMediaSession(env, reuseFirstSource, isH264);
}

AXLiveServerMediaSession::AXLiveServerMediaSession(UsageEnvironment& env, bool reuseFirstSource, bool isH264)
:OnDemandServerMediaSubsession(env,reuseFirstSource),
m_bH264(isH264),
fAuxSDPLine(NULL),
fDoneFlag(0),
fDummySink(NULL),
m_pSource(NULL)
{
    pthread_spin_init(&m_tLock, 0);
}

AXLiveServerMediaSession::~AXLiveServerMediaSession(void) {
    delete[] fAuxSDPLine;
    pthread_spin_destroy(&m_tLock);
}


static void afterPlayingDummy(void* clientData) {
    AXLiveServerMediaSession *session = (AXLiveServerMediaSession*)clientData;
    session->afterPlayingDummy1();
}

void AXLiveServerMediaSession::afterPlayingDummy1() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
    AXLiveServerMediaSession* session = (AXLiveServerMediaSession*)clientData;
    session->checkForAuxSDPLine1();
}

void AXLiveServerMediaSession::checkForAuxSDPLine1() {
    // printf("AXLiveServerMediaSession::checkForAuxSDPLine1+++\n");
    // setDoneFlag();
    // return;
    char const* dasl;
    if(fAuxSDPLine != NULL) {
        setDoneFlag();
        printf("%s\n",fAuxSDPLine);
    }
    else if(fDummySink != NULL && (dasl = fDummySink->auxSDPLine()) != NULL) {
        fAuxSDPLine = strDup(dasl);
        fDummySink = NULL;
        setDoneFlag();
        printf("%s\n",fAuxSDPLine);
    }
    else {
        int uSecsDelay = 100000;
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* AXLiveServerMediaSession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
    printf("AXLiveServerMediaSession::getAuxSDPLine+++\n");
    if(fAuxSDPLine != NULL) {
        printf("%s\n",fAuxSDPLine);
        return fAuxSDPLine;
    }
    if(fDummySink == NULL) {
        fDummySink = rtpSink;
        fDummySink->startPlaying(*inputSource, afterPlayingDummy, this);
        checkForAuxSDPLine(this);
    }
    if (fAuxSDPLine != NULL){
        printf("%s\n",fAuxSDPLine);
    }
    envir().taskScheduler().doEventLoop(&fDoneFlag);
    return fAuxSDPLine;
}

FramedSource* AXLiveServerMediaSession::createNewStreamSource(unsigned clientSessionID, unsigned& estBitRate) {
    printf("AXLiveServerMediaSession::createNewStreamSource +++\n");
    // Based on encoder configuration i kept it 90000
    // estBitRate = 6000000;
    AXFramedSource *source = AXFramedSource::createNew(envir());
    pthread_spin_lock(&m_tLock);
    m_pSource = source;
    pthread_spin_unlock(&m_tLock);
    // are you trying to keep the reference of the source somewhere? you shouldn't.
    // Live555 will create and delete this class object many times. if you store it somewhere
    // you will get memory access violation. instead you should configure you source to always read from your data source
    if (m_bH264) {
        return H264VideoStreamFramer::createNew(envir(),source);
    }
    else {
        return H265VideoStreamFramer::createNew(envir(),source);
    }
}

void AXLiveServerMediaSession::closeStreamSource(FramedSource *inputSource) {
    printf("AXLiveServerMediaSession::closeStreamSource +++\n");
    pthread_spin_lock(&m_tLock);
    m_pSource = NULL;
    Medium::close(inputSource);
    pthread_spin_unlock(&m_tLock);
}

char const* AXLiveServerMediaSession::sdpLines() {
    return OnDemandServerMediaSubsession::sdpLines();
}

RTPSink* AXLiveServerMediaSession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
    increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), 500*1024);
    if (m_bH264) {
        return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    }
    else {
        return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    }
}

void AXLiveServerMediaSession::SendNalu(AX_U8 nChn, const AX_U8* pBuf, AX_U32 nLen, AX_U64 nPts/*=0*/, AX_BOOL bIFrame/*=AX_FALSE*/)
{
    pthread_spin_lock(&m_tLock);
    if (m_pSource) {
        m_pSource->AddFrameBuff(nChn, pBuf, nLen, nPts, bIFrame);
    }
    pthread_spin_unlock(&m_tLock);
}