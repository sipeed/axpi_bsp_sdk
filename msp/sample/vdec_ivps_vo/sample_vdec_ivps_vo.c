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
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

//#include "openssl/md5.h"
#include "ax_vdec_api.h"
#include "ax_base_type.h"
#include "ax_sys_api.h"
#include "ax_buffer_tool.h"
#include "sample_ivps_hal.h"
#include "sample_vdec_hal.h"
#include "sample_vo_hal.h"

typedef struct axSAMPLE_CHN_ARGS_S {
    AX_VDEC_GRP VdecGrp;
    AX_U8 *pu8StreamMem;
    AX_S32 s32StreamLen;
} SAMPLE_CHN_ARGS_S;

typedef struct _SAMPLE_BSPARSER {
    FILE *fInput;
    AX_S32 sSize;
} SAMPLE_BSPARSER_T;

typedef struct _SAMPLE_VDEC_FUNPARAM {
    AX_VDEC_GRP VdecGrp;
    AX_VIDEO_FRAME_INFO_S *pstUsrPic;
    AX_CHAR *sFile;
} SAMPLE_ARGS_S;

typedef enum _SAMPLE_BSBOUNDAR_YTYPE {
    BSPARSER_NO_BOUNDARY = 0,
    BSPARSER_BOUNDARY = 1,
    BSPARSER_BOUNDARY_NON_SLICE_NAL = 2
} SAMPLE_BSBOUNDAR_YTYPE_E;

AX_S32 gChnNumber = 1;
AX_S32 gLoopDecodeNumber = 1;
AX_S32 gLoopExit = 0;
AX_S32 gWriteFrames = 1;
AX_S32 userPicTest = 0;

/*If the device is not connected to a peripheral,uses offline mode*/
static AX_BOOL gOffLine = AX_TRUE;


static SAMPLE_CHN_ARGS_S GrpChnArgs[AX_VDEC_MAX_GRP_SIZE];
static SAMPLE_ARGS_S GrpArgs[AX_VDEC_MAX_GRP_SIZE];

static AX_VDEC_GRP_ATTR_S gGrpAttr = {
    .enType = PT_JPEG,
    .enMode = VIDEO_MODE_FRAME,
    .u32PicWidth = 4096,
    .u32PicHeight = 4096,
    .u32StreamBufSize = 10 * 1024 * 1024,
    .u32FrameBufSize = 10 * 1024 * 1024,
    .u32FrameBufCnt = 1,
};


#define SAMPLE_VO_DEV0 0

static SAMPLE_PARAM_S gInputParam = {
    .u32PicWidth = 1920,
    .u32PicHeight = 1088,
};

SAMPLE_VO_CONFIG_S stVoConf = {
    /* for device */
    .VoDev = SAMPLE_VO_DEV0,
    .enVoIntfType = VO_INTF_DSI0,
    .enIntfSync = VO_OUTPUT_USER,
    .stReso = {
        .u32Width = 1920,
        .u32Height = 1080,
        .u32RefreshRate = 60,
    },
    .u32LayerNr = 1,
    .stVoLayer = {
        {
            // layer0
            .VoLayer = 0,
            .stVoLayerAttr = {
                .stDispRect = {0, 0, 1920, 1080},
                .stImageSize = {1920, 1080},
                .enPixFmt = AX_YUV420_SEMIPLANAR,
                .enLayerSync = VO_LAYER_SYNC_PRIMARY,
                .u32PrimaryChnId = 0,
                .u32FrameRate = 0,
                .u32FifoDepth = 0,
                .u32ChnNr = 2,
                .u32BkClr = 0,
                .enLayerBuf = VO_LAYER_OUT_BUF_POOL,
                .u32InplaceChnId = 0,
                .u32PoolId = 0,
                .enDispatchMode = VO_LAYER_OUT_TO_FIFO,
                .u64KeepChnPrevFrameBitmap = 0x1,
            },
            .enVoMode = VO_MODE_1MUX,
        },
    },
};

