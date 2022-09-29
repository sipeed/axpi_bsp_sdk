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

#include "ax_base_type.h"
#include "ax_sys_api.h"
#include "ax_buffer_tool.h"
#include "ax_ivps_api.h"


#ifndef AX_SUCCESS
    #define AX_SUCCESS                          0
#endif

extern AX_S32 gLoopExit;


#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"\n", "sample_vdec_ivps.c", __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s: %s:%d Error! "str"\n", "sample_vdec_ivps.c", __func__, __LINE__, ##arg); \
    }while(0)


static IVPS_GRP gIvpsGrpId = 0;

#define IVPS_BUF_POOL_MEM_SIZE (0x100000 * 40) // 40M
#define ALIGN_UP(x, align) (((x) + ((align)-1)) & ~((align)-1))

typedef struct {
    AX_U32 nSize;
    AX_U32 nCnt;
} BLK_INFO_S;


AX_U32 CalcImgSize(AX_U32 nStride, AX_U32 nW, AX_U32 nH, AX_IMG_FORMAT_E eType, AX_U32 nAlign)
{
    AX_U32 nBpp = 0;
    if (nW == 0 || nH == 0) {
        printf("%s: Invalid width %d or height %d!\n", __func__, nW, nH);
        return 0;
    }

    if (0 == nStride) {
        nStride = (0 == nAlign) ? nW : ALIGN_UP(nW, nAlign);
    } else {
        if (nAlign > 0) {
            if (nStride % nAlign) {
                printf("%s: stride: %u not %u aligned.!\n", __func__, nStride, nAlign);
                return 0;
            }
        }
    }

    switch (eType) {
    case AX_YUV420_PLANAR:
    case AX_YUV420_SEMIPLANAR:
    case AX_YUV420_SEMIPLANAR_VU:
        nBpp = 12;
        break;
    case AX_YUV422_INTERLEAVED_YUYV:
    case AX_YUV422_INTERLEAVED_UYVY:
        nBpp = 16;
        break;
    case AX_YUV444_PACKED:
    case AX_FORMAT_RGB888:
        nBpp = 24;
        break;
    case AX_FORMAT_RGBA8888:
    case AX_FORMAT_ARGB8888:
        nBpp = 32;
        break;
    default:
        nBpp = 0;
        break;
    }

    return nStride * nH * nBpp / 8;
}

AX_S32 IVPS_PoolFloorInit(BLK_INFO_S *pBlkInfo, AX_U32 nNum)
{
    AX_S32 ret, i;
    AX_POOL_FLOORPLAN_T PoolFloorPlan;

    if (nNum > 16) {
        printf("Pool Floor num should be smaller than 16\n");
        return -1;
    }
    memset(&PoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));

    for (i = 0; i < nNum; i++) {
        PoolFloorPlan.CommPool[i].MetaSize = 512;
        PoolFloorPlan.CommPool[i].BlkSize = pBlkInfo->nSize;
        PoolFloorPlan.CommPool[i].BlkCnt = pBlkInfo->nCnt;
        PoolFloorPlan.CommPool[i].CacheMode = POOL_CACHE_MODE_NONCACHE;

        memset(PoolFloorPlan.CommPool[i].PartitionName, 0, sizeof(PoolFloorPlan.CommPool[i].PartitionName));
        strcpy((char *)PoolFloorPlan.CommPool[i].PartitionName, "anonymous");
        pBlkInfo++;
    }

    ret = AX_POOL_SetConfig(&PoolFloorPlan);
    if (ret) {
        printf("AX_POOL_SetConfig fail!Error Code:0x%X\n", ret);
        return -1;
    } else {
        printf("AX_POOL_SetConfig success!\n");
    }

    ret = AX_POOL_Init();
    if (ret) {
        printf("AX_POOL_Init fail!!Error Code:0x%X\n", ret);
        return -1;
    } else {
        printf("AX_POOL_Init success!\n");
    }
    return 0;
}

