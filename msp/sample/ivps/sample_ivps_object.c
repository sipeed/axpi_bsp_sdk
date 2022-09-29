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
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>

#include "sample_ivps_util.h"
#include "sample_ivps_object.h"

#define __DYNAMIC_SWITCH_CHANNEL_TEST__

#define DEFAULT_DELIM "@"
#define EPOLL_MAXUSERS 128

static volatile AX_BOOL bThreadLoopExit;

AX_IVPS_TDP_CFG_S gSampleTdpCfg0 = {
    .bMirror = AX_FALSE,
    .bFlip = AX_FALSE,
    .bCrop = AX_FALSE,
    .tCropRect = {
        .nX = 0,
        .nY = 0,
        .nW = 0,
        .nH = 0,
    },
    .eRotation = AX_IVPS_ROTATION_180, // IVPS_ROTATION_90
    //.tAspectRatio = {.eMode = IVPS_ASPECT_RATIO_MANUAL, .tRect = {64, 20, 500, 500}},
    .tAspectRatio = {.eMode = AX_IVPS_ASPECT_RATIO_STRETCH, .eAligns = {AX_IVPS_ASPECT_RATIO_HORIZONTAL_RIGHT, AX_IVPS_ASPECT_RATIO_VERTICAL_TOP}},
    //.eCompressMode = AX_COMPRESS_MODE_TILE,

};

AX_IVPS_TDP_CFG_S gSampleTdpCfg1 = {
    .bMirror = AX_FALSE,
    .bFlip = AX_FALSE,
    .bCrop = AX_FALSE,
    .tCropRect = {
        .nX = 0,
        .nY = 0,
        .nW = 0,
        .nH = 0,
    },
    .eRotation = AX_IVPS_ROTATION_180,
    .tAspectRatio = {.eMode = AX_IVPS_ASPECT_RATIO_STRETCH},
    //.eCompressMode = AX_COMPRESS_MODE_TILE,
};

AX_IVPS_TDP_CFG_S gSampleTdpCfg2 = {
    .bMirror = AX_FALSE,
    .bFlip = AX_TRUE,
    .bCrop = AX_FALSE,
    .tCropRect = {
        .nX = 0,
        .nY = 0,
        .nW = 0,
        .nH = 0,
    },
    .eRotation = AX_IVPS_ROTATION_90,
    .tAspectRatio = {.eMode = AX_IVPS_ASPECT_RATIO_STRETCH},
};

SAMPLE_IVPS_DEWARP_CFG_S gSampleGdcCfg = {
    .pMeshFile = "/opt/data/ivps/mesh/4k_1080p_r0.bin",
    .tGdcCfg = {
        .nInterStartX = 0,
        .nInterStartY = 36,
        .nMeshStartX = 4,
        .nMeshStartY = 7,
        .nMeshWidth = 79,
        .nMeshHeight = 63,
        .eRotation = AX_IVPS_ROTATION_0,
		.bCrop = AX_FALSE,
    	.tCropRect = {
        .nX = 0,
        .nY = 0,
        .nW = 1920,
        .nH = 1080,
    },
    },
};

/*2688x1520=>2688x1520*/
SAMPLE_IVPS_DEWARP_CFG_S gSampleGdcCfg_4M_4M = {
    .pMeshFile = "/opt/data/ivps/mesh/4m_r0.bin",
    .tGdcCfg = {
        .bDewarpEnable = AX_TRUE,
        .nInterStartX = 0,
        .nInterStartY = 0,
        .nMeshStartX = 0,
        .nMeshStartY = 0,
        .nMeshWidth = 127,
        .nMeshHeight = 127,
    },
};

/*3840x2160 rotate 90*/
SAMPLE_IVPS_DEWARP_CFG_S gSampleGdcCfg_4K_R90 = {
    .pMeshFile = "/opt/data/ivps/mesh/4k_r90.bin",
    .tGdcCfg = {
        .bDewarpEnable = AX_TRUE,
        .nInterStartX = 72,
        .nInterStartY = 96,
        .nMeshStartX = 4,
        .nMeshStartY = 2,
        .nMeshWidth = 95,
        .nMeshHeight = 143,
    },
};

/*3840x2160 rotate 45*/
SAMPLE_IVPS_DEWARP_CFG_S gSampleGdcCfg_4K_R45 = {
    .pMeshFile = "/opt/data/ivps/mesh/4k_r45.bin",
    .tGdcCfg = {
        .bDewarpEnable = AX_TRUE,
        .nInterStartX = 96,
        .nInterStartY = 72,
        .nMeshStartX = 2,
        .nMeshStartY = 4,
        .nMeshWidth = 143,
        .nMeshHeight = 95,
    },
};

AX_IVPS_VO_CFG_S gSampleVoCfg = {
    .bCrop = AX_FALSE,
    .tCropRect = {
        .nX = 100,
        .nY = 200,
        .nW = 1800,
        .nH = 800,
    },
};