AX_S32 LinkInit()
{
    AX_MOD_INFO_S DstMod = {0};
    AX_MOD_INFO_S SrcMod = {0};

    SrcMod.enModId = AX_ID_VDEC;
    SrcMod.s32GrpId = 0;
    SrcMod.s32ChnId = 0;
    DstMod.enModId = AX_ID_IVPS;
    DstMod.s32GrpId = 0;
    DstMod.s32ChnId = 0;
    AX_SYS_Link(&SrcMod, &DstMod);

    SrcMod.enModId = AX_ID_IVPS;
    SrcMod.s32GrpId = 0;
    SrcMod.s32ChnId = 0;
    DstMod.enModId = AX_ID_VO;
    DstMod.s32GrpId = 0;
    DstMod.s32ChnId = 0;
    AX_SYS_Link(&SrcMod, &DstMod);


    return 0;
}

AX_S32 LinkExit()
{
    AX_S32 s32Ret = -1;

    AX_MOD_INFO_S DstMod = {0};
    AX_MOD_INFO_S SrcMod = {0};

    SrcMod.enModId = AX_ID_IVPS;
    SrcMod.s32GrpId = 0;
    SrcMod.s32ChnId = 0;
    DstMod.enModId = AX_ID_VO;
    DstMod.s32GrpId = 0;
    DstMod.s32ChnId = 0;
    s32Ret = AX_SYS_UnLink(&SrcMod, &DstMod);
    printf("vdec unlink ivps ret:%x\n", s32Ret);

    SrcMod.enModId = AX_ID_VDEC;
    SrcMod.s32GrpId = 0;
    SrcMod.s32ChnId = 0;
    DstMod.enModId = AX_ID_IVPS;
    DstMod.s32GrpId = 0;
    DstMod.s32ChnId = 0;
    s32Ret = AX_SYS_UnLink(&SrcMod, &DstMod);
    printf("ivps unlink vo ret:%x\n", s32Ret);

    return s32Ret;
}

static void SigInt(int sigNo)
{
    printf("Catch signal %d\n", sigNo);
    gLoopExit = 1;

    return ;
}


static void PrintHelp()
{
    printf("usage: sample_vdec streamFile <args>\n");
    printf("args:\n");
    printf("  -w:       write YUV frame number to file.    (0: not write, others: write), default: 1\n");
    printf("  -T:       video type.                        (96: PT_H264, 26: PT_JPEG), default: 26 (PT_JPEG)\n");
    printf("  -v        <interface>@<w>x<h>@<refresh>\tset vo pub-attr\n");
    printf("  -u:       flag of userPic test.       (0: not test userPic, 1: test userPic), default: 0\n");
}

