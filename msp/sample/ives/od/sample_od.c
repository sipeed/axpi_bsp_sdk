/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include "sample_od.h"
#include <string.h>
#include "sample_util.h"
#include "trace.h"

AX_S32 SAMPLE_IVES_OD_ENTRY(AX_VOID) {
    AX_S32 ret = -1;
    AX_U64 u64StartTick, u64EndTick;
    const AX_U32 IMG_WIDTH = 1920;
    const AX_U32 IMG_HEIGTH = 1080;
    const AX_CHAR *IMAGE_FILE_PATH = {"/opt/data/ives/1920x1080_ref0.nv12.yuv"};
    AX_IVES_OD_IMAGE_S stImg;
    AX_U8 u8OdRslt;

    AX_OD_CHN_ATTR_S stChnAttr = {.odChn = 0,
                                  .stArea = {0, 0, 640, 480},
                                  .u32FrameRate = 30,
                                  .u8ThrdY = 100,
                                  .u8ConfidenceY = 80,
                                  .u32LuxThrd = 0,
                                  .u32LuxDiff = 0};

    /* load images */
    memset(&stImg, 0, sizeof(stImg));
    stImg.pstImg = SAMPLE_LOAD_IMAGE(IMAGE_FILE_PATH, IMG_WIDTH, IMG_HEIGTH, AX_YUV420_SEMIPLANAR);
    if (!stImg.pstImg) {
        goto EXIT0;
    }

    ret = AX_IVES_OD_Init();
    if (0 != ret) {
        ALOGE("AX_IVES_OD_Init() fail, ret = 0x%x", ret);
        goto EXIT0;
    }

    ret = AX_IVES_OD_CreateChn(stChnAttr.odChn, &stChnAttr);
    if (0 != ret) {
        ALOGE("AX_IVES_OD_CreateChn(Chn: %d) fail, ret = 0x%x", stChnAttr.odChn, ret);
        goto EXIT1;
    }

    u64StartTick = SAMPLE_GET_TICK_COUNT();
    ret = AX_IVES_OD_Process(stChnAttr.odChn, &stImg, &u8OdRslt);
    u64EndTick = SAMPLE_GET_TICK_COUNT();
    printf("OD (Chn %d, img: %lld, od: %d, elapsed time: %lld ms\n", stChnAttr.odChn, stImg.pstImg->u64SeqNum, u8OdRslt,
           u64EndTick - u64StartTick);
    if (0 != ret) {
        ALOGE("AX_IVES_OD_Process(Chn: %d, seq: %lld) fail, ret = 0x%x", stChnAttr.odChn, stImg.pstImg->u64SeqNum, ret);
        goto EXIT2;
    }

    ret = AX_IVES_OD_DestoryChn(stChnAttr.odChn);
    if (0 != ret) {
        ALOGE("AX_IVES_OD_DestoryChn(Chn: %d) fail, ret = 0x%x", stChnAttr.odChn, ret);
        goto EXIT1;
    }

    ret = AX_IVES_OD_DeInit();
    if (0 != ret) {
        ALOGE("AX_IVES_OD_DeInit(Chn: %d) fail, ret = 0x%x", stChnAttr.odChn, ret);
        return ret;
    }

    return 0;

EXIT2:
    AX_IVES_OD_DestoryChn(stChnAttr.odChn);

EXIT1:
    AX_IVES_OD_DeInit();

EXIT0:
    if (stImg.pstImg) {
        SAMPLE_FREE_IMAGE(stImg.pstImg);
        stImg.pstImg = NULL;
    }

    return ret;
}