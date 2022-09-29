/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>
#include <memory>
#include <stdio.h>

#include "global.h"
#include "picojson.h"
#include "JsonCfgParser.h"
#include "ConfigParser.h"
#include "OptionHelper.h"
#include "PrintHelper.h"
#include "CommonUtils.h"

/* components */
#include "Camera.h"
#include "VideoEncoder.h"
#include "Mpeg4Encoder.h"
#include "JpgEncoder.h"
#include "ax_ives_api.h"

/* stages */
#include "IVPSStage.h"
#include "DetectStage.h"
#include "TrackCropStage.h"

/* etc. */
#include "AXRtspServer.h"
#include "WebServer.h"
#include "Md.h"
#include "Od.h"
#include "HotBalance.h"

using namespace std;

#define MAIN "MAIN"
#define RESULT_CHECK(ret) {if(!ret) { goto END; }}

/* global define */
COptionHelper gOptions;
CPrintHelper  gPrintHelper;

string g_SDKVersion = "Unreconigzed";
AX_BOOL g_isSleeped = AX_FALSE;

static AX_BOOL gRunning    = AX_FALSE;
static AX_S32  gExit_count = 0;

/* common functions */
extern AX_VOID exit_handler(int s);
extern AX_VOID ignore_sig_pipe();
extern AX_VOID get_sdk_version();
extern AX_S32  GlobalApiInit();
extern AX_VOID GlobalApiDeInit();
extern AX_S32  APP_SYS_Init();
extern AX_S32  APP_SYS_DeInit();

extern AX_VOID EISSupportStateInit();

typedef AX_S32 (*FUNC_INIT_PTR)();
typedef AX_S32 (*FUNC_DEINIT_PTR)();

typedef struct _FUNC_CALL_STAT {
    FUNC_INIT_PTR   pfInit;
    FUNC_DEINIT_PTR pfDeInit;
    AX_BOOL         bInited;
} FUNC_CALL_STAT_T;

FUNC_CALL_STAT_T g_tFuncCallStat[] = {
    {APP_SYS_Init,      APP_SYS_DeInit,     AX_FALSE},
    {AX_VIN_Init,       AX_VIN_Deinit,      AX_FALSE},
    {AX_MIPI_RX_Init,   AX_MIPI_RX_DeInit,  AX_FALSE},
    {AX_IVPS_Init,      AX_IVPS_Deinit,     AX_FALSE},
    {AX_IVES_Init,      AX_IVES_DeInit,     AX_FALSE},
};

END_POINT_OPTIONS g_tEPOptions[MAX_VENC_CHANNEL_NUM] = {
    /* {eEPType, nChannel, nInnerIndex} */
    {E_END_POINT_VENC, 0, 0},
    {E_END_POINT_DET, 1, 0},
    {E_END_POINT_VENC, 2, 1},
};

CCamera g_camera;
CIVPSStage g_stageIVPS;
vector<CJpgEncoder*> g_vecJEnc;
vector<CVideoEncoder*> g_vecVEnc;
CDetectStage g_stageDetect;
CTrackCropStage g_stageTrackCrop;
AXRtspServer g_rtspServer;
CWebServer g_webserver;

static AX_VOID* ThreadCheckAutoSleep(AX_VOID *__this)
{
    prctl(PR_SET_NAME, "IPC_MAIN_CheckAutoSleep");
    AX_U64 u64LastSleepFameNum = 0;
    while (gRunning) {
        AX_U64 u64Venc0SeqNum = gOptions.GetVenc0SeqNum();
        const AX_U64 u64NowFameNum = u64Venc0SeqNum - u64LastSleepFameNum;
        const AX_U64 u64MaxFameNum = gOptions.GetAutoSleepFrameNum();

        if ( u64MaxFameNum > 0 && u64MaxFameNum < u64NowFameNum) {
            LOG_M(MAIN, "Call AX_SYS_Sleep %llu", u64Venc0SeqNum);
            g_isSleeped = AX_TRUE;
            AX_SYS_Sleep();
            u64LastSleepFameNum = u64Venc0SeqNum;
            g_isSleeped = AX_FALSE;
        }
        CTimeUtils::msSleep(1);
    }

    return nullptr;
}