SAMPLE_IVPS_GRP_S gSampleIvpsGrp = {
    .nIvpsGrp = SAMPLE_IVPS_GRP1,

    .tGrpAttr = {
        .ePipeline = AX_IVPS_PIPELINE_DEFAULT,
    },
    .tPipelineAttr = {
        .tFbInfo = {
            .PoolId = AX_INVALID_POOLID,
        },
        .nOutChnNum = 3,
        .nOutFifoDepth = {2, 2, 2},
        .tFilter = {
            {
                /*group filter0*/
                {
                    .bEnable = AX_FALSE,
                    .eEngine = AX_IVPS_ENGINE_TDP,
                    .nDstPicWidth = SAMPLE_IVPS_GRP0_CHN1_WIDTH,
                    .nDstPicHeight = SAMPLE_IVPS_GRP0_CHN1_HEIGHT,
                    .nDstPicStride = SAMPLE_IVPS_GRP0_CHN1_STRIDE,
                    .eDstPicFormat = AX_YUV420_SEMIPLANAR,
                },
            },

            {
                /*channel 0 filter0*/
                {
                    .bEnable = AX_TRUE,
                    .eEngine = AX_IVPS_ENGINE_TDP,
                    .nDstPicWidth = SAMPLE_IVPS_GRP0_CHN1_WIDTH,
                    .nDstPicHeight = SAMPLE_IVPS_GRP0_CHN1_HEIGHT,
                    .nDstPicStride = SAMPLE_IVPS_GRP0_CHN1_STRIDE,
                    .eDstPicFormat = AX_YUV420_SEMIPLANAR,
                },
                /*channel 0 filter1*/
                {
                    .bEnable = AX_FALSE,
                    .eEngine = AX_IVPS_ENGINE_VO,
                    .nDstPicWidth = SAMPLE_IVPS_GRP0_CHN1_WIDTH,
                    .nDstPicHeight = SAMPLE_IVPS_GRP0_CHN1_HEIGHT,
                    .nDstPicStride = SAMPLE_IVPS_GRP0_CHN1_STRIDE,
                    .eDstPicFormat = AX_YUV420_SEMIPLANAR,
                },
            },
            {
                /*channel 1 filter0*/
                {
                    .bEnable = AX_TRUE,
                    .eEngine = AX_IVPS_ENGINE_TDP,
                    .nDstPicWidth = 2688,  // SAMPLE_IVPS_GRP0_CHN1_WIDTH
                    .nDstPicHeight = 1520, // SAMPLE_IVPS_GRP0_CHN1_HEIGHT
                    .nDstPicStride = 2688, // SAMPLE_IVPS_GRP0_CHN1_STRIDE
                    .eDstPicFormat = AX_FORMAT_RGB888,
                },
            },
            {
                /*channel 2 filter0*/
                {
                    .bEnable = AX_TRUE,
                    .eEngine = AX_IVPS_ENGINE_TDP,
                    .nDstPicWidth = SAMPLE_IVPS_GRP0_CHN2_HEIGHT,
                    .nDstPicHeight = SAMPLE_IVPS_GRP0_CHN2_WIDTH,
                    .nDstPicStride = SAMPLE_IVPS_GRP0_CHN2_HEIGHT,
                    .eDstPicFormat = AX_FORMAT_RGB888,
                },
            },
        },
    },
    .nIvpsRepeatNum = 3,
};

SAMPLE_IVPS_GRP_S gSampleIvpsSingle = {
    .nIvpsGrp = SAMPLE_IVPS_GRP0,

    .tGrpAttr = {
        .ePipeline = AX_IVPS_PIPELINE_DEFAULT,
    },
    .tPipelineAttr = {
        .nOutChnNum = 1,
        .tFbInfo = {
            .PoolId = AX_INVALID_POOLID,
        },
        .tFilter = {
            {
                /*group filter0*/
                {
                    .bEnable = AX_FALSE,
                    .eEngine = AX_IVPS_ENGINE_TDP,
                    .nDstPicWidth = SAMPLE_IVPS_GRP0_CHN1_WIDTH,
                    .nDstPicHeight = SAMPLE_IVPS_GRP0_CHN1_HEIGHT,
                    .nDstPicStride = SAMPLE_IVPS_GRP0_CHN1_STRIDE,
                    .eDstPicFormat = AX_YUV420_SEMIPLANAR,
                },
            },
            {
                /*channel 0 filter0*/
                {
                    .bEnable = AX_TRUE,
                    .eEngine = AX_IVPS_ENGINE_TDP,
                    .nDstPicWidth = 1920,  // SAMPLE_IVPS_GRP0_CHN1_HEIGHT,
                    .nDstPicHeight = 1080, // SAMPLE_IVPS_GRP0_CHN1_WIDTH,
                    .nDstPicStride = 1920, // SAMPLE_IVPS_GRP0_CHN1_HEIGHT,
                    .nDstFrameWidth = 1920,
                    .nDstFrameHeight = 1080,
                    .eDstPicFormat = AX_YUV420_SEMIPLANAR,
                },
            },
        },
    },
};

AX_VOID ThreadLoopStateSet(AX_BOOL bValue)
{
    bThreadLoopExit = bValue;
}

AX_BOOL ThreadLoopStateGet(AX_VOID)
{
    return bThreadLoopExit;
}

static AX_S32 DevEfdCreate(AX_U32 nNum)
{
    int efd = epoll_create(nNum);
    if (efd < 0)
    {
        ALOGE("failed to create epoll");
        return -1;
    }

    ALOGI("+++ efd:%d\n", efd);
    return efd;
}
static int DevEfdRelease(AX_U32 nEfd)
{
    ALOGI("--- efd:%d\n", nEfd);

    close(nEfd);
    return 0;
}

static int DevFdRelease(AX_U32 nFd)
{
    ALOGI("--- fd:%d\n", nFd);

    close(nFd);
    return 0;
}

static AX_S32 DevFdListen(AX_S32 nEfd, AX_S32 nFd)
{
    struct epoll_event event;

    event.data.fd = nFd;
    event.events = EPOLLET | EPOLLIN;
    epoll_ctl(nEfd, EPOLL_CTL_ADD, nFd, &event);

    ALOGI("+++ efd:%d fd :%d\n", nEfd, nFd);
    return 0;
}

