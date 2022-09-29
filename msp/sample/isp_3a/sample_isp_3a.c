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

#include "ax_isp_api.h"
#include "sample_isp_3a.h"
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
    SYS_CASE_NONE  = -1,
    SYS_CASE_SINGLE_OS04A10  = 0,
    SYS_CASE_SINGLE_IMX334   = 1,
    SYS_CASE_SINGLE_GC4653   = 2,
    SYS_CASE_DUAL_OS04A10    = 3,
    SYS_CASE_BUTT
} COMMON_SYS_CASE_E;

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3},
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3},
};


COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Sdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Hdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleGc4653[] = {

    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2560, 1440, 2560, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
    {640, 360, 640, AX_YUV420_SEMIPLANAR, 3},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolDoubleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15 * 2},   /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5 * 2},   /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6 * 2},   /*vin nv21/nv21 use */
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3 * 2},
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3 * 2},
};

CAMERA_T gCams[MAX_CAMERAS] = {0};

static volatile AX_S32 gLoopExit = 0;

AX_S32 g_isp_force_loop_exit = 0;

void *IspRun(void *args)
{
    AX_U32 i = (AX_U32)args;

    COMM_ISP_PRT("cam %d is running...\n", i);

    while (!g_isp_force_loop_exit) {
        if (gCams[i].bOpen)
            AX_ISP_Run(gCams[i].nPipeId);
    }
    return NULL;
}

AX_S32 SysRun()
{
    AX_S32 axRet = 0, i;
    g_isp_force_loop_exit = 0;
    for (i = 0; i < MAX_CAMERAS; i++) {
        if (gCams[i].bOpen)
        {
            pthread_create(&gCams[i].tIspProcThread, NULL, IspRun, (AX_VOID *)i);
            pthread_create(&gCams[i].tIspAFProcThread, NULL, AfRun, (AX_VOID *)i);
        }
    }

    while (!gLoopExit) {
        sleep(1);
    }

    g_isp_force_loop_exit = 1;

    for (i = 0; i < MAX_CAMERAS; i++) {
        if (gCams[i].bOpen) {
            pthread_cancel(gCams[i].tIspProcThread);
            pthread_cancel(gCams[i].tIspAFProcThread);
            axRet = pthread_join(gCams[i].tIspProcThread, NULL);
            if (axRet < 0) {
                COMM_ISP_PRT(" isp run thread exit failed, ret=0x%x.\n", axRet);
            }
            axRet = pthread_join(gCams[i].tIspAFProcThread, NULL);
            if (axRet < 0) {
                COMM_ISP_PRT(" isp run 3A thread exit failed, ret=0x%x.\n", axRet);
            }
        }
    }

    return 0;
}

AX_VOID PrintHelp()
{
    COMM_ISP_PRT("command:\n");
    COMM_ISP_PRT("\t-c: ISP Test Case:\n");
    COMM_ISP_PRT("\t\t0: Single OS04A10\n");
    COMM_ISP_PRT("\t\t1: Single IMX334\n");
    COMM_ISP_PRT("\t\t2: Single GC4653\n");
    COMM_ISP_PRT("\t\t3: DUAL OS04A10\n");

    COMM_ISP_PRT("\t-e: SDR/HDR Mode:\n");
    COMM_ISP_PRT("\t\t1: SDR\n");
    COMM_ISP_PRT("\t\t2: HDR 2DOL\n");
}

static AX_VOID __sigint(int iSigNo)
{
    COMM_ISP_PRT("Catch signal %d\n", iSigNo);
    gLoopExit = 1;

    return ;
}

