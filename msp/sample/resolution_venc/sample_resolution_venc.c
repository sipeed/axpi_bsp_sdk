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

//#define ENABLE_DYNAMICAL_CROP

/* Max resolution: 5584x4188 */
#define VENC_MAX_WIDTH  5584
#define VENC_MAX_HEIGHT 4188
#define VENC_MIN_WIDTH  136
#define VENC_MIN_HEIGHT 136

#define SAMPLE_RESOLUTION_NAME "SampleResolutionVenc"
#define VENC_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"", SAMPLE_RESOLUTION_NAME, __func__, __LINE__, ##arg); \
    } while(0)

/* roi map delta qp */
static AX_VENC_QPMAP_QP_TYPE_E gEnableQpmap = VENC_QPMAP_DISABLE;
static AX_VENC_RC_CTBRC_MODE_E gCtbRcType = VENC_RC_CTBRC_QUALITY;
static AX_U32 gQpmapBlockUnit = 0;

static AX_S32 gLoopExit = 0;
static AX_S8 *gFileInput, *gFileNextInput;
static AX_S32 gFormat = 0;
static AX_S32 gFormatNext = 0;
static AX_S32 gChangeResolutionNum = -1;
static AX_S32 gEncoderNum = 2;
static AX_S32 gFrameRate = 30;
static AX_S32 gBitRate = 2000; //2000kbps
static AX_S32 gChangeEncodeType = 0;

static pthread_mutex_t gOutputMutex[MAX_VENC_NUM];
static pthread_cond_t gOutputCond[MAX_VENC_NUM];
static AX_BOOL gSaveOneFrameEnd[MAX_VENC_NUM] = {false};

typedef enum _SAMPLE_CODEC_FORMAT
{
    VIDEO_CODEC_H264 = 0,
    VIDEO_CODEC_H265 = 1
} SAMPLE_CODEC_FORMAT_E;

typedef struct _SAMPLE_VENC_ENCODE_PARA
{
    AX_BOOL bThreadStart;
    VENC_CHN VeChn;
    AX_S32 width;
    AX_S32 height;
    AX_S32 stride;
    AX_S32 widthNext;
    AX_S32 heightNext;
    AX_S32 strideNext;
    SAMPLE_CODEC_FORMAT_E enCodecFormat;
    AX_U32 frameSize;
    AX_U32 nextResolutionFrameSize;
    AX_U32 memFrameSize;
    AX_POOL userPoolId;
    AX_U32 sendFrameNum;
} SAMPLE_VENC_ENCODE_PARA_T;

typedef struct _SAMPLE_VENC_GETSTREAM_PARA
{
    AX_BOOL bThreadStart;
    VENC_CHN VeChn;
    AX_U32 getFrameNum;
} SAMPLE_VENC_GETSTREAM_PARA_T;

static SAMPLE_VENC_GETSTREAM_PARA_T gGetStreamPara[MAX_VENC_NUM];
static pthread_t gGetStreamPid[MAX_VENC_NUM];

static SAMPLE_VENC_GETSTREAM_PARA_T gstGetStreamSelectPara = {.bThreadStart = AX_TRUE};
static pthread_t gstGetStreamSelectPid;

static SAMPLE_VENC_ENCODE_PARA_T gEncodePara[MAX_VENC_NUM];
static pthread_t gEncodePid[MAX_VENC_NUM];