static AX_S32 DevPolWait(AX_S32 nEfd, AX_U32 nNum, AX_S32 nMs)
{
    int i, events_num = 0;
    struct epoll_event events[nNum];

    events_num = epoll_wait(nEfd, events, nNum, nMs); //-1: block
    ALOGI("poll events_num %d num:%d\n", events_num, nNum);

    for (i = 0; i < events_num; i++)
    {
        if (events[i].data.fd < 0)
        {
            ALOGE("fd invalid errno:%d\n", errno);
            continue;
        }
        else if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
        {
            ALOGE("epoll error\n");
            epoll_ctl(nEfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
            continue;
        }
    }
    return events_num;
}
static AX_VOID *GetFrameThread(AX_VOID *pArg)
{

    AX_S32 i, ret = 0;
    SAMPLE_IVPS_GRP_S *pThis = (SAMPLE_IVPS_GRP_S *)pArg;
    ALOGI("--- %s IVPS Grp: %d, Chn num: %d\n", __func__, pThis->nIvpsGrp, pThis->tPipelineAttr.nOutChnNum);

    for (i = 0; i < pThis->tPipelineAttr.nOutChnNum; i++)
    {
        pThis->arrOutChns[i].bEmpty = AX_TRUE;
    }

    AX_VIDEO_FRAME_INFO_S tDstFrame[pThis->tPipelineAttr.nOutChnNum];
    while (!bThreadLoopExit)
    {
        for (i = 0; i < pThis->tPipelineAttr.nOutChnNum; i++)
        {
            if (!pThis->arrOutChns[i].bEmpty)
            {
                break;
            }
        }
        if (i == pThis->tPipelineAttr.nOutChnNum)
        {
            ALOGI("Enter DevPolWait\n");
            DevPolWait(pThis->nEfd, pThis->tPipelineAttr.nOutChnNum, SAMPLE_IVPS_GETFRAME_TIMEOUT);

            for (i = 0; i < pThis->tPipelineAttr.nOutChnNum; i++)
            { // TODO...
                pThis->arrOutChns[i].bEmpty = AX_FALSE;
            }
        }

        for (i = 0; i < pThis->tPipelineAttr.nOutChnNum; i++)
        {
            if (!pThis->arrOutChns[i].bEmpty)
            {
                ret = AX_IVPS_GetChnFrame(pThis->nIvpsGrp, i, &tDstFrame[i].stVFrame, 0);

                if (ret == AX_ERR_IVPS_BUF_EMPTY)
                {
                    /* reach EOF */
                    ALOGW("warning! CHN[%d] is empty ret:%x\n", i, ret);
                    pThis->arrOutChns[i].bEmpty = AX_TRUE;
                    continue;
                }

                printf("Got a IVPS frame: Chn: %d, %d x %d stride: %d, phy: %llx, userdata: %llx, seq_num: %lld, pts: %llx, blockid: %x\n", i,
                       tDstFrame[i].stVFrame.u32Width, tDstFrame[i].stVFrame.u32Height, tDstFrame[i].stVFrame.u32PicStride[0], tDstFrame[i].stVFrame.u64PhyAddr[0],
                       tDstFrame[i].stVFrame.u64UserData, tDstFrame[i].stVFrame.u64SeqNum, tDstFrame[i].stVFrame.u64PTS, tDstFrame[i].stVFrame.u32BlkId[0]);

                SaveFile(&tDstFrame[i], pThis->nIvpsGrp, i, pThis->pFilePath);

/*
 *
 *
 */
#if 0
                /*release block*/
                if (tDstFrame[i].stVFrame.u32BlkId[0])
                {
                    AX_POOL_DecreaseRefCnt(tDstFrame[i].stVFrame.u32BlkId[0], AX_ID_IVPS);
                    ret = AX_POOL_ReleaseBlock(tDstFrame[i].stVFrame.u32BlkId[0]);
                    if (ret)
                    {
                        ALOGE("IVPS Release BlkId 0 fail, ret=0x%x\n", ret);
                        return (AX_VOID *)-1;
                    }
                }
                if (tDstFrame[i].stVFrame.u32BlkId[1])
                {
                    AX_POOL_DecreaseRefCnt(tDstFrame[i].stVFrame.u32BlkId[1], AX_ID_IVPS);
                    ret = AX_POOL_ReleaseBlock(tDstFrame[i].stVFrame.u32BlkId[1]);
                    if (ret)
                    {
                        ALOGE("IVPS Release BlkId 1 fail, ret=0x%x\n", ret);
                        return (AX_VOID *)-1;
                    }
                }
#else
                ret = AX_IVPS_ReleaseChnFrame(pThis->nIvpsGrp, i, &tDstFrame[i].stVFrame);
                if (ret)
                {
                    ALOGE("AX_IVPS_ReleaseFrame fail, ret=0x%x\n", ret);
                    return (AX_VOID *)-1;
                }
#endif
            }
        }

        ALOGI("AX_IVPS_GetFrame\n");
    }

    ALOGI("%s IVPS END\n", __func__);
    return (AX_VOID *)0;
}

static AX_VOID *SendFrameThread(AX_VOID *pArg)
{
    AX_S32 ret = 0, i = 0;
    SAMPLE_IVPS_GRP_S *pThis = (SAMPLE_IVPS_GRP_S *)pArg;
    AX_S32 nRepeatNum = pThis->nIvpsRepeatNum;
    AX_VIDEO_FRAME_INFO_S tSrcFrame = {0};
    ALOGI("+++ %s IVPS Grp: %d +++\n", __func__, pThis->nIvpsGrp);

    tSrcFrame.stVFrame.enImgFormat = pThis->tInputInfo.eFormat;
    tSrcFrame.stVFrame.u32Width = pThis->tInputInfo.nW;
    tSrcFrame.stVFrame.u32Height = pThis->tInputInfo.nH;
    tSrcFrame.stVFrame.s16OffsetLeft = pThis->tInputInfo.nX;
    tSrcFrame.stVFrame.s16OffsetTop = pThis->tInputInfo.nY;
    tSrcFrame.stVFrame.u32PicStride[0] = pThis->tInputInfo.nStride;
    tSrcFrame.stVFrame.u32PicStride[1] = pThis->tInputInfo.nStride;

    tSrcFrame.stVFrame.u64UserData = 0x515457; // rand();
    tSrcFrame.stVFrame.u64SeqNum = 0;

    pThis->tPipelineAttr.tFbInfo.bInPlace = AX_FALSE;

    tSrcFrame.stVFrame.u32BlkId[0] = pThis->BlkId0;
    tSrcFrame.stVFrame.u32BlkId[1] = 0; // pThis->BlkId1;
    tSrcFrame.stVFrame.u64PhyAddr[0] = pThis->nPhyAddr[0];
    tSrcFrame.stVFrame.u64PhyAddr[1] = pThis->nPhyAddr[1];
    if (pThis->BlkId0)
    {
        AX_POOL_IncreaseRefCnt(pThis->BlkId0, AX_ID_IVPS);
    }
    if (pThis->BlkId1)
    {
        AX_POOL_IncreaseRefCnt(pThis->BlkId1, AX_ID_IVPS);
    }
    while (!bThreadLoopExit && (nRepeatNum == -1 || nRepeatNum-- > 0))
    {
        ALOGI("nRepeatNum:%d\n", nRepeatNum);
        tSrcFrame.stVFrame.u64SeqNum++;
        tSrcFrame.stVFrame.u64PTS = GetTickCount();
        ALOGI("AX_IVPS_SendFrame(nChnNum: %d) seq num:%lld PTS:%lld userdata: %llx +++\n", pThis->tPipelineAttr.nOutChnNum, tSrcFrame.stVFrame.u64SeqNum, tSrcFrame.stVFrame.u64PTS, tSrcFrame.stVFrame.u64UserData);

        ret = AX_IVPS_SendFrame(pThis->nIvpsGrp, &tSrcFrame.stVFrame, SAMPLE_IVPS_SENDFRAME_TIMEOUT);
        sleep(1); // usleep(20000);
        ALOGI("AX_IVPS_SendFrame(Chn: %d) ---, ret: 0x%x\n", pThis->arrOutChns[i].nIvpsChn, ret);
        if (IVPS_SUCC != ret)
        {
            ALOGE("AX_IVPS_SendFrame(Chn %d) failed, ret=0x%x.\n", pThis->arrOutChns[i].nIvpsChn, ret);
            continue;
        }
    }

    ALOGI("%s IVPS Grp: %d ---\n", __func__, pThis->nIvpsGrp);
    return (AX_VOID *)0;
}

AX_S32 IVPS_UpdateChnAttrPara(AX_VOID)
{
    SAMPLE_IVPS_GRP_S *p = &gSampleIvpsGrp;
    p->tPipelineAttr.tFilter[1][0].nDstPicStride = 704;
    p->tPipelineAttr.tFilter[1][0].nDstPicWidth = 704;
    p->tPipelineAttr.tFilter[1][0].nDstPicHeight = 576;
    p->tPipelineAttr.tFilter[1][1].nDstPicStride = 704;
    p->tPipelineAttr.tFilter[1][1].nDstPicWidth = 704;
    p->tPipelineAttr.tFilter[1][1].nDstPicHeight = 576;
    printf("Stride:%d\n", p->tPipelineAttr.tFilter[1][0].nDstPicStride);
    printf("W:%d\n", p->tPipelineAttr.tFilter[1][0].nDstPicWidth);
    printf("H:%d\n", p->tPipelineAttr.tFilter[1][0].nDstPicHeight);

    AX_IVPS_SetPipelineAttr(p->nIvpsGrp, &p->tPipelineAttr);
    return IVPS_SUCC;
}

AX_S32 IVPS_ChnAttrChange(AX_S32 nIvpsGrp, AX_S32 nIvpsChn)
{
    int ret;
    IVPS_UpdateChnAttrPara();
    printf("Dynamic enable Grp:%d Chn:%d\n", nIvpsGrp, nIvpsChn);
    ret = AX_IVPS_DisableChn(nIvpsGrp, nIvpsChn);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_DisableChn(Chn: %d) failed, ret=0x%x.\n", nIvpsChn, ret);
        return ret;
    }
    printf("Dynamic disable Grp:%d Chn:%d\n", nIvpsGrp, nIvpsChn);
    ret = AX_IVPS_EnableChn(nIvpsGrp, nIvpsChn);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_EnableChn(Chn: %d) failed, ret=0x%x.\n", nIvpsChn, ret);
        return ret;
    }
    sleep(1);
    return IVPS_SUCC;
}