int SampleIVPS_Init()
{
    AX_S32 axRet = 0;
    AX_IVPS_GRP_ATTR_S  stGrpAttr = {0};
    AX_IVPS_PIPELINE_ATTR_S  stPipelineAttr = {0};
    int ch = 0;
    BLK_INFO_S arrBlkInfo[16];

    /****************************IVPS Prepare*********************************/
    /*
     * 1. Create memory pool for IVPS
     */
    arrBlkInfo[0].nSize = CalcImgSize(1920, 1920, 1088, AX_YUV420_SEMIPLANAR, 16);
    arrBlkInfo[0].nCnt = 6;


    axRet = IVPS_PoolFloorInit(&arrBlkInfo[0], 1);
    if (0 != axRet) {
        printf("IVPS_PoolFloorInit failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_IVPS_Init();
    if (0 != axRet) {
        printf("AX_IVPS_Init axRet:%#x\n", axRet);
        return -1;
    }

    stPipelineAttr.tFbInfo.PoolId = AX_INVALID_POOLID;
    stPipelineAttr.nOutChnNum = 1;
    ch = 0;

    stPipelineAttr.tFilter[ch][0].bEnable = AX_TRUE;
    stPipelineAttr.tFilter[ch][0].tFRC.nSrcFrameRate = 25;
    stPipelineAttr.tFilter[ch][0].tFRC.nDstFrameRate = 25;
    stPipelineAttr.tFilter[ch][0].nDstPicOffsetX0 = 0;
    stPipelineAttr.tFilter[ch][0].nDstPicOffsetY0 = 0;
    stPipelineAttr.tFilter[ch][0].nDstPicWidth = 1920;
    stPipelineAttr.tFilter[ch][0].nDstPicHeight = 1088;
    stPipelineAttr.tFilter[ch][0].nDstPicStride = stPipelineAttr.tFilter[ch][0].nDstPicWidth;
    stPipelineAttr.tFilter[ch][0].nDstFrameWidth = 1920;
    stPipelineAttr.tFilter[ch][0].nDstFrameHeight = 1088;
    stPipelineAttr.tFilter[ch][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
    stPipelineAttr.tFilter[ch][0].eEngine = AX_IVPS_ENGINE_TDP;
    stPipelineAttr.tFilter[ch][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
    stPipelineAttr.tFilter[ch][0].tTdpCfg.eCompressMode = AX_COMPRESS_MODE_NONE;
    stPipelineAttr.nOutFifoDepth[ch] = 4;

    stGrpAttr.ePipeline = AX_IVPS_PIPELINE_DEFAULT;
    axRet = AX_IVPS_CreateGrp(gIvpsGrpId, &stGrpAttr);
    if (0 != axRet) {
        printf("AX_IVPS_CreateGrp axRet:%#x\n", axRet);
        return -2;
    }
    axRet = AX_IVPS_SetPipelineAttr(gIvpsGrpId, &stPipelineAttr);
    if (0 != axRet) {
        printf("AX_IVPS_SetPipelineAttr axRet:%#x\n", axRet);
        return -20;
    }

    axRet = AX_IVPS_EnableChn(gIvpsGrpId, ch);
    if (0 != axRet) {
        printf("AX_IVPS_EnableChn ch.%d, axRet:%#x\n", ch, axRet);
        return -3;
    }

    axRet = AX_IVPS_StartGrp(gIvpsGrpId);
    if (0 != axRet) {
        printf("AX_IVPS_StartGrp axRet:%#x\n", axRet);
        return -3;
    }

    return 0;
}

AX_S32 SampleIvpsExit()
{
    AX_S32 axRet = 0;
    int ch = 0;


    ch = 0;
    axRet = AX_IVPS_DisableChn(gIvpsGrpId, ch);
    if (0 != axRet) {
        printf("AX_IVPS_DisableChn ch.%d, axRet:%#x\n", ch, axRet);
    }
    if (0) {
        ch++;
        axRet = AX_IVPS_DisableChn(gIvpsGrpId, ch);
        if (0 != axRet) {
            printf("AX_IVPS_DisableChn ch.%d, axRet:%#x\n", ch, axRet);
        }
    }
    if (0) {
        ch++;
        axRet = AX_IVPS_DisableChn(gIvpsGrpId, ch);
        if (0 != axRet) {
            printf("AX_IVPS_DisableChn ch.%d, axRet:%#x\n", ch, axRet);
        }
    }
    axRet = AX_IVPS_StopGrp(gIvpsGrpId);
    if (0 != axRet) {
        printf("AX_IVPS_StopGrp axRet:%#x\n", axRet);
    }
    axRet = AX_IVPS_DestoryGrp(gIvpsGrpId);
    if (0 != axRet) {
        printf("AX_IVPS_DestoryGrp axRet:%#x\n", axRet);
    }
    axRet = AX_IVPS_Deinit();
    if (0 != axRet) {
        printf("AX_IVPS_Deinit axRet:%#x\n", axRet);
    }

    return 0;
}

AX_VOID SaveFileExt(AX_VIDEO_FRAME_S *tDstFrame, AX_U32 nChnIdx, FILE *fp)
{
    AX_U32 nPixelSize;
    AX_U8 nPixelByte;
    AX_U32 nMapSize;
    char szOutImgFile[128] = {0};
    AX_S32 s32Ret;

    nPixelSize = (AX_U32)tDstFrame->u32PicStride[0] * tDstFrame->u32Height;

    switch (tDstFrame->enImgFormat) {
    case AX_YUV420_SEMIPLANAR_VU:
    case AX_YUV420_PLANAR:
    case AX_YUV420_SEMIPLANAR:
        nPixelByte = 1;
        nMapSize = nPixelSize * 3 / 2;
        break;
    case AX_YUV422_INTERLEAVED_YUYV:
    case AX_YUV422_INTERLEAVED_UYVY:
        nPixelByte = 2;
        nMapSize = nPixelSize * nPixelByte;
        break;
    case AX_YUV444_PACKED:
    case AX_FORMAT_RGB888:
        nPixelByte = 3;
        nMapSize = nPixelSize * nPixelByte;
        break;
    case AX_FORMAT_RGBA8888:
        nPixelByte = 4;
        nMapSize = nPixelSize * nPixelByte;
        break;
    default:
        nPixelByte = 2;
        nMapSize = nPixelSize * nPixelByte;
        break;
    }
    tDstFrame->u64VirAddr[0] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[0], nMapSize);

    printf("SaveFileExt nMapSize: %d  nPixelByte: %d PhyAddr:%llu PhyAddr_UV:%llu\n", nMapSize, nPixelByte,
           tDstFrame->u64PhyAddr[0], tDstFrame->u64PhyAddr[1]);

    if (fp) {

        if (tDstFrame->u64VirAddr[1]) {
            fwrite((AX_VOID *)((AX_U32)tDstFrame->u64VirAddr[0]), nPixelByte, nPixelSize, fp);
            fwrite((AX_VOID *)((AX_U32)tDstFrame->u64VirAddr[1]), nPixelByte, nPixelSize / 2, fp);
        } else {
            fwrite((AX_VOID *)((AX_U32)tDstFrame->u64VirAddr[0]), nPixelByte, nMapSize, fp);
            printf("save yuv\n");
        }

        printf("%s is saved!\n", szOutImgFile);
    }
    s32Ret = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[0], nMapSize);
    if (s32Ret)    {
        printf("AX_SYS_Munmap failed,s32Ret=0x%x\n", s32Ret);
    }

}

void *IvpsGetFrameRun(void *arg)
{
    AX_S32 gWriteFrames = *(AX_S32 *)arg;

    AX_S32 i = 0;
    FILE *fp_out = NULL;
    AX_S32 yuvnum = 0;

    if (gWriteFrames) {
        AX_CHAR file_path[128];
        snprintf(file_path, 128, "%s.yuv", "output");
        printf("Write YUV to: %s\n", file_path);
        fp_out = fopen(file_path, "w");
        if (fp_out == NULL) {
            printf("Ivps open file failed\n");
            return NULL;
        }
    }

    while (1) { // get ivps frame
        AX_S32 s32Ret = 0;
        AX_VIDEO_FRAME_S stIvpsStream = {0};
        memset(&stIvpsStream, 0, sizeof(stIvpsStream));
        s32Ret = AX_IVPS_GetChnFrame(0, 0, &stIvpsStream, -1);
        if (AX_SUCCESS == s32Ret) {
            printf("get ivps (%dx%d)%dB [%#x]\n",
                   stIvpsStream.u32Width, stIvpsStream.u32Height, stIvpsStream.u32FrameSize, stIvpsStream.enImgFormat);
            yuvnum ++;
            //SaveFileExt(&stIvpsStream, 0, fp_out);


            s32Ret = AX_IVPS_ReleaseChnFrame(0, 0, &stIvpsStream);
            if (AX_SUCCESS != s32Ret) {
                printf("AX_IVPS_ReleaseChnFrame failed!\n");
            }
        } else {
            printf("AX_IVPS_GetChnFrame(%d) : %#x\n", i, s32Ret);
        }
        if (yuvnum == gWriteFrames)
            break;

        if (gLoopExit)
            break;
    }

    if (gWriteFrames)
        fclose(fp_out);

    return (void *)0;
}