int main(int argc, const char *argv[])
{
    AX_MTRACE_ENTER(ipcdemo);
    get_sdk_version();

    signal(SIGINT, exit_handler);
    ignore_sig_pipe();

    if (!gOptions.ParseArgs(argc, (const AX_CHAR **)argv)) {
        printf("Parse args error.\n");
        _exit(1);
    }

    APP_SetLogLevel((APP_LOG_LEVEL)gOptions.GetLogLevel());
    APP_SetLogTarget((APP_LOG_TARGET)gOptions.GetLogTarget());
    APP_LogOpen("ipc_demo");

    PRINT(E_LOG_LV_CRITAL, "############## IPCDemo(APP Ver: %s, SDK Ver: %s) Started %s %s ##############\n", APP_BUILD_VERSION, g_SDKVersion.c_str(), __DATE__, __TIME__);

    AX_CHAR szIP[64] = {0};
    vector<string> vNetType;
    vNetType.push_back("usb");
    if (!CCommonUtils::GetIP(vNetType, &szIP[0])) {
        LOG_M_E(MAIN, "Get IP address failed.");
        APP_LogClose();
        _exit(1);
    }

    AX_S32 nRet = GlobalApiInit();
    if (0 != nRet) {
        APP_LogClose();
        _exit(1);
    }

    gPrintHelper.Start();

    AX_POOL_FLOORPLAN_T tVBConfig = {0};
    AX_U8 nPoolCount = 0;
    thread* pThreadCheckAutoSleep = nullptr;

    //Check whether support EIS
#ifndef AX_SIMPLIFIED_MEM_VER
    EISSupportStateInit();
#endif

    CMPEG4Encoder *pMpeg4Encoder = CMPEG4Encoder::GetInstance();
    RESULT_CHECK(pMpeg4Encoder);

    CMD::GetInstance()->SetWebServer(&g_webserver);
    COD::GetInstance()->SetWebServer(&g_webserver);

    /* Init encoder */
    for (size_t i = 0; i < MAX_VENC_CHANNEL_NUM; i++) {
        if (E_END_POINT_JENC == g_tEPOptions[i].eEPType) {
            CJpgEncoder* pJenc = new CJpgEncoder(g_tEPOptions[i].nChannel, g_tEPOptions[i].nInnerIndex);
            pJenc->SetWebServer(&g_webserver);
            g_vecJEnc.emplace_back(pJenc);
        } else if (E_END_POINT_VENC == g_tEPOptions[i].eEPType) {
            CVideoEncoder* pVenc = new CVideoEncoder(g_tEPOptions[i].nChannel, g_tEPOptions[i].nInnerIndex);
            pVenc->SetRTSPServer(&g_rtspServer);
            pVenc->SetWebServer(&g_webserver);

            if (0 == i) {
                if (gOptions.IsEnableMp4Record()) {
                    pVenc->SetMp4ENC(pMpeg4Encoder);
                }
            }
            g_vecVEnc.emplace_back(pVenc);
        } else if (E_END_POINT_DET == g_tEPOptions[i].eEPType) {
            g_stageDetect.SetChannel(g_tEPOptions[i].nChannel, g_tEPOptions[i].nInnerIndex);
            g_stageTrackCrop.SetChannel(g_tEPOptions[i].nChannel, g_tEPOptions[i].nInnerIndex);
        }
    }

    g_camera.BindIvpsStage(&g_stageIVPS);

    g_stageIVPS.SetVENC(&g_vecVEnc);
    g_stageIVPS.SetJENC(&g_vecJEnc);

    if (gOptions.IsActivedDetect()) {
        g_stageIVPS.SetDetect(&g_stageDetect);
    }

    if (gOptions.IsActivedTrack()) {
        //Bind Crop Stage
        g_stageDetect.BindCropStage(&g_stageTrackCrop);

        //set webserver
        g_stageTrackCrop.SetWebServer(&g_webserver);
    }

    /* Init camera */
    RESULT_CHECK(g_camera.Init(&tVBConfig, nPoolCount, AX_SENSOR_ID, AX_DEV_SOURCE_SNS_ID, AX_PIPE_ID));

    AX_VENC_MOD_ATTR_S tModAttr;
    tModAttr.enVencType = VENC_MULTI_ENCODER;
    AX_VENC_Init(&tModAttr);

    /* FillCameraAttr should be invoked before detect state start */
    RESULT_CHECK(g_stageIVPS.FillCameraAttr(&g_camera));

    /* Start detector */
    if (gOptions.IsActivedDetect()) {
        RESULT_CHECK(g_stageDetect.Start());
    }

    /* Init buffer pool */
    RESULT_CHECK(CBaseSensor::BuffPoolInit(&tVBConfig, nPoolCount));

    /* Init rtsp server */
    g_rtspServer.Init(g_vecVEnc[0]->m_bH265 ? false : true);
    RESULT_CHECK(g_rtspServer.Start());

    /* Init web server */
    g_webserver.Init(g_camera.GetChnAttr());
    RESULT_CHECK(g_webserver.Start());

    RESULT_CHECK(g_camera.Open());

    /* Start venc encoder */
    for (size_t i = 0; i < g_vecVEnc.size(); i++) {
        g_vecVEnc[i]->SetChnAttr(g_camera.GetChnAttr());

        RESULT_CHECK(g_vecVEnc[i]->Start());
        if (0 == i && gOptions.IsEnableMp4Record()) {
            pMpeg4Encoder->Start(); //Note: Make sure call this fuction after venc start
        }
    }

    /* Start jpeg encoder */
    for (size_t i = 0; i < g_vecJEnc.size(); i++) {
        g_vecJEnc[i]->SetChnAttr(g_camera.GetChnAttr());
        RESULT_CHECK(g_vecJEnc[i]->Start());
    }

    if (gOptions.IsActivedTrack()) {
        RESULT_CHECK(g_stageTrackCrop.Start());
    }

    RESULT_CHECK(g_stageIVPS.Start(AX_TRUE));
    RESULT_CHECK(g_camera.Start());

    CHotBalance::GetInstance()->Start(CStageOptionHelper().GetInstance()->GetHotBalanceAttr().tConfig);

    CTimeUtils::msSleep(100);
    LOG_M(MAIN, "Preview the video using URL: <<<<< http://%s:8080 >>>>>", szIP);

    gRunning = AX_TRUE;

    if (gOptions.IsEnableAutoSleep()) {
        pThreadCheckAutoSleep = new thread(&ThreadCheckAutoSleep, nullptr);
        pThreadCheckAutoSleep->detach();
    }

    while (gRunning) {
        CTimeUtils::msSleep(100);
    }

    CHotBalance::GetInstance()->Stop();

    g_webserver.StopAction();

END:
    if (gOptions.IsActivedDetect()) {
        g_stageDetect.Stop();
    }

    g_camera.Stop();

    g_stageIVPS.Stop();

    for (AX_U32 i = 0; i < g_vecVEnc.size(); i++) {
        g_vecVEnc[i]->Stop();
    }

    if (gOptions.IsActivedTrack()) {
        g_stageTrackCrop.Stop();
    }

    for (AX_U32 i = 0; i < g_vecJEnc.size(); i++) {
        g_vecJEnc[i]->Stop();
    }

    g_camera.Close();

    g_rtspServer.Stop();
    g_webserver.Stop();
    gPrintHelper.Stop();

    if (pMpeg4Encoder && gOptions.IsEnableMp4Record()) {
        pMpeg4Encoder->Stop();
    }

    AX_VENC_Deinit();

    GlobalApiDeInit();

    for (AX_U32 i = 0; i < g_vecVEnc.size(); i++) {
        SAFE_DELETE_PTR(g_vecVEnc[i]);
    }

    PRINT(E_LOG_LV_CRITAL, "############### IPCDemo(APP Ver: %s, SDK Ver: %s) Exited %s %s ###############\n", APP_BUILD_VERSION, g_SDKVersion.c_str(), __DATE__, __TIME__);

    APP_LogClose();

    AX_MTRACE_LEAVE;
    return 0;
}