/*
 * IVPS_AttrChangeThread()
 * Dynamic switch channel test.
 */
static AX_VOID *IVPS_AttrChangeThread(AX_VOID *pArg)
{

    SAMPLE_IVPS_GRP_S *pThis = (SAMPLE_IVPS_GRP_S *)pArg;
    AX_S32 nRepeatNum = pThis->nIvpsAttrRepeatNum;
    printf("AttrChange nRepeatNum:%d\n", nRepeatNum);

    sleep(10);
    while (!bThreadLoopExit && (nRepeatNum == -1 || nRepeatNum-- > 0))
    {

        IVPS_ChnAttrChange(pThis->nIvpsGrp, 0);
    }

    return (AX_VOID *)0;
}

AX_S32 IVPS_StartGrp(SAMPLE_IVPS_GRP_S *p)
{
    int ret = 0;
    AX_U64 nVirAddr = 0;

    AX_S32 nFd;
    ALOGI("+++ SAMPLE_IVPS_StartGrp\n");

    p->nEfd = DevEfdCreate(EPOLL_MAXUSERS);
    AX_BLK BlkId;
    LoadPureHex(-1, gSampleGdcCfg.pMeshFile, 0, &BlkId, &gSampleGdcCfg.tGdcCfg.nPhyAddr, (AX_VOID **)&nVirAddr);
    LoadPureHex(-1, gSampleGdcCfg_4K_R45.pMeshFile, 0, &BlkId, &gSampleGdcCfg_4K_R45.tGdcCfg.nPhyAddr, (AX_VOID **)&nVirAddr);
    LoadPureHex(-1, gSampleGdcCfg_4K_R90.pMeshFile, 0, &BlkId, &gSampleGdcCfg_4K_R90.tGdcCfg.nPhyAddr, (AX_VOID **)&nVirAddr);

    switch (p->tPipelineAttr.tFilter[1][0].eEngine)
    {
    case AX_IVPS_ENGINE_TDP:
        if (p->nIvpsGrp == 1)
        {
            memcpy(&p->tPipelineAttr.tFilter[1][0].tTdpCfg, &gSampleTdpCfg1, sizeof(AX_IVPS_TDP_CFG_S));
        }
        else
        {
            memcpy(&p->tPipelineAttr.tFilter[1][0].tTdpCfg, &gSampleTdpCfg0, sizeof(AX_IVPS_TDP_CFG_S));
        }
        break;
    case AX_IVPS_ENGINE_GDC:
        memcpy(&p->tPipelineAttr.tFilter[1][0].tGdcCfg, &gSampleGdcCfg.tGdcCfg, sizeof(AX_IVPS_GDC_CFG_S));
        break;
    case AX_IVPS_ENGINE_VO:
        memcpy(&p->tPipelineAttr.tFilter[1][0].tVoCfg, &gSampleVoCfg, sizeof(AX_IVPS_VO_CFG_S));
        break;
    default:
        break;
    }

    printf("Mesh1 nPhyAddr:0x%llx\n", gSampleGdcCfg.tGdcCfg.nPhyAddr);
    printf("Mesh2 nPhyAddr:0x%llx\n", gSampleGdcCfg_4K_R45.tGdcCfg.nPhyAddr);
    printf("Mesh3 nPhyAddr:0x%llx\n", gSampleGdcCfg_4K_R90.tGdcCfg.nPhyAddr);
    if (p->tPipelineAttr.tFilter[2][0].eEngine == AX_IVPS_ENGINE_TDP)
    {
        memcpy(&p->tPipelineAttr.tFilter[2][0].tTdpCfg, &gSampleTdpCfg1, sizeof(AX_IVPS_TDP_CFG_S));
    }
    else
    {
        memcpy(&p->tPipelineAttr.tFilter[2][0].tGdcCfg, &gSampleGdcCfg_4K_R90.tGdcCfg, sizeof(AX_IVPS_GDC_CFG_S));
    }

    memcpy(&p->tPipelineAttr.tFilter[3][0].tTdpCfg, &gSampleTdpCfg2, sizeof(AX_IVPS_TDP_CFG_S));
    ret = AX_IVPS_CreateGrp(p->nIvpsGrp, &p->tGrpAttr);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_CreateGrp(Grp: %d) failed, ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }
    ret = AX_IVPS_SetPipelineAttr(p->nIvpsGrp, &p->tPipelineAttr);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_SetPipelineAttr(Grp: %d) failed, ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }

    for (IVPS_CHN chn = 0; chn < p->tPipelineAttr.nOutChnNum; chn++)
    {

        ALOGI("chn id :%d\n", p->arrOutChns[chn].nIvpsChn);

        ret = AX_IVPS_EnableChn(p->nIvpsGrp, chn);
        if (IVPS_SUCC != ret)
        {
            ALOGE("AX_IVPS_EnableChn(Chn: %d) failed, ret=0x%x.\n", chn, ret);
            return -1;
        }

        nFd = AX_IVPS_GetChnFd(p->nIvpsGrp, chn);
        p->arrOutChns[chn].nFd = nFd;
        DevFdListen(p->nEfd, nFd);
    }

    ret = AX_IVPS_StartGrp(p->nIvpsGrp);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_StartGrp(Grp: %d) failed, ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }

#ifdef __DYNAMIC_SWITCH_CHANNEL_TEST__
    /*
     * This function will start the thread and switch the IVPS channel every second.
     * if p->nIvpsAttrRepeatNum == -1, The above actions will cycle all the time.
     */
    p->nIvpsAttrRepeatNum = -1;
    IVPS_AttrThreadStart(p);
#endif
    return 0;
}

AX_S32 IVPS_StopGrp(const SAMPLE_IVPS_GRP_S *p)
{
    ALOGI("%s +++\n", __func__);
    AX_S32 ret = IVPS_SUCC;

    ret = AX_IVPS_StopGrp(p->nIvpsGrp);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_StopGrp(Grp: %d) failed(this grp is not started) ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }

    for (IVPS_CHN chn = 0; chn < p->tPipelineAttr.nOutChnNum; ++chn)
    {
        ret = AX_IVPS_DisableChn(p->nIvpsGrp, chn);
        if (IVPS_SUCC != ret)
        {
            ALOGE("AX_IVPS_DestoryChn(Chn: %d) failed, ret=0x%x.\n", chn, ret);
            return -1;
        }
    }
    ret = AX_IVPS_DestoryGrp(p->nIvpsGrp);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_DestoryGrp(Grp: %d) failed, ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }

    // AX_IVPS_CloseAllFd();
    DevEfdRelease(p->nEfd);

    ALOGI("%s ---\n", __func__);
    return 0;
}

AX_S32 IVPS_ThreadStart(AX_VOID *p)
{
    pthread_t tid = 0;

    if (0 != pthread_create(&tid, NULL, GetFrameThread, p))
    {
        return -1;
    }
    pthread_detach(tid);

    if (0 != pthread_create(&tid, NULL, SendFrameThread, p))
    {
        return -1;
    }
    pthread_detach(tid);

    return 0;
}

AX_S32 IVPS_ThreadStartV2(AX_VOID *src, AX_VOID *dst)
{
    pthread_t tid = 0;
    if (dst)
    {
        if (0 != pthread_create(&tid, NULL, GetFrameThread, dst))
        {
            return -1;
        }
        pthread_detach(tid);
    }

    if (src)
    {
        if (0 != pthread_create(&tid, NULL, SendFrameThread, src))
        {
            return -1;
        }
        pthread_detach(tid);
    }
    return 0;
}

AX_S32 IVPS_AttrThreadStart(SAMPLE_IVPS_GRP_S *pGrp)
{
    pthread_t tid = 0;

    if (0 != pthread_create(&tid, NULL, IVPS_AttrChangeThread, (AX_VOID *)pGrp))
    {
        return -1;
    }
    pthread_detach(tid);

    return 0;
}

/****************************************************************************************************/
/************************************* SINGLE MODE **************************************************/
/****************************************************************************************************/