AX_U32 TimeDiff(struct timeval end, struct timeval start)
{
   return (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
}


/* get stream task */
void *VencGetStreamProc(void *arg)
{
    AX_S32 s32Ret = -1;
    SAMPLE_VENC_GETSTREAM_PARA_T *pstPara;
    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_T *)arg;
    AX_VENC_STREAM_S stStream;

    AX_S8 esName[50];
    AX_S8 nextEsName[50];
    FILE *pStrm = NULL;
    FILE *pNextStream = NULL;

    if (gChangeEncodeType)
    {
        if (pstPara->VeChn % 2 == 0)
        {
            sprintf(esName, "enc_%d.hevc", pstPara->VeChn);
            sprintf(nextEsName, "enc_%d.h264", pstPara->VeChn);
        }
        else
        {
            sprintf(esName, "enc_%d.h264", pstPara->VeChn);
            sprintf(nextEsName, "enc_%d.hevc", pstPara->VeChn);
        }

        pStrm = fopen(esName, "wb");
        if (pStrm == NULL)
        {
            printf("%s %d: open output file error!\n", SAMPLE_RESOLUTION_NAME, pstPara->VeChn);
            return NULL;
        }

        pNextStream = fopen(nextEsName, "wb");
        if (pNextStream == NULL)
        {
            printf("%s %d: open output next encode type file error!\n", SAMPLE_RESOLUTION_NAME, pstPara->VeChn);
            return NULL;
        }

        memset(&stStream, 0, sizeof(AX_VENC_PACK_S));
        while (AX_TRUE == pstPara->bThreadStart)
        {
            s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, -1);
            if (AX_SUCCESS ==s32Ret)
            {
                pstPara->getFrameNum++;
                if (s32Ret)
                    VENC_LOG("release block err.\n");

                if (stStream.stPack.enType == PT_H265)
                {
                    if (pstPara->VeChn % 2 == 0)
                    {
                        fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
                        fflush(pStrm);
                    }
                    else
                    {
                        fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pNextStream);
                        fflush(pNextStream);
                    }
                }
                if (stStream.stPack.enType == PT_H264)
                {
                    if (pstPara->VeChn % 2 == 0)
                    {
                        fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pNextStream);
                        fflush(pNextStream);
                    }
                    else
                    {
                        fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
                        fflush(pStrm);
                    }
                }
                s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
                if (AX_SUCCESS != s32Ret) {
                    VENC_LOG("AX_VENC_ReleaseStream failed!\n");
                    goto EXIT;
                }
            }
        }
    }
    else
    {
        if (pstPara->VeChn % 2 == 0)
            sprintf(esName, "enc_%d.hevc", pstPara->VeChn);
        else
            sprintf(esName, "enc_%d.h264", pstPara->VeChn);

        pStrm = fopen(esName, "wb");
        if (pStrm == NULL)
        {
            printf("%s %d: open output file error!\n", SAMPLE_RESOLUTION_NAME, pstPara->VeChn);
            return NULL;
        }

        memset(&stStream, 0, sizeof(AX_VENC_PACK_S));
        while (AX_TRUE == pstPara->bThreadStart) {
            s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, -1);
            if (AX_SUCCESS == s32Ret) {
                pstPara->getFrameNum++;

                if (s32Ret)
                    VENC_LOG("release block err.\n");
                fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
                fflush(pStrm);

                s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
                if (AX_SUCCESS != s32Ret) {
                    VENC_LOG("AX_VENC_ReleaseStream failed!\n");
                    goto EXIT;
                }
            }
        }
    }

EXIT:
    if (gChangeEncodeType)
    {
        if (pStrm != NULL) {
            fclose(pStrm);
            pStrm = NULL;
        }
        if (pNextStream != NULL) {
            fclose(pNextStream);
            pNextStream = NULL;
        }
    }
    else
    {
        if (pStrm != NULL) {
            fclose(pStrm);
            pStrm = NULL;
        }
    }

    VENC_LOG("venc %d: Total get %u encoded frames. getStream Exit!\n", pstPara->VeChn, pstPara->getFrameNum);
    return (void *)(intptr_t)s32Ret;
}

static int LoadFrameFromFile(FILE *pFileIn, AX_S32 widthSrc, AX_S32 strideSrc, AX_S32 heightSrc, AX_IMG_FORMAT_E eFmt, AX_VOID *pVaddr)
{
    int i, rows, realRead, readSize;

    if (!pFileIn)
        return -1;
    if (!pVaddr)
        return -1;

    readSize = 0;

    switch(eFmt) {
        case AX_YUV420_PLANAR:
            rows = heightSrc;
            for (i = 0; i < rows; i++) {
                realRead = fread(pVaddr, 1, strideSrc, pFileIn);
                if (realRead < strideSrc)
                    break;
                readSize += realRead;
                pVaddr += strideSrc;
            }
            rows = heightSrc;
            for (i = 0; i < rows; i++) {
                realRead = fread(pVaddr, 1, strideSrc >> 1, pFileIn);
                if (realRead < (strideSrc >> 1))
                    break;
                readSize += realRead;
                pVaddr += (strideSrc >> 1);
            }
            break;
        case AX_YUV420_SEMIPLANAR:
        case AX_YUV420_SEMIPLANAR_VU:
            rows = heightSrc * 3 / 2;
            for (i = 0; i < rows; i++) {
                realRead = fread(pVaddr, 1, strideSrc, pFileIn);
                if (realRead < strideSrc)
                    break;
                readSize += realRead;
                pVaddr += strideSrc;
            }
            break;
        case AX_YUV422_INTERLEAVED_YUYV:
        case AX_YUV422_INTERLEAVED_UYVY:
            rows = heightSrc;
            for (i = 0; i < rows; i++) {
                realRead = fread(pVaddr, 1, strideSrc, pFileIn);
                if (realRead < strideSrc)
                    break;
                readSize += realRead;
                pVaddr += strideSrc;
            }
            break;
        default:
            VENC_LOG("Invalid format, eFmt = %d\n", eFmt);
    }

    return readSize;
}