void exit_handler(int s) {
    LOG_M(MAIN, "\nCaught signal: SIGINT\n");
    gRunning = AX_FALSE;
    gExit_count++;
    if (gExit_count >= 3) {
        LOG_M(MAIN, "Force to exit\n");
        _exit(1);
    }
}

void ignore_sig_pipe()
{
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigemptyset(&sa.sa_mask) == -1 ||
        sigaction(SIGPIPE, &sa, 0) == -1) {
        perror("failed to ignore SIGPIPE, sigaction");
        exit(EXIT_FAILURE);
    }
}

void get_sdk_version()
{
    if (FILE* pFile = fopen("/proc/ax_proc/version", "r")) {
        char szSDKVer[64] = {0};
        fread(&szSDKVer[0], 64, 1, pFile);
        fclose(pFile);
        szSDKVer[strlen(szSDKVer) - 1] = 0;
        g_SDKVersion = szSDKVer + strlen(SDK_VERSION_PREFIX) + 1; // 1:blank
    }
}

AX_S32 APP_SYS_Init() {
    AX_S32 nRet = AX_SUCCESS;

    nRet = AX_SYS_Init();
    if (0 != nRet) {
        return nRet;
    }

    nRet = AX_POOL_Exit();
    if (0 != nRet) {
        return nRet;
    }

    return AX_SUCCESS;
}

