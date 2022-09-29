/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "AXRtspServer.h"
#include "AXFramedSource.h"
#include "global.h"
#include "OptionHelper.h"
#include "GroupsockHelper.hh"
#include "StringUtils.h"
#include "CommonUtils.h"


extern COptionHelper gOptions;
extern END_POINT_OPTIONS g_tEPOptions[MAX_VENC_CHANNEL_NUM];
static char gStopEventLoop = 0;

#define BASE_PORT     18888
#define RTSPSERVER     "RTSPSERVER"

char const *inputFilename="axstream";

#ifdef MULTICAST
UsageEnvironment *uEnv;
H264VideoStreamFramer *videoSource;
RTPSink *videoSink;

void afterPlaying(void * /*clientData*/) {
    LOG_M_I(RTSPSERVER, "...done reading from file");
    videoSink->stopPlaying();
    Medium::close(videoSource);
    //play();
}

void play() {

    AXFramedSource* source = AXFramedSource::createNew(*uEnv);
    if (source == NULL) {
        LOG_M_E(RTSPSERVER, "unable to open AXFramedSource");
    }

    FramedSource* videoES = source;

    // Create a framer for the Video Elementary Stream:
    videoSource = H264VideoStreamFramer::createNew(*uEnv, videoES);
    //videoSource = H264VideoStreamDiscreteFramer::createNew(*uEnv, videoES);

    // Finally, start playing:
    LOG_M_I(RTSPSERVER, "Beginning to read from file...");
    videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
}

#endif

void * RtspServerThreadFunc(void *args)
{
    AXRtspServer * pThis = (AXRtspServer *)args;

    LOG_M(RTSPSERVER, "+++");

    prctl(PR_SET_NAME, "IPC_RTSP_Main");

#ifdef MULTICAST
    // Begin by setting up our usage environment:
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    uEnv = BasicUsageEnvironment::createNew(*scheduler);

    // Create 'groupsocks' for RTP and RTCP:
    struct in_addr destinationAddress;
    destinationAddress.s_addr = our_inet_addr("10.126.11.244"); //chooseRandomIPv4SSMAddress(*uEnv);
    const unsigned short rtpPortNum = pThis->m_uBasePort;
    const unsigned short rtcpPortNum = pThis->m_uBasePort + 1;
    const unsigned char ttl = 255;

    const Port rtpPort(rtpPortNum);
    const Port rtcpPort(rtcpPortNum);

    Groupsock rtpGroupsock(*uEnv, destinationAddress, rtpPort, ttl);
    rtpGroupsock.multicastSendOnly(); // we're a SSM source
    Groupsock rtcpGroupsock(*uEnv, destinationAddress, rtcpPort, ttl);
    rtcpGroupsock.multicastSendOnly(); // we're a SSM source

    // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
    OutPacketBuffer::maxSize = 200000;
    videoSink = H264VideoRTPSink::createNew(*uEnv, &rtpGroupsock, 96);

    // Create (and start) a 'RTCP instance' for this RTP sink:
    const unsigned estimatedSessionBandwidth = 500; // in kbps; for RTCP b/w share
    const unsigned maxCNAMElen = 100;
    unsigned char CNAME[maxCNAMElen + 1];
    gethostname((char *) CNAME, maxCNAMElen);
    CNAME[maxCNAMElen] = '\0'; // just in case
    RTCPInstance *rtcp = RTCPInstance::createNew(*uEnv, &rtcpGroupsock,
                                                 estimatedSessionBandwidth, CNAME, videoSink,
                                                 NULL /* we're a server */);
    // Note: This starts RTCP running automatically

    RTSPServer *rtspServer = RTSPServer::createNew(*uEnv, 8554);
    if (rtspServer == NULL) {
        printf("Failed to create RTSP server: %s\n", uEnv->getResultMsg());
    }
    ServerMediaSession *sms = ServerMediaSession::createNew(*uEnv, inputFilename,
                                                            "test.h264",
                                                            "Session streamed by \"AX630A\"");
    sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
    rtspServer->addServerMediaSession(sms);

    char *url = rtspServer->rtspURL(sms);
    printf("Play this stream using the URL \"%s\"\n", url);
    delete[] url;

    // Start the streaming:
    printf("Beginning streaming...\n");
    play();

    uEnv->taskScheduler().doEventLoop(); // does not return

#else
    OutPacketBuffer::maxSize = 700000;
    TaskScheduler* taskSchedular = BasicTaskScheduler::createNew();
    pThis->m_pUEnv = BasicUsageEnvironment::createNew(*taskSchedular);
    pThis->m_rtspServer = RTSPServer::createNew(*pThis->m_pUEnv, 8554, NULL);
    if(pThis->m_rtspServer == NULL) {
        *pThis->m_pUEnv << "Failed to create rtsp server ::" << pThis->m_pUEnv->getResultMsg() <<"\n";
        return nullptr;
    }

    AX_CHAR szIP[64] = {0};
    vector<string> vNetType;
    vNetType.push_back("usb");
    AX_BOOL bGetIPRet = AX_FALSE;
    if (CCommonUtils::GetIP(vNetType, &szIP[0])) {
        bGetIPRet = AX_TRUE;
    }

    AX_U32 nRTSPIndex = 0;
    for (size_t i = 0; i < MAX_VENC_CHANNEL_NUM; i++) {
        if (E_END_POINT_VENC == g_tEPOptions[i].eEPType) {
            std::string strStream = CStringUtils::string_format("axstream%d", nRTSPIndex++);
            ServerMediaSession* sms = ServerMediaSession::createNew(*pThis->m_pUEnv, strStream.c_str(), strStream.c_str(), "Live Stream");
            pThis->m_pLiveServerMediaSession[i] = AXLiveServerMediaSession::createNew(*pThis->m_pUEnv, true, pThis->m_isH264);
            sms->addSubsession(pThis->m_pLiveServerMediaSession[i]);
            pThis->m_rtspServer->addServerMediaSession(sms);

            char* url = nullptr;
            if (bGetIPRet) {
                url = new char[64];
                sprintf(url, "rtsp://%s:8554/%s", szIP, strStream.c_str());
            } else {
                url = pThis->m_rtspServer->rtspURL(sms);
            }

            LOG_M(RTSPSERVER, "Play the stream using url: <<<<< %s >>>>>", url);
            delete[] url;
        }
    }

    taskSchedular->doEventLoop(&gStopEventLoop);

    delete(taskSchedular);

    LOG_M(RTSPSERVER, "---");

#endif
    return nullptr;
}