AX_S32 IVPS_StartSingle(IVPS_CHN IvpsChn)
{
    SAMPLE_IVPS_GRP_S *p = &gSampleIvpsSingle;
    int ret = 0;

    ALOGI("%s +++\n", __func__);

    p->nEfd = DevEfdCreate(1);

    ALOGI("chn id :%d\n", IvpsChn);
    ret = AX_IVPS_CreateGrp(p->nIvpsGrp, &p->tGrpAttr);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_CreateGrp(Grp: %d) failed, ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }

    ret = AX_IVPS_EnableChn(p->nIvpsGrp, IvpsChn);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_CreateChn(Chn: %d) failed, ret=0x%x.\n", IvpsChn, ret);
        return -1;
    }

    ret = AX_IVPS_StartGrp(p->nIvpsGrp);

    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_StartChn(Chn: %d) failed, ret=0x%x.\n", IvpsChn, ret);
        return -1;
    }

    p->arrOutChns[0].nFd = AX_IVPS_GetChnFd(p->nIvpsGrp, IvpsChn);
    DevFdListen(p->nEfd, p->arrOutChns[0].nFd);

    ALOGI("%s ---\n", __func__);
    return 0;
}

AX_S32 IVPS_StopSingle(IVPS_CHN IvpsChn)
{
    ALOGI("%s +++\n", __func__);
    AX_S32 ret = IVPS_SUCC;

    const SAMPLE_IVPS_GRP_S *p = &gSampleIvpsSingle;

    ret = AX_IVPS_StopGrp(p->nIvpsGrp);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_StopChn(Chn: %d) failed(this chn is not started) ret=0x%x.\n", IvpsChn, ret);
        return -1;
    }

    ret = AX_IVPS_DisableChn(p->nIvpsGrp, IvpsChn);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_DestoryChn(Chn: %d) failed, ret=0x%x.\n", IvpsChn, ret);
        return -1;
    }

    ret = AX_IVPS_DestoryGrp(p->nIvpsGrp);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_DestoryGrp(Grp: %d) failed, ret=0x%x.\n", p->nIvpsGrp, ret);
        return -1;
    }

    DevFdRelease(p->arrOutChns[0].nFd);

    DevEfdRelease(p->nEfd);

    ALOGI("%s ---\n", __func__);
    return 0;
}

AX_S32 IVPS_ProcessSingle(IVPS_CHN IvpsChn)
{
    AX_VIDEO_FRAME_INFO_S tDstFrame = {0};
    AX_VIDEO_FRAME_INFO_S tSrcFrame = {0};
    AX_S32 ret = 0;
    SAMPLE_IVPS_GRP_S *pThis = &gSampleIvpsSingle;

    /* inplace */
    AX_BLK BlkId;
    AX_U64 nPhyAddr;
    AX_VOID *pVirAddr;
    AX_U32 nFileSize = CalcImgSize(pThis->tPipelineAttr.tFilter[0][0].eDstPicFormat, pThis->tPipelineAttr.tFilter[0][0].nDstPicWidth, pThis->tPipelineAttr.tFilter[0][0].nDstPicHeight, pThis->tPipelineAttr.tFilter[0][0].eDstPicFormat /*ptImage->stVFrame.enImgFormat*/, 16);
    BufPoolBlockAddrGet(pThis->tPipelineAttr.tFbInfo.PoolId, nFileSize, &nPhyAddr, &pVirAddr, &BlkId);
    pThis->tPipelineAttr.tFbInfo.nPhyAddr = nPhyAddr;
    pThis->tPipelineAttr.tFbInfo.nPhyAddrUV = nPhyAddr + pThis->tPipelineAttr.tFilter[0][0].eDstPicFormat * pThis->tPipelineAttr.tFilter[0][0].nDstPicHeight;
    pThis->tPipelineAttr.tFbInfo.bInPlace = AX_TRUE;
    tDstFrame.stVFrame.u64VirAddr[0] = (AX_U32)pVirAddr;

    ALOGI("nPhyAddr:%llx eFormat:%x\n", nPhyAddr, pThis->tPipelineAttr.tFilter[0][0].eDstPicFormat);

    tSrcFrame.stVFrame.enImgFormat = pThis->tInputInfo.eFormat;
    tSrcFrame.stVFrame.u32Width = pThis->tInputInfo.nW;
    tSrcFrame.stVFrame.u32Height = pThis->tInputInfo.nH;
    tSrcFrame.stVFrame.s16OffsetLeft = pThis->tInputInfo.nX;
    tSrcFrame.stVFrame.s16OffsetTop = pThis->tInputInfo.nY;
    tSrcFrame.stVFrame.u32PicStride[0] = pThis->tInputInfo.nStride;

    tSrcFrame.stVFrame.u64PTS = 0x040702121;
    tSrcFrame.stVFrame.u64UserData = 0xE1F4A7; // rand();
    tSrcFrame.stVFrame.u64SeqNum = 0x19860329;

    tSrcFrame.stVFrame.u32BlkId[0] = pThis->BlkId0;
    tSrcFrame.stVFrame.u32BlkId[1] = pThis->BlkId1;

    tSrcFrame.stVFrame.u64PhyAddr[0] = pThis->nPhyAddr[0];
    tSrcFrame.stVFrame.u64PhyAddr[1] = pThis->nPhyAddr[1];
    ALOGI("INPUT: stVFrame.u32BlkId[0]:%x stVFrame.u32BlkId[1]:%x\n", tSrcFrame.stVFrame.u32BlkId[0], tSrcFrame.stVFrame.u32BlkId[1]);
    ALOGI("u64PhyAddr[0]:%llx u64PhyAddr[1]:%llx\n", tSrcFrame.stVFrame.u64PhyAddr[0], tSrcFrame.stVFrame.u64PhyAddr[1]);

    CHECK_RESULT(AX_IVPS_SendFrame(pThis->nIvpsGrp, &tSrcFrame.stVFrame, SAMPLE_IVPS_SENDFRAME_TIMEOUT));
    DevPolWait(pThis->nEfd, 1, SAMPLE_IVPS_GETFRAME_TIMEOUT);
    ret = AX_IVPS_GetChnFrame(pThis->nIvpsGrp, IvpsChn, &tDstFrame.stVFrame, SAMPLE_IVPS_GETFRAME_TIMEOUT);

    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_GetFrame fail, ret=0x%x\n", ret);
        return -1;
    }

    ALOGI("OUTPUT: stVFrame.u32BlkId[0]:%x stVFrame.u32BlkId[1]:%x\n", tDstFrame.stVFrame.u32BlkId[0], tDstFrame.stVFrame.u32BlkId[1]);
    ALOGI("u64PhyAddr[0]:%llx u64PhyAddr[1]:%llx\n", tDstFrame.stVFrame.u64PhyAddr[0], tDstFrame.stVFrame.u64PhyAddr[1]);
    SaveFileExt(&tDstFrame.stVFrame, pThis->nIvpsGrp, IvpsChn, pThis->pFilePath, "Single");
    ret = AX_IVPS_ReleaseChnFrame(pThis->nIvpsGrp, IvpsChn, &tDstFrame.stVFrame);
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_ReleaseChnFrame fail, ret=0x%x\n", ret);
        return -1;
    }
    return 0;
}

