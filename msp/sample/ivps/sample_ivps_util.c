/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "sample_ivps_util.h"

#define IVPS_BUF_POOL_MEM_SIZE (0x100000 * 40) // 40M

typedef struct
{
    AX_BOOL bActive;
    AX_POOL PoolId;
} BUF_POOL_S;

static BUF_POOL_S tBufPool[100];
static AX_U32 BufPoolCnt = 0;

AX_S32 IVPS_PoolFloorInit(BLK_INFO_S *pBlkInfo, AX_U32 nNum)
{
    AX_S32 ret, i;
    AX_POOL_FLOORPLAN_T PoolFloorPlan;

    if (nNum > 16)
    {
        printf("Pool Floor num should be smaller than 16\n");
        return -1;
    }

    ret = AX_POOL_Exit();
    if (ret)
    {
        printf("AX_POOL_Exit fail!!Error Code:0x%X\n", ret);
        return -1;
    }
    memset(&PoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));

    for (i = 0; i < nNum; i++)
    {
        PoolFloorPlan.CommPool[i].MetaSize = 512;
        PoolFloorPlan.CommPool[i].BlkSize = pBlkInfo->nSize;
        PoolFloorPlan.CommPool[i].BlkCnt = pBlkInfo->nCnt;
        PoolFloorPlan.CommPool[i].CacheMode = POOL_CACHE_MODE_NONCACHE;

        memset(PoolFloorPlan.CommPool[i].PartitionName, 0, sizeof(PoolFloorPlan.CommPool[i].PartitionName));
        strcpy((char *)PoolFloorPlan.CommPool[i].PartitionName, "anonymous");
        pBlkInfo++;
    }

    ret = AX_POOL_SetConfig(&PoolFloorPlan);
    if (ret)
    {
        printf("AX_POOL_SetConfig fail!Error Code:0x%X\n", ret);
        return -1;
    }
    else
    {
        printf("AX_POOL_SetConfig success!\n");
    }

    ret = AX_POOL_Init();
    if (ret)
    {
        printf("AX_POOL_Init fail!!Error Code:0x%X\n", ret);
        return -1;
    }
    else
    {
        printf("AX_POOL_Init success!\n");
    }
    return 0;
}

