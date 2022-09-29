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

#include "ax_sys_api.h"
#include "ax_venc_api.h"
#include "sample_utils.h"

/* Max resolution: 5584x4188 */
// #define VENC_MAX_WIDTH  5584
// #define VENC_MAX_HEIGHT 4188
// #define VENC_MIN_WIDTH  136
// #define VENC_MIN_HEIGHT 136
// /**************************************
// * QPMAP
// **************************************/
// #define QpMapBufNum 10
// #define HEVC_MAX_CU_SIZE 64
// #define AVC_MAX_CU_SIZE 16
// #define MAX_CU_SIZE 64
// #define ROIMAP_PREFETCH_EXT_SIZE 1536
// #define ENABLE_REQUEST_IDR 1
// #define REQUEST_IDR_INTERVAL 5
// #define HEVC_STREAM 1 //0: HEVC, 1:264

//#define GET_STREAM_SELECT
//#define ROI_ENABLE
//#define DYNAMIC_RC
//#define DYNAMIC_FRAMERATE
//#define DYNAMIC_SET_HW_FREQUENCE
//#define ENABLE_DYNAMIC_VUI
//#define ENABLE_START_STOP_TEST  // stopRecv immediately after startRevc

#define SAMPLE_NAME "SampleVenc"
#define VENC_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"", SAMPLE_NAME, __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"\n", SAMPLE_NAME, __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s: %s:%d Error! "str"", "sample_venc.c", __func__, __LINE__, ##arg); \
    }while(0)

// #define CLIP3(x, y, z)  ((z) < (x) ? (x) : ((z) > (y) ? (y) : (z)))
// #define MIN(a, b)       ((a) < (b) ? (a) : (b))

static AX_S32 gLoopExit = 0;


AX_S32 SampleVencUsage1(SAMPLE_VENC_CMD_PARA_T *pstArg)
{
    AX_S32 s32Ret = -1;
    AX_S32 i;
    AX_S32 chn_num = 0;

    SAMPLE_VENC_ATTR_S *pVencChnAttr = NULL;

    if (pstArg == NULL) {
        SAMPLE_ERR_LOG("SampleVencUsage1 failed, pstArg is null.\n");
        return -1;
    }

    pVencChnAttr = malloc(sizeof(SAMPLE_VENC_ATTR_S));
    if (pVencChnAttr == NULL) {
        SAMPLE_ERR_LOG("pVencChnAttr malloc failed!\n");
        return -1;
    } else {
        memset(pVencChnAttr, 0, sizeof(SAMPLE_VENC_ATTR_S));
    }

    pVencChnAttr->nChnNum = pstArg->chnNum; // 1;//CHN_NUM;

    for (i = 0; i < pVencChnAttr->nChnNum; i++) {
        pVencChnAttr->tDevAttr[i].enLinkMode = AX_NONLINK_MODE;
        pVencChnAttr->tDevAttr[i].eImgFormat = AX_YUV420_SEMIPLANAR;
        pVencChnAttr->tDevAttr[i].pCmdl = pstArg;
        // SAMPLE_LOG("VENC %d widthSrc:%d, heightSrc:%d, strideSrc:%d",
        //    i, pstArg->nSrcWidth, pstArg->nSrcHeight, pstArg->picStride);
    }

    s32Ret = SampleVencStart(pVencChnAttr);
    if (0 != s32Ret) {
        SAMPLE_ERR_LOG("SampleVencStart failed, ret=0x%x.\n", s32Ret);
        goto END;
    }

#ifndef ENABLE_START_STOP_TEST
    ///////////////////////////////////////////////////////
    while (!gLoopExit) {
        sleep(2);
    }
#endif

END:

    s32Ret = SampleVencStop(pVencChnAttr);
    if (0 != s32Ret) {
        SAMPLE_ERR_LOG("SampleVencStart failed, ret=0x%x.\n", s32Ret);
        free(pVencChnAttr);
        return s32Ret;
    }

    free(pVencChnAttr);
    return 0;
}

static void SigInt(int sigNo)
{
    VENC_LOG("Catch signal %d\n", sigNo);
    gLoopExit = 1;
}

int main(int argc, char *argv[])
{
    VENC_LOG("Build at %s %s\n", __DATE__, __TIME__);

    AX_S32 isExit = 1;
    AX_S32 s32Ret = -1;
    SAMPLE_VENC_CMD_PARA_T *pCmdLinePara = NULL;
    AX_VENC_MOD_ATTR_S stModAttr = {.enVencType = VENC_VIDEO_ENCODER};

    SetMaskSIGALRM();

    pCmdLinePara = malloc(sizeof(SAMPLE_VENC_CMD_PARA_T));
    if (pCmdLinePara == NULL) {
        SAMPLE_ERR_LOG("pCmdLinePara malloc failed!\n");
        return -1;
    } else {
        memset(pCmdLinePara, 0, sizeof(SAMPLE_VENC_CMD_PARA_T));
    }

    signal(SIGINT, SigInt);

    SetDefaultParameter(pCmdLinePara);

    s32Ret = VencParameterGet(argc, argv, pCmdLinePara);
    if (0 != s32Ret) {
        VENC_LOG("Invalid input argument!\n");
        goto err0;
    }

    s32Ret = AX_SYS_Init();
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_SYS_Init failed! Error Code:0x%X\n", s32Ret);
        goto err0;
    }

    s32Ret = AX_POOL_Exit();
    if(s32Ret) {
        VENC_LOG("AX_POOL_Exit failed! Error Code:0x%X\n", s32Ret);
        goto err1;
    }

    s32Ret = AX_VENC_Init(&stModAttr);
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_VENC_Init error.\n");
        goto err2;
    }

    SAMPLE_LOG("&cmdLinePara:0x%x ", pCmdLinePara);

    s32Ret = SampleVencUsage1(pCmdLinePara);
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("SampleVencUsage1 error. s32Ret:0x%x\n", s32Ret);
        goto err3;
    }

    s32Ret = AX_VENC_Deinit();
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_VENC_Deinit failed! Error Code:0x%X\n", s32Ret);
    }

    s32Ret = AX_POOL_Exit();
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_POOL_Exit failed! Error Code:0x%X\n", s32Ret);
    }

    s32Ret = AX_SYS_Deinit();
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_SYS_Deinit failed! Error Code:0x%X\n", s32Ret);
    }

    free(pCmdLinePara);
    return 0;

err3:
    AX_VENC_Deinit();
err2:
    AX_POOL_Exit();
err1:
    AX_SYS_Deinit();
err0:
    free(pCmdLinePara);

    return -1;
}
