/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include "sample_md.h"
#include <string.h>
#include "sample_util.h"
#include "trace.h"

#define MAX_DET_IMAGE_COUNT (3)
AX_S32 SAMPLE_IVES_MD_ENTRY(AX_VOID) {
    AX_S32 ret = -1;
    AX_U32 i, j;
    AX_U64 u64StartTick, u64EndTick;
    const AX_U32 IMG_WIDTH = 1920;
    const AX_U32 IMG_HEIGTH = 1080;
    const AX_CHAR *IMAGE_FILE_PATH[MAX_DET_IMAGE_COUNT] = {"/opt/data/ives/1920x1080_ref0.nv12.yuv",
                                                           "/opt/data/ives/1920x1080_det1.nv12.yuv",
                                                           "/opt/data/ives/1920x1080_det2.nv12.yuv"};
    AX_IVES_IMAGE_S *pstDetImgs[MAX_DET_IMAGE_COUNT] = {NULL};
    AX_MD_MB_THR_S stThrs;
    AX_U32 u32MbXCount;
    AX_U32 u32MbYCount;
    AX_IVES_CCBLOB_S stBlob;
    memset(&stBlob, 0, sizeof(stBlob));
    stBlob.enMode = IVES_CCL_MODE_8C;

    AX_MD_CHN_ATTR_S stChnAttr = {
        .mdChn = 0, .enAlgMode = AX_MD_MODE_REF, .stMbSize = {32, 32}, .stArea = {100, 200, 640, 480}, .u8ThrY = 20};

    u32MbXCount = stChnAttr.stArea.u32W / stChnAttr.stMbSize.u32W;
    u32MbYCount = stChnAttr.stArea.u32H / stChnAttr.stMbSize.u32H;

    /* load images */
    for (j = 0; j < MAX_DET_IMAGE_COUNT; ++j) {
        pstDetImgs[j] = SAMPLE_LOAD_IMAGE(IMAGE_FILE_PATH[j], IMG_WIDTH, IMG_HEIGTH, AX_YUV420_SEMIPLANAR);
        if (!pstDetImgs[j]) {
            goto EXIT0;
        }

        pstDetImgs[j]->u64SeqNum = j;
    }

    ret = AX_IVES_MD_Init();
    if (0 != ret) {
        ALOGE("AX_IVES_MD_Init() fail, ret = 0x%x", ret);
        goto EXIT0;
    }

    ret = AX_IVES_MD_CreateChn(stChnAttr.mdChn, &stChnAttr);
    if (0 != ret) {
        ALOGE("AX_IVES_MD_CreateChn(Chn: %d) fail, ret = 0x%x", stChnAttr.mdChn, ret);
        goto EXIT1;
    }

    for (j = 0; j < MAX_DET_IMAGE_COUNT; ++j) {
        if (j > 0) {
            u64StartTick = SAMPLE_GET_TICK_COUNT();
        }

        if (1 == j) {
            ret = AX_IVES_MD_ProcessV2(stChnAttr.mdChn, pstDetImgs[j], &stThrs, &stBlob);
        } else {
            ret = AX_IVES_MD_Process(stChnAttr.mdChn, pstDetImgs[j], &stThrs);
        }

        if (j > 0) {
            u64EndTick = SAMPLE_GET_TICK_COUNT();
            printf("MD (Chn %d, img: %lld, elapsed time: %lld ms\n", stChnAttr.mdChn, pstDetImgs[j]->u64SeqNum,
                   u64EndTick - u64StartTick);
        }

        if (0 != ret) {
            ALOGE("AX_IVES_MD_Process(Chn: %d, img: %lld, ref) fail, ret = 0x%x", stChnAttr.mdChn,
                  pstDetImgs[j]->u64SeqNum, ret);
            goto EXIT2;
        }

        if (u32MbXCount * u32MbYCount != stThrs.u32Count) {
            ALOGE("received mb count is not right, %d != %d", stThrs.u32Count, u32MbXCount * u32MbYCount);
            goto EXIT2;
        }

        if (j > 0) {
            printf("IMAGE %lld MB THRS: %d x %d = %d\n", pstDetImgs[j]->u64SeqNum, u32MbXCount, u32MbYCount,
                   stThrs.u32Count);
            for (i = 0; i < stThrs.u32Count; ++i) {
                printf("%2d ", stThrs.pMbThrs[i]);
                if (0 == ((i + 1) % u32MbXCount)) {
                    printf("\n");
                }
            }

            if (1 == j) {
                for (i = 0; i < stBlob.u32RegionNum; ++i) {
                    printf("region %2d, (%4d, %4d) (%4d, %4d)\n",
                            i + 1,
                            stBlob.arrRegion[i].u16Left,
                            stBlob.arrRegion[i].u16Top,
                            stBlob.arrRegion[i].u16Right,
                            stBlob.arrRegion[i].u16Bottom
                            );
                }
            }
        }
    }

    ret = AX_IVES_MD_DestoryChn(stChnAttr.mdChn);
    if (0 != ret) {
        ALOGE("AX_IVES_MD_DestoryChn(Chn: %d) fail, ret = 0x%x", stChnAttr.mdChn, ret);
    }

    goto EXIT1;

EXIT2:
    AX_IVES_MD_DestoryChn(stChnAttr.mdChn);

EXIT1:
    AX_IVES_MD_DeInit();

EXIT0:
    for (j = 0; j < MAX_DET_IMAGE_COUNT; ++j) {
        if (pstDetImgs[j]) {
            SAMPLE_FREE_IMAGE(pstDetImgs[j]);
            pstDetImgs[j] = NULL;
        }
    }

    return ret;
}
