/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include "sample_util.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "ax_sys_api.h"
#include "trace.h"

static AX_BOOL LoadImgFile(const AX_CHAR *pImgFile, AX_U64 *pPhyAddr, AX_VOID **ppVirAddr, AX_U32 *pImgSize) {
    AX_S32 ret;
    AX_U32 nFileSize;
    FILE *fp = NULL;

    if (!pImgFile) {
        ALOGE("nil pointer");
        return AX_FALSE;
    }

    fp = fopen(pImgFile, "rb");

    if (fp) {
        fseek(fp, 0, SEEK_END);
        nFileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (pImgSize && *pImgSize > 0 && *pImgSize != nFileSize) {
            ALOGE("file %s size not right, %d != %d", pImgFile, *pImgSize, nFileSize);
            goto EXIT;
        }

        ret = AX_SYS_MemAlloc(pPhyAddr, ppVirAddr, nFileSize, 0, (const AX_S8 *)"sample_ives");
        if (0 != ret) {
            ALOGE("AX_SYS_MemAlloc(%d) fail, ret=0x%x", nFileSize, ret);
            goto EXIT;
        }

        if (fread(*ppVirAddr, 1, nFileSize, fp) != nFileSize) {
            ALOGE("fread(%s) fail, %s", pImgFile, strerror(errno));
            goto EXIT;
        }

        fclose(fp);

        if (pImgSize) {
            *pImgSize = nFileSize;
        }

        return AX_TRUE;

    } else {
        ALOGE("open file %s fail, %s", pImgFile, strerror(errno));
        return AX_FALSE;
    }

EXIT:
    if (fp) {
        fclose(fp);
    }

    return AX_FALSE;
}

AX_U32 SAMPLE_CALC_IMAGE_SIZE(AX_U32 u32Width, AX_U32 u32Height, AX_IMG_FORMAT_E eImgType, AX_U32 u32Stride) {
    AX_U32 u32Bpp = 0;
    if (u32Width == 0 || u32Height == 0) {
        ALOGE("Invalid width %d or height %d!", u32Width, u32Height);
        return 0;
    }

    if (0 == u32Stride) {
        u32Stride = u32Width;
    }

    switch (eImgType) {
        case AX_YUV420_PLANAR:
        case AX_YUV420_SEMIPLANAR:
        case AX_YUV420_SEMIPLANAR_VU:
            u32Bpp = 12;
            break;
        case AX_YUV422_INTERLEAVED_YUYV:
        case AX_YUV422_INTERLEAVED_UYVY:
            u32Bpp = 16;
            break;
        case AX_YUV444_PACKED:
        case AX_FORMAT_RGB888:
        case AX_FORMAT_BGR888:
            u32Bpp = 24;
            break;
        case AX_FORMAT_ARGB8888:
            u32Bpp = 32;
            break;
        default:
            u32Bpp = 0;
            break;
    }

    return u32Stride * u32Height * u32Bpp / 8;
}

AX_IVES_IMAGE_S *SAMPLE_LOAD_IMAGE(const AX_CHAR *pImgFile, AX_U32 u32Width, AX_U32 u32Height,
                                   AX_IMG_FORMAT_E eImgType) {
    AX_IVES_IMAGE_S *pstImg = NULL;
    AX_U64 u64PhyAddr;
    AX_U64 u64VirAddr;
    AX_U32 u32ImgSize = SAMPLE_CALC_IMAGE_SIZE(u32Width, u32Height, eImgType, 0);
    if (!LoadImgFile(pImgFile, &u64PhyAddr, (AX_VOID **)&u64VirAddr, &u32ImgSize)) {
        return NULL;
    }

    pstImg = (AX_IVES_IMAGE_S *)malloc(sizeof(AX_IVES_IMAGE_S));
    if (pstImg) {
        memset(pstImg, 0, sizeof(*pstImg));
        pstImg->u32Width = u32Width;
        pstImg->u32Height = u32Height;
        pstImg->enImgFormat = eImgType;

        pstImg->u64PhyAddr[0] = u64PhyAddr;
        pstImg->u64VirAddr[0] = u64VirAddr;
        pstImg->u32PicStride[0] = u32Width;
    }

    return pstImg;
}

AX_VOID SAMPLE_FREE_IMAGE(AX_IVES_IMAGE_S *pstImg) {
    if (pstImg) {
        if (pstImg->u64PhyAddr[0] > 0) {
            AX_SYS_MemFree(pstImg->u64PhyAddr[0], (AX_VOID *)pstImg->u64VirAddr[0]);
        }

        free(pstImg);
    }
}

AX_U64 SAMPLE_GET_TICK_COUNT(AX_VOID)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}