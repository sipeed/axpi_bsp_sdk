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

#include "ax_isp_api.h"
#include "sample_vin.h"
#include "common_sys.h"
#include "common_vin.h"
#include "common_cam.h"

#include "ax_interpreter_external_api.h"

#include "sample_comm_vo.h"

typedef enum {
    SYS_CASE_NONE  = -1,
    SYS_CASE_SINGLE_OS04A10  = 0,
    SYS_CASE_SINGLE_IMX334   = 1,
    SYS_CASE_SINGLE_GC4653   = 2,
    SYS_CASE_DUAL_OS04A10    = 3,
    SYS_CASE_BUTT
} COMMON_SYS_CASE_E;

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3},
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3},
};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Sdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Hdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleGc4653[] = {

    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2560, 1440, 2560, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
    {640, 360, 640, AX_YUV420_SEMIPLANAR, 3},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolDoubleOs04a10[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15 * 2},   /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5 * 2},   /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 6 * 2},   /*vin nv21/nv21 use */
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3 * 2},
    {1344, 760, 1344, AX_YUV420_SEMIPLANAR, 3 * 2},
};

static CAMERA_T gCams[MAX_CAMERAS] = {0};

static volatile AX_S32 gLoopExit = 0;

static AX_S32 g_isp_force_loop_exit = 0;

void *IspRun(void *args)
{
    AX_U32 i = (AX_U32)args;

    SAMPLE_PRT("cam %d is running...\n", i);

    while (!g_isp_force_loop_exit) {
        if (gCams[i].bOpen)
            AX_ISP_Run(gCams[i].nPipeId);
    }
    return NULL;
}

AX_S32 SysRun()
{
    AX_S32 s32Ret = 0, i;
    g_isp_force_loop_exit = 0;
    for (i = 0; i < MAX_CAMERAS; i++) {
        if (gCams[i].bOpen)
            pthread_create(&gCams[i].tIspProcThread, NULL, IspRun, (AX_VOID *)i);;
    }

    while (!gLoopExit) {
        sleep(1);
    }

    g_isp_force_loop_exit = 1;

    for (i = 0; i < MAX_CAMERAS; i++) {
        if (gCams[i].bOpen) {
            pthread_cancel(gCams[i].tIspProcThread);
            s32Ret = pthread_join(gCams[i].tIspProcThread, NULL);
            if (s32Ret < 0) {
                SAMPLE_PRT(" isp run thread exit failed, ret=0x%x.\n", s32Ret);
            }
        }
    }

    return 0;
}

AX_VOID PrintHelp()
{
    SAMPLE_PRT("command:\n");
    SAMPLE_PRT("\t-c: ISP Test Case:\n");
    SAMPLE_PRT("\t\t0: Single OS04A10\n");
    SAMPLE_PRT("\t\t1: Single IMX334\n");
    SAMPLE_PRT("\t\t2: Single GC4653\n");
    SAMPLE_PRT("\t\t3: DUAL OS04A10\n");

    SAMPLE_PRT("\t-e: SDR/HDR Mode:\n");
    SAMPLE_PRT("\t\t1: SDR\n");
    SAMPLE_PRT("\t\t2: HDR 2DOL\n");

    SAMPLE_PRT("\t-v Interface@Resolution@VRefresh:\n");
    SAMPLE_PRT("\t\tdpi@<w>x<h>@<refresh>: dpi output\n");
    SAMPLE_PRT("\t\tdsi0@<w>x<h>@<refresh>: mipi dsi0 output\n");
    SAMPLE_PRT("\t\tdsi1@<w>x<h>@<refresh>: mipi dsi1 output\n");
    SAMPLE_PRT("\t\t2dsi@<w>x<h>@<refresh>: dual mipi dsi output\n");
    SAMPLE_PRT("\t\tbt656@<w>x<h>@<refresh>: bt656 output\n");
    SAMPLE_PRT("\t\tbt1120@<w>x<h>@<refresh>: bt1120 output\n");
    SAMPLE_PRT("\t-s: camera chn\n");

    SAMPLE_PRT("Example:\n");
    SAMPLE_PRT("\t./sample_vin_vo -c 2 -e 1 -s 0 -v dsi0@1280x800@45\n");
    SAMPLE_PRT("\t./sample_vin_vo -c 0 -e 1 -s 0 -v dsi0@480x360@60\n");
}