static AX_BOOL VencPoolInit(AX_U32 frameSize,VENC_CHN VeChn)
{
    /* use pool to alloc buffer */
    AX_POOL_CONFIG_T stPoolConfig;
    AX_POOL s32UserPoolId;

    memset(&stPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    stPoolConfig.MetaSize = 512;
    stPoolConfig.BlkCnt = 10;
    stPoolConfig.BlkSize = frameSize;
    stPoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(stPoolConfig.PartitionName, 0, sizeof(stPoolConfig.PartitionName));
    strcpy(stPoolConfig.PartitionName, "anonymous");

    s32UserPoolId = AX_POOL_CreatePool(&stPoolConfig);
    if (AX_INVALID_POOLID == s32UserPoolId) {
        VENC_LOG("VeChn:%d Create pool err.\n",VeChn);
    }

    return s32UserPoolId;
}

/* encode task */
void *VencEncodeProc(void *arg)
{
    AX_S32 s32Ret = 0;
    AX_S32 readSize;
    AX_BOOL bChangeResolution = AX_FALSE;
    struct timeval timeFrameStart;
    struct timeval timeFrameEnd;
    AX_POOL s32UserPoolId;
    AX_BLK BlkId;
    AX_U32 frameSize;
    AX_U32 nextResolutionFrameSize;
    AX_U32 memFrameSize;

    SAMPLE_VENC_ENCODE_PARA_T *pEncodePara = NULL;
    pEncodePara = (SAMPLE_VENC_ENCODE_PARA_T *)arg;

    s32UserPoolId = pEncodePara->userPoolId;
    frameSize = pEncodePara->frameSize;
    nextResolutionFrameSize = pEncodePara->nextResolutionFrameSize;
    memFrameSize = pEncodePara->memFrameSize;

    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    AX_VIDEO_FRAME_INFO_S stFrame;

    /* input yuv file */
    FILE *fFileIn = NULL;
    fFileIn = fopen(gFileInput, "rb");
    if (fFileIn == NULL)
    {
        printf("%s %d: open input file error!\n", SAMPLE_RESOLUTION_NAME, pEncodePara->VeChn);
        return NULL;
    }

    /* input next resolution yuv file */
    FILE *fFileNextIn = NULL;
    fFileNextIn = fopen(gFileNextInput, "rb");
    if (fFileNextIn == NULL)
    {
        printf("%s %d: open next input file error!\n", SAMPLE_RESOLUTION_NAME, pEncodePara->VeChn);
        return NULL;
    }

    stRecvParam.s32RecvPicNum = -1;
    s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("Encoder thread exit, ret=%x\n", s32Ret);
        goto EXIT;
    }

    //MUST clear frame struct before use
    memset(&stFrame, 0, sizeof(AX_VIDEO_FRAME_INFO_S));
    while (pEncodePara->bThreadStart) {
        if (!bChangeResolution)
        {
            BlkId = AX_POOL_GetBlock(s32UserPoolId, memFrameSize, NULL);
            if (AX_INVALID_BLOCKID == BlkId) {
                //VENC_LOG("VeChn:%d,Get block fail.s32UserPoolId=%d,frameSize=0x%x\n",pEncodePara->VeChn,s32UserPoolId,frameSize);
                usleep(5*1000);
                continue;
            }
            stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
            stFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);
            stFrame.stVFrame.enImgFormat = gFormat;
            stFrame.stVFrame.u32Width = pEncodePara->width;
            stFrame.stVFrame.u32Height = pEncodePara->height;
            stFrame.stVFrame.u32PicStride[0] = pEncodePara->stride;
            stFrame.stVFrame.u32BlkId[0] = BlkId;
            stFrame.stVFrame.u32BlkId[1] = 0; //must set 0 if not used
            stFrame.stVFrame.u32BlkId[2] = 0; //must set 0 if not used

            /* read frame data from yuv file */
            readSize = LoadFrameFromFile(fFileIn,
                                     pEncodePara->width,
                                     pEncodePara->stride,
                                     pEncodePara->height,
                                     gFormat,
                                     (void *)stFrame.stVFrame.u64VirAddr[0]);

            if (readSize <= 0) {
                VENC_LOG("Warning: read frame size : %d less than %d\n", readSize, memFrameSize);
            }
            if (feof(fFileIn)) {
                VENC_LOG("End of input file!\n");
                /* no more frames, stop encoder */
                goto EXIT;
            }
        }
        else
        {
            BlkId = AX_POOL_GetBlock(s32UserPoolId, memFrameSize, NULL);
            if (AX_INVALID_BLOCKID == BlkId) {
                //VENC_LOG("VeChn:%d,Get block fail.s32UserPoolId=%d,frameSize=0x%x\n",pEncodePara->VeChn,s32UserPoolId,memFrameSize);
                usleep(5*1000);
                continue;
            }
            stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
            stFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);
            stFrame.stVFrame.enImgFormat = gFormatNext;
            stFrame.stVFrame.u32Width = pEncodePara->widthNext;
            stFrame.stVFrame.u32Height = pEncodePara->heightNext;
            stFrame.stVFrame.u32PicStride[0] = pEncodePara->strideNext;
            stFrame.stVFrame.u32BlkId[0] = BlkId;
            stFrame.stVFrame.u32BlkId[1] = 0;//must set 0 if not used
            stFrame.stVFrame.u32BlkId[2] = 0;//must set 0 if not used

            /* read frame data from yuv file */
            readSize = LoadFrameFromFile(fFileNextIn,
                                     pEncodePara->widthNext,
                                     pEncodePara->strideNext,
                                     pEncodePara->heightNext,
                                     gFormatNext,
                                     (void *)stFrame.stVFrame.u64VirAddr[0]);

            if (readSize <= 0) {
                VENC_LOG("Warning: read frame size : %d less than %d\n", readSize, memFrameSize);
            }
            if (feof(fFileNextIn)) {
                VENC_LOG("End of input file!\n");
                /* no more frames, stop encoder */
                goto EXIT;
            }
        }

        /* the input frame sequence number */
        stFrame.stVFrame.u64SeqNum = pEncodePara->sendFrameNum + 1;
        stFrame.bEof = AX_FALSE;

        if (stFrame.stVFrame.enImgFormat == AX_YUV420_PLANAR) {
            stFrame.stVFrame.u32PicStride[1] = stFrame.stVFrame.u32PicStride[0] / 2;
            stFrame.stVFrame.u32PicStride[2] = stFrame.stVFrame.u32PicStride[0] / 2;
        } else if (stFrame.stVFrame.enImgFormat == AX_YUV420_SEMIPLANAR || stFrame.stVFrame.enImgFormat == AX_YUV420_SEMIPLANAR_VU) {
            stFrame.stVFrame.u32PicStride[1] = stFrame.stVFrame.u32PicStride[0];
            stFrame.stVFrame.u32PicStride[2] = 0;
        } else if (stFrame.stVFrame.enImgFormat == AX_YUV422_INTERLEAVED_YUYV || stFrame.stVFrame.enImgFormat == AX_YUV422_INTERLEAVED_YUYV) {
            stFrame.stVFrame.u32PicStride[1] = 0;
            stFrame.stVFrame.u32PicStride[2] = 0;
        }

        s32Ret = AX_VENC_SendFrame(pEncodePara->VeChn, &stFrame, -1);
        if (AX_SUCCESS != s32Ret) {
            VENC_LOG("AX_VENC_SendFrame failed, ret=%x\n", s32Ret);
            s32Ret = AX_POOL_ReleaseBlock(stFrame.stVFrame.u32BlkId[0]);
            if (s32Ret)
                VENC_LOG("release block err.\n");
            continue;
        }

        pEncodePara->sendFrameNum++;

        if (pEncodePara->sendFrameNum == gChangeResolutionNum)
        {
            VENC_LOG("%s %d: Want to encode %d frames, than change to another resolution: %d x %d!\n",
                   SAMPLE_RESOLUTION_NAME, pEncodePara->VeChn, pEncodePara->sendFrameNum, pEncodePara->widthNext, pEncodePara->heightNext);

            s32Ret = AX_VENC_StopRecvFrame(pEncodePara->VeChn);
            if (AX_SUCCESS != s32Ret)
            {
                VENC_LOG("%s %d:  AX_VENC_StopRecvFrame failed, ret=%x\n", SAMPLE_RESOLUTION_NAME, pEncodePara->VeChn, s32Ret);
            }
            AX_VENC_CHN_ATTR_S stVencChnAttr;
            memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
            AX_VENC_GetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (gChangeEncodeType)
            {
                if (0 == (pEncodePara->VeChn % 2))
                {
                    stVencChnAttr.stVencAttr.enType = PT_H264;
                    stVencChnAttr.stVencAttr.enProfile = VENC_H264_BASE_PROFILE;
                    stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5;
                    pEncodePara->enCodecFormat = VIDEO_CODEC_H264;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                    stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32Gop = 30;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32SrcFrameRate = gFrameRate;
                    stVencChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = gFrameRate;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32BitRate = gBitRate;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32MinQp = 0;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32MaxQp = 51;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32MinIQp = 0;
                    stVencChnAttr.stRcAttr.stH264Cbr.u32MaxIQp = 51;
                    stVencChnAttr.stRcAttr.stH264Cbr.s32IntraQpDelta = -2;
                    stVencChnAttr.stRcAttr.stH264Cbr.stQpmapInfo.enQpmapQpType = gEnableQpmap;
                    stVencChnAttr.stRcAttr.stH264Cbr.stQpmapInfo.enCtbRcMode = gCtbRcType;
                    stVencChnAttr.stRcAttr.stH264Cbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                }
                else
                {
                    stVencChnAttr.stVencAttr.enType = PT_H265;
                    stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_10_PROFILE;
                    stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_5;
                    pEncodePara->enCodecFormat = VIDEO_CODEC_H265;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                    stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32Gop = 30;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32SrcFrameRate = gFrameRate;
                    stVencChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = gFrameRate;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32BitRate = gBitRate;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32MinQp = 0;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32MaxQp = 51;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32MinIQp = 0;
                    stVencChnAttr.stRcAttr.stH265Cbr.u32MaxIQp = 51;
                    stVencChnAttr.stRcAttr.stH265Cbr.s32IntraQpDelta = -2;
                    stVencChnAttr.stRcAttr.stH265Cbr.stQpmapInfo.enQpmapQpType = gEnableQpmap;
                    stVencChnAttr.stRcAttr.stH265Cbr.stQpmapInfo.enCtbRcMode = gCtbRcType;
                    stVencChnAttr.stRcAttr.stH265Cbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                }
            }
            stVencChnAttr.stVencAttr.u32PicWidthSrc = pEncodePara->widthNext;
            stVencChnAttr.stVencAttr.u32PicHeightSrc = pEncodePara->heightNext;
            stVencChnAttr.stVencAttr.s32StopWaitTime = -1;

#ifdef ENABLE_DYNAMICAL_CROP
            stVencChnAttr.stVencAttr.u32CropOffsetX = 320;
            stVencChnAttr.stVencAttr.u32CropOffsetY = 320;
            stVencChnAttr.stVencAttr.u32CropWidth = 640;
            stVencChnAttr.stVencAttr.u32CropHeight = 640;
#endif

            s32Ret = AX_VENC_SetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (AX_SUCCESS != s32Ret)
            {
                VENC_LOG("%s %d:  AX_VENC_SetChnAttr failed, ret=%x\n", SAMPLE_RESOLUTION_NAME, pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }
            bChangeResolution = AX_TRUE;

            s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
            if (AX_SUCCESS != s32Ret) {
                VENC_LOG("Encoder thread exit, ret=%x\n", s32Ret);
                goto EXIT;
            }
        }
    }

