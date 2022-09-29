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
#include "sample_vin_slave.h"
#include "common_sys.h"
#include "common_vin.h"
#include "common_cam.h"

/* tuning server */
#ifdef TUNING_CTRL
    #include "ax_nt_ctrl_api.h"
#endif


#include "ax_interpreter_external_api.h"

typedef enum {
    SYS_CASE_NONE                   = -1,
    SYS_CASE_SINGLE_OS04A10         = 0,
    SYS_CASE_SINGLE_OS08A20          = 1,
    SYS_CASE_OS08A20_AND_OS04A10     = 2,
    SYS_CASE_ISP_BYPASS              = 3,
    SYS_CASE_BUTT
} COMMON_SYS_CASE_E;

#define TUNING_BIN_FILE_OS08A20_HDR      "/opt/etc/os08a20_hdr_2x_npu.bin"
#define TUNING_BIN_FILE_OS04A10_HDR     "/opt/etc/os04a10_hdr_2x_npu.bin"

#define TUNING_BIN_FILE_OS08A20_SDR      "/opt/etc/os08a20_sdr_1x_npu.bin"
#define TUNING_BIN_FILE_OS04A10_SDR     "/opt/etc/os04a10_sdr_1x_npu.bin"


static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
};


static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs08a20[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},

};


static COMMON_SYS_POOL_CFG_T gtSysCommPoolOs08a20AndOs04a10Sdr[] = {
    /* Os08a20 common pool buf alloc */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 12},      /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},      /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 4},           /*vin nv21/nv21 use */

    /* os04a10 common pool buf alloc */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 12},      /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},      /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 4},           /*vin nv21/nv21 use */
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolOs08a20AndOs04a10Hdr[] = {
    /* Os08a20 common pool buf alloc */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 15},      /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},      /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 4},           /*vin nv21/nv21 use */

    /* os04a10 common pool buf alloc */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15},      /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},      /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 4},           /*vin nv21/nv21 use */
};



CAMERA_T gCams[MAX_CAMERAS] = {0};
volatile AX_S32 gLoopExit = 0;
AX_S32 g_isp_force_loop_exit = 0;
static AX_S32 g_tx_transfer_loop_exit[2] = {0};

static void *IspRun(void *args)
{
    AX_U32 i = (AX_U32)args;

    COMM_ISP_PRT("cam %d  pipe %d is running...\n", i, gCams[i].nPipeId);

    while (!g_isp_force_loop_exit) {
        AX_ISP_Run(gCams[i].nPipeId);
    }
    return NULL;
}

static void *YuvTxTransfer(void *arg)
{
    AX_S32 timeOutMs = -1;
    int retval = -1;
    CAMERA_T *pCam = (CAMERA_T *)arg;
    AX_IMG_INFO_T   img_info = {0};
    AX_S32 nPipeId = pCam->nPipeId;
    AX_S32 chn = AX_YUV_SOURCE_ID_MAIN;
    AX_S32 nTxDev = pCam->nTxDev;

    //prctl(PR_SET_NAME, "mipi tx transfer");

    while (g_tx_transfer_loop_exit[nTxDev] == 0) {

        retval = AX_VIN_GetYuvFrame(nPipeId, chn, &img_info, timeOutMs);
        if (retval != 0) {
            COMM_ISP_PRT("return error! [%d].\n", chn);
            usleep(20 * 1000);
            continue;
        }

        usleep(10 * 1000);
#if 0
        COMM_ISP_PRT(" pipe=%d, nTxDev = %d, w/h %d %d, u64SeqNum = %lld, u64PTS = %lld\n",
                     nPipeId, nTxDev,
                     img_info.tFrameInfo.stVFrame.u32Width, img_info.tFrameInfo.stVFrame.u32Height,
                     img_info.tFrameInfo.stVFrame.u64SeqNum, img_info.tFrameInfo.stVFrame.u64PTS);
#endif

        retval = AX_VIN_TxSendData(nTxDev, &img_info.tFrameInfo.stVFrame, timeOutMs);
        if (retval != 0) {
            AX_VIN_ReleaseYuvFrame(nPipeId, chn, &img_info);
            COMM_ISP_PRT("AX_VIN_TxSendData failed pipe=%d\n", nPipeId);
            usleep(20 * 1000);
            continue;
        }

        AX_VIN_ReleaseYuvFrame(nPipeId, chn, &img_info);
    }

    COMM_ISP_PRT("YuvTxTransfer txId=%d exit!\n", nTxDev);

}