static AX_VOID __sigint(int iSigNo)
{
    SAMPLE_PRT("Catch signal %d\n", iSigNo);
    gLoopExit = 1;

    return ;
}

typedef struct axSAMPLE_VO_CHN_THREAD_PARAM {
    pthread_t ThreadID;

    AX_U32 u32ThreadForceStop;

    AX_U32 u32LayerID;
    AX_U32 u32ChnID;
    AX_POOL u32UserPoolId;
} SAMPLE_VO_CHN_THREAD_PARAM_S;

#define SAMPLE_VO_DEV0 0

SAMPLE_VO_CONFIG_S stVoConf = {
    /* for device */
    .VoDev = SAMPLE_VO_DEV0,
    .enVoIntfType = VO_INTF_DSI0,
    .enIntfSync = VO_OUTPUT_USER,
    .stReso = {
        .u32Width = 1280,
        .u32Height = 800,
        .u32RefreshRate = 45,
    },
    .u32LayerNr = 1,
    .stVoLayer = {
        {
            // layer0
            .VoLayer = 0,
            .stVoLayerAttr = {
                .stDispRect = {0, 0, 1280, 800},
                .stImageSize = {1280, 800},
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
        {
            // layer1
            .VoLayer = 1,
            .stVoLayerAttr = {
                .stDispRect = {0, 0, 1280, 800},
                .stImageSize = {1280, 800},
                .enPixFmt = AX_YUV420_SEMIPLANAR,
                .enLayerSync = VO_LAYER_SYNC_NORMAL,
                .u32PrimaryChnId = 0,
                .u32FrameRate = 0,
                .u32FifoDepth = 0,
                .u32ChnNr = 16,
                .u32BkClr = 0,
                .enLayerBuf = VO_LAYER_OUT_BUF_POOL,
                .u32InplaceChnId = 0,
                .u32PoolId = 0,
                .enDispatchMode = VO_LAYER_OUT_TO_FIFO,
                .u64KeepChnPrevFrameBitmap = ~0x0,
            },
            .enVoMode = VO_MODE_1MUX,
        },
    },
};

static AX_S32 ParseVoPubAttr(AX_CHAR *pStr, SAMPLE_VO_CONFIG_S *pstVoConf)
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
    AX_U64 u64VirAddr, u64PhysAddr;
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
                          u64VirAddr);

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

static AX_S32 VoInit(AX_U32 u32CamChn, CAMERA_T *pstCam, SAMPLE_VO_CONFIG_S *pstVoConf)
{
    AX_S32 i, s32Ret;
    AX_U64 u64BlkSize = 0;
    SAMPLE_VO_LAYER_CONFIG_S *pstVoLayer;
    VO_VIDEO_LAYER_ATTR_S *pstVoLayerAttr;
    SAMPLE_VO_CHN_THREAD_PARAM_S *pstChnThreadParam = &g_stChnThreadParam;

    s32Ret = AX_SYS_Init();
    if (s32Ret) {
        SAMPLE_PRT("AX_SYS_Init failed, s32Ret = 0x%x\n", s32Ret);
        return -1;
    }

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
            pstVoLayerAttr->stImageSize.u32Width = (pstCam->stChnAttr.tChnAttr[u32CamChn].nWidth + 0xF) & (~0xF);
            pstVoLayerAttr->stImageSize.u32Height = pstCam->stChnAttr.tChnAttr[u32CamChn].nHeight;
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

static AX_VOID VoDeInit(SAMPLE_VO_CONFIG_S *pstVoConf)
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

    AX_SYS_Deinit();
}

int main(int argc, char *argv[])
{
    SAMPLE_PRT("ISP Sample. Build at %s %s\n", __DATE__, __TIME__);

    int c;
    int isExit = 0, i;
    AX_S32 s32Ret = 0;
    AX_U32 u32ChnSel = 0;
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_NONE;
    COMMON_SYS_ARGS_T tCommonArgs = {0};
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_LINEAR_MODE;
    SAMPLE_SNS_TYPE_E eSnsType = OMNIVISION_OS04A10;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigint);

    if (argc < 2) {
        PrintHelp();
        exit(0);
    }

    while ((c = getopt(argc, argv, "c:e:hv:s:")) != -1) {
        isExit = 0;
        switch (c) {
        case 'c':
            eSysCase = (COMMON_SYS_CASE_E)atoi(optarg);
            break;
        case 'e':
            eHdrMode = (AX_SNS_HDR_MODE_E)atoi(optarg);
            break;
        case 'v':
            s32Ret = ParseVoPubAttr(optarg, &stVoConf);
            if (s32Ret) {
                isExit = 1;
            }
            break;
        case 's':
            u32ChnSel = atoi(optarg);
            if (u32ChnSel > 2)
                u32ChnSel = 2;
            break;
        case 'h':
            isExit = 1;
            break;
        default:
            isExit = 1;
            break;
        }
    }

    if (isExit) {
        PrintHelp();
        exit(0);
    }

    if (eSysCase >= SYS_CASE_BUTT || eSysCase <= SYS_CASE_NONE) {
        SAMPLE_PRT("error case type\n");
        exit(0);
    }

    if (eSysCase == SYS_CASE_SINGLE_OS04A10) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10) / sizeof(gtSysCommPoolSingleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
    } else if (eSysCase == SYS_CASE_SINGLE_IMX334) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = SONY_IMX334;
        COMMON_ISP_GetSnsConfig(SONY_IMX334, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE) {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleImx334Sdr) / sizeof(gtSysCommPoolSingleImx334Sdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleImx334Sdr;
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
        } else {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleImx334Hdr) / sizeof(gtSysCommPoolSingleImx334Hdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleImx334Hdr;
        }
    } else if (eSysCase == SYS_CASE_SINGLE_GC4653) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = GALAXYCORE_GC4653;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleGc4653) / sizeof(gtSysCommPoolSingleGc4653[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleGc4653;
        COMMON_ISP_GetSnsConfig(GALAXYCORE_GC4653, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
    } else if (eSysCase == SYS_CASE_DUAL_OS04A10) {
        tCommonArgs.nCamCnt = 2;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr,
                                &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[1].stSnsAttr, &gCams[1].stSnsClkAttr, &gCams[1].stDevAttr,
                                &gCams[1].stPipeAttr,
                                &gCams[1].stChnAttr);
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolDoubleOs04a10) / sizeof(gtSysCommPoolDoubleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolDoubleOs04a10;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        gCams[i].eSnsType = eSnsType;
        gCams[i].stSnsAttr.eSnsMode = eHdrMode;
        gCams[i].stDevAttr.eSnsMode = eHdrMode;
        gCams[i].stPipeAttr.eSnsMode = eHdrMode;

        if (i == 0) {
            gCams[i].nDevId = 0;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_0;
            gCams[i].nPipeId = 0;
        } else if (i == 1) {
            gCams[i].nDevId = 2;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_2;
            gCams[i].nPipeId = 2;
        }
    }

    AX_NPU_SDK_EX_ATTR_T sNpuAttr;
    sNpuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
    s32Ret = AX_NPU_SDK_EX_Init_with_attr(&sNpuAttr);
    if (0 != s32Ret) {
        SAMPLE_PRT("AX_NPU_SDK_EX_Init_with_attr failed, ret=0x%x.\n", s32Ret);
        return -1;
    }

    s32Ret = COMMON_SYS_Init(&tCommonArgs);
    if (s32Ret) {
        SAMPLE_PRT("isp sys init fail\n");
        goto EXIT;
    }

    COMMON_CAM_Init();

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        s32Ret = COMMON_CAM_Open(&gCams[i]);
        if (s32Ret)
            goto EXIT;
        gCams[i].bOpen = AX_TRUE;
        SAMPLE_PRT("camera %d is open\n", i);
    }

    s32Ret = VoInit(u32ChnSel, &gCams[0], &stVoConf);
    if (s32Ret) {
        SAMPLE_PRT("VoInit failed, s32Ret = %d\n", s32Ret);
        goto EXIT;
    }

    SysRun();

EXIT:
    for (i = 0; i < MAX_CAMERAS; i++) {
        if (!gCams[i].bOpen)
            continue;
        COMMON_CAM_Close(&gCams[i]);
    }

    COMMON_CAM_Deinit();
    COMMON_SYS_DeInit();

    VoDeInit(&stVoConf);

    /* exit */
    exit(0);
}