EXIT:

    if (NULL != fFileIn) {
        fclose(fFileIn);
        fFileIn = NULL;
    }
    if (NULL != fFileNextIn) {
        fclose(fFileNextIn);
        fFileNextIn = NULL;
    }

    VENC_LOG("venc %d - Total input %u frames, Encoder exit!\n", pEncodePara->VeChn, pEncodePara->sendFrameNum);

    return (void *)(intptr_t)s32Ret;
}

AX_S32 SampleResolutionVenc(AX_S32 widthSrc, AX_S32 strideSrc, AX_S32 heightSrc, AX_S32 changeNum, AX_S32 widthNext, AX_S32 strideNext, AX_S32 heightNext)
{
    AX_S32 s32Ret = -1;
    AX_U32 u32StartTime = 1;
    AX_U32 FrameSize = 0;
    AX_U32 NextResolutionFrameSize = 0;
    AX_POOL userPoolId;

    if (!strideSrc)
    {
        strideSrc = widthSrc;
        if (gFormat == AX_YUV422_INTERLEAVED_YUYV || gFormat == AX_YUV422_INTERLEAVED_UYVY) // for
            strideSrc *= 2;
    }
    if (!strideNext)
    {
        strideNext = widthNext;
        if (gFormatNext == AX_YUV422_INTERLEAVED_YUYV || gFormatNext == AX_YUV422_INTERLEAVED_UYVY) // for
            strideNext *= 2;
    }

    switch (gFormatNext) {
    case 0:
        NextResolutionFrameSize = strideNext * heightNext * 3 / 2;
        break;
    case 1:
        NextResolutionFrameSize = strideNext * heightNext * 3 / 2;
        break;
    case 2:
        NextResolutionFrameSize = strideNext * heightNext * 3 / 2;
        break;
    case 3:
        NextResolutionFrameSize = strideNext * heightNext;
        break;
    case 4:
        NextResolutionFrameSize = strideNext * heightNext;
        break;
    default:
        VENC_LOG("gFormatNext:%d", gFormatNext);
        return AX_ERR_VENC_ILLEGAL_PARAM;
    }

    switch (gFormat) {
    case 0:
        FrameSize = strideSrc * heightSrc * 3 / 2;
        break;
    case 1:
        FrameSize = strideSrc * heightSrc * 3 / 2;
        break;
    case 2:
        FrameSize = strideSrc * heightSrc * 3 / 2;
        break;
    case 3:
        FrameSize = strideSrc * heightSrc;
        break;
    case 4:
        FrameSize = strideSrc * heightSrc;
        break;
    default:
        VENC_LOG("gFormat:%d", gFormat);
        return AX_ERR_VENC_ILLEGAL_PARAM;
    }

    AX_U32 MemFrameSize = FrameSize < NextResolutionFrameSize ? NextResolutionFrameSize : FrameSize;


    for (int i = 0; i < gEncoderNum; i++)
    {
        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
        /* set codec format */
        if (0 == (i % 2)) {
            stVencChnAttr.stVencAttr.enType = PT_H265;
            gEncodePara[i].enCodecFormat = VIDEO_CODEC_H265;
        }
        else {
            stVencChnAttr.stVencAttr.enType = PT_H264;
            gEncodePara[i].enCodecFormat = VIDEO_CODEC_H264;
        }

        stVencChnAttr.stVencAttr.u32MaxPicWidth = VENC_MAX_WIDTH;
        stVencChnAttr.stVencAttr.u32MaxPicHeight = VENC_MAX_HEIGHT;
        stVencChnAttr.stVencAttr.u32PicWidthSrc = widthSrc;   /*the input picture width*/
        stVencChnAttr.stVencAttr.u32PicHeightSrc = heightSrc; /*the input picture height*/

#ifdef ENABLE_DYNAMICAL_CROP
        stVencChnAttr.stVencAttr.u32CropOffsetX = 0;
        stVencChnAttr.stVencAttr.u32CropOffsetY = 0;
        stVencChnAttr.stVencAttr.u32CropWidth = 640;
        stVencChnAttr.stVencAttr.u32CropHeight = 640;
#endif


        if (stVencChnAttr.stVencAttr.enType == PT_H264)
        {
            stVencChnAttr.stVencAttr.enProfile = VENC_H264_MAIN_PROFILE;                  //main profile
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
        }
        else if (stVencChnAttr.stVencAttr.enType == PT_H265)
        {
            stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_PROFILE;                  //main profile
            stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;
            stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;
        } else
        {
            printf("%s %d: invalid codec format!\n", SAMPLE_RESOLUTION_NAME, i);
            goto END;
        }



        /* u32Mode: 1, not link with isp */
        stVencChnAttr.stVencAttr.enLinkMode = AX_NONLINK_MODE;
        /* how many frames take to do GDR */
        stVencChnAttr.stVencAttr.u32GdrDuration = 0;
        /* stream buffer configure */
        stVencChnAttr.stVencAttr.u32BufSize = 0;

        /* RC setting */
        if (PT_H265 == stVencChnAttr.stVencAttr.enType)
        {
            AX_VENC_H265_CBR_S stH265Cbr;
            stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
            stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
            stH265Cbr.u32Gop = 30;
            stH265Cbr.u32SrcFrameRate = gFrameRate;  /* input frame rate */
            stH265Cbr.fr32DstFrameRate = gFrameRate; /* target frame rate */
            stH265Cbr.u32BitRate = gBitRate;
            stH265Cbr.u32MinQp = 0;
            stH265Cbr.u32MaxQp = 51;
            stH265Cbr.u32MinIQp = 0;
            stH265Cbr.u32MaxIQp = 51;
            stH265Cbr.s32IntraQpDelta = -2;
            stH265Cbr.stQpmapInfo.enQpmapQpType = VENC_QPMAP_DISABLE;
            stH265Cbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
            stH265Cbr.stQpmapInfo.enCtbRcMode = gCtbRcType;
            memcpy(&stVencChnAttr.stRcAttr.stH265Cbr, &stH265Cbr, sizeof(AX_VENC_H265_CBR_S));
        }
        else if (PT_H264 == stVencChnAttr.stVencAttr.enType)
        {
            AX_VENC_H264_CBR_S stH264Cbr;
            stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
            stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
            stH264Cbr.u32Gop = 30;
            stH264Cbr.u32SrcFrameRate = gFrameRate;  /* input frame rate */
            stH264Cbr.fr32DstFrameRate = gFrameRate; /* target frame rate */
            stH264Cbr.u32BitRate = gBitRate;
            stH264Cbr.u32MinQp = 0;
            stH264Cbr.u32MaxQp = 51;
            stH264Cbr.u32MinIQp = 0;
            stH264Cbr.u32MaxIQp = 51;
            stH264Cbr.s32IntraQpDelta = -2;
            stH264Cbr.stQpmapInfo.enQpmapQpType = VENC_QPMAP_DISABLE;
            stH264Cbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
            stH264Cbr.stQpmapInfo.enCtbRcMode = gCtbRcType;
            memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_S));
        }
        stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;

        userPoolId = VencPoolInit(MemFrameSize,i);
        if(userPoolId == AX_INVALID_POOLID)
        {
            VENC_LOG("VeChn %d VencPoolInit failed\n", i);
            goto END;
        }

        s32Ret = AX_VENC_CreateChn(i, &stVencChnAttr);

        if (AX_SUCCESS != s32Ret) {
            VENC_LOG("VeChn %d AX_VENC_CreateChn failed, ret=0x%x\n", i,s32Ret);
            goto END;
        }

        /* create get output stream thread */
        gGetStreamPara[i].VeChn = i;
        gGetStreamPara[i].getFrameNum = 0;
        gGetStreamPara[i].bThreadStart = AX_TRUE;
        pthread_create(&gGetStreamPid[i], NULL, VencGetStreamProc, (void *)&gGetStreamPara[i]);
        sleep(1);

        /* init frame/stream buffer parameters */
        gEncodePara[i].width = widthSrc;
        gEncodePara[i].height = heightSrc;
        gEncodePara[i].stride = strideSrc;
        gEncodePara[i].widthNext = widthNext;
        gEncodePara[i].heightNext = heightNext;
        gEncodePara[i].strideNext = strideNext;
        gEncodePara[i].bThreadStart = AX_TRUE;
        gEncodePara[i].VeChn = i;
        gEncodePara[i].frameSize = FrameSize;
        gEncodePara[i].nextResolutionFrameSize = NextResolutionFrameSize;
        gEncodePara[i].memFrameSize = MemFrameSize;
        gEncodePara[i].sendFrameNum = 0;
        gEncodePara[i].userPoolId = userPoolId;
        pthread_create(&gEncodePid[i], NULL, VencEncodeProc, (void *)&gEncodePara[i]);
    }

    ///////////////////////////////////////////////////////
    while (!gLoopExit) {
        sleep(2);
    }

