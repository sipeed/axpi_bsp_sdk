/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <common.h>
#include <minigui.h>
#include <gdi.h>
#include <window.h>

#include "ax_sys_api.h"
#include "ax_base_type.h"
#include "ax_vo_api.h"

#include "sample_comm_vo.h"
#include "sample_vo_pattern.h"

#include "sample_res_en.h"

typedef struct axSAMPLE_VO_CHN_THREAD_PARAM {
    pthread_t ThreadID;

    AX_U32 u32ThreadForceStop;

    AX_U32 u32LayerID;
    AX_U32 u32ChnID;
    AX_POOL u32UserPoolId;
} SAMPLE_VO_CHN_THREAD_PARAM_S;

#define SAMPLE_VO_DEV0 0

SAMPLE_VO_CONFIG_S stVoConf = {
    /* for device */
    .VoDev = SAMPLE_VO_DEV0,
    .enVoIntfType = VO_INTF_DSI,
    .enIntfSync = VO_OUTPUT_USER,
    .stReso = {
        .u32Width = 720,
        .u32Height = 1280,
        .u32RefreshRate = 60,
    },

    /* for layer */
    .VoLayer = 0,
    .stVoLayerAttr = {
        .stDispRect = {0, 0, 720, 1280},
        .stImageSize = {720, 1280},
        .enPixFmt = AX_YUV420_SEMIPLANAR,
        .enLayerSync = VO_LAYER_SYNC_NORMAL,
        .u32PrimaryChnId = 0,
        .u32FrameRate = 0,
        .u32FifoDepth = 0,
        .u32ChnNr = 16,
        .u32BkClr = 0,
        .enLayerBuf = VO_LAYER_OUT_BUF_POOL,
        .u32InplaceChnId = 0,
        .u32PoolId = 0,
        .enDispatchMode = VO_LAYER_OUT_TO_FIFO,
        .u64KeepChnPrevFrameBitmap = ~0x0,
    },

    /* for chnnel */
    .enVoMode = VO_MODE_2MUX,

    /* for graphic layer */
    .s32EnableGLayer = 1,
    .GraphicLayer = 0,
};

static AX_S32 SAMPLE_VO_POOL_DESTROY(AX_U32 u32PoolID)
{
    return AX_POOL_MarkDestroyPool(u32PoolID);
}

static AX_S32 SAMPLE_VO_CREATE_POOL(AX_U32 u32BlkCnt, AX_U64 u64BlkSize, AX_U64 u64MetaSize, AX_U32 *pPoolID)
{
    AX_POOL_CONFIG_T stPoolCfg = {0};

    stPoolCfg.MetaSize = u64MetaSize;
    stPoolCfg.BlkCnt = u32BlkCnt;
    stPoolCfg.BlkSize = u64BlkSize;
    stPoolCfg.CacheMode = POOL_CACHE_MODE_NONCACHE;
    strcpy((AX_CHAR *)stPoolCfg.PartitionName, "anonymous");

    *pPoolID = AX_POOL_CreatePool(&stPoolCfg);
    if (*pPoolID == AX_INVALID_POOLID) {
        SAMPLE_PRT("AX_POOL_CreatePool failed, u32BlkCnt = %d, u64BlkSize = 0x%llx, u64MetaSize = 0x%llx\n", u32BlkCnt,
                   u64BlkSize, u64MetaSize);
        return -1;
    }

    SAMPLE_PRT("u32BlkCnt = %d, u64BlkSize = 0x%llx, pPoolID = %d\n", u32BlkCnt, u64BlkSize, *pPoolID);

    return 0;
}

