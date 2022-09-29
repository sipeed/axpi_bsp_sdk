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
    AX_CHAR *sFile;
} SAMPLE_ARGS_S;

typedef enum _SAMPLE_BSBOUNDAR_YTYPE {
    BSPARSER_NO_BOUNDARY = 0,
    BSPARSER_BOUNDARY = 1,
    BSPARSER_BOUNDARY_NON_SLICE_NAL = 2
} SAMPLE_BSBOUNDAR_YTYPE_E;

AX_S32 gChnNumber = 1;
static AX_S32 gLoopDecodeNumber = 1;
static AX_S32 gMilliSec = -1;
AX_S32 gLoopExit = 0;
static AX_S32 gWriteFrames = 1;

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

    return 0;
}

AX_S32 LinkExit()
{
    AX_S32 s32Ret = -1;

    AX_MOD_INFO_S DstMod = {0};
    AX_MOD_INFO_S SrcMod = {0};

    SrcMod.enModId = AX_ID_VDEC;
    SrcMod.s32GrpId = 0;
    SrcMod.s32ChnId = 0;
    DstMod.enModId = AX_ID_IVPS;
    DstMod.s32GrpId = 0;
    DstMod.s32ChnId = 0;
    s32Ret = AX_SYS_UnLink(&SrcMod, &DstMod);
    printf("unlink ret:%x\n", s32Ret);

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
}

int main(int argc, char *argv[])
{
    extern int optind;
    AX_S32 c;
    AX_S32 isExit = 0;
    AX_S32 type = 26;
    AX_S32 s32Ret = -1;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SigInt);

    while ((c = getopt(argc, argv, "c:L:t:M:T:h:w:n")) != -1) {
        isExit = 0;
        switch (c) {
        case 'c':
            gChnNumber = atoi(optarg);
            break;
        case 'L':
            gLoopDecodeNumber = atoi(optarg);
            break;
        case 't':
            gMilliSec = atoi(optarg);
            break;
        case 'M':
            gGrpAttr.enMode = atoi(optarg);
            break;
        case 'w':
            gWriteFrames = atoi(optarg);
            break;
        case 'T':
            type = atoi(optarg);
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

    s32Ret = SampleIVPS_Init();
    if (AX_SUCCESS != s32Ret) {
        printf("SampleIVPS_Init error. s32Ret:0x%x \n", s32Ret);
        return -1;
    }

    printf("SampleIVPS_Init success\n");

    s32Ret = SampleVdecInit(gGrpAttr.enType);
    if (AX_SUCCESS != s32Ret) {
        printf("SampleVdecInit error. s32Ret:0x%x \n", s32Ret);
        return -1;
    }

    pthread_t chnTids[AX_VDEC_MAX_GRP_SIZE];
    AX_S32 i;

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

            pthread_create(&chnTids[i], NULL, H264DecFrameFunc, (void *)&GrpArgs[i]);
        }
    }

    printf("Vdec create thread ok\n");

    ///////////////////////////////////////////////////////
    /*create thread for get frame*/
    pthread_t recvTid;
    if (pthread_create(&recvTid, NULL, IvpsGetFrameRun, &gWriteFrames) != 0) {
        printf("__read_thread error!\n");
    }

    for (i = 0; i < gChnNumber; i++) {
        pthread_join(chnTids[i], NULL);
    }

    pthread_join(recvTid, NULL);

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

    if (gGrpAttr.enType == PT_JPEG) {
        for (i = 0; i < gChnNumber; i++) {
            JpegDecExitFunc(i);
        }
    } else {
        for (i = 0; i < gChnNumber; i++) {
            H264DecExitFunc(i);
        }
    }

    AX_VDEC_DeInit();
    SampleVdecExit();
    AX_POOL_Exit();
    AX_SYS_Deinit();

    return 0;
}