int main(int argc, char *argv[])
{
    COMM_ISP_PRT("ISP Sample. Build at %s %s\n", __DATE__, __TIME__);

    int c;
    int isExit = 0, i;
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_NONE;
    COMMON_SYS_ARGS_T tCommonArgs = {0};
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_LINEAR_MODE;
    SAMPLE_SNS_TYPE_E eSnsType = OMNIVISION_OS04A10;
    AX_S32 axRet = 0;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigint);

    if (argc < 2) {
        PrintHelp();
        exit(0);
    }

    while ((c = getopt(argc, argv, "c:e:h")) != -1) {
        isExit = 0;
        switch (c) {
        case 'c':
            eSysCase = (COMMON_SYS_CASE_E)atoi(optarg);
            break;
        case 'e':
            eHdrMode = (AX_SNS_HDR_MODE_E)atoi(optarg);
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

    if (eSysCase >= SYS_CASE_BUTT || eSysCase <= SYS_CASE_NONE) {
        COMM_ISP_PRT("error case type\n");
        exit(0);
    }

    if (eSysCase == SYS_CASE_SINGLE_OS04A10) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10) / sizeof(gtSysCommPoolSingleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
    } else if (eSysCase == SYS_CASE_SINGLE_IMX334) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = SONY_IMX334;
        COMMON_ISP_GetSnsConfig(SONY_IMX334, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE) {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleImx334Sdr) / sizeof(gtSysCommPoolSingleImx334Sdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleImx334Sdr;
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
        } else {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleImx334Hdr) / sizeof(gtSysCommPoolSingleImx334Hdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleImx334Hdr;
        }
    } else if (eSysCase == SYS_CASE_SINGLE_GC4653) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = GALAXYCORE_GC4653;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleGc4653) / sizeof(gtSysCommPoolSingleGc4653[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleGc4653;
        COMMON_ISP_GetSnsConfig(GALAXYCORE_GC4653, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
    } else if (eSysCase == SYS_CASE_DUAL_OS04A10) {
        tCommonArgs.nCamCnt = 2;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[1].stSnsAttr, &gCams[1].stSnsClkAttr, &gCams[1].stDevAttr, &gCams[1].stPipeAttr, &gCams[1].stChnAttr);
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolDoubleOs04a10) / sizeof(gtSysCommPoolDoubleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolDoubleOs04a10;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        gCams[i].eSnsType = eSnsType;
        gCams[i].stSnsAttr.eSnsMode = eHdrMode;
        gCams[i].stDevAttr.eSnsMode = eHdrMode;
        gCams[i].stPipeAttr.eSnsMode = eHdrMode;
        gCams[i].bUser3a = AX_TRUE;
        gCams[i].tAeFuncs.pfnAe_Init = sample_ae_init;
        gCams[i].tAeFuncs.pfnAe_Exit = sample_ae_deinit;
        gCams[i].tAeFuncs.pfnAe_Run  = sample_ae_run;
        gCams[i].tAwbFuncs.pfnAwb_Init = sample_awb_init;
        gCams[i].tAwbFuncs.pfnAwb_Run = sample_awb_run;
        gCams[i].tAwbFuncs.pfnAwb_Exit = sample_awb_deinit;
        gCams[i].tLscFuncs.pfnLsc_Init = sample_lsc_init;
        gCams[i].tLscFuncs.pfnLsc_Run = sample_lsc_run;
        gCams[i].tLscFuncs.pfnLsc_Exit = sample_lsc_deinit;

        if (i == 0) {
            gCams[i].nDevId = 0;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_0;
            gCams[i].nPipeId = 0;
        } else if (i == 1) {
            gCams[i].nDevId = 2;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_2;
            gCams[i].nPipeId = 2;
        }
        SetSensorFunc(gCams[i].nPipeId, gCams[i].eSnsType);
    }

    AX_NPU_SDK_EX_ATTR_T sNpuAttr;
    sNpuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
    axRet = AX_NPU_SDK_EX_Init_with_attr(&sNpuAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_NPU_SDK_EX_Init_with_attr failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_SYS_Init(&tCommonArgs);
    if (axRet) {
        COMM_ISP_PRT("isp sys init fail\n");
        goto EXIT;
    }

    COMMON_CAM_Init();

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        axRet = COMMON_CAM_Open(&gCams[i]);
        if (axRet)
            goto EXIT;
        gCams[i].bOpen = AX_TRUE;
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

    SysRun();

EXIT:
    for (i = 0; i < MAX_CAMERAS; i++) {
        if (!gCams[i].bOpen)
            continue;
        COMMON_CAM_Close(&gCams[i]);
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

