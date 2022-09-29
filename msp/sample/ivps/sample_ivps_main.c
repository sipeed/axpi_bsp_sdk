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
#include "sample_ivps_region.h"
#include "sample_ivps_object.h"
#include "sample_ivps_util.h"
#include "sample_ivps_help.h"
#include "sample_ivps_dewarp.h"
#include "sample_ivps_venc.h"
#include "ax_venc_api.h"

static AX_VOID SigInt(AX_S32 signo)
{
    ALOGW("SigInt Catch signal %d\n", signo);
    ThreadLoopStateSet(AX_TRUE);
}

static AX_VOID SigStop(AX_S32 signo)
{
    ALOGW("SigStop Catch signal %d\n", signo);
    ThreadLoopStateSet(AX_TRUE);
}

#define __IVPS_TDP_OFFLINE_TEST__
//#define __IVPS_GDC_OFFLINE_TEST__
//#define __IVPS_NPU_OFFLINE_TEST__

#define __IVPS_GROUP_TEST__
//#define __IVPS_VENC_LINK_TEST__

extern SAMPLE_IVPS_GRP_S gSampleIvpsGrp;
extern SAMPLE_IVPS_GRP_S gSampleIvpsSingle;
extern AX_S32 IVPS_ChnAttrChange(AX_S32 nIvpsGrp, AX_S32 nIvpsChn);
extern AX_S32 IVPS_VencInitEx(AX_U8 pipe);

