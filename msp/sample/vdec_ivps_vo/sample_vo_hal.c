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


#include "ax_interpreter_external_api.h"
#include "sample_vo_hal.h"

#define SIZE_ALIGN(x,align) ((((x)+(align)-1)/(align))*(align))
#ifndef AX_SUCCESS
    #define AX_SUCCESS                          0
#endif

static AX_BOOL gWriteYUV = AX_TRUE;
extern AX_S32 userPicTest;
extern AX_S32 gLoopExit;


typedef struct axSAMPLE_VO_CHN_THREAD_PARAM {
    pthread_t ThreadID;
    AX_U32 u32ThreadForceStop;
    AX_U32 u32LayerID;
    AX_U32 u32ChnID;
    AX_POOL u32UserPoolId;
} SAMPLE_VO_CHN_THREAD_PARAM_S;




AX_S32 ParseVoPubAttr(AX_CHAR *pStr, SAMPLE_VO_CONFIG_S *pstVoConf)
{
    AX_CHAR *p, *end;

    if (!pStr || !pstVoConf)
        return -EINVAL;

    p = pStr;

    if (strstr(p, "dpi")) {
        SAMPLE_PRT("dpi output\n");
        pstVoConf->enVoIntfType = VO_INTF_DPI;
    } else if (strstr(p, "dsi0")) {
        SAMPLE_PRT("dsi0 output\n");
        pstVoConf->enVoIntfType = VO_INTF_DSI0;
    } else if (strstr(p, "dsi1")) {
        SAMPLE_PRT("dsi1 output\n");
        pstVoConf->enVoIntfType = VO_INTF_DSI1;
    } else if (strstr(p, "2dsi")) {
        SAMPLE_PRT("dsi0 output\n");
        pstVoConf->enVoIntfType = VO_INTF_2DSI;
        pstVoConf->u32LayerNr = 2;
    } else if (strstr(p, "bt656")) {
        SAMPLE_PRT("bt656 output\n");
        pstVoConf->enVoIntfType = VO_INTF_BT656;
    } else if (strstr(p, "bt1120")) {
        SAMPLE_PRT("bt1120 output\n");
        pstVoConf->enVoIntfType = VO_INTF_BT1120;
    } else {
        SAMPLE_PRT("unsupported interface type, %s\n", p);
        return -EINVAL;
    }

    end = strstr(p, "@");
    p = end + 1;
    pstVoConf->stReso.u32Width = strtoul(p, &end, 10);
    end = strstr(p, "x");
    p = end + 1;
    pstVoConf->stReso.u32Height = strtoul(p, &end, 10);
    end = strstr(p, "@");
    p = end + 1;
    pstVoConf->stReso.u32RefreshRate = strtoul(p, &end, 10);

    SAMPLE_PRT("reso: %dx%d-%d\n", pstVoConf->stReso.u32Width,
               pstVoConf->stReso.u32Height, pstVoConf->stReso.u32RefreshRate);

    return 0;
}

AX_VOID SaveYUVFile(AX_VIDEO_FRAME_S *tDstFrame, FILE *fp)
{
    AX_U32 nPixelSize;
    AX_S32 s32Ret;
    AX_VOID *pLumaVirAddr = NULL;

    if(tDstFrame == NULL || fp == NULL){
        printf("SaveYUVFile: Pointer is NULL\n");
        return;
    }
    nPixelSize = ((AX_U32)tDstFrame->u32PicStride[0] * tDstFrame->u32Height) * 3 / 2;

    pLumaVirAddr = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[0], nPixelSize);
    if(pLumaVirAddr == NULL){
        printf("SaveYUVFile:AX_SYS_Mmap luma failed\n");
        return;
    }
    printf("SaveYUVFile nPixelByte: %d PhyAddr:%llu PhyAddr_UV:%llu\n", nPixelSize,
           tDstFrame->u64PhyAddr[0], tDstFrame->u64PhyAddr[1]);

    if (fp) {
        fwrite((AX_VOID *)((AX_U32)pLumaVirAddr), 1, nPixelSize, fp);
    }
    s32Ret = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)pLumaVirAddr, nPixelSize);
    if (s32Ret)    {
        printf("AX_SYS_Munmap failed,s32Ret=0x%x\n", s32Ret);
    }

    printf("save yuv end\n");
}