static AX_S32 AxMipiTxThreadRun(CAMERA_T *pCam)
{
    AX_S32 nTxDev = pCam->nTxDev;

    g_tx_transfer_loop_exit[nTxDev] = 0;
    pthread_create(&pCam->tTxTransferThread, AX_NULL, YuvTxTransfer, pCam);

    return 0;
}

static AX_S32 AxMipiTxThreadExit(CAMERA_T *pCam)
{
    AX_S32 axRet = 0;
    AX_S32 nTxDev = pCam->nTxDev;

    g_tx_transfer_loop_exit[nTxDev] = 1;

    axRet = pthread_join(pCam->tTxTransferThread, AX_NULL);
    if (axRet < 0) {
        COMM_ISP_PRT(" mipi tx transfer thread exit failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

static AX_S32 SysRun(int idx)
{
    g_isp_force_loop_exit = 0;

    if (gCams[idx].bOpen) {
        pthread_create(&gCams[idx].tIspProcThread, AX_NULL, IspRun, (AX_VOID *)idx);;
        AxMipiTxThreadRun(&gCams[idx]);
    }

    return 0;
}

static AX_S32 SysExit(int idx)
{
    AX_S32 axRet = 0;

    g_isp_force_loop_exit = 1;

    if (gCams[idx].bOpen) {

        /* mipi tx thread exit */
        AxMipiTxThreadExit(&gCams[idx]);

        axRet = pthread_join(gCams[idx].tIspProcThread, AX_NULL);
        if (axRet < 0) {
            COMM_ISP_PRT(" isp run thread exit failed, ret=0x%x.\n", axRet);
        }
    }

    return 0;
}


AX_VOID PrintHelp()
{
    COMM_ISP_PRT("command:\n");
    COMM_ISP_PRT("\t-c: ISP Test Case:\n");
    COMM_ISP_PRT("\t\t0: Single OS04A10\n");
    COMM_ISP_PRT("\t\t1: Single OS08A20\n");
    COMM_ISP_PRT("\t\t2:  DUAL RX0: OS08A20, RX2: OS04A10\n");
    COMM_ISP_PRT("\t\t3: RX0: OS08A20, RX1: OS04A10, ISP Bypass Mode\n");

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
    COMM_ISP_PRT("ISP Sample slave. Build at %s %s\n", __DATE__, __TIME__);

    int c;
    int isExit = 0, i = 0;
    AX_S32 axRet = 0;
    AX_U8 ispBypass = 0;
    COMMON_SYS_ARGS_T tCommonArgs = {0};
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_HDR_2X_MODE;
    AX_NPU_SDK_EX_ATTR_T sNpuAttr;
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_NONE;

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

    if (eSysCase == SYS_CASE_SINGLE_OS04A10) {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_HDR_2X_MODE) {
            snprintf(gCams[0].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS04A10_HDR);
        } else if (eHdrMode == AX_SNS_LINEAR_MODE) {
            snprintf(gCams[0].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS04A10_SDR);
        }

        gCams[0].eRawType = AX_RT_RAW10;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10) / sizeof(gtSysCommPoolSingleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10;
    } else if (eSysCase == SYS_CASE_SINGLE_OS08A20) {
        tCommonArgs.nCamCnt = 1;
        gCams[0].eSnsType = OMNIVISION_OS08A20;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS08A20, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_HDR_2X_MODE) {
            snprintf(gCams[0].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS08A20_HDR);
            gCams[0].eRawType = AX_RT_RAW10;
        } else if (eHdrMode == AX_SNS_LINEAR_MODE) {
            gCams[0].eRawType = AX_RT_RAW12;
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;

            snprintf(gCams[0].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS08A20_SDR);
        }
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
        gCams[0].stSnsClkAttr.nSnsClkIdx = 0;    /* mclk0 only by AX DEMO board, User defined */
        gCams[1].stSnsClkAttr.nSnsClkIdx = 2;    /* mclk2 only by AX DEMO board, User defined */
        if (eHdrMode == AX_SNS_HDR_2X_MODE) {
            snprintf(gCams[0].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS08A20_HDR);
            snprintf(gCams[1].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS04A10_HDR);
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolOs08a20AndOs04a10Hdr) / sizeof(gtSysCommPoolOs08a20AndOs04a10Hdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolOs08a20AndOs04a10Hdr;

            gCams[0].stSnsAttr.nFrameRate = 25;
            gCams[1].stSnsAttr.nFrameRate = 25;
        } else if (eHdrMode == AX_SNS_LINEAR_MODE) {
            snprintf(gCams[0].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS08A20_SDR);
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            snprintf(gCams[1].szTuningFileName, MAX_FILE_NAME_CHAR_SIZE, "%s", TUNING_BIN_FILE_OS04A10_SDR);
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolOs08a20AndOs04a10Sdr) / sizeof(gtSysCommPoolOs08a20AndOs04a10Sdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolOs08a20AndOs04a10Sdr;
        }
        if (eSysCase == SYS_CASE_ISP_BYPASS) {
            gCams[0].stDevAttr.bIspBypass = AX_TRUE;
            gCams[1].stDevAttr.bIspBypass = AX_TRUE;
        }
    }

    /* step1: param config */
    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        gCams[i].stSnsAttr.eSnsMode = eHdrMode;
        gCams[i].stDevAttr.eSnsMode = eHdrMode;
        gCams[i].stPipeAttr.eSnsMode = eHdrMode;
        gCams[i].stChnAttr.tChnAttr[1].bEnable = 0;
        gCams[i].stChnAttr.tChnAttr[2].bEnable = 0;

        if (i == 0) {
            gCams[i].nDevId = 0;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_0;
            gCams[i].nTxDev = AX_MIPI_TX_DEV_0;
            gCams[i].nPipeId = 0;
        } else if (i == 1) {
            gCams[i].nDevId = 2;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_2;
            gCams[i].nTxDev = AX_MIPI_TX_DEV_1;
            gCams[i].nPipeId = 1;
        }
    }

#ifdef TUNING_CTRL
    AX_NT_CtrlInitMode(AX_NT_CTRL_MODE_CAS_SLAVE);

    axRet =  AX_NT_CtrlInit(8082);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_NT_CtrlInit failed, ret=0x%x.\n", axRet);
        return -1;
    }
