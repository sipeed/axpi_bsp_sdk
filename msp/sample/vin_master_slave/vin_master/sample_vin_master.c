/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "ax_isp_api.h"
#include "sample_vin_master.h"
#include "common_sys.h"
#include "common_vin.h"
#include "common_cam.h"

/* tuning server */
#ifdef TUNING_CTRL
    #include "ax_nt_stream_api.h"
    #include "ax_nt_ctrl_api.h"
#endif

#include "ax_interpreter_external_api.h"

typedef enum {
    SYS_CASE_NONE                   = -1,
    SYS_CASE_SINGLE_OS04A10         = 0,
    SYS_CASE_SINGLE_OS08A20         = 1,
    SYS_CASE_OS08A20_AND_OS04A10    = 2,
    SYS_CASE_ISP_BYPASS             = 3,
    SYS_CASE_BUTT
} COMMON_SYS_CASE_E;

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 40},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs08a20[] = {
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 40},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolOs08a20AndOs04a10[] = {
    /* Os08a20 common pool buf alloc */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 1},      /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 6},      /* sensor raw 12 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 4},           /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 4},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 4},

    /* os04a10 common pool buf alloc */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 1},      /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 6},       /* sensor raw10 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 4},           /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 4},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 4},
};

static CAMERA_T gCams[MAX_CAMERAS];
volatile AX_S32 gLoopExit = 0;
static AX_S32 g_yuv_cap_loop_exit[4] = {0};

static void *getYuv(void *arg)
{
    AX_S32 timeOutMs = 200;
    CAMERA_T *pCam = (CAMERA_T *)arg;
    int retval = -1;
    AX_IMG_INFO_T   img_info = {0};
    AX_S32 nDevId = pCam->nDevId;
    AX_S32 nPipeId = pCam->nPipeId;

    while (g_yuv_cap_loop_exit[nDevId] == 0) {
        /* you can open here for get frame debug */
#if 0
        retval = AX_VIN_GetSnsFrame(nDevId, AX_SNS_HDR_FRAME_L, &img_info, timeOutMs);
        if (retval != 0) {
            COMM_ISP_PRT("return error! nDevId[%d] retval = 0x%x\n", nDevId, retval);
            usleep(20 * 1000);
            continue;
        }
        usleep(20 * 1000);
#if 0
        COMM_ISP_PRT(" nDevId = %d, w/h %d %d, enImgFormat %d, u64SeqNum = %lld, u64PTS = %lld\n",
                     nDevId,
                     img_info.tFrameInfo.stVFrame.u32Width, img_info.tFrameInfo.stVFrame.u32Height,
                     img_info.tFrameInfo.stVFrame.enImgFormat,
                     img_info.tFrameInfo.stVFrame.u64SeqNum, img_info.tFrameInfo.stVFrame.u64PTS);
#endif

        retval = AX_VIN_SendYuvFrame(nPipeId, &img_info, timeOutMs);
        if (retval != 0) {
            AX_VIN_ReleaseSnsFrame(nDevId, AX_SNS_HDR_FRAME_L, &img_info);
            COMM_ISP_PRT("AX_VIN_SendYuvFrame failed nDevId=%d\n", nDevId);
            usleep(20 * 1000);
            continue;
        }

        AX_VIN_ReleaseSnsFrame(nDevId, AX_SNS_HDR_FRAME_L, &img_info);
#endif
        usleep(200 * 1000);
    }

    COMM_ISP_PRT("getYuv thread nDevId=%d exit!\n", nDevId);
}

static AX_S32 AxCapYuvThreadRun(CAMERA_T *pCam)
{
    AX_S32 nDevId = pCam->nDevId;

    g_yuv_cap_loop_exit[nDevId] = 0;
    pthread_create(&pCam->tIspProcThread, AX_NULL, getYuv, pCam);

    return 0;
}