void *VoGetFrameThread(void *arg)
{
    AX_S32 gWriteFrames = *(AX_S32 *)arg;
    FILE *fp_out = NULL;
    AX_S32 yuvnum = 0;
    
    if (gWriteYUV) {
        AX_CHAR file_path[128];
        snprintf(file_path, 128, "%s.yuv", "output");
        printf("Write YUV to: %s\n", file_path);
        fp_out = fopen(file_path, "w");
        if (fp_out == NULL) {
            printf("Open file failed\n");
            return NULL;
        }
    }

    while (1) {
        AX_S32 s32Ret = 0;
        AX_VIDEO_FRAME_S stVoStream = {0};
        memset(&stVoStream, 0, sizeof(stVoStream));
        s32Ret = AX_VO_GetLayerFrame(0, &stVoStream, 0);
        if (AX_SUCCESS == s32Ret) {
            printf("Vou get frame (%dx%d)%dB [%#x],blkid: %x, num=%d \n",
                   stVoStream.u32Width, stVoStream.u32Height, stVoStream.u32FrameSize,
                   stVoStream.enImgFormat, stVoStream.u32BlkId[0], yuvnum);
            yuvnum ++;
            
            if (gWriteYUV){
                if(userPicTest){
                    if(yuvnum == gWriteFrames + 1)
                        SaveYUVFile(&stVoStream, fp_out);
                }else{
                    SaveYUVFile(&stVoStream, fp_out);
                }
            }

            s32Ret = AX_VO_ReleaseLayerFrame(0, &stVoStream);
            if (AX_SUCCESS != s32Ret) {
                printf("AX_VO_ReleaseLayerFrame failed!\n");
                usleep(10 *1000);
            }
        }

        if(userPicTest){
            if (yuvnum == gWriteFrames + 1)
                break;
        }else{
            if (yuvnum == gWriteFrames)
                break;
        }

        if (gLoopExit)
            break;
    }

    if (gWriteYUV)
        fclose(fp_out);

    return (void *)0;
}

static int SAMPLE_VO_POOL_DESTROY(AX_U32 u32PoolID)
{
    return AX_POOL_MarkDestroyPool(u32PoolID);
}

static AX_S32 SAMPLE_VO_CREATE_POOL(AX_U32 u32BlkCnt, AX_U64 u64BlkSize, AX_U64 u64MetaSize, AX_U32 *pPoolID)
{
    AX_POOL_CONFIG_T stPoolCfg = {0};

    stPoolCfg.MetaSize = u64MetaSize;
    stPoolCfg.BlkCnt = u32BlkCnt;
    stPoolCfg.BlkSize = u64BlkSize;
    stPoolCfg.CacheMode = POOL_CACHE_MODE_NONCACHE;
    strcpy((char *)stPoolCfg.PartitionName, "anonymous");

    *pPoolID = AX_POOL_CreatePool(&stPoolCfg);
    if (*pPoolID == AX_INVALID_POOLID) {
        SAMPLE_PRT("AX_POOL_CreatePool failed, u32BlkCnt = %d, u64BlkSize = 0x%llx, u64MetaSize = 0x%llx\n", u32BlkCnt,
                   u64BlkSize, u64MetaSize);
        return -1;
    }

    SAMPLE_PRT("u32BlkCnt = %d, u64BlkSize = 0x%llx, pPoolID = %d\n", u32BlkCnt, u64BlkSize, *pPoolID);

    return 0;
}