END:
    for (int i = 0; i < gEncoderNum; i++) {
        if (AX_TRUE == gEncodePara[i].bThreadStart) {
            gEncodePara[i].bThreadStart = AX_FALSE;
            pthread_join(gEncodePid[i], NULL);
        }

        AX_VENC_StopRecvFrame(i);

        /* VENC release */
        AX_VENC_DestroyChn(i);

        if (AX_TRUE == gGetStreamPara[i].bThreadStart) {
            gGetStreamPara[i].bThreadStart = AX_FALSE;
            pthread_join(gGetStreamPid[i], NULL);
        }

    }

    return 0;
}

void SampleUsage(void)
{
    printf("command:\n");
    printf("\t-w: Raw input frame width.(MAX 5584)\n");
    printf("\t-h: Raw input frame height.(MAX 4188)\n");
    printf("\t-d: Raw input frame stride.(MAX 5584)\n");
    printf("\t-i: yuv file to encode.\n");
    printf("\t-t: input frame format, 0: yuv420p 1: nv12 2: nv21 3: YUYV422 4: UYVY422.(default 0)\n");
    printf("\t-n: total encoder number change to next resolution(Max support 1 way 4k@60fps).(default 50)\n");
    printf("\t-b: enable encoder type change, 0: disable, 1: enable\n");
    printf("\t-W: Next resolution raw input frame width.(MAX 5584)\n");
    printf("\t-H: Next resolution raw input frame height.(MAX 4188)\n");
    printf("\t-D: Next resolution raw input frame stride.(MAX 5584)\n");
    printf("\t-I: Next resolution yuv file to encode.\n");
    printf("\t-T: Next resolution input frame format, 0: yuv420p 1: nv12 2: nv21 3: YUYV422 4: UYVY422.(default 0)\n");
}