int main(int argc, char *argv[])
{
    extern int optind;
    AX_S32 c;
    AX_S32 isExit = 0;
    AX_S32 type = 26;
    AX_S32 s32Ret = -1;
    AX_U32 u32ChnSel = 0;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SigInt);

    while ((c = getopt(argc, argv, "c:L:T:h:w:v:u")) != -1) {
        isExit = 0;
        switch (c) {
        case 'w':
            gWriteFrames = atoi(optarg);
            break;
        case 'T':
            type = atoi(optarg);
            break;
        case 'v':
            s32Ret = ParseVoPubAttr(optarg, &stVoConf);
            if (s32Ret) {
                isExit = 1;
            } else {
                gOffLine = AX_FALSE;
            }
            break;
        case 'u':
            userPicTest = 1;
            break;
        case 'h':
            isExit = 1;
            break;
        default:
            isExit = 1;
            break;
        }
    }
    if (isExit || optind >= argc) {
        PrintHelp();
        exit(0);
    }

    if (gChnNumber < 1 || gChnNumber > AX_VDEC_MAX_GRP_SIZE) {
        printf("Invalid group number\n");
        return -1;
    }
    if (gGrpAttr.enMode != VIDEO_MODE_STREAM && gGrpAttr.enMode != VIDEO_MODE_FRAME) {
        printf("Invalid decode mode\n");
        return -1;
    }

    AX_SYS_Init();

    s32Ret = AX_POOL_Exit();
    if (s32Ret) {
        printf("VDEC AX_POOL_Exit fail!!Error Code:0x%X\n", s32Ret);
        return -1;
    }

    AX_VDEC_Init();
    printf("main get type %d\n", gGrpAttr.enType);

    AX_CHAR *ps8StreamFile = (AX_CHAR *)argv[optind];
    AX_U8 *pu8StreamMem = NULL;
    AX_S32 s32StreamLen = 0;
    #ifdef SAMPLE_VDEC_USERPIC_TEST
    AX_VIDEO_FRAME_INFO_S tUserPic[gChnNumber];
    AX_VIDEO_FRAME_INFO_S *pstUserPic[gChnNumber];
    #endif

    if (type == 96) {
        gGrpAttr.enType = PT_H264;
    } else if (type == 26) {
        gGrpAttr.enType = PT_JPEG;
    } else {
        printf("Invalid decode type, can not supported\n");
        return -1;
    }

    if (gGrpAttr.enType == PT_JPEG) {
        if (LoadFileToMem(ps8StreamFile, &pu8StreamMem, &s32StreamLen)) {
            printf("LoadFileToMem error\n");
            return -1;
        }
    }

    /*vdec link ivps*/
    s32Ret = LinkInit();
    if (AX_SUCCESS != s32Ret) {
        printf("LinkInit error.\n");
    }

    s32Ret = VoInit(u32ChnSel, &gInputParam, &stVoConf);
    if (s32Ret) {
        SAMPLE_PRT("VoInit failed, s32Ret = %d\n", s32Ret);
        return -1;
    }

    s32Ret = SampleIVPS_Init();
    if (AX_SUCCESS != s32Ret) {
        printf("SampleIVPS_Init error. s32Ret:0x%x \n", s32Ret);
        return -1;
    }

    printf("SampleIVPS_Init success\n");

    pthread_t chnTids[AX_VDEC_MAX_GRP_SIZE];
    AX_S32 i;
    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest) {
        for(i = 0; i< gChnNumber;i++)
        {
            memset(&tUserPic[i], 0 , sizeof(AX_VIDEO_FRAME_INFO_S));
            pstUserPic[i] = &tUserPic[i];
            s32Ret = PoolUserPicInit(pstUserPic[i]);
            if (AX_SUCCESS != s32Ret)
            {
                printf("PoolUserPicInit Failed!\n");
                return -1;
            }

            s32Ret = PoolUserPicDisplay(pstUserPic[i]);
            if(AX_SUCCESS != s32Ret)
            {
                printf("PoolUserPicInit Failed!\n");
                return -1;
            }
        }
    }
    #endif
    if (gGrpAttr.enType == PT_JPEG) {
        for (i = 0; i < gChnNumber; i++) {
            GrpChnArgs[i].VdecGrp = i;
            GrpChnArgs[i].pu8StreamMem = pu8StreamMem;
            GrpChnArgs[i].s32StreamLen = s32StreamLen;
            pthread_create(&chnTids[i], NULL, VdecThread, (void *)&GrpChnArgs[i]);
        }
    } else {
        for (i = 0; i < gChnNumber; i++) {
            GrpArgs[i].VdecGrp = i;
            GrpArgs[i].sFile = ps8StreamFile;
            #ifdef SAMPLE_VDEC_USERPIC_TEST
            GrpArgs[i].pstUsrPic = pstUserPic[i];
            #endif
            pthread_create(&chnTids[i], NULL, H264DecFrameFunc, (void *)&GrpArgs[i]);
        }
    }

    printf("Vdec create thread ok\n");

    ///////////////////////////////////////////////////////
    /*create thread for vo get frame if not display peripherals*/
    pthread_t recvTid;
    if (gOffLine) {
        if (pthread_create(&recvTid, NULL, VoGetFrameThread, &gWriteFrames) != 0) {
            printf("__read_thread error!\n");
        }
    }

    for (i = 0; i < gChnNumber; i++) {
        pthread_join(chnTids[i], NULL);
    }

    if (gOffLine) {
        pthread_join(recvTid, NULL);
    }

    if (pu8StreamMem) {
        free(pu8StreamMem);
    }

    s32Ret = LinkExit();
    if (AX_SUCCESS != s32Ret) {
        printf("LinkExit error: %x\n", s32Ret);
    }

    s32Ret = SampleIvpsExit();
    if (AX_SUCCESS != s32Ret) {
        printf("SampleIvpsExit error.\n");
    }

    VoDeInit(&stVoConf);

    if (gGrpAttr.enType == PT_JPEG) {
        for (i = 0; i < gChnNumber; i++) {
            JpegDecExitFunc(i);
        }
    } else {
        for (i = 0; i < gChnNumber; i++) {
            H264DecExitFunc(i);
        }
    }

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest) {
        for(i = 0; i < gChnNumber; i++)
        {
            PoolUserPicDeinit(pstUserPic[i]);
        }
    }
    #endif
    AX_VDEC_DeInit();
    AX_SYS_Deinit();

    return 0;
}