static AX_VOID *SAMPLE_VO_CHN_THREAD(AX_VOID *pData)
{
    AX_S32 s32Ret = 0;
    AX_VIDEO_FRAME_S stFrame = {0};
    AX_U32 u32FrameSize;
    AX_U32 u32LayerID, u32ChnID;
    AX_BLK BlkId;
    VO_CHN_ATTR_S stChnAttr;
    AX_U64  u64PhysAddr;
    AX_VOID *u64VirAddr;
    SAMPLE_VO_CHN_THREAD_PARAM_S *pstChnThreadParam = (SAMPLE_VO_CHN_THREAD_PARAM_S *)pData;

    u32LayerID = pstChnThreadParam->u32LayerID;
    u32ChnID = pstChnThreadParam->u32ChnID;

    s32Ret = AX_VO_GetChnAttr(u32LayerID, u32ChnID, &stChnAttr);
    if (s32Ret) {
        SAMPLE_PRT("layer%d-chn%d AX_VO_GetChnAttr failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
        goto exit;
    }
    SAMPLE_PRT("layer%d-chn%d u32Width = %d, u32Height = %d\n",
               u32LayerID, u32ChnID,
               stChnAttr.stRect.u32Width, stChnAttr.stRect.u32Height);

    stFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
    stFrame.u32Width = stChnAttr.stRect.u32Width;
    stFrame.u32Height = stChnAttr.stRect.u32Height;
    stFrame.u32PicStride[0] = stChnAttr.stRect.u32Width;
    u32FrameSize = stFrame.u32PicStride[0] * stFrame.u32Height * 3 / 2;

    s32Ret = SAMPLE_VO_CREATE_POOL(3, u32FrameSize, 512, &pstChnThreadParam->u32UserPoolId);
    if (s32Ret) {
        SAMPLE_PRT("layer%d-chn%d SAMPLE_VO_CREATE_POOL failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
        goto exit;
    }

    while (!pstChnThreadParam->u32ThreadForceStop) {
        BlkId = AX_POOL_GetBlock(pstChnThreadParam->u32UserPoolId, u32FrameSize, NULL);
        if (AX_INVALID_BLOCKID == BlkId) {
            SAMPLE_PRT("layer%d-chn%d AX_POOL_GetBlock failed\n", u32LayerID, u32ChnID);
            usleep(10000);
            continue;
        }
        stFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.u64VirAddr[0] = 0;

        u64PhysAddr = AX_POOL_Handle2PhysAddr(BlkId);

        if (!u64PhysAddr) {
            SAMPLE_PRT("AX_POOL_Handle2PhysAddr failed,BlkId=0x%x\n", BlkId);
            break;
        }

        u64VirAddr = AX_SYS_Mmap(u64PhysAddr, u32FrameSize);

        if (!u64VirAddr) {
            SAMPLE_PRT("AX_SYS_Mmap failed\n");
            break;
        }

        SAMPLE_Fill_Color(stFrame.enImgFormat,
                          stFrame.u32Width,
                          stFrame.u32Height,
                          stFrame.u32PicStride[0],
                          (AX_U8 *)u64VirAddr);

        s32Ret = AX_SYS_Munmap(u64VirAddr, u32FrameSize);

        if (s32Ret) {
            SAMPLE_PRT("AX_SYS_Munmap failed,s32Ret=0x%x\n", s32Ret);
            break;
        }

        stFrame.u32BlkId[0] = BlkId;
        stFrame.u32BlkId[1] = AX_INVALID_BLOCKID;
        //SAMPLE_PRT("layer%d-chn%d start send frame, BlkId = 0x%x\n", u32LayerID, u32ChnID, BlkId);
        s32Ret = AX_VO_SendFrame(u32LayerID, u32ChnID, &stFrame, 0);
        if (s32Ret) {
            SAMPLE_PRT("layer%d-chn%d AX_VO_SendFrame failed, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);
            AX_POOL_ReleaseBlock(BlkId);
            usleep(16666);
            continue;
        }

        AX_POOL_ReleaseBlock(BlkId);

        usleep(16666);

    }

exit:
    SAMPLE_PRT("layer%d-chn%d exit, s32Ret = 0x%x\n", u32LayerID, u32ChnID, s32Ret);

    return NULL;
}

SAMPLE_VO_CHN_THREAD_PARAM_S g_stChnThreadParam = {0};

AX_S32 VoInit(AX_U32 u32CamChn, SAMPLE_PARAM_S *pstInParam, SAMPLE_VO_CONFIG_S *pstVoConf)
{
    AX_S32 i, s32Ret;
    AX_U64 u64BlkSize = 0;
    SAMPLE_VO_LAYER_CONFIG_S *pstVoLayer;
    VO_VIDEO_LAYER_ATTR_S *pstVoLayerAttr;
    SAMPLE_VO_CHN_THREAD_PARAM_S *pstChnThreadParam = &g_stChnThreadParam;

    s32Ret = AX_VO_Init();
    if (s32Ret) {
        SAMPLE_PRT("AX_VO_Init failed, s32Ret = 0x%x\n", s32Ret);
        goto exit0;
    }

    SAMPLE_PRT("u32LayerNr = %d\n", pstVoConf->u32LayerNr);

    for (i = 0; i < pstVoConf->u32LayerNr; i++) {
        pstVoLayer = &pstVoConf->stVoLayer[i];
        pstVoLayerAttr = &pstVoLayer->stVoLayerAttr;
        if (i == 0) {
            pstVoLayerAttr->stImageSize.u32Width = (pstInParam->u32PicWidth + 0xF) & (~0xF);;
            pstVoLayerAttr->stImageSize.u32Height = pstInParam->u32PicHeight;;
        } else {
            pstVoLayer->enVoMode = VO_MODE_1MUX;
            pstVoLayerAttr->stImageSize.u32Width = (pstVoConf->stReso.u32Width + 0xF) & (~0xF);
            pstVoLayerAttr->stImageSize.u32Height = pstVoConf->stReso.u32Height;
            pstVoLayerAttr->stDispRect.u32X = pstVoConf->stReso.u32Width;
            pstVoLayerAttr->u32ChnNr = 1;
        }

        pstVoLayerAttr->stDispRect.u32Width = pstVoLayerAttr->stImageSize.u32Width;
        pstVoLayerAttr->stDispRect.u32Height = pstVoLayerAttr->stImageSize.u32Height;

        SAMPLE_PRT("layer%d u32Width = %d, u32Height = %d\n", i, pstVoLayerAttr->stImageSize.u32Width,
                   pstVoLayerAttr->stImageSize.u32Height);

        pstVoLayerAttr->u32PoolId = ~0;
        u64BlkSize = pstVoLayerAttr->stImageSize.u32Width * pstVoLayerAttr->stImageSize.u32Height * 3 / 2;
        s32Ret = SAMPLE_VO_CREATE_POOL(3, u64BlkSize, 512, &pstVoLayerAttr->u32PoolId);
        if (s32Ret) {
            SAMPLE_PRT("SAMPLE_VO_CREATE_POOL failed, s32Ret = 0x%x\n", s32Ret);
            goto exit1;
        }

        SAMPLE_PRT("layer pool id = 0x%x\n", pstVoLayerAttr->u32PoolId);
    }

    s32Ret = SAMPLE_COMM_VO_StartVO(pstVoConf);
    if (s32Ret) {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartVO failed, s32Ret = 0x%x\n", s32Ret);
        goto exit1;
    }

    for (i = 0; i < pstVoConf->u32LayerNr; i++) {
        pstVoLayer = &pstVoConf->stVoLayer[i];
        if (i == 0) {
            pstVoLayer->stSrcMod.enModId = AX_ID_VIN;
            pstVoLayer->stSrcMod.s32GrpId = 0;
            pstVoLayer->stSrcMod.s32ChnId = u32CamChn;

            pstVoLayer->stDstMod.enModId = AX_ID_VO;
            pstVoLayer->stDstMod.s32GrpId = 0;
            pstVoLayer->stDstMod.s32ChnId = 0;
            AX_SYS_Link(&pstVoLayer->stSrcMod, &pstVoLayer->stDstMod);
        } else {
            pstChnThreadParam->u32LayerID = pstVoLayer->VoLayer;
            pstChnThreadParam->u32ChnID = 0;
            pstChnThreadParam->u32ThreadForceStop = 0;
            pstChnThreadParam->u32UserPoolId = ~0;
            pthread_create(&pstChnThreadParam->ThreadID, NULL, SAMPLE_VO_CHN_THREAD, pstChnThreadParam);
        }
    }

    SAMPLE_PRT("done\n");

    return 0;

exit1:
    for (i = 0; i < pstVoConf->u32LayerNr; i++) {
        pstVoLayer = &pstVoConf->stVoLayer[i];
        pstVoLayerAttr = &pstVoLayer->stVoLayerAttr;
        if (pstVoLayerAttr->u32PoolId != ~0)
            SAMPLE_VO_POOL_DESTROY(pstVoLayerAttr->u32PoolId);
    }

    AX_VO_Deinit();

exit0:
    AX_SYS_Deinit();

    SAMPLE_PRT("done, s32Ret = 0x%x\n", s32Ret);

    return s32Ret;
}

AX_VOID VoDeInit(SAMPLE_VO_CONFIG_S *pstVoConf)
{
    AX_S32 i;
    SAMPLE_VO_LAYER_CONFIG_S *pstVoLayer;
    VO_VIDEO_LAYER_ATTR_S *pstVoLayerAttr;
    SAMPLE_VO_CHN_THREAD_PARAM_S *pstChnThreadParam = &g_stChnThreadParam;

    for (i = 0; i < pstVoConf->u32LayerNr; i++) {
        pstVoLayer = &pstVoConf->stVoLayer[i];
        if (i == 0) {
            AX_SYS_UnLink(&pstVoLayer->stSrcMod, &pstVoLayer->stDstMod);
        } else {
            if (pstChnThreadParam->ThreadID) {
                pstChnThreadParam->u32ThreadForceStop = 1;
                pthread_join(pstChnThreadParam->ThreadID, NULL);
            }
        }
    }

    SAMPLE_COMM_VO_StopVO(pstVoConf);

    AX_VO_Deinit();

    for (i = 0; i < pstVoConf->u32LayerNr; i++) {
        pstVoLayer = &pstVoConf->stVoLayer[i];
        pstVoLayerAttr = &pstVoLayer->stVoLayerAttr;
        if (i != 0) {
            if (pstChnThreadParam->u32UserPoolId != ~0)
                SAMPLE_VO_POOL_DESTROY(pstChnThreadParam->u32UserPoolId);
        }

        if (pstVoLayerAttr->u32PoolId != ~0)
            SAMPLE_VO_POOL_DESTROY(pstVoLayerAttr->u32PoolId);
    }
}