#endif

    /* step2: config npu no-virtual mode */
    sNpuAttr.eHardMode = AX_NPU_VIRTUAL_DISABLE;
    axRet = AX_NPU_SDK_EX_Init_with_attr(&sNpuAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_NPU_SDK_EX_Init_with_attr failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* step3: config common pool */
    axRet = COMMON_SYS_Init(&tCommonArgs);
    if (axRet) {
        COMM_ISP_PRT("isp sys init fail\n");
        goto EXIT;
    }

    /* step4: vin and mipi init */
    axRet = COMMON_CAM_Init();
    axRet |= COMMON_ISP_InitTx();
    if (0 != axRet) {
        COMM_ISP_PRT("failed, ret=0x%x.\n", axRet);
        goto EXIT;
    }

    if (eSysCase == SYS_CASE_ISP_BYPASS) {
        ispBypass = 1;
    }
    /* step5: vin and isp enable, mipi tx enable */
    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        gCams[i].bOpen = AX_TRUE;

        axRet = COMMON_ISP_SetMipiTxAttr(gCams[i].nTxDev, gCams[i].eSnsType, eHdrMode, ispBypass);
        if (0 != axRet) {
            COMM_ISP_PRT(" failed, ret=0x%x.\n", axRet);
            goto EXIT;
        }

        axRet = COMMON_ISP_OpenTx(gCams[i].nTxDev, gCams[i].eSnsType, ispBypass);
        if (axRet) {
            COMM_ISP_PRT(" failed, ret=0x%x.\n", axRet);
            goto EXIT;
        }

        if (eSysCase == SYS_CASE_ISP_BYPASS) {
            axRet = AX_VIN_SetDevBindTx(gCams[i].nDevId, gCams[i].nTxDev);
            if (axRet) {
                COMM_ISP_PRT(" failed, ret=0x%x.\n", axRet);
                goto EXIT;
            }
        }

        axRet = COMMON_CAM_Open(&gCams[i]);
        if (axRet) {
            COMM_ISP_PRT(" failed, ret=0x%x.\n", axRet);
            goto EXIT;
        }

        /* step6: itp run thread working */
        if (eSysCase != SYS_CASE_ISP_BYPASS) {
            SysRun(i);
        }

        COMM_ISP_PRT("camera %d is open\n", i);
    }

    while (!gLoopExit) {
        sleep(1);
    }

EXIT:
    for (i = 0; i < MAX_CAMERAS; i++) {
        if (!gCams[i].bOpen) {
            continue;
        }

        if (eSysCase != SYS_CASE_ISP_BYPASS) {
            SysExit(i);
        }

        COMMON_CAM_Close(&gCams[i]);
        COMMON_ISP_CloseTx(gCams[i].nTxDev);
    }

#ifdef TUNING_CTRL
    AX_NT_CtrlDeInit();
#endif

    COMMON_ISP_DeinitTx();
    COMMON_CAM_Deinit();
    COMMON_SYS_DeInit();

    /* exit */
    exit(0);
}