/****************************************************************************************************/
/****************************************************************************************************/

static int ImgInfo_Parse(SAMPLE_IVPS_IMAGE_S *tImage, char *pArg)
{
    char *end, *p = NULL;

    if (pArg && strlen(pArg) > 0)
    {
        char *ppToken[4] = {NULL};
        AX_S32 nNum = 0;
        if (!Split(pArg, "@", ppToken, 4, &nNum))
        {
            // SAMPLE_IVPS_ObjHelp();

            ALOGE("error! para is not right!\n");
            return -1;
        }
        else
        {
            if (nNum == 1)
            {

                p = ppToken[0];
            }
            else if (nNum == 4)
            {
                tImage->pImgFile = ppToken[0];
                tImage->tImage.eFormat = atoi(ppToken[1]);
                p = ppToken[2];

                tImage->tImage.nStride = strtoul(p, &end, 10);
                if (*end != 'x')
                    return -1;
                p = end + 1;
                tImage->tImage.nH = strtoul(p, &end, 10);

                p = ppToken[3];
            }
        }
    }

    tImage->tRect.nW = strtoul(p, &end, 10);
    if (*end != 'x')
        return -1;

    p = end + 1;
    tImage->tRect.nH = strtoul(p, &end, 10);

    if (*end == '+' || *end == '-')
    {
        p = end + 1;
        tImage->tRect.nX = strtol(p, &end, 10);
        if (*end != '+' && *end != '-')
            return -1;
        p = end + 1;
        tImage->tRect.nY = strtol(p, &end, 10);
    }
    else
    {
        tImage->tRect.nX = 0;
        tImage->tRect.nY = 0;
    }

    if (*end == '*')
    {
        p = end + 1;
        tImage->nAlpha = strtoul(p, &end, 10);
    }
    else
    {
        tImage->nAlpha = 255;
    }

    if (*end == '%')
    {
        p = end + 1;
        tImage->eBlkSize = strtoul(p, &end, 10);
    }
    else
    {
        tImage->eBlkSize = 1;
    }

    if (*end == ':')
    {
        p = end + 1;
        tImage->nColor = strtoul(p, &end, 16);
    }
    else
    {
        tImage->nColor = 0x808080; // white
    }

    if (*end == '-')
    {
        p = end + 1;
        tImage->nLineW = strtoul(p, &end, 10);
    }
    else
    {
        tImage->nLineW = 1;
    }

    if (*end == '#')
    {
        p = end + 1;
        tImage->nChn = strtoul(p, &end, 10);
    }
    else
    {
        tImage->nChn = 0;
    }

    ALOGI("path:%s,format:%d, stride:%d (w:%d, h:%d, x:%d, y:%d) alpha:%d\n",
          tImage->pImgFile, tImage->tImage.eFormat, tImage->tImage.nStride, tImage->tRect.nW, tImage->tRect.nH, tImage->tRect.nX,
          tImage->tRect.nY, tImage->nAlpha);

    ALOGI("blksize:%d, color:%x, line:%d, chn:%d\n",
          tImage->eBlkSize, tImage->nColor, tImage->nLineW, tImage->nChn);

    ALOGI("image parse end nPhyAddr:%x\n", tImage->tImage.nPhyAddr);
    return 0;
}

char *FrameInfo_Get(char *optArg, AX_VIDEO_FRAME_INFO_S *ptFrame)
{
    ALOGI("%s:\n", __func__);

    SAMPLE_IVPS_IMAGE_S tImage;
    memset(&tImage, 0, sizeof(tImage));

    ImgInfo_Parse(&tImage, optArg);

    ptFrame->stVFrame.s16OffsetLeft = tImage.tRect.nX;
    ptFrame->stVFrame.s16OffsetTop = tImage.tRect.nY;
    ptFrame->stVFrame.s16OffsetRight = tImage.tRect.nX + tImage.tRect.nW;
    ptFrame->stVFrame.s16OffsetBottom = tImage.tRect.nY + tImage.tRect.nH;
    ptFrame->stVFrame.enImgFormat = tImage.tImage.eFormat;
    ptFrame->stVFrame.u32PicStride[0] = tImage.tImage.nStride;
    ptFrame->stVFrame.u32Width = tImage.tImage.nStride;
    ptFrame->stVFrame.u32Height = tImage.tImage.nH;

    ALOGI("ptFrame nW:%d nH:%d\n", ptFrame->stVFrame.u32Width, ptFrame->stVFrame.u32Height);
    ALOGI(" nX0:%d nY0:%d\n", ptFrame->stVFrame.s16OffsetLeft, ptFrame->stVFrame.s16OffsetTop);
    ALOGI(" nX1:%d nY1:%d\n", ptFrame->stVFrame.s16OffsetRight, ptFrame->stVFrame.s16OffsetBottom);

    ptFrame->stVFrame.u64PTS = 0x2020;
    ptFrame->stVFrame.u64SeqNum = 0;
    ptFrame->bEof = 0;

    return tImage.pImgFile;
}