static void SigInt(int sigNo)
{
    VENC_LOG("Catch signal %d\n", sigNo);
    gLoopExit = 1;
}

int main(int argc, char *argv[])
{
    VENC_LOG("Build at %s %s\n", __DATE__, __TIME__);

    AX_S32 c;
    AX_S32 isExit = 1;
    AX_S32 rawWidth = 640;
    AX_S32 rawHeight = 480;
    AX_S32 rawStride = 0;
    AX_S32 widthNext = 352;
    AX_S32 heightNext = 288;
    AX_S32 strideNext = 0;
    AX_S32 s32Ret = -1;
    AX_VENC_MOD_ATTR_S stModAttr = {.enVencType = VENC_VIDEO_ENCODER};

    signal(SIGINT, SigInt);

    while ((c = getopt(argc, argv, "w:h:d:i:t:n:b:W:H:D:I:T:")) != -1)
    {
        isExit = 0;
        switch (c)
        {
            case 'w':
                rawWidth = atoi(optarg);
                break;
            case 'h':
                rawHeight = atoi(optarg);
                break;
            case 'd':
                rawStride = atoi(optarg);
                break;
            case 'i':
                gFileInput = optarg;
                break;
            case 't':
                gFormat = atoi(optarg);
                break;
            case 'n':
                gChangeResolutionNum = atoi(optarg);
                break;
            case 'b':
                gChangeEncodeType = atoi(optarg);
                break;
            case 'W':
                widthNext = atoi(optarg);
                break;
            case 'H':
                heightNext = atoi(optarg);
                break;
            case 'D':
                strideNext = atoi(optarg);
                break;
            case 'I':
                gFileNextInput = optarg;
                break;
            case 'T':
                gFormatNext = atoi(optarg);
                break;
            default:
                printf("invalid parameter\n");
                isExit = 1;
                break;
        }
    }

    if (isExit)
    {
        SampleUsage();
        exit(0);
    }

    AX_SYS_Init();

    s32Ret = AX_POOL_Exit();
    if(s32Ret) {
        VENC_LOG("AX_POOL_Exit failed! Error Code:0x%X\n", s32Ret);
        goto err0;
    }

    s32Ret = AX_VENC_Init(&stModAttr);
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_VENC_Init error.\n");
        goto err1;
    }

    s32Ret = SampleResolutionVenc(rawWidth, rawStride, rawHeight, gChangeResolutionNum, widthNext, strideNext, heightNext);
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("SampleVencUsage1 error.\n");
        goto err2;
    }

    s32Ret = AX_VENC_Deinit();
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_VENC_Deinit failed! Error Code:0x%X\n", s32Ret);
    }

    s32Ret = AX_SYS_Deinit();
    if (AX_SUCCESS != s32Ret) {
        VENC_LOG("AX_SYS_Deinit failed! Error Code:0x%X\n", s32Ret);
    }

    return 0;

err2:
    AX_VENC_Deinit();
err1:
    AX_POOL_Exit();
err0:
    AX_SYS_Deinit();

    return -1;
}