static AX_S32 AxCapYuvThreadExit(CAMERA_T *pCam)
{
    AX_S32 axRet = 0;
    AX_S32 nDevId = pCam->nDevId;

    g_yuv_cap_loop_exit[nDevId] = 1;

    axRet = pthread_join(pCam->tIspProcThread, AX_NULL);
    if (axRet < 0) {
        COMM_ISP_PRT(" AxCapYuvThreadExit thread exit failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 ISP_MASTER_Open(CAMERA_T *pCam, COMMON_SYS_CASE_E eSysCase)
{
    AX_S32 axRet;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;
    AX_MIPI_RX_DEV_E nRxDev = pCam->nRxDev;
    SAMPLE_SNS_TYPE_E eSnsType = pCam->eSnsType;
    AX_IMG_FORMAT_E ePixelFmt = pCam->ePixelFmt;
    AX_SNS_HDR_MODE_E eHdrMode = pCam->eHdrMode;
    AX_VIN_DEV_BIND_PIPE_T tDevBindPipe = {0};
    AX_VIN_SNS_DUMP_ATTR_T  tDumpAttr = {0};
    AX_PIPE_ATTR_T tPipeAttr = {0};
    tDevBindPipe.nNum = 1;
    tDevBindPipe.nPipeId[0] = nPipeId;

    /* AX vin init */
    axRet = AX_VIN_Create(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_Create failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetRunMode(nPipeId, AX_ISP_PIPELINE_NONE_NPU);
    if (0 != axRet) {
        COMM_ISP_PRT(" failed, ret=0x%x.\n", axRet);
        return -1;
    }

    if (eSysCase == SYS_CASE_ISP_BYPASS) {
        axRet = COMMON_ISP_SetDevAttrEx(nDevId, eSnsType, ePixelFmt, eHdrMode,
                                        AX_DEV_WORK_MODE_MASTER, AX_TRUE, AX_FALSE, AX_TRUE);
    } else {
        axRet = COMMON_ISP_SetDevAttrEx(nDevId, eSnsType, ePixelFmt, eHdrMode,
                                        AX_DEV_WORK_MODE_MASTER, AX_FALSE, AX_TRUE, AX_FALSE);
    }

    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetDevAttr failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_ISP_SetMipiAttr(nRxDev, eSnsType, AX_TRUE);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_MIPI_RX_SetAttr failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetChnAttr(nPipeId, &pCam->stChnAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetChnAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_ISP_SetPipeAttr(nPipeId, eSnsType, ePixelFmt, eHdrMode);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VI_SetPipeAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }
    axRet = AX_VIN_GetPipeAttr(nPipeId, &tPipeAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_GetPipeAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }
    tPipeAttr.tCompCtrl.bitIFEBypass = 1;
    /*change hde mode*/
    axRet = AX_VIN_SetPipeAttr(nPipeId, &tPipeAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetPipeAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }
    /* config bind */
    axRet = AX_VIN_SetDevBindPipe(nDevId, &tDevBindPipe);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetDevBindPipe failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_ISP_Open(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_ISP_Open failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_VIN_Start(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_Start failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_PIPE_FRAME_SOURCE_ID_YUV, AX_PIPE_FRAME_SOURCE_TYPE_USER);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetPipeFrameSource failed, ret=0x%x\n", axRet);
        return -1;
    }

#ifdef USE_USER_FRAME_MODE
    axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_PIPE_FRAME_SOURCE_ID_IFE, AX_PIPE_FRAME_SOURCE_TYPE_USER);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetPipeFrameSource failed, ret=0x%x\n", axRet);
        return -1;
    }
#endif

    if (eSysCase == SYS_CASE_ISP_BYPASS) {
        axRet = AX_VIN_SetPipeFrameSource(nPipeId, AX_PIPE_FRAME_SOURCE_ID_IFE, AX_PIPE_FRAME_SOURCE_TYPE_USER);
        if (0 != axRet) {
            COMM_ISP_PRT("AX_VIN_SetPipeFrameSource failed, ret=0x%x\n", axRet);
            return -1;
        }
    }

    axRet = AX_VIN_EnableDev(nDevId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_EnableDev failed, ret=0x%x.\n", axRet);
        return -1;
    }
    if (eSysCase == SYS_CASE_ISP_BYPASS) {
        tDumpAttr.bEnable = AX_TRUE;
        tDumpAttr.nDepth = 2;
        axRet = AX_VIN_SetSnsDumpAttr(nDevId, &tDumpAttr);
        if (0 != axRet) {
            COMM_ISP_PRT(" AX_VIN_SetSnsDumpAttr failed, ret=0x%x.\n", axRet);
            return -1;
        }
    } else {
        axRet = AX_VIN_SetNonImageAttr(nDevId);
        if (0 != axRet) {
            COMM_ISP_PRT("AX_VIN_SetNonImageAttr failed, ret=0x%x.\n", axRet);
            return -1;
        }
    }

    return 0;
}

AX_S32 ISP_MASTER_Close(CAMERA_T *pCam)
{
    AX_S32 axRet = 0;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;

    axRet = AX_VIN_DisableDev(nDevId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_DisableDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_Stop(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_Stop failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    axRet = AX_ISP_Close(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_ISP_Close failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    AX_VIN_Destory(nPipeId);

    COMM_ISP_PRT("%s: pipe %d: exit.\n", __func__, nPipeId);
    return 0;
}

AX_VOID PrintHelp()
{
    COMM_ISP_PRT("command:\n");
    COMM_ISP_PRT("\t-c: ISP Test Case:\n");
    COMM_ISP_PRT("\t\t0: Single OS04A10\n");
    COMM_ISP_PRT("\t\t1: Single OS08A20\n");
    COMM_ISP_PRT("\t\t2:  RX0: OS08A20, RX1: OS04A10\n");
    COMM_ISP_PRT("\t\t3: RX0: OS08A20, RX1: OS04A10, ISP Bypass Mode\n");

    COMM_ISP_PRT("\t-e: SDR/HDR Mode:\n");
    COMM_ISP_PRT("\t\t1: SDR\n");
    COMM_ISP_PRT("\t\t2: HDR 2DOL\n");

    COMM_ISP_PRT("\t-d: dump continues raw count:\n");
    COMM_ISP_PRT("\tfor example:\n");
    COMM_ISP_PRT("\tSDR dump continues raw count 50:\n");
    COMM_ISP_PRT("\t\tcd /opt/bin && ./sample_vin_master -c3 -e1 -d50\n");
    COMM_ISP_PRT("\tHDR dump continues raw count 50:\n");
    COMM_ISP_PRT("\t\tcd /opt/bin && ./sample_vin_master -c3 -e2 -d100\n");

}

static AX_VOID __sigint(int iSigNo)
{
    COMM_ISP_PRT("Catch signal %d\n", iSigNo);
    gLoopExit = 1;

    return ;
}

int main(int argc, char *argv[])
{
    COMM_ISP_PRT("ISP Sample isp master. Build at %s %s\n", __DATE__, __TIME__);

    int c;
    int isExit = 0, i;
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_NONE;
    COMMON_SYS_ARGS_T tCommonArgs = {0};
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_LINEAR_MODE;
    AX_S32 axRet = 0;
    AX_U32 nDumpFrame = 0;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigint);

    if (argc < 2) {
        PrintHelp();
        exit(0);
    }

    while ((c = getopt(argc, argv, "c:e:d:h")) != -1) {
        isExit = 0;
        switch (c) {
        case 'c':
            eSysCase = (COMMON_SYS_CASE_E)atoi(optarg);
            break;
        case 'e':
            eHdrMode = (AX_SNS_HDR_MODE_E)atoi(optarg);
            break;
        case 'd':
            nDumpFrame = atoi(optarg);
            break;
        case 'h':
            isExit = 1;
            break;
        default:
            isExit = 1;
            break;
        }
    }

    if (isExit) {
        PrintHelp();
        exit(0);
    }

    if (eSysCase == SYS_CASE_SINGLE_OS04A10) {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr, &gCams[0].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10) / sizeof(gtSysCommPoolSingleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10;

    } else if (eSysCase == SYS_CASE_SINGLE_OS08A20) {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = OMNIVISION_OS08A20;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS08A20, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr, &gCams[0].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs08a20) / sizeof(gtSysCommPoolSingleOs08a20[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs08a20;

    } else if ((eSysCase == SYS_CASE_OS08A20_AND_OS04A10) || (eSysCase == SYS_CASE_ISP_BYPASS)) {
        tCommonArgs.nCamCnt = 2;
        gCams[0].eSnsType = OMNIVISION_OS08A20;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS08A20, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        gCams[1].eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[1].stSnsAttr, &gCams[1].stSnsClkAttr, &gCams[1].stDevAttr,
                                &gCams[1].stPipeAttr, &gCams[1].stChnAttr);

        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolOs08a20AndOs04a10) / sizeof(gtSysCommPoolOs08a20AndOs04a10[0]);
        for(i=0; i< tCommonArgs.nPoolCfgCnt; i++){
           if(gtSysCommPoolOs08a20AndOs04a10[i].nFmt == AX_FORMAT_BAYER_RAW_12BPP ||
               gtSysCommPoolOs08a20AndOs04a10[i].nFmt == AX_FORMAT_BAYER_RAW_10BPP){
                   gtSysCommPoolOs08a20AndOs04a10[i].nBlkCnt += nDumpFrame;
           }
        }
        tCommonArgs.pPoolCfg  = gtSysCommPoolOs08a20AndOs04a10;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        gCams[i].eHdrMode = eHdrMode;
        if (eSysCase == SYS_CASE_ISP_BYPASS) {
            if ((gCams[i].eSnsType == OMNIVISION_OS08A20) && (eHdrMode == AX_SNS_LINEAR_MODE)) {
                gCams[i].eRawType = AX_RT_RAW12;
                gCams[i].ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            } else {
                gCams[i].eRawType = AX_RT_RAW10;
                gCams[i].ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP;
            }
            gCams[i].stDevAttr.eDevWorkMode = AX_DEV_WORK_MODE_MASTER;
        } else {
            gCams[i].eRawType = AX_RT_RAW10;
            gCams[i].ePixelFmt = AX_YUV420_SEMIPLANAR;
        }

        gCams[i].stChnAttr.tChnAttr[0].bEnable = 0;
        if (i == 0) {
            gCams[i].nDevId = 0;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_0;
            gCams[i].nPipeId = 0;
        } else if (i == 1) {
            if ((eHdrMode == AX_SNS_HDR_2X_MODE) && (eSysCase == SYS_CASE_ISP_BYPASS)) {
                gCams[i].nDevId = 2;
                gCams[i].nRxDev = AX_MIPI_RX_DEV_2;
            } else {
                gCams[i].nDevId = 1;
                gCams[i].nRxDev = AX_MIPI_RX_DEV_1;
            }
            gCams[i].nPipeId = 1;
        }
    }

    axRet = COMMON_SYS_Init(&tCommonArgs);
    if (axRet) {
        COMM_ISP_PRT("isp sys init fail\n");
        goto EXIT;
    }

    COMMON_CAM_Init();

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        axRet = ISP_MASTER_Open(&gCams[i], eSysCase);
        if (axRet) {
            goto EXIT;
        }
        gCams[i].bOpen = AX_TRUE;

        AxCapYuvThreadRun(&gCams[i]);

        COMM_ISP_PRT("camera %d is open\n", i);
    }

#ifdef TUNING_CTRL
    /* Net Preview */
    COMM_ISP_PRT("Start the service on the tuning device side.\n");

    axRet =  AX_NT_StreamInit(6000);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_NT_StreamInit failed, ret=0x%x.\n", axRet);
        return -1;
    }
    axRet =  AX_NT_CtrlInit(8082);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_NT_CtrlInit failed, ret=0x%x.\n", axRet);
        return -1;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        AX_NT_SetStreamSource(gCams[i].nPipeId);
    }
    COMM_ISP_PRT("tuning runing.\n");
#endif

    while (!gLoopExit) {
        sleep(1);
    }

EXIT:
    for (i = 0; i < MAX_CAMERAS; i++) {
        if (!gCams[i].bOpen) {
            continue;
        }

        AxCapYuvThreadExit(&gCams[i]);
        ISP_MASTER_Close(&gCams[i]);
    }

#ifdef TUNING_CTRL
    AX_NT_CtrlDeInit();
    AX_NT_StreamDeInit();
#endif

    COMMON_CAM_Deinit();
    COMMON_SYS_DeInit();

    /* exit */
    exit(0);
}