char *OverlayInfo_Get(char *optArg, AX_VIDEO_FRAME_INFO_S *ptOverlay)
{
    ALOGI("%s:\n", __func__);

    SAMPLE_IVPS_IMAGE_S tImage;
    memset(&tImage, 0, sizeof(tImage));
    ImgInfo_Parse(&tImage, optArg);

    ptOverlay->stVFrame.s16OffsetLeft = tImage.tRect.nX;
    ptOverlay->stVFrame.s16OffsetTop = tImage.tRect.nY;
    ptOverlay->stVFrame.s16OffsetRight = tImage.tRect.nX + tImage.tRect.nW;
    ptOverlay->stVFrame.s16OffsetBottom = tImage.tRect.nY + tImage.tRect.nH;
    ptOverlay->stVFrame.enImgFormat = tImage.tImage.eFormat;
    ptOverlay->stVFrame.u32PicStride[0] = tImage.tImage.nStride;
    ptOverlay->stVFrame.u32Width = tImage.tImage.nStride;
    ptOverlay->stVFrame.u32Height = tImage.tImage.nH;
    ALOGI("ptOverlay nW:%d nH:%d\n", ptOverlay->stVFrame.u32Width, ptOverlay->stVFrame.u32Height);
    ALOGI(" nX0:%d nY0:%d\n", ptOverlay->stVFrame.s16OffsetLeft, ptOverlay->stVFrame.s16OffsetTop);
    ALOGI(" nX1:%d nY1:%d\n", ptOverlay->stVFrame.s16OffsetRight, ptOverlay->stVFrame.s16OffsetBottom);

    ptOverlay->stVFrame.u64PTS = 0x2021;
    ptOverlay->stVFrame.u64SeqNum = 0;
    ptOverlay->bEof = 0;

    return tImage.pImgFile;
}

AX_S32 ShapeAttrInfo_Get(char *optArg, AX_IVPS_TDP_CFG_S *ptTdpCfg)
{
    char *end, *p;
    AX_S32 chn_id;
    if (!optArg)
    {
        ALOGE("error! para is null!\n");
        return -1;
    }
    p = optArg;

    chn_id = strtoul(p, &end, 10);
    if (*end != '#')
        return -1;
#if 0
    ptShape->nStride = strtoul(p, &end, 10);
    if (*end != '@')
        return -1;

    p = end + 1;
    ptShape->nW = strtoul(p, &end, 10);
    if (*end != 'x')
        return -1;

    p = end + 1;
    ptShape->nH = strtoul(p, &end, 10);
    if (*end != '@')
        return -1;

    p = end + 1;
    ptShape->eFormat = strtoul(p, &end, 10);
#endif

    if (*end == '#')
    {
        p = end + 1;
        ptTdpCfg->eRotation = strtoul(p, &end, 10);
    }
    else
    {
        ptTdpCfg->eRotation = 0;
    }
    if (*end == '-')
    {
        p = end + 1;
        ptTdpCfg->bMirror = strtoul(p, &end, 10);
    }
    else
    {
        ptTdpCfg->bMirror = 1;
    }
    if (*end == '+')
    {
        p = end + 1;
        ptTdpCfg->bFlip = strtoul(p, &end, 10);
    }
    else
    {
        ptTdpCfg->bFlip = 1;
    }

    ALOGI("chn_id:%d rotate:%d mirror:%d flip:%d\n",
          chn_id,
          ptTdpCfg->eRotation, ptTdpCfg->bMirror, ptTdpCfg->bFlip);
    return chn_id;
}

AX_S32 ImageBuf_Get(AX_VIDEO_FRAME_INFO_S *ptImage, char *pImgFile)
{
    AX_BOOL isYUVSplit = AX_FALSE;

    if (pImgFile)
    {

#ifdef IVPS_BUF_POOL_MEM_ALLOC
        if (ptImage->stVFrame.enImgFormat == AX_YUV420_SEMIPLANAR || ptImage->stVFrame.enImgFormat == AX_YUV420_SEMIPLANAR_VU)
        {
            // isYUVSplit = AX_TRUE;
        }

        if (!LoadImageExt(ptImage->u32PoolId, pImgFile, 0, &ptImage->stVFrame.u32BlkId[0], isYUVSplit, &ptImage->stVFrame.u64PhyAddr[0], (AX_VOID **)&ptImage->stVFrame.u64VirAddr[0]))
        {
            return -1;
        }
        ALOGI("VirAddr:%x VirAddr_UV:%x\n", (AX_U32)ptImage->stVFrame.u64VirAddr[0], (AX_U32)ptImage->stVFrame.u64VirAddr[1]);

#else
        // if (!LoadImage(pImgFile, nImgSize, &ptImage->stVFrame.u64PhyAddr[0], (AX_VOID **)&ptImage->stVFrame.u64VirAddr))
        // {
        //     return -1;
        // }
#endif
    }
    return 0;
}

AX_S32 CoverInfo_Get(char *optArg, AX_IVPS_RGN_MOSAIC_S *ptMosaic)
{
    ALOGI("%s:\n", __func__);

    SAMPLE_IVPS_IMAGE_S tImage;
    memset(&tImage, 0, sizeof(tImage));

    ImgInfo_Parse(&tImage, optArg);
    ptMosaic->tRect.nW = tImage.tRect.nW;
    ptMosaic->tRect.nH = tImage.tRect.nH;
    ptMosaic->tRect.nX = tImage.tRect.nX;
    ptMosaic->tRect.nY = tImage.tRect.nY;
    ptMosaic->eBklSize = tImage.eBlkSize;

    return 0;
}