AX_S32 APP_SYS_DeInit() {
    AX_S32 nRet = AX_SUCCESS;

    nRet = AX_POOL_Exit();
    if (0 != nRet) {
        return nRet;
    }

    nRet = AX_SYS_Deinit();
    if (0 != nRet) {
        return nRet;
    }

    return AX_SUCCESS;
}

AX_S32 GlobalApiInit() {
    AX_S32 nRet = 0;
    for (AX_U32 i = 0; i < sizeof(g_tFuncCallStat) / sizeof(FUNC_CALL_STAT_T); i++) {
        FUNC_CALL_STAT_T & tFuncInfo = g_tFuncCallStat[i];
        if (tFuncInfo.pfInit) {
            nRet = tFuncInfo.pfInit();
            if (0 != nRet) {
                LOG_M_E(MAIN, "FAIL, ret=0x%x", nRet);
                break;
            } else {
                tFuncInfo.bInited = AX_TRUE;
                LOG_M(MAIN, "SUCCESS");
            }
        }
    }
    if (0 != nRet) {
        GlobalApiDeInit();
    }
    return nRet;
}

AX_VOID GlobalApiDeInit() {
    AX_S32 nRet = 0;
    for (AX_S32 j = sizeof(g_tFuncCallStat) / sizeof(FUNC_CALL_STAT_T) - 1; j >= 0; j--) {
        FUNC_CALL_STAT_T & tFuncInfo = g_tFuncCallStat[j];
        if (tFuncInfo.bInited && tFuncInfo.pfDeInit) {
            nRet = tFuncInfo.pfDeInit();
            if (0 != nRet) {
                LOG_M_E(MAIN, "FAIL, ret=0x%x", nRet);
                break;
            } else {
                tFuncInfo.bInited = AX_TRUE;
                LOG_M(MAIN, "SUCCESS");
            }
        }
    }
}

AX_VOID EISSupportStateInit()
{
#ifndef AX_SIMPLIFIED_MEM_VER
    SENSOR_CONFIG_T tSensorCfg;
    CConfigParser().GetInstance()->GetCameraCfg(tSensorCfg, E_SENSOR_ID_0);
    if (AX_FALSE == tSensorCfg.bEnableEIS) {
        LOG_M(MAIN, "EIS is unsupported!");
        gOptions.SetEISSupport(AX_FALSE);
        return;
    }

    if (access(tSensorCfg.aEISSdrBin, F_OK) != 0
        || access(tSensorCfg.aEISHdrBin, F_OK) != 0) {
        gOptions.SetEISSupport(AX_FALSE);
        LOG_M(MAIN, "EIS is unsupported for EIS sdr bin(%s) or hdr bin(%s) is not exist.",
                                             tSensorCfg.aEISSdrBin, tSensorCfg.aEISHdrBin);
    } else {
        gOptions.SetEISSupport(AX_TRUE);
    }
#endif
}