AXRtspServer::AXRtspServer(void)
{
    m_isH264 = true;
    m_uBasePort = BASE_PORT;
    m_tidServer = 0;
    for (int i = 0; i < MAX_VENC_CHANNEL_NUM; i++) {
        m_pLiveServerMediaSession[i] = NULL;
    }
    m_pUEnv = NULL;
    m_rtspServer = nullptr;
}

AXRtspServer::~AXRtspServer(void)
{

}

AX_BOOL AXRtspServer::Init(bool isH264, AX_U16 uBasePort)
{
    m_isH264 = isH264;
    if (uBasePort == 0) {
        uBasePort = BASE_PORT;
    }
    m_uBasePort = uBasePort;
    gStopEventLoop = 0;
    return AX_TRUE;
}

void AXRtspServer::SendNalu(AX_U8 nChn, const AX_U8* pBuf, AX_U32 nLen, AX_U64 nPts/*=0*/, AX_BOOL bIFrame/*=AX_FALSE*/)
{
#ifdef MULTICAST
    if (videoSource) {
        ((AXFramedSource *)videoSource->inputSource())->AddFrameBuff(pBuf, nLen);
    }
#else
    if (m_pLiveServerMediaSession[nChn]) {
        m_pLiveServerMediaSession[nChn]->SendNalu(nChn, pBuf, nLen, nPts, bIFrame);
    }
#endif
}

AX_BOOL AXRtspServer::Start(void)
{
    LOG_M_I(RTSPSERVER, "+++");
    if (0 != pthread_create(&m_tidServer, NULL, RtspServerThreadFunc, this)) {
        m_tidServer = 0;
        LOG_M_E(RTSPSERVER, "pthread_create(RtspServerThreadFunc) fail");
        return AX_FALSE;
    }

    LOG_M_I(RTSPSERVER, "---");
    return AX_TRUE;
}

void AXRtspServer::Stop(void)
{
    LOG_M_I(RTSPSERVER, "+++");

    AX_U32 nRTSPIndex = 0;
    for (size_t i = 0; i < MAX_VENC_CHANNEL_NUM; i++) {
        if (E_END_POINT_VENC == g_tEPOptions[i].eEPType) {
            std::string strStream = CStringUtils::string_format("axstream%d", nRTSPIndex++);
            ServerMediaSession* sms = m_rtspServer->lookupServerMediaSession(strStream.c_str());
            if (sms) {
                m_rtspServer->removeServerMediaSession(sms);
                m_rtspServer->closeAllClientSessionsForServerMediaSession(sms);
                sms->deleteAllSubsessions();
            }

            m_pLiveServerMediaSession[i] = NULL;
        }
    }

    m_pUEnv = NULL;

    gStopEventLoop = 1;

    LOG_M_I(RTSPSERVER, "---");
}