static AX_VOID *SAMPLE_VO_CHN_THREAD(AX_VOID *pData)
{
    AX_S32 s32Ret = 0;
    AX_VIDEO_FRAME_S stFrame = {0};
    AX_U32 u32FrameSize;
    AX_U32 u32LayerID, u32ChnID;
    AX_BLK BlkId;
    VO_CHN_ATTR_S stChnAttr;
    SAMPLE_VO_CHN_THREAD_PARAM_S *pstChnThreadParam = (SAMPLE_VO_CHN_THREAD_PARAM_S *)pData;

    u32LayerID = pstChnThreadParam->u32LayerID;
    u32ChnID = pstChnThreadParam->u32ChnID;

    s32Ret = AX_VO_GetChnAttr(u32LayerID, u32ChnID, &stChnAttr);
    if (s32Ret) {
        SAMPLE_PRT("layer%d-chn%d AX_VO_GetChnAttr failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
        goto exit;
    }
    SAMPLE_PRT("layer%d-chn%d u32Width = %d, u32Height = %d\n",
               u32LayerID, u32ChnID,
               stChnAttr.stRect.u32Width, stChnAttr.stRect.u32Height);

    stFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
    stFrame.u32Width = stChnAttr.stRect.u32Width;
    stFrame.u32Height = stChnAttr.stRect.u32Height;
    stFrame.u32PicStride[0] = stChnAttr.stRect.u32Width;
    u32FrameSize = stFrame.u32PicStride[0] * stFrame.u32Height * 3 / 2;

    s32Ret = SAMPLE_VO_CREATE_POOL(5, u32FrameSize, 512, &pstChnThreadParam->u32UserPoolId);
    if (s32Ret) {
        SAMPLE_PRT("layer%d-chn%d SAMPLE_VO_CREATE_POOL failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
        goto exit;
    }

    while (!pstChnThreadParam->u32ThreadForceStop) {
        BlkId = AX_POOL_GetBlock(pstChnThreadParam->u32UserPoolId, u32FrameSize, NULL);
        if (AX_INVALID_BLOCKID == BlkId) {
            SAMPLE_PRT("layer%d-chn%d AX_POOL_GetBlock failed\n", u32LayerID, u32ChnID);
            usleep(10000);
            continue;
        }
        stFrame.u64PhyAddr[0] = (AX_U64)AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.u64VirAddr[0] = (AX_U64)AX_POOL_GetBlockVirAddr(BlkId);

        SAMPLE_Fill_Color(stFrame.enImgFormat,
                          stFrame.u32Width,
                          stFrame.u32Height,
                          stFrame.u32PicStride[0],
                          (AX_U8 *)stFrame.u64VirAddr[0]);

        stFrame.u32BlkId[0] = BlkId;
        stFrame.u32BlkId[1] = AX_INVALID_BLOCKID;
        //SAMPLE_PRT("layer%d-chn%d start send frame, BlkId = 0x%x\n", u32LayerID, u32ChnID, BlkId);
        s32Ret = AX_VO_SendFrame(u32LayerID, u32ChnID, &stFrame, 0);
        if (s32Ret) {
            SAMPLE_PRT("layer%d-chn%d AX_VO_SendFrame failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
            AX_POOL_ReleaseBlock(BlkId);
            usleep(16666);
            continue;
        }

        AX_POOL_ReleaseBlock(BlkId);

        usleep(16666);

    }

exit:
    SAMPLE_PRT("layer%d-chn%d exit, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);

    return NULL;
}

static AX_S32 gLoopExit = 0;

static AX_VOID SigInt(AX_S32 s32SigNo)
{
    SAMPLE_PRT("Catch signal %d\n", s32SigNo);
    gLoopExit = 1;
}

static AX_VOID SigStop(AX_S32 s32SigNo)
{
    SAMPLE_PRT("Catch signal %d\n", s32SigNo);
    gLoopExit = 1;
}

static AX_U32 SAMPLE_VO_WIN_NUM(SAMPLE_VO_MODE_E enMode)
{
    AX_U32 u32WndNum = 0;

    switch (enMode) {
    case VO_MODE_1MUX:
        u32WndNum = 1;
        break;
    case VO_MODE_2MUX:
        u32WndNum = 2;
        break;
    case VO_MODE_4MUX:
        u32WndNum = 4;
        break;
    case VO_MODE_8MUX:
        u32WndNum = 8;
        break;
    case VO_MODE_9MUX:
        u32WndNum = 9;
        break;
    case VO_MODE_16MUX:
        u32WndNum = 16;
        break;
    case VO_MODE_25MUX:
        u32WndNum = 25;
        break;
    case VO_MODE_36MUX:
        u32WndNum = 36;
        break;
    case VO_MODE_49MUX:
        u32WndNum = 49;
        break;
    case VO_MODE_64MUX:
        u32WndNum = 64;
        break;
    case VO_MODE_2X4:
        u32WndNum = 8;
        break;
    case VO_MODE_BUTT:
        u32WndNum = 8;
        break;
    }

    SAMPLE_PRT("u32WndNum = %d\n", u32WndNum);

    return u32WndNum;
}

static AX_S32 ParseVoPubAttr(AX_CHAR *pStr, SAMPLE_VO_CONFIG_S *pstVoConf)
{
    AX_CHAR *p, *end;

    if (!pStr || !pstVoConf)
        return -EINVAL;

    p = pStr;

    if (strstr(p, "dpi")) {
        SAMPLE_PRT("dpi output\n");
        pstVoConf->enVoIntfType = VO_INTF_DPI;
    } else if (strstr(p, "dsi")) {
        SAMPLE_PRT("dsi output\n");
        pstVoConf->enVoIntfType = VO_INTF_DSI;
    } else if (strstr(p, "bt656")) {
        SAMPLE_PRT("bt656 output\n");
        pstVoConf->enVoIntfType = VO_INTF_BT656;
    } else if (strstr(p, "bt1120")) {
        SAMPLE_PRT("bt1120 output\n");
        pstVoConf->enVoIntfType = VO_INTF_BT1120;
    } else {
        SAMPLE_PRT("unsupported interface type, %s\n", p);
        return -EINVAL;
    }

    end = strstr(p, "@");
    p = end + 1;
    pstVoConf->stReso.u32Width = strtoul(p, &end, 10);
    end = strstr(p, "x");
    p = end + 1;
    pstVoConf->stReso.u32Height = strtoul(p, &end, 10);
    end = strstr(p, "@");
    p = end + 1;
    pstVoConf->stReso.u32RefreshRate = strtoul(p, &end, 10);

    SAMPLE_PRT("reso: %dx%d-%d\n", pstVoConf->stReso.u32Width,
               pstVoConf->stReso.u32Height, pstVoConf->stReso.u32RefreshRate);

    return 0;
}

static AX_CHAR WelcomeText [512];
static AX_CHAR MsgText [256];
static RECT stWelcomeRect = {10, 100, 600, 400};
static RECT stMsgRect = {10, 100, 600, 400};
static RECT stVideoRect = {0};

static const AX_CHAR *SysKey = "";

static AX_S32 s32LastKey = -1;
static AX_S32 s32LastKeyCount = 0;

static AX_VOID MakeWelcomeText(AX_VOID)
{
    const AX_CHAR *format;

    SetRect(&stWelcomeRect,  10, 10, g_rcScr.right - 10, g_rcScr.bottom / 2 - 10);
    SetRect(&stMsgRect, 10, stWelcomeRect.bottom + 10, g_rcScr.right - 10, g_rcScr.bottom - 20);

    format = "Welcome to the world of MiniGUI. \nIf you can see this text, MiniGUI Version %d.%d.%d can run on this hardware board.";

    sprintf(WelcomeText, format, MINIGUI_MAJOR_VERSION, MINIGUI_MINOR_VERSION, MINIGUI_MICRO_VERSION);

    strcpy(MsgText, HL_ST_NOMES);
}

static LRESULT HelloWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    RECT box_rc;

    SysKey = "";

    switch (message) {
    case MSG_CREATE:
        SAMPLE_PRT("%s MSG_CREATE enter\n", __func__);
        MakeWelcomeText();
        SetTimer(hWnd, 100, 200);
        break;

    case MSG_TIMER:
        sprintf(MsgText, HL_ST_TIMER, (PVOID)GetTickCount());
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_LBUTTONDOWN:
        SAMPLE_PRT("%s MSG_LBUTTONDOWN enter\n", __func__);
        strcpy(MsgText, HL_ST_LBD);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_LBUTTONUP:
        SAMPLE_PRT("%s MSG_LBUTTONUP enter\n", __func__);
        strcpy(MsgText, HL_ST_LBU);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_RBUTTONDOWN:
        SAMPLE_PRT("%s MSG_RBUTTONDOWN enter\n", __func__);
        strcpy(MsgText, HL_ST_RBD);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_RBUTTONUP:
        SAMPLE_PRT("%s MSG_RBUTTONUP enter\n", __func__);
        strcpy(MsgText, HL_ST_RBU);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_PAINT:
        hdc = BeginPaint(hWnd);
        SetBrushColor(hdc, 0x0);
        box_rc = stVideoRect;
        ScreenToClient(hWnd, &box_rc.left, &box_rc.top);
        ScreenToClient(hWnd, &box_rc.right, &box_rc.bottom);
        FillBox(hdc,  box_rc.left, box_rc.top,
                RECTW(box_rc), RECTH(box_rc));
        TextOut(hdc, 60, 60, "Hello world!");
        DrawText(hdc, WelcomeText, -1, &stWelcomeRect, DT_LEFT | DT_WORDBREAK);
        DrawText(hdc, MsgText, -1, &stMsgRect, DT_LEFT | DT_WORDBREAK);
        EndPaint(hWnd, hdc);
        return 0;

    case MSG_SYSKEYDOWN:
        SysKey = HL_ST_SYS;
    case MSG_KEYDOWN:
        SAMPLE_PRT("%s MSG_KEYDOWN enter\n", __func__);
        if (s32LastKey == wParam)
            s32LastKeyCount++;
        else {
            s32LastKey = wParam;
            s32LastKeyCount = 1;
        }
        sprintf(MsgText, HL_ST_KEYD,
                (AX_S32)wParam, SysKey, s32LastKeyCount);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        return 0;

    case MSG_KEYLONGPRESS:
        SAMPLE_PRT("%s MSG_KEYLONGPRESS enter\n", __func__);
        sprintf(MsgText, HL_ST_KEYLONG, (AX_S32)wParam);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_KEYALWAYSPRESS:
        SAMPLE_PRT("%s MSG_KEYALWAYSPRESS enter\n", __func__);
        sprintf(MsgText, HL_ST_KEYALWAY, (AX_S32)wParam);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        break;

    case MSG_KEYUP:
        SAMPLE_PRT("%s MSG_KEYUP enter\n", __func__);
        sprintf(MsgText, HL_ST_KEYU, (AX_S32)wParam);
        InvalidateRect(hWnd, &stMsgRect, TRUE);
        return 0;

    case MSG_CLOSE:
        SAMPLE_PRT("%s MSG_CLOSE enter\n", __func__);
        KillTimer(hWnd, 100);
        DestroyMainWindow(hWnd);
        PostQuitMessage(hWnd);
        return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}


static AX_VOID MainLoop()
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;

    SetRect(&stVideoRect, 100, 100, 680, 600);


#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER, "helloworld", 0, 0);
#endif

    CreateInfo.dwStyle = WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = HL_ST_CAP;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow(&CreateInfo);
    if (hMainWnd == HWND_INVALID) {
        SAMPLE_PRT("CreateMainWindow failed\n");
        return;
    }

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    while (!gLoopExit && GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    DestroyMainWindow(hMainWnd);

    MainWindowThreadCleanup(hMainWnd);

    SAMPLE_PRT("exit\n");

}

static AX_VOID Usage(AX_CHAR *name)
{
    fprintf(stderr, "\n\nusage: %s [-vm]\n", name);

    fprintf(stderr, "\n Test options:\n\n");
    fprintf(stderr, "\t-v <interface>@<w>x<h>@<refresh>\tset vo pub-attr\n");
    fprintf(stderr, "\t-m <mux-num>\tset channel mode. 0: 1-MUX, 1: 2-MUX, 2: 4-MUX, 3: 8-MUX, 4: 9-MUX, 5: 16-MUX\n");
    fprintf(stderr, "\n Example:\n\n");
    fprintf(stderr,
            "\t./sample_minigui -v dsi@720x1280@60 -m 5\n");
}

int MiniGUIMain(int argc, const char *argv[])
{
    AX_S32 c;
    AX_S32 i, s32Ret = 0;
    AX_U32 u32WndNum = 0, u32Mux = VO_MODE_2MUX;
    AX_U64 u64BlkSize = 0;
    SAMPLE_VO_CHN_THREAD_PARAM_S stChnThreadParam[64] = {0};

    if (argc < 2) {
        SAMPLE_PRT("parameters need to be specified\n");
        Usage(argv[0]);
        return 0;
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SigInt);
    signal(SIGTSTP, SigStop);

    opterr = 0;
    while ((c = getopt(argc, argv, "m:v:")) != -1) {
        switch (c) {
        case 'm':
            u32Mux = strtoul(optarg, NULL, 10);
            if (u32Mux > VO_MODE_16MUX)
                u32Mux = VO_MODE_16MUX;
            break;
        case 'v':
            s32Ret = ParseVoPubAttr(optarg, &stVoConf);
            if (s32Ret) {
                Usage(argv[0]);
                return s32Ret;
            }
            break;
        default:
            Usage(argv[0]);
            return 0;
        }
    }

    s32Ret = AX_SYS_Init();
    if (s32Ret) {
        SAMPLE_PRT("AX_SYS_Init failed, s32Ret = 0x%x\n", s32Ret);
        return -1;
    }

    s32Ret = AX_POOL_Exit();
    if (s32Ret) {
        SAMPLE_PRT("AX_POOL_Exit failed, s32Ret = 0x%x\n", s32Ret);
        goto exit0;
    }

    s32Ret = AX_VO_Init();
    if (s32Ret) {
        SAMPLE_PRT("AX_VO_Init failed, s32Ret = 0x%x\n", s32Ret);
        goto exit0;
    }

    u64BlkSize = stVoConf.stVoLayerAttr.stImageSize.u32Width * stVoConf.stVoLayerAttr.stImageSize.u32Height * 3 / 2;
    s32Ret = SAMPLE_VO_CREATE_POOL(16, u64BlkSize, 512, &stVoConf.stVoLayerAttr.u32PoolId);
    if (s32Ret) {
        SAMPLE_PRT("SAMPLE_VO_CREATE_POOL failed, s32Ret = 0x%x\n", s32Ret);
        goto exit1;
    }

    SAMPLE_PRT("layer pool id = 0x%x\n", stVoConf.stVoLayerAttr.u32PoolId);

    stVoConf.enVoMode = u32Mux;
    u32WndNum = SAMPLE_VO_WIN_NUM(stVoConf.enVoMode);
    stVoConf.stVoLayerAttr.u32ChnNr = u32WndNum;
    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConf);
    if (s32Ret) {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed, s32Ret = 0x%x\n", s32Ret);
        goto exit2;
    }

    for (i = 0; i < u32WndNum; i++) {
        stChnThreadParam[i].u32LayerID = stVoConf.VoLayer;
        stChnThreadParam[i].u32ChnID = i;
        stChnThreadParam[i].u32ThreadForceStop = 0;
        pthread_create(&stChnThreadParam[i].ThreadID, NULL, SAMPLE_VO_CHN_THREAD, &stChnThreadParam[i]);
    }

    MainLoop();

    for (i = 0; i < u32WndNum; i++) {
        stChnThreadParam[i].u32ThreadForceStop = 1;
        pthread_join(stChnThreadParam[i].ThreadID, NULL);
    }

    SAMPLE_COMM_VO_StopVO(&stVoConf);

exit2:
    SAMPLE_VO_POOL_DESTROY(stVoConf.stVoLayerAttr.u32PoolId);

exit1:
    AX_VO_Deinit();

    for (i = 0; i < u32WndNum; i++) {
        if (stChnThreadParam[i].u32UserPoolId)
            SAMPLE_VO_POOL_DESTROY(stChnThreadParam[i].u32UserPoolId);
    }

exit0:
    AX_SYS_Deinit();

    SAMPLE_PRT("done, s32Ret = 0x%x\n", s32Ret);

    return s32Ret;
}