static AX_POOL BufPoolCreate(AX_U32 nBlkSize, AX_U32 nBlkCnt)
{
    AX_POOL PoolId;
    AX_POOL_CONFIG_T PoolConfig;

    // create user_pool_0 :blocksize=1000,metasize=512,block count =2 ,noncache type
    memset(&PoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    PoolConfig.MetaSize = 512;
    PoolConfig.BlkSize = nBlkSize;
    PoolConfig.BlkCnt = nBlkCnt;
    PoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(PoolConfig.PartitionName, 0, sizeof(PoolConfig.PartitionName));
    strcpy((char *)PoolConfig.PartitionName, "anonymous");

    PoolId = AX_POOL_CreatePool(&PoolConfig);

    if (AX_INVALID_POOLID == PoolId)
    {
        printf("AX_POOL_CreatePool error!!!\n");
        return AX_INVALID_POOLID;
    }
    else
    {
        printf("AX_POOL_CreatePool[%d] success\n", PoolId);
    }
    return PoolId;
}

AX_S32 BufCreate(AX_POOL *PoolId, AX_U32 nBlkSize, AX_U32 nBlkCnt)
{
    int ret;

    if ((*PoolId = BufPoolCreate(nBlkSize, nBlkCnt)) == AX_INVALID_POOLID)
    {
        goto ERROR;
    }
    printf("PoolId:%x\n", *PoolId);
    tBufPool[BufPoolCnt].PoolId = *PoolId;
    tBufPool[BufPoolCnt].bActive = AX_TRUE;
    BufPoolCnt++;

    return 0;

ERROR:
    printf("\nsample_pool test fail!\n");
    ret = AX_POOL_Exit();
    if (ret)
    {
        printf("AX_POOL_Exit fail!!Error Code:0x%X\n", ret);
        return -1;
    }
    printf("AX_POOL_Exit success!\n");
    return -1;
}

AX_S32 BufDestroy(AX_POOL PoolId)
{
    int ret;

    ret = AX_POOL_MarkDestroyPool(PoolId);
    if (ret)
    {
        printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n", ret);
        return -1;
    }
    printf("AX_POOL_MarkDestroyPool success pool id:%d!\n", PoolId);

    return 0;
}

AX_S32 BufDestroyAll(AX_VOID)
{
    int i, ret;
    for (i = BufPoolCnt - 1; i >= 0; i--)
    {

        ret = AX_POOL_MarkDestroyPool(tBufPool[i].PoolId);
        if (ret)
        {
            printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n", ret);
            return -1;
        }
        tBufPool[BufPoolCnt].bActive = AX_FALSE;
        printf("AX_POOL_MarkDestroyPool success pool id:%d!\n", tBufPool[i].PoolId);
    }
    return 0;
}

AX_S32 BufPoolBlockAddrGet(AX_POOL PoolId, AX_U32 BlkSize, AX_U64 *nPhyAddr, AX_VOID **pVirAddr, AX_BLK *BlkId)
{

    *BlkId = AX_POOL_GetBlock(PoolId, BlkSize, NULL);

    *nPhyAddr = AX_POOL_Handle2PhysAddr(*BlkId);

    if (!(*nPhyAddr))
    {
        printf("AX_POOL_Handle2PhysAddr fail!\n");
        return -1;
    }

    printf("AX_POOL_Handle2PhysAddr success!(Blockid:0x%X --> PhyAddr=0x%llx)\n", *BlkId, *nPhyAddr);

    *pVirAddr = AX_POOL_GetBlockVirAddr(*BlkId);

    if (!(*pVirAddr))
    {
        printf("AX_POOL_GetBlockVirAddr fail!\n");
        return -1;
    }

    printf("AX_POOL_GetBlockVirAddr success!blockVirAddr=0x%p\n", *pVirAddr);
    return 0;
}

AX_U64 GetTickCount(AX_VOID)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

AX_BOOL Split(char *pSrc, const char *pDelim, char **ppDst, AX_S32 nDstCnt, AX_S32 *pNum)
{

    if (!pSrc || 0 == strlen(pSrc) || 0 == nDstCnt || !pDelim || 0 == strlen(pDelim))
    {
        printf("%s:Invalid para\n", __func__);
        return AX_FALSE;
    }

    AX_S32 nCount = 0;
    char *pToken = strtok(pSrc, pDelim);
    while (NULL != pToken)
    {
        if (++nCount > nDstCnt)
        {
            printf("nCount overflow\n");
            return AX_FALSE;
        }

        *ppDst++ = pToken;
        pToken = strtok(NULL, pDelim);
    }
    if (pNum)
    {
        *pNum = nCount;
    }

    return AX_TRUE;
}

AX_U32 CalcImgSize(AX_U32 nStride, AX_U32 nW, AX_U32 nH, AX_IMG_FORMAT_E eType, AX_U32 nAlign)
{
    AX_U32 nBpp = 0;
    if (nW == 0 || nH == 0)
    {
        printf("%s: Invalid width %d or height %d!\n", __func__, nW, nH);
        return 0;
    }

    if (0 == nStride)
    {
        nStride = (0 == nAlign) ? nW : ALIGN_UP(nW, nAlign);
    }
    else
    {
        if (nAlign > 0)
        {
            if (nStride % nAlign)
            {
                printf("%s: stride: %u not %u aligned.!\n", __func__, nStride, nAlign);
                return 0;
            }
        }
    }

    switch (eType)
    {
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
    case AX_FORMAT_BGR888:
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

/* Load image file and alloc memory */
AX_BOOL LoadImage(const char *pszImge, AX_U32 pImgSize, AX_U64 *pPhyAddr, AX_VOID **ppVirAddr)
{
    AX_S32 ret;
    FILE *fp = fopen(pszImge, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        AX_U32 nFileSize = ftell(fp);
        if (pImgSize > 0 && pImgSize != nFileSize)
        {
            printf("%s: file size not right, %d != %d\n", __func__, pImgSize, nFileSize);
            fclose(fp);
            return AX_FALSE;
        }
        fseek(fp, 0, SEEK_SET);
        if (!nFileSize)
        {
            printf("%s nFileSize is 0 !!\n", pszImge);
            return AX_FALSE;
        }
        ret = AX_SYS_MemAlloc((AX_U64 *)pPhyAddr, ppVirAddr, nFileSize, SAMPLE_PHY_MEM_ALIGN_SIZE, NULL);
        if (0 != ret)
        {
            printf("%s AX_SYS_MemAlloc fail, ret=0x%x\n", __func__, ret);
            fclose(fp);
            return AX_FALSE;
        }
        if (fread(*ppVirAddr, 1, nFileSize, fp) != nFileSize)
        {
            printf("%s fread fail, %s\n", __func__, strerror(errno));
            fclose(fp);
            return AX_FALSE;
        }
        fclose(fp);

        return AX_TRUE;
    }
    else
    {
        printf("%s fopen %s fail, %s\n", __func__, pszImge, strerror(errno));
        return AX_FALSE;
    }
}

AX_BOOL LoadImageExt(AX_POOL PoolId, const char *pszImge, AX_U32 pImgSize, AX_BLK *BlkId, AX_BOOL isYUVSplit, AX_U64 *nPhyAddr, AX_VOID **pVirAddr)
{
    AX_S32 ret;
    FILE *fp = fopen(pszImge, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        AX_U32 nFileSize = ftell(fp);
        if (pImgSize > 0 && pImgSize != nFileSize)
        {
            printf("%s: file size not right, %d != %d\n", __func__, pImgSize, nFileSize);
            fclose(fp);
            return AX_FALSE;
        }
        fseek(fp, 0, SEEK_SET);

        if (isYUVSplit)
        {
            nFileSize = nFileSize / 3;
            ret = BufPoolBlockAddrGet(PoolId, nFileSize * 2, nPhyAddr, pVirAddr, BlkId);
            if (0 != ret)
            {
                printf("%s:%d BufPoolBlockAddrGet fail, ret=0x%x\n", __func__, __LINE__, ret);
                fclose(fp);
                return AX_FALSE;
            }
            if (fread(*pVirAddr, 1, nFileSize * 2, fp) != nFileSize * 2)
            {
                printf("%s fread fail, %s\n", __func__, strerror(errno));
                fclose(fp);
                return AX_FALSE;
            }

            fseek(fp, nFileSize * 2, SEEK_SET);
            nPhyAddr++;
            pVirAddr += sizeof(pVirAddr) / 2;
            BlkId++;
            ret = BufPoolBlockAddrGet(PoolId, nFileSize, nPhyAddr, pVirAddr, BlkId);
            if (0 != ret)
            {
                printf("%s:%d BufPoolBlockAddrGet fail, ret=0x%x\n", __func__, __LINE__, ret);
                fclose(fp);
                return AX_FALSE;
            }
            if (fread(*pVirAddr, 1, nFileSize, fp) != nFileSize)
            {
                printf("%s fread fail, %s\n", __func__, strerror(errno));
                fclose(fp);
                return AX_FALSE;
            }
        }
        else
        {
            ret = BufPoolBlockAddrGet(PoolId, nFileSize, nPhyAddr, pVirAddr, BlkId);
            if (0 != ret)
            {
                printf("%s:%d BufPoolBlockAddrGet UV fail, ret=0x%x\n", __func__, __LINE__, ret);
                fclose(fp);
                return AX_FALSE;
            }
            if (fread(*pVirAddr, 1, nFileSize, fp) != nFileSize)
            {
                printf("%s fread fail, %s\n", __func__, strerror(errno));
                fclose(fp);
                return AX_FALSE;
            }
        }
        fclose(fp);

        return AX_TRUE;
    }
    else
    {
        printf("%s fopen %s fail, %s\n", __func__, pszImge, strerror(errno));
        return AX_FALSE;
    }
}

#define MESH_DATA_READ_LENDTH 14
AX_BOOL LoadPureHex(AX_POOL PoolId, const char *pszImge, AX_U32 pImgSize, AX_BLK *BlkId, AX_U64 *nPhyAddr, AX_VOID **pVirAddr)
{
    AX_S32 ret;
    char *p, buf[20];
    AX_U64 value;
    AX_U64 *pData;
    AX_U32 line = 1;
    FILE *fp = fopen(pszImge, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        AX_U32 nFileSize = ftell(fp);
        if (pImgSize > 0 && pImgSize != nFileSize)
        {
            printf("%s: file size not right, %d != %d\n", __func__, pImgSize, nFileSize);
            fclose(fp);
            return AX_FALSE;
        }
        fseek(fp, 0, SEEK_SET);

        ret = BufPoolBlockAddrGet(PoolId, nFileSize, nPhyAddr, pVirAddr, BlkId);
        if (0 != ret)
        {
            printf("%s:%d BufPoolBlockAddrGet UV fail, ret=0x%x\n", __func__, __LINE__, ret);
            fclose(fp);
            return AX_FALSE;
        }

        pData = *pVirAddr;

        while (1)
        {
            if (!(p = fgets(buf, MESH_DATA_READ_LENDTH, fp)))
            {
                break;
            }
            sscanf(p, "%llx", &value);
            *pData = value;
            pData++;
#if 0
			if(line%33==0 && line){
				*pData = 0;
				pData++;
				*pData = 0;
				pData++;
				*pData = 0;
				pData++;
			}
#endif
            line++;
        }
        fclose(fp);

        return AX_TRUE;
    }
    else
    {
        printf("%s fopen %s fail, %s\n", __func__, pszImge, strerror(errno));
        return AX_FALSE;
    }
}

/* Save image file and  free memory */
AX_BOOL SaveImage(const char *pszImge, AX_U32 pBufSize, AX_U64 pPhyAddr, AX_VOID *ppVirAddr)
{
    FILE *fp = fopen(pszImge, "wb");
    if (fp)
    {

        fseek(fp, 0, SEEK_SET);

        if (fwrite(ppVirAddr, 1, pBufSize, fp) != pBufSize)
        {
            printf("%s fwrite fail, %s\n", __func__, strerror(errno));
            fclose(fp);
            return AX_FALSE;
        }
        fclose(fp);

        AX_SYS_MemFree(pPhyAddr, ppVirAddr);

        return AX_TRUE;
    }
    else
    {
        printf("%s fopen %s fail, %s\n", __func__, pszImge, strerror(errno));
        return AX_FALSE;
    }
}

void Swap(int *a, int *b)
{
    int temp;
    temp = *a;
    *a = *b;
    *b = temp;
}

static timer_t gtimed = 0;
AX_BOOL StartFpsTimer(AX_U32 microseconds, pTimeOutHandler pfn)
{
    struct sigevent evp = {0};
    struct sigaction sigact = {0};
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = (SA_SIGINFO | SA_RESTART);
    sigact.sa_sigaction = pfn;
    sigaction(SIGUSR2, &sigact, NULL);

    evp.sigev_value.sival_ptr = NULL;
    evp.sigev_notify = SIGEV_SIGNAL;
    evp.sigev_signo = SIGUSR2;

    if (timer_create(CLOCK_MONOTONIC, &evp, &gtimed) == -1)
    {
        printf("timer_create() fail, error: %s\n", strerror(errno));
        return AX_FALSE;
    }

    struct itimerspec it;
    it.it_interval.tv_sec = (microseconds / 1000000);
    it.it_interval.tv_nsec = (microseconds % 1000000) * 1000;
    it.it_value = it.it_interval;
    if (timer_settime(gtimed, 0, &it, NULL) == -1)
    {
        printf("timer_settime() fail, error: %s\n", strerror(errno));
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_VOID KillFpsTimer(AX_VOID)
{
    if (0 != gtimed)
    {
        timer_delete(gtimed);
        gtimed = 0;
    }
}

AX_VOID usDelay(AX_U32 nUs)
{
    int i;
    volatile int j;
    for (i = 0; i < nUs; i++)
    {
        for (j = 0; j < 10; j++)
            ;
    }
}

char *ExtractFilePath(char *pFile)
{
    int len, i;
    len = strlen(pFile);
    for (i = len - 1; i >= 0; i--)
    {
        if (pFile[i] == '/')
        {
            pFile[i] = '\0';
            break;
        }
    }
    printf(">>>Path= %s", pFile);
    return pFile;
}

AX_VOID SaveFile(AX_VIDEO_FRAME_INFO_S *tDstFrame, AX_S32 nGrpIdx, AX_S32 nChnIdx, char *pFilePath)
{
   SaveFileExt(&tDstFrame->stVFrame, nGrpIdx, nChnIdx, pFilePath, "VIDEO_FRAME_INFO");
}

AX_VOID SaveFileExt(AX_VIDEO_FRAME_S *tDstFrame, AX_S32 nGrpIdx, AX_S32 nChnIdx, char *pFilePath, char *pFileName)
{
    AX_U32 nPixelSize;
    AX_S32 s32Ret1 = 0;
    AX_S32 s32Ret2 = 0;
    AX_S32 s32Ret3 = 0;
    AX_U8 nStoragePlanarNum = 0;
    char szOutImgFile[128] = {0};

    nPixelSize = (AX_U32)tDstFrame->u32PicStride[0] * tDstFrame->u32Height;

    switch (tDstFrame->enImgFormat)
    {
    case AX_YUV420_SEMIPLANAR_VU:
    case AX_YUV420_PLANAR:
    case AX_YUV420_SEMIPLANAR:
        nStoragePlanarNum = 2;
        break;
    case AX_YUV422_INTERLEAVED_YUYV:
    case AX_YUV422_INTERLEAVED_UYVY:
        nStoragePlanarNum = 3;
        break;
    case AX_YUV444_PACKED:
    case AX_FORMAT_RGB888:
        nStoragePlanarNum = 1;
        break;
    case AX_FORMAT_RGBA8888:
        nStoragePlanarNum = 1;
        break;
    default:
        nStoragePlanarNum = 1;
        break;
    }

    printf("SaveFileExt nPixelSize: %d PhyAddr:%llx PhyAddr_UV:%llx\n", nPixelSize, tDstFrame->u64PhyAddr[0], tDstFrame->u64PhyAddr[1]);
    printf("PhyAddr:0x%x PhyAddr_UV:0x%x\n", (AX_U32)tDstFrame->u64PhyAddr[0], (AX_U32)tDstFrame->u64PhyAddr[1]);
    printf("VirAddr:0x%x VirAddr_UV:0x%x\n", (AX_U32)tDstFrame->u64VirAddr[0], (AX_U32)tDstFrame->u64VirAddr[1]);

    sprintf(szOutImgFile, "%s/%soutput_grp%d_chn%d_%dx%d.fmt_%x", pFilePath, pFileName, nGrpIdx, nChnIdx, tDstFrame->u32PicStride[0], tDstFrame->u32Height, tDstFrame->enImgFormat);
    FILE *fp = fopen(szOutImgFile, "wb");

    if (fp)
    {
        switch (nStoragePlanarNum)
        {
            case 2:
                tDstFrame->u64VirAddr[0] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[0], nPixelSize);
                tDstFrame->u64VirAddr[1] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[1], nPixelSize / 2);
                fwrite((AX_VOID *)((AX_ULONG)tDstFrame->u64VirAddr[0]), 1, nPixelSize, fp);
                fwrite((AX_VOID *)((AX_ULONG)tDstFrame->u64VirAddr[1]), 1, nPixelSize / 2, fp);
                s32Ret1 = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[0], nPixelSize);
                s32Ret1 = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[1], nPixelSize / 2);
                break;
            case 3:
                tDstFrame->u64VirAddr[0] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[0], nPixelSize);
                tDstFrame->u64VirAddr[1] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[1], nPixelSize / 2);
                tDstFrame->u64VirAddr[2] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[2], nPixelSize / 2);
                fwrite((AX_VOID *)((AX_ULONG)tDstFrame->u64VirAddr[0]), 1, nPixelSize, fp);
                fwrite((AX_VOID *)((AX_ULONG)tDstFrame->u64VirAddr[1]), 1, nPixelSize / 2, fp);
                fwrite((AX_VOID *)((AX_ULONG)tDstFrame->u64VirAddr[2]), 1, nPixelSize / 2, fp);
                s32Ret1 = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[0], nPixelSize);
                s32Ret2 = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[1], nPixelSize / 2);
                s32Ret3 = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[2], nPixelSize / 2);
                break;
            default:
                tDstFrame->u64VirAddr[0] = (AX_ULONG)AX_SYS_Mmap(tDstFrame->u64PhyAddr[0], nPixelSize * 3);
                fwrite((AX_VOID *)((AX_ULONG)tDstFrame->u64VirAddr[0]), 1, nPixelSize * 3, fp);
                s32Ret1 = AX_SYS_Munmap((AX_VOID *)(AX_ULONG)tDstFrame->u64VirAddr[0], nPixelSize * 3);
                break;
        }
        fclose(fp);
        printf("%s is saved!\n", szOutImgFile);
        if (s32Ret1 || s32Ret2 || s32Ret3)
        {
            printf("AX_SYS_Munmap failed,s32Ret1=0x%x ,s32Ret2=0x%x ,s32Ret2=0x%x\n", s32Ret1, s32Ret2, s32Ret3);
        }
    }


}