AX_S32 main(AX_S32 argc, char *argv[])
{
    AX_S32 ret = IVPS_SUCC;

    AX_S32 c;
    AX_BOOL isExit = AX_TRUE;
    AX_S32 nHelpIdx = -1;

    AX_S32 nRepeatCount = -1;
    AX_S32 nRegionNum = 0;

    char *pFrameFile = NULL, *pOverlayFile = NULL;
    AX_VIDEO_FRAME_INFO_S tFrame;
    AX_VIDEO_FRAME_INFO_S tOverlay;
    AX_IVPS_RGN_MOSAIC_S tMosaic;
    SAMPLE_IVPS_GRP_S *pGrp = &gSampleIvpsGrp;
    SAMPLE_IVPS_GRP_S *pSingle = &gSampleIvpsSingle;
    BLK_INFO_S arrBlkInfo[16];

    ALOGI("IVPS Sample. Build at %s %s\n", __DATE__, __TIME__);

    memset(&tFrame, 0, sizeof(tFrame));
    memset(&tOverlay, 0, sizeof(tOverlay));

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SigInt);
    signal(SIGTSTP, SigStop);

    while ((c = getopt(argc, argv, "v:a:g:c:p:t:n:h:r:")) != -1)
    {
        isExit = AX_FALSE;
        switch (c)
        {
        case 'v':
            pFrameFile = FrameInfo_Get(optarg, &tFrame);
            break;

        case 'a':
            break;

        case 'g':
            pOverlayFile = OverlayInfo_Get(optarg, &tOverlay);
            break;

        case 'c':
            CoverInfo_Get(optarg, &tMosaic);
            break;

        case 'r':
            nRegionNum = atoi(optarg);
            if (nRegionNum < 0)
                nRegionNum = 0;
            break;

        case 'n':
            nRepeatCount = atoi(optarg);
            if (nRepeatCount <= 0)
            {
                ALOGE("Invalid repeat count parameter, reset to default value: -1\n");
                nRepeatCount = -1;
            }
            break;
        case 'h':
            if (optarg)
            {
                nHelpIdx = atoi(optarg);
            }
            isExit = AX_TRUE;
            break;
        default:
            isExit = AX_TRUE;
            break;
        }
    }

    if (isExit)
    {
        ShowUsage(nHelpIdx);
        exit(0);
    }

    /* SYS global init */
    AX_SYS_Init();

    ALOGI("tFrame nW:%d nH:%d File:%s\n", tFrame.stVFrame.u32Width, tFrame.stVFrame.u32Height, pFrameFile);

    /****************************IVPS Prepare*********************************/
    /*
     * 1. Create memory pool for IVPS
     */
    arrBlkInfo[0].nSize = CalcImgSize(1920, 1920, 1080, AX_FORMAT_RGBA8888, 16);
    arrBlkInfo[0].nCnt = 6;
    arrBlkInfo[1].nSize = CalcImgSize(4096, 4096, 2160, AX_FORMAT_RGBA8888, 16);
    arrBlkInfo[1].nCnt = 5;

    tFrame.u32PoolId = AX_INVALID_POOLID;
    tOverlay.u32PoolId = AX_INVALID_POOLID;

    ret = IVPS_PoolFloorInit(&arrBlkInfo[0], 2);
    if (ret)
    {
        ALOGE("AX_IVPS_Init failed, ret=0x%x.\n", ret);
        goto error0;
    }

    /*
     * 2. IVPS initialization
     */
    ret = AX_IVPS_Init();
    if (IVPS_SUCC != ret)
    {
        ALOGE("AX_IVPS_Init failed, ret=0x%x.\n", ret);
        goto error1;
    }

    /* source image buffer get */
    ImageBuf_Get(&tFrame, pFrameFile);
    if (pOverlayFile)
    {
        ImageBuf_Get(&tOverlay, pOverlayFile);
    }
    pFrameFile = ExtractFilePath(pFrameFile);

    if (tFrame.stVFrame.u32BlkId[0])
    {

        ALOGI("stVFrame.u32BlkId[0]:%x stVFrame.u32BlkId[1]:%x\n", tFrame.stVFrame.u32BlkId[0], tFrame.stVFrame.u32BlkId[1]);
        tFrame.stVFrame.u64PhyAddr[1] = tFrame.stVFrame.u64PhyAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;
        tFrame.stVFrame.u64VirAddr[1] = tFrame.stVFrame.u64VirAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;

        pGrp->nPhyAddr[0] = (AX_U32)tFrame.stVFrame.u64PhyAddr[0];
        pGrp->pVirAddr[0] = (AX_U8 *)((AX_U32)tFrame.stVFrame.u64VirAddr[0]);
        pGrp->nPhyAddr[1] = (AX_U32)tFrame.stVFrame.u64PhyAddr[1];
        pGrp->pVirAddr[1] = (AX_U8 *)((AX_U32)tFrame.stVFrame.u64VirAddr[1]);
        pGrp->BlkId0 = tFrame.stVFrame.u32BlkId[0];
        pGrp->BlkId1 = tFrame.stVFrame.u32BlkId[1];

        pGrp->tInputInfo.nStride = tFrame.stVFrame.u32PicStride[0];

        pGrp->tInputInfo.nW = tFrame.stVFrame.s16OffsetRight - tFrame.stVFrame.s16OffsetLeft;
        pGrp->tInputInfo.nH = tFrame.stVFrame.s16OffsetBottom - tFrame.stVFrame.s16OffsetTop;
        pGrp->tInputInfo.nX = tFrame.stVFrame.s16OffsetLeft;
        pGrp->tInputInfo.nY = tFrame.stVFrame.s16OffsetTop;
        pGrp->tInputInfo.eFormat = tFrame.stVFrame.enImgFormat;
        pGrp->pFilePath = pFrameFile;

        pSingle->nPhyAddr[0] = (AX_U32)tFrame.stVFrame.u64PhyAddr[0];
        pSingle->pVirAddr[0] = (AX_U8 *)((AX_U32)tFrame.stVFrame.u64VirAddr[0]);
        pSingle->nPhyAddr[1] = (AX_U32)tFrame.stVFrame.u64PhyAddr[1];
        pSingle->pVirAddr[1] = (AX_U8 *)((AX_U32)tFrame.stVFrame.u64VirAddr[1]);
        pSingle->BlkId0 = tFrame.stVFrame.u32BlkId[0];
        pSingle->BlkId1 = tFrame.stVFrame.u32BlkId[1];

        pSingle->tInputInfo.nStride = tFrame.stVFrame.u32PicStride[0];

        pSingle->tInputInfo.nW = tFrame.stVFrame.s16OffsetRight - tFrame.stVFrame.s16OffsetLeft;
        pSingle->tInputInfo.nH = tFrame.stVFrame.s16OffsetBottom - tFrame.stVFrame.s16OffsetTop;
        pSingle->tInputInfo.nX = tFrame.stVFrame.s16OffsetLeft;
        pSingle->tInputInfo.nY = tFrame.stVFrame.s16OffsetTop;
        pSingle->tInputInfo.eFormat = tFrame.stVFrame.enImgFormat;
        pSingle->pFilePath = pFrameFile;
    }

    /****************************OFFLINE*********************************/

#ifdef __IVPS_TDP_OFFLINE_TEST__
    AX_VIDEO_FRAME_S tDstFrame;
    AX_BLK BlkId;
    AX_U32 nImgSize;

    /*
     * 1. AX_IVPS_CmmCopy
     * Function: Move a piece of memory data.
     * Note: When copy, 16K Byte automatically aligned, 64M maximum.
     */
    AX_IVPS_CmmCopy(tFrame.stVFrame.u64PhyAddr[0], 0xa8000000, 2688 * 1520 * 3);

    /*
     * 2. AX_IVPS_CSC
     * Function: Color space conversion.
     * Note: Stride and width should be 16 Byte aligned.
     *       The u64PhyAddr[0] of ptDst should be set. If format is AX_YUV420_SEMIPLANAR, u64PhyAddr[1] should be set.
     */
    ALOGI("CSC u32Width =%d\n", tFrame.stVFrame.u32Width);
    memset(&tDstFrame, 0, sizeof(tDstFrame));
    tDstFrame.enImgFormat = AX_FORMAT_RGB888;
    tDstFrame.enCompressMode = AX_FALSE; /*AX_TRUE : FBC enable, reg:0x4430030 should be set 0x3e140*/
    nImgSize = CalcImgSize(tFrame.stVFrame.u32PicStride[0], tFrame.stVFrame.u32Width, tFrame.stVFrame.u32Height, tDstFrame.enImgFormat, 16);
    if (tDstFrame.enCompressMode)
    {
        /* If enable compress, add header information to the address of Y and UV.
        Y headers size = H/2*128
        UV header size = H/2*64 */
        nImgSize = nImgSize + DIV_ROUND_UP(tFrame.stVFrame.u32Height, 2) * 64 * 3;
    }
    CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nImgSize, &tDstFrame.u64PhyAddr[0], (AX_VOID **)&tDstFrame.u64VirAddr[0], &BlkId));
    tDstFrame.u64PhyAddr[1] = tDstFrame.u64PhyAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;

    if (tDstFrame.enCompressMode)
    {
        tDstFrame.u64PhyAddr[0] += DIV_ROUND_UP(tFrame.stVFrame.u32Height, 2) * 64 * 2;
        tDstFrame.u64PhyAddr[1] += DIV_ROUND_UP(tFrame.stVFrame.u32Height, 2) * 64 * 3;
    }
    AX_IVPS_CSC(&tFrame.stVFrame, &tDstFrame);

    {
        AX_VIDEO_FRAME_S tDstFrame1;
        AX_BLK BlkId1;
        memset(&tDstFrame1, 0, sizeof(tDstFrame1));
        tDstFrame1.enImgFormat = AX_YUV420_SEMIPLANAR;
        tDstFrame1.enCompressMode = AX_FALSE;
        nImgSize = CalcImgSize(tDstFrame.u32PicStride[0], tDstFrame.u32Width, tDstFrame.u32Height, tDstFrame1.enImgFormat, 16);
        if (tDstFrame1.enCompressMode)
        {
            nImgSize = nImgSize + DIV_ROUND_UP(tDstFrame1.u32Height, 2) * 64 * 3;
        }
        ALOGI("CSC nImgSize:%d\n", nImgSize);
        CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nImgSize, &tDstFrame1.u64PhyAddr[0], (AX_VOID **)&tDstFrame1.u64VirAddr[0], &BlkId1));
        tDstFrame1.u64PhyAddr[1] = tDstFrame1.u64PhyAddr[0] + tDstFrame.u32PicStride[0] * tDstFrame.u32Height;

        if (tDstFrame1.enCompressMode)
        {
            tDstFrame1.u64PhyAddr[0] += DIV_ROUND_UP(tDstFrame1.u32Height, 2) * 64 * 2;
            tDstFrame1.u64PhyAddr[1] += DIV_ROUND_UP(tDstFrame1.u32Height, 2) * 64 * 3;
        }
        AX_IVPS_CSC(&tDstFrame, &tDstFrame1);
        SaveFileExt(&tDstFrame1, 0, 0, pSingle->pFilePath, "CSC");
        ret = AX_POOL_ReleaseBlock(BlkId);
        if (ret)
        {
            ALOGE("[line:%d] IVPS Release BlkId fail, ret=0x%x\n", __LINE__, ret);
        }
        ret = AX_POOL_ReleaseBlock(BlkId1);
        if (ret)
        {
            ALOGE("[line:%d] IVPS Release BlkId1 fail, ret=0x%x\n", __LINE__, ret);
        }
    }

    /*
     * 3. AX_IVPS_Rotation
     * Function: Rotate 90/180/270.
     * Note: Stride and width should be 16 Byte aligned.
     *       The u64PhyAddr[0] of ptDst should be set. If format is AX_YUV420_SEMIPLANAR, u64PhyAddr[1] should be set.
     */
    ALOGI("Rotate u32Width =%d\n", tFrame.stVFrame.u32Width);
    memset(&tDstFrame, 0, sizeof(tDstFrame));
    nImgSize = CalcImgSize(tFrame.stVFrame.u32PicStride[0], tFrame.stVFrame.u32Width, tFrame.stVFrame.u32Height, tFrame.stVFrame.enImgFormat, 16);
    CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nImgSize, &tDstFrame.u64PhyAddr[0], (AX_VOID **)&tDstFrame.u64VirAddr[0], &BlkId));
    tDstFrame.u64PhyAddr[1] = tDstFrame.u64PhyAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;
    AX_IVPS_Rotation(&tFrame.stVFrame, AX_IVPS_ROTATION_90, &tDstFrame);
    SaveFileExt(&tDstFrame, 0, 0, pSingle->pFilePath, "Rotate");
    ret = AX_POOL_ReleaseBlock(BlkId);
    if (ret)
    {
        ALOGE("[line:%d] IVPS Release BlkId fail, ret=0x%x\n", __LINE__, ret);
    }

    /*
     * 4. AX_IVPS_FlipAndRotation
     * Function: Flip/Mirror/Rotate 0/90/180/270.
     * Note: Stride and width should be 16 Byte aligned.
     *       The u64PhyAddr[0] of ptDst should be set. If format is AX_YUV420_SEMIPLANAR, u64PhyAddr[1] should be set.
     */
    memset(&tDstFrame, 0, sizeof(tDstFrame));
    nImgSize = CalcImgSize(tFrame.stVFrame.u32PicStride[0], tFrame.stVFrame.u32Width, tFrame.stVFrame.u32Height, tFrame.stVFrame.enImgFormat, 16);
    CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nImgSize, &tDstFrame.u64PhyAddr[0], (AX_VOID **)&tDstFrame.u64VirAddr[0], &BlkId));
    tDstFrame.u64PhyAddr[1] = tDstFrame.u64PhyAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;
    AX_IVPS_FlipAndRotation(&tFrame.stVFrame, 1, AX_IVPS_ROTATION_90, &tDstFrame);

    SaveFileExt(&tDstFrame, 0, 0, pSingle->pFilePath, "FlipMirrorRotate");
    ret = AX_POOL_ReleaseBlock(BlkId);
    if (ret)
    {
        ALOGE("[line:%d] IVPS Release BlkId fail, ret=0x%x\n", __LINE__, ret);
    }

    /*
     * 5. AX_IVPS_CropResize
     * Function: Crop and Resize.
     * Note: Stride and width should be 16 Byte aligned.
     *       The u64PhyAddr[0] of ptDst should be set. If format is AX_YUV420_SEMIPLANAR, u64PhyAddr[1] should be set.
     *       If enable crop, s16OffsetTop/s16OffsetBottom/s16OffsetRight/s16OffsetLeft should be set.
     *       If AX_YUV420_SEMIPLANAR, u64PhyAddr[1] should be set.
     */
    AX_IVPS_ASPECT_RATIO_S tAspectRatio;
    tAspectRatio.eMode = AX_IVPS_ASPECT_RATIO_AUTO;
    tAspectRatio.eAligns[0] = AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER;
    tAspectRatio.eAligns[1] = AX_IVPS_ASPECT_RATIO_VERTICAL_CENTER;
    tAspectRatio.nBgColor = 0x0000FF;

    memset(&tDstFrame, 0, sizeof(tDstFrame));
    tDstFrame.u32PicStride[0] = 1280; // 1920;//1024
    AX_U32 WidthTemp, HeightTemp;
    WidthTemp = tDstFrame.u32Width = 1280;        // 1920;//1024
    HeightTemp = tDstFrame.u32Height = 720;       // 1080;//512
    tDstFrame.enImgFormat = AX_YUV420_SEMIPLANAR; // AX_FORMAT_RGB888

    nImgSize = CalcImgSize(tDstFrame.u32PicStride[0], tDstFrame.u32Width, tDstFrame.u32Height, tFrame.stVFrame.enImgFormat, 16);
    CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nImgSize, &tDstFrame.u64PhyAddr[0], (AX_VOID **)(&tDstFrame.u64VirAddr[0]), &BlkId));
    tDstFrame.u64PhyAddr[1] = tDstFrame.u64PhyAddr[0] + WidthTemp * HeightTemp;

    // memset((AX_VOID *)((AX_U32)tDstFrame.u64VirAddr[0]), tAspectRatio.nBgColor, nImgSize * 2);

    ALOGI("tAspectRatio.eMode =%d\n", tAspectRatio.eMode);

    AX_IVPS_CropResize(&tFrame.stVFrame, &tDstFrame, &tAspectRatio);
    tDstFrame.u32Width = WidthTemp;
    tDstFrame.u32Height = HeightTemp;
    printf("OFFSET left:%d right:%d top:%d bottom:%d\n", tDstFrame.s16OffsetLeft, tDstFrame.s16OffsetRight, tDstFrame.s16OffsetTop, tDstFrame.s16OffsetBottom);
    SaveFileExt(&tDstFrame, 0, 0, pSingle->pFilePath, "CropResize");
    ret = AX_POOL_ReleaseBlock(BlkId);
    if (ret)
    {
        ALOGE("[line:%d] IVPS Release BlkId fail, ret=0x%x\n", __LINE__, ret);
    }

    if (pOverlayFile)
    {
        /*
         * 6. AX_IVPS_AlphaBlending
         * Function: Overlay two images.
         * Note: Stride and width should be 16 Byte aligned.
         *       The u64PhyAddr[0] of ptDst should be set. If format is AX_YUV420_SEMIPLANAR, u64PhyAddr[1] should be set.
         *       Overlay between YUV and YUV is not supported.
         */
        AX_IVPS_POINT_S tOffset;
        memset(&tDstFrame, 0, sizeof(tDstFrame));
        tOffset.nX = 128;
        tOffset.nY = 80;
        nImgSize = CalcImgSize(tFrame.stVFrame.u32PicStride[0], tFrame.stVFrame.u32Width, tFrame.stVFrame.u32Height, tFrame.stVFrame.enImgFormat, 16);
        CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nImgSize, &tDstFrame.u64PhyAddr[0], (AX_VOID **)(&tDstFrame.u64VirAddr[0]), &BlkId));
        tDstFrame.u64PhyAddr[1] = tDstFrame.u64PhyAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;

        AX_IVPS_AlphaBlending(&tFrame.stVFrame, &tOverlay.stVFrame, tOffset, &tDstFrame);
        SaveFileExt(&tDstFrame, 0, 0, pSingle->pFilePath, "AlphaBlend");
        ret = AX_POOL_ReleaseBlock(BlkId);
        if (ret)
        {
            ALOGE("[line:%d] IVPS Release BlkId fail, ret=0x%x\n", __LINE__, ret);
        }

        /*
         * 7. AX_IVPS_DrawOSD
         * Function: Draw OSD in cavans.
         * Note: Stride and width should be 16 Byte aligned.
         */
        AX_OSD_BMP_ATTR_S tBmpAttr[32];
        AX_IVPS_RGN_CANVAS_INFO_S tCanvas;
        AX_U32 BMPWidthTemp, BMPHeightTemp;
        AX_S32 i;

        tCanvas.nPhyAddr = tFrame.stVFrame.u64PhyAddr[0];
        tCanvas.nStride = tFrame.stVFrame.u32PicStride[0];
        tCanvas.nW = tFrame.stVFrame.u32Width;
        tCanvas.nH = tFrame.stVFrame.u32Height;
        tCanvas.eFormat = tFrame.stVFrame.enImgFormat;

        for (i = 0; i < 32; i++)
        {
            tBmpAttr[i].bEnable = AX_TRUE;
            tBmpAttr[i].u16Alpha = 512;
            tBmpAttr[i].enRgbFormat = tOverlay.stVFrame.enImgFormat;
            tBmpAttr[i].u32BmpWidth = tOverlay.stVFrame.u32Width;
            tBmpAttr[i].u32BmpHeight = tOverlay.stVFrame.u32Height;
            tBmpAttr[i].u32DstXoffset = tOverlay.stVFrame.s16OffsetLeft + 16 * i;
            tBmpAttr[i].u32DstYoffset = tOverlay.stVFrame.s16OffsetTop + 4 * i;
            tBmpAttr[i].u64PhyAddr = tOverlay.stVFrame.u64PhyAddr[0];

            // tBmpAttr.bColorKey = AX_TRUE;
            // tBmpAttr.bColorKeyInv = AX_FALSE;
            tBmpAttr[i].u32BgColorLo = 0x000000;
            tBmpAttr[i].u32BgColorHi = 0x2020FF;
        }

        BMPWidthTemp = tCanvas.nW;
        BMPHeightTemp = tCanvas.nH;

        AX_IVPS_DrawOsd(&tCanvas, tBmpAttr, 23);

        tFrame.stVFrame.u32Width = BMPWidthTemp;
        tFrame.stVFrame.u32Height = BMPHeightTemp;
        SaveFileExt(&tFrame.stVFrame, 0, 0, pSingle->pFilePath, "OSD"); /*Frame will be changed*/
    }
#endif

#ifdef __IVPS_GDC_OFFLINE_TEST__
    /*
     * 8. AX_IVPS_Dewarp
     * Function: It can scale, rotate, fish eye correction and other functions.
     * Note: The mesh table should be set.
     */
    DewarpSingleSample(&tFrame, pSingle->pFilePath);
#endif

#ifdef __IVPS_NPU_OFFLINE_TEST__
    /*
     * 9. AX_IVPS_CropResizeNPU
     * Function: Crop and Resize by NPU.
     * Note: 1. Images in input and output formats must be consistent.
             2. Crop image number max to 16.
     */
#define NPU_CROP_NUM 1
    AX_S32 idx;
    AX_VIDEO_FRAME_S tNpuDstFrame[16], *ptNpuDstFrame[16];
    AX_IVPS_BOX_S tBox[16];
    AX_BLK NpuBlkId[16];
    AX_U32 nNpuImgSize;
    AX_IVPS_ASPECT_RATIO_S tNpuAspectRatio;
    tNpuAspectRatio.eMode = AX_IVPS_ASPECT_RATIO_AUTO;
    tNpuAspectRatio.eAligns[0] = AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER;
    tNpuAspectRatio.eAligns[1] = AX_IVPS_ASPECT_RATIO_VERTICAL_CENTER;
    tNpuAspectRatio.nBgColor = 0x0000FF;

    for (idx = 0; idx < 10; idx++)
    {
        memset(&tBox[idx], 0, sizeof(AX_IVPS_BOX_S));
        memset(&tNpuDstFrame[idx], 0, sizeof(AX_VIDEO_FRAME_S));
        tNpuDstFrame[idx].u32PicStride[0] = 1920;
        tNpuDstFrame[idx].u32Width = 1920;
        tNpuDstFrame[idx].u32Height = 1080;

        tBox[idx].nX = 200 * idx;
        tBox[idx].nY = 100 * idx;
        tBox[idx].nW = 400;
        tBox[idx].nH = 240;

        nNpuImgSize = CalcImgSize(tNpuDstFrame[idx].u32PicStride[0], tNpuDstFrame[idx].u32Width, tNpuDstFrame[idx].u32Height, tFrame.stVFrame.enImgFormat, 16);
        CHECK_RESULT(BufPoolBlockAddrGet(tFrame.u32PoolId, nNpuImgSize, &tNpuDstFrame[idx].u64PhyAddr[0], (AX_VOID **)(&tNpuDstFrame[idx].u64VirAddr[0]), &NpuBlkId[idx]));

        tNpuDstFrame[idx].u64PhyAddr[1] = tNpuDstFrame[idx].u64PhyAddr[0] + tNpuDstFrame[idx].u32PicStride[0] * tNpuDstFrame[idx].u32Height;
        tNpuDstFrame[idx].u64VirAddr[1] = tNpuDstFrame[idx].u64VirAddr[0] + tNpuDstFrame[idx].u32PicStride[0] * tNpuDstFrame[idx].u32Height;
        ptNpuDstFrame[idx] = &tNpuDstFrame[idx];
    }

    ALOGI("tAspectRatio.eMode =%d\n", tNpuAspectRatio.eMode);

    AX_IVPS_CropResizeNpu(&tFrame.stVFrame, tBox, NPU_CROP_NUM, ptNpuDstFrame, &tNpuAspectRatio);

    for (idx = 0; idx < NPU_CROP_NUM; idx++)
    {
        printf("OFFSET left:%d right:%d top:%d bottom:%d\n", tNpuDstFrame[idx].s16OffsetLeft, tNpuDstFrame[idx].s16OffsetRight,
               tNpuDstFrame[idx].s16OffsetTop, tNpuDstFrame[idx].s16OffsetBottom);

        SaveFileExt(&tNpuDstFrame[idx], grpidx, chnidx, pSingle->pFilePath, "NPUCropResize");
        ret = AX_POOL_ReleaseBlock(NpuBlkId[idx]);
        if (ret)
        {
            ALOGE("[line:%d] IVPS NPU Release BlkId fail, ret=0x%x\n", __LINE__, ret);
        }
    }
#endif

    /****************************ONLINE**********************************/

#ifdef __IVPS_GROUP_TEST__
    pGrp->nIvpsRepeatNum = nRepeatCount;
    pSingle->nIvpsRepeatNum = nRepeatCount;

    ALOGI("nRepeatCount:%d nRegionNum:%d\n", nRepeatCount, nRegionNum);
    AX_MOD_INFO_S tSrcMod = {0};
    AX_MOD_INFO_S tDstMod = {0};

    tSrcMod.enModId = AX_ID_IVPS;
    tSrcMod.s32GrpId = 0;
    tSrcMod.s32ChnId = 0;

    tDstMod.enModId = AX_ID_IVPS;
    tDstMod.s32GrpId = 1;
    tDstMod.s32ChnId = 0;
    AX_SYS_Link(&tSrcMod, &tDstMod);

    CHECK_RESULT(IVPS_StartGrp(&gSampleIvpsGrp));
    CHECK_RESULT(IVPS_StartGrp(&gSampleIvpsSingle));

    if (nRegionNum > 0)
    {
        /*start region with parameter*/
        if (0 != IVPS_StartRegion(nRegionNum))
        {
            ThreadLoopStateSet(AX_TRUE);
        }
        IVPS_ThreadStartRegion(nRegionNum);
    }
    IVPS_ThreadStart(&gSampleIvpsGrp);
    // IVPS_ThreadStart(&gSampleIvpsSingle);
    // IVPS_ThreadStartV2(pSingle, pGrp);

    while (!ThreadLoopStateGet())
    {
        sleep(1);
    }

    if (nRegionNum > 0)
    {
        /*stop region*/
        IVPS_StopRegion();
    }

    /*stop group*/
    IVPS_StopGrp(&gSampleIvpsGrp);

    IVPS_StopGrp(&gSampleIvpsSingle);
#endif

#ifdef __IVPS_VENC_LINK_TEST__
    pGrp->nIvpsRepeatNum = nRepeatCount;

    /* Build IVPS->VENC link relationship */
    AX_MOD_INFO_S tSrcMod = {0};
    AX_MOD_INFO_S tDstMod = {0};
    tSrcMod.enModId = AX_ID_IVPS;
    tSrcMod.s32GrpId = 1;
    tSrcMod.s32ChnId = 0;
    tDstMod.enModId = AX_ID_VENC;
    tDstMod.s32GrpId = 0;
    tDstMod.s32ChnId = 0;
    AX_SYS_Link(&tSrcMod, &tDstMod);

    /* Start IVPS group */
    CHECK_RESULT(IVPS_StartGrp(&gSampleIvpsGrp));

    /* Start VENC group */
    AX_VENC_MOD_ATTR_S tModAttr;
    tModAttr.enVencType = VENC_VIDEO_ENCODER;
    AX_VENC_Init(&tModAttr);
    CHECK_RESULT(IVPS_VencInit(0))

    /* Start the thread of IVPS group */
    IVPS_ThreadStartV2(pGrp, NULL);

    /* Start the thread of VENC group */
    IVPS_VencThreadStart(NULL);

    sleep(10);

    /* Resolution Change and Restart for IVPS */
    IVPS_ChnAttrChange(tSrcMod.s32GrpId, tSrcMod.s32ChnId);

    /* Resolution Change and Restart for VENC */
    IVPS_VencDeInit(0);
    CHECK_RESULT(IVPS_VencInitEx(0));

    while (!ThreadLoopStateGet())
    {
        sleep(1);
    }

    /* Stop VENC group */
    IVPS_VencDeInit(0);
    AX_VENC_Deinit();

    /* Stop IVPS group */
    IVPS_StopGrp(&gSampleIvpsGrp);

    /* Delete IVPS->VENC link relationship */
    tSrcMod.enModId = AX_ID_IVPS;
    tSrcMod.s32GrpId = 1;
    tSrcMod.s32ChnId = 0;
    tDstMod.enModId = AX_ID_VENC;
    tDstMod.s32GrpId = 0;
    tDstMod.s32ChnId = 0;
    AX_SYS_UnLink(&tSrcMod, &tDstMod);
#endif

    /***********************IVPS Destroy*******************************/
    if (tOverlay.stVFrame.u32BlkId[0])
    {
        ret = AX_POOL_ReleaseBlock(tOverlay.stVFrame.u32BlkId[0]);
        if (ret)
        {
            ALOGE("[line:%d] IVPS Release Overlay BlkId fail, ret=0x%x\n", __LINE__, ret);
        }
    }
    /* IVPS release */
    AX_IVPS_Deinit();
error1:

    AX_POOL_Exit();

error0:
    /* sys release */
    AX_SYS_Deinit();

    printf("\nsample test run success\n");
    return 0;
}
