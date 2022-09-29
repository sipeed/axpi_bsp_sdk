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

#include "ax_venc_api.h"
#include "ax_ivps_api.h"
#include "ax_isp_api.h"

#include "common_sys.h"
#include "common_vin.h"
#include "common_cam.h"
#include "common_venc.h"
#include "sample_vin_ivps_venc.h"


#define SAMPLE_IVPS_GROUP_NUM (3)
#define SAMPLE_IVPS_CHN_NUM   (1)
#define SAMPLE_VENC_CHN_NUM   (3)

//#define SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS

#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s:%d "str"\n", __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s:%d Error! "str"\n", __func__, __LINE__, ##arg); \
    }while(0)


typedef struct _stVencGetStreamParam
{
    AX_S32 VeChn;
    AX_BOOL bThreadStart;
    AX_PAYLOAD_TYPE_E ePayloadType;
} VENC_GETSTREAM_PARAM_T;

typedef struct _stRCInfo
{
    SAMPLE_VENC_RC_E eRCType;
    AX_U32 nMinQp;
    AX_U32 nMaxQp;
    AX_U32 nMinIQp;
    AX_U32 nMaxIQp;
    AX_S32 nIntraQpDelta;
} RC_INFO_T;

typedef struct _stVideoConfig
{
    AX_PAYLOAD_TYPE_E ePayloadType;
    AX_U32 nGOP;
    AX_U32 nSrcFrameRate;
    AX_U32 nDstFrameRate;
    AX_U32 nStride;
    AX_S32 nInWidth;
    AX_S32 nInHeight;
    AX_S32 nOutWidth;
    AX_S32 nOutHeight;
    AX_S32 nOffsetCropX;
    AX_S32 nOffsetCropY;
    AX_S32 nOffsetCropW;
    AX_S32 nOffsetCropH;
    AX_IMG_FORMAT_E eImgFormat;
    RC_INFO_T stRCInfo;
    AX_S32 nBitrate;
} VIDEO_CONFIG_T;

typedef enum {
    SYS_CASE_NONE  = -1,
    SYS_CASE_SINGLE_OS04A10  = 0,
    SYS_CASE_SINGLE_IMX334   = 1,
    SYS_CASE_SINGLE_GC4653   = 2,
    SYS_CASE_DUAL_OS04A10    = 3,
    SYS_CASE_SINGLE_OS08A20  = 4,
    SYS_CASE_SINGLE_OS04A10_ONLINE  = 5,
    SYS_CASE_SINGLE_DVP  = 6,
    SYS_CASE_BUTT
} COMMON_SYS_CASE_E;

#ifdef __AX620U_PROJECT__
static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Sdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 4},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 4},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 5},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 5},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 5},
};
static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10OnlineSdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 3},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR,6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
};
#else
static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Sdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 8},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 8},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 5},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 5},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 5},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10OnlineSdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 3},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 8},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 3},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 2},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 2},
};
#endif

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Hdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 5},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 4},
    {720, 576, 720, AX_YUV420_SEMIPLANAR, 4},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10OnlineHdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 6},     /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 8},     /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 3},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 2},
    {720, 576, 720, AX_YUV420_SEMIPLANAR, 2},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Sdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleImx334Hdr[] = {

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

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs08a20Sdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_12BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs08a20Hdr[] = {

    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_10BPP, 15},     /*vin raw10 use */
    {3840, 2160, 3840, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {3840, 2160, 3840, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 3},
    {960, 540, 960, AX_YUV420_SEMIPLANAR, 3},

};

static CAMERA_T gCams[MAX_CAMERAS] = {0};

static volatile AX_S32 gLoopExit = 0;
static AX_S32 g_isp_force_loop_exit = 0;

static VENC_GETSTREAM_PARAM_T gGetStreamPara[SAMPLE_VENC_CHN_NUM];
static pthread_t gGetStreamPid[SAMPLE_VENC_CHN_NUM];

/* venc get stream task */
static void *VencGetStreamProc(void *arg)
{
    AX_S32 s32Ret = -1;
    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    VENC_GETSTREAM_PARAM_T *pstPara;
    pstPara = (VENC_GETSTREAM_PARAM_T *)arg;
    AX_VENC_STREAM_S stStream;
    AX_S16 syncType = -1;
    FILE *pStrm = NULL;
    AX_S8 esName[150];
    AX_U32 totalGetStream = 0;

    s32Ret = AX_VENC_StartRecvFrame(pstPara->VeChn, &stRecvParam);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("AX_VENC_StartRecvFrame failed, s32Ret:0x%x\n", s32Ret);
        return NULL;
    }

    memset(esName, 0, 150);

    if (pstPara->ePayloadType == PT_H265)
        sprintf((char*)esName, "enc_%d.265", pstPara->VeChn);
    else if (pstPara->ePayloadType == PT_H264)
        sprintf((char*)esName, "enc_%d.264", pstPara->VeChn);

    pStrm = fopen((char*)esName, "wb");

    if (pStrm == NULL) {
        SAMPLE_ERR_LOG("Open output file error!\n");
        return NULL;
    }

    memset(&stStream, 0, sizeof(AX_VENC_STREAM_S));

    while (AX_TRUE == pstPara->bThreadStart) {
        s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, syncType);

        if (AX_SUCCESS == s32Ret) {
            totalGetStream++;

            //默认保存30帧
            if(totalGetStream <= 30){
                fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
                fflush(pStrm);
			}

            s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
            if (AX_SUCCESS != s32Ret) {
                //SAMPLE_ERR_LOG("VencChn %d: AX_VENC_ReleaseStream failed!s32Ret:0x%x\n",pstPara->VeChn,s32Ret);
                goto EXIT;
            }
        }
    }

EXIT:

    if (pStrm != NULL) {
        fclose(pStrm);
        pStrm = NULL;
    }

    SAMPLE_LOG("VencChn %d: Total get %u encoded frames. getStream Exit!\n", pstPara->VeChn, totalGetStream);
    return NULL;
}
static AX_S32 SampleVencInit()
{
    AX_VENC_CHN_ATTR_S stVencChnAttr;
    VIDEO_CONFIG_T config = {0};
    AX_S32 VencChn = 0,s32Ret = 0;

    AX_VENC_MOD_ATTR_S stModAttr;
    stModAttr.enVencType = VENC_MULTI_ENCODER;

    s32Ret = AX_VENC_Init(&stModAttr);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("AX_VENC_Init failed, s32Ret:0x%x", s32Ret);
        return s32Ret;
    }

    config.stRCInfo.eRCType = VENC_RC_CBR;
    config.nGOP = 30;
    config.nBitrate = 2000;
    config.stRCInfo.nMinQp = 10;
    config.stRCInfo.nMaxQp = 51;
    config.stRCInfo.nMinIQp = 10;
    config.stRCInfo.nMaxIQp = 51;
    config.stRCInfo.nIntraQpDelta = -2;
    config.nOffsetCropX = 0;
    config.nOffsetCropY = 0;
    config.nOffsetCropW = 0;
    config.nOffsetCropH = 0;

    for(VencChn = 0;VencChn < SAMPLE_VENC_CHN_NUM;VencChn++)
    {
        switch(VencChn)
        {
        case 0:
            config.nInWidth = 2688;
            config.nInHeight = 1520;
            config.nStride = 2688;
            config.ePayloadType = PT_H264;
            config.nSrcFrameRate = 30;
            config.nDstFrameRate = 30;
            break;
        case 1:
            config.nInWidth = 1920;
            config.nInHeight = 1080;
            config.nStride = 1080;
            config.ePayloadType = PT_H264;
            config.nSrcFrameRate = 30;
            config.nDstFrameRate = 30;
            break;
        case 2:
            config.nInWidth = 720;
            config.nInHeight = 576;
            config.nStride = 720;
            config.ePayloadType = PT_H264;
            config.nSrcFrameRate = 30;
            config.nDstFrameRate = 30;
            break;
        }

        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));

        stVencChnAttr.stVencAttr.u32MaxPicWidth = 0;
        stVencChnAttr.stVencAttr.u32MaxPicHeight = 0;

        stVencChnAttr.stVencAttr.u32PicWidthSrc = config.nInWidth;   /*the picture width*/
        stVencChnAttr.stVencAttr.u32PicHeightSrc = config.nInHeight; /*the picture height*/

        stVencChnAttr.stVencAttr.u32CropOffsetX = config.nOffsetCropX;
        stVencChnAttr.stVencAttr.u32CropOffsetY = config.nOffsetCropY;
        stVencChnAttr.stVencAttr.u32CropWidth = config.nOffsetCropW;
        stVencChnAttr.stVencAttr.u32CropHeight = config.nOffsetCropH;
        stVencChnAttr.stVencAttr.u32VideoRange = 1; /* 0: Narrow Range(NR), Y[16,235], Cb/Cr[16,240]; 1: Full Range(FR), Y/Cb/Cr[0,255] */

        SAMPLE_LOG("VencChn %d:w:%d, h:%d, s:%d, Crop:(%d, %d, %d, %d) rcType:%d, payload:%d"
            , VencChn
            , stVencChnAttr.stVencAttr.u32PicWidthSrc
            , stVencChnAttr.stVencAttr.u32PicHeightSrc
            , config.nStride
            , stVencChnAttr.stVencAttr.u32CropOffsetX
            , stVencChnAttr.stVencAttr.u32CropOffsetY
            , stVencChnAttr.stVencAttr.u32CropWidth
            , stVencChnAttr.stVencAttr.u32CropHeight
            , config.stRCInfo.eRCType
            , config.ePayloadType);

        stVencChnAttr.stVencAttr.u32BufSize = config.nStride * config.nInHeight * 3/2; /*stream buffer size*/
        stVencChnAttr.stVencAttr.u32MbLinesPerSlice = 0; /*get stream mode is slice mode or frame mode?*/
        stVencChnAttr.stVencAttr.enLinkMode = AX_LINK_MODE;
        stVencChnAttr.stVencAttr.u32GdrDuration = 0;
        /* GOP Setting */
        stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;

        stVencChnAttr.stVencAttr.enType = config.ePayloadType;
        switch (stVencChnAttr.stVencAttr.enType) {
            case PT_H265: {
                stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_PROFILE;
                stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;
                stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;

                if (config.stRCInfo.eRCType == VENC_RC_CBR) {
                    AX_VENC_H265_CBR_S stH265Cbr;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                    stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                    stH265Cbr.u32Gop = config.nGOP;
                    stH265Cbr.u32SrcFrameRate = config.nSrcFrameRate; /* input frame rate */
                    stH265Cbr.fr32DstFrameRate = config.nDstFrameRate; /* target frame rate */
                    stH265Cbr.u32BitRate = config.nBitrate;
                    stH265Cbr.u32MinQp = config.stRCInfo.nMinQp;
                    stH265Cbr.u32MaxQp = config.stRCInfo.nMaxQp;
                    stH265Cbr.u32MinIQp = config.stRCInfo.nMinIQp;
                    stH265Cbr.u32MaxIQp = config.stRCInfo.nMaxIQp;
                    stH265Cbr.s32IntraQpDelta = config.stRCInfo.nIntraQpDelta;
                    memcpy(&stVencChnAttr.stRcAttr.stH265Cbr, &stH265Cbr, sizeof(AX_VENC_H265_CBR_S));
                } else if (config.stRCInfo.eRCType == VENC_RC_VBR) {
                    AX_VENC_H265_VBR_S stH265Vbr;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                    stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                    stH265Vbr.u32Gop = config.nGOP;
                    stH265Vbr.u32SrcFrameRate = config.nSrcFrameRate;
                    stH265Vbr.fr32DstFrameRate = config.nDstFrameRate;
                    stH265Vbr.u32MaxBitRate = config.nBitrate;
                    stH265Vbr.u32MinQp = config.stRCInfo.nMinQp;
                    stH265Vbr.u32MaxQp = config.stRCInfo.nMaxQp;
                    stH265Vbr.u32MinIQp = config.stRCInfo.nMinIQp;
                    stH265Vbr.u32MaxIQp = config.stRCInfo.nMaxIQp;
                    stH265Vbr.s32IntraQpDelta = config.stRCInfo.nIntraQpDelta;
                    memcpy(&stVencChnAttr.stRcAttr.stH265Vbr, &stH265Vbr, sizeof(AX_VENC_H265_VBR_S));
                } else if (config.stRCInfo.eRCType == VENC_RC_FIXQP) {
                    AX_VENC_H265_FIXQP_S stH265FixQp;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                    stH265FixQp.u32Gop = config.nGOP;
                    stH265FixQp.u32SrcFrameRate = config.nSrcFrameRate;
                    stH265FixQp.fr32DstFrameRate = config.nDstFrameRate;
                    stH265FixQp.u32IQp = 25;
                    stH265FixQp.u32PQp = 30;
                    stH265FixQp.u32BQp = 32;
                    memcpy(&stVencChnAttr.stRcAttr.stH265FixQp, &stH265FixQp, sizeof(AX_VENC_H265_FIXQP_S));
                }
                break;
            }
            case PT_H264: {
                stVencChnAttr.stVencAttr.enProfile = VENC_H264_MAIN_PROFILE;
                stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
	    
                if (config.stRCInfo.eRCType == VENC_RC_CBR) {
                    AX_VENC_H264_CBR_S stH264Cbr;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                    stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                    stH264Cbr.u32Gop = config.nGOP;
                    stH264Cbr.u32SrcFrameRate = config.nSrcFrameRate; /* input frame rate */
                    stH264Cbr.fr32DstFrameRate = config.nDstFrameRate; /* target frame rate */
                    stH264Cbr.u32BitRate = config.nBitrate;
                    stH264Cbr.u32MinQp = config.stRCInfo.nMinQp;
                    stH264Cbr.u32MaxQp = config.stRCInfo.nMaxQp;
                    stH264Cbr.u32MinIQp = config.stRCInfo.nMinIQp;
                    stH264Cbr.u32MaxIQp = config.stRCInfo.nMaxIQp;
                    stH264Cbr.s32IntraQpDelta = config.stRCInfo.nIntraQpDelta;
                    memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_S));
                } else if (config.stRCInfo.eRCType == VENC_RC_VBR) {
                    AX_VENC_H264_VBR_S stH264Vbr;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                    stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                    stH264Vbr.u32Gop = config.nGOP;
                    stH264Vbr.u32SrcFrameRate = config.nSrcFrameRate;
                    stH264Vbr.fr32DstFrameRate = config.nDstFrameRate;
                    stH264Vbr.u32MaxBitRate = config.nBitrate;
                    stH264Vbr.u32MinQp = config.stRCInfo.nMinQp;
                    stH264Vbr.u32MaxQp = config.stRCInfo.nMaxQp;
                    stH264Vbr.u32MinIQp = config.stRCInfo.nMinIQp;
                    stH264Vbr.u32MaxIQp = config.stRCInfo.nMaxIQp;
                    stH264Vbr.s32IntraQpDelta = config.stRCInfo.nIntraQpDelta;
                    memcpy(&stVencChnAttr.stRcAttr.stH264Vbr, &stH264Vbr, sizeof(AX_VENC_H264_VBR_S));
                } else if (config.stRCInfo.eRCType == VENC_RC_FIXQP) {
                    AX_VENC_H264_FIXQP_S stH264FixQp;
                    stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                    stH264FixQp.u32Gop = config.nGOP;
                    stH264FixQp.u32SrcFrameRate = config.nSrcFrameRate;
                    stH264FixQp.fr32DstFrameRate = config.nDstFrameRate;
                    stH264FixQp.u32IQp = 25;
                    stH264FixQp.u32PQp = 30;
                    stH264FixQp.u32BQp = 32;
                    memcpy(&stVencChnAttr.stRcAttr.stH264FixQp, &stH264FixQp, sizeof(AX_VENC_H264_FIXQP_S));
                }
                break;
            }
            default:
                SAMPLE_ERR_LOG("VencChn %d:Payload type unrecognized.",VencChn);
				return -1;
        }

        AX_S32 ret = AX_VENC_CreateChn(VencChn, &stVencChnAttr);
        if (AX_SUCCESS != ret) {
            SAMPLE_ERR_LOG("VencChn %d: AX_VENC_CreateChn failed, s32Ret:0x%x", VencChn, ret);
            return -1;
        }

        /* create get output stream thread */
        gGetStreamPara[VencChn].VeChn = VencChn;
        gGetStreamPara[VencChn].bThreadStart = AX_TRUE;
        gGetStreamPara[VencChn].ePayloadType = config.ePayloadType;
        pthread_create(&gGetStreamPid[VencChn], NULL, VencGetStreamProc, (void *)&gGetStreamPara[VencChn]);
    }

    return 0;
}

static AX_S32 SampleVencDeInit()
{
    AX_S32 VencChn = 0,s32Ret = 0;

    for (VencChn = 0;VencChn < SAMPLE_VENC_CHN_NUM;VencChn++) {

        s32Ret = AX_VENC_StopRecvFrame(VencChn);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("VencChn %d:AX_VENC_StopRecvFrame failed,s32Ret:0x%x.\n", VencChn, s32Ret);
            return s32Ret;
        }

        s32Ret = AX_VENC_DestroyChn(VencChn);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("VencChn %d:AX_VENC_DestroyChn failed,s32Ret:0x%x.\n", VencChn, s32Ret);
            return s32Ret;
        }

        if (AX_TRUE == gGetStreamPara[VencChn].bThreadStart) {
            gGetStreamPara[VencChn].bThreadStart = AX_FALSE;
            pthread_join(gGetStreamPid[VencChn], NULL);
        }
    }
    s32Ret = AX_VENC_Deinit();
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("AX_VENC_Deinit failed, s32Ret=0x%x", s32Ret);
        return s32Ret;
    }

    return 0;
}

#ifndef SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS
static int SampleIvpsInit()
{
    AX_S32 s32Ret = 0;
    AX_S32 nGrp = 0,nChn = 0;
    AX_IVPS_GRP_ATTR_S stGrpAttr = {0};
    AX_IVPS_PIPELINE_ATTR_S stPipelineAttr = {0};

    s32Ret = AX_IVPS_Init();
    if (0 != s32Ret)
    {
        SAMPLE_ERR_LOG("AX_IVPS_Init failed,s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }

    stPipelineAttr.tFbInfo.PoolId = AX_INVALID_POOLID;
    stPipelineAttr.nOutChnNum = 3;

    for(nGrp = 0; nGrp < SAMPLE_IVPS_GROUP_NUM; nGrp++)
    {
        stGrpAttr.nInFifoDepth = 1;
        stGrpAttr.ePipeline = AX_IVPS_PIPELINE_DEFAULT;
        s32Ret = AX_IVPS_CreateGrp(nGrp, &stGrpAttr);
        if (0 != s32Ret)
        {
            SAMPLE_ERR_LOG("AX_IVPS_CreateGrp failed,nGrp %d,s32Ret:0x%x\n",nGrp,s32Ret);
            return s32Ret;
        }

        for(nChn = 0;nChn < SAMPLE_IVPS_CHN_NUM;nChn++)
        {

            if(nGrp == 0){
                stPipelineAttr.tFilter[nChn+1][0].bEnable = AX_TRUE;
                stPipelineAttr.tFilter[nChn+1][0].tFRC.nSrcFrameRate = 30;
                stPipelineAttr.tFilter[nChn+1][0].tFRC.nDstFrameRate = 30;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicOffsetX0 = 0;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicOffsetY0 = 0;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicWidth = 2688;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicHeight = 1520;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicStride = stPipelineAttr.tFilter[nChn+1][0].nDstPicWidth;
                stPipelineAttr.tFilter[nChn+1][0].nDstFrameWidth = 2688;
                stPipelineAttr.tFilter[nChn+1][0].nDstFrameHeight = 1520;
                stPipelineAttr.tFilter[nChn+1][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
                stPipelineAttr.tFilter[nChn+1][0].eEngine = AX_IVPS_ENGINE_GDC;
                stPipelineAttr.tFilter[nChn+1][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
                stPipelineAttr.nOutFifoDepth[nChn] = 1;
            }else if(nGrp == 1){
                stPipelineAttr.tFilter[nChn+1][0].bEnable = AX_TRUE;
                stPipelineAttr.tFilter[nChn+1][0].tFRC.nSrcFrameRate = 30;
                stPipelineAttr.tFilter[nChn+1][0].tFRC.nDstFrameRate = 30;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicOffsetX0 = 0;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicOffsetY0 = 0;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicWidth = 1920;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicHeight = 1080;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicStride = stPipelineAttr.tFilter[nChn+1][0].nDstPicWidth;
                stPipelineAttr.tFilter[nChn+1][0].nDstFrameWidth = 1920;
                stPipelineAttr.tFilter[nChn+1][0].nDstFrameHeight = 1080;
                stPipelineAttr.tFilter[nChn+1][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
                stPipelineAttr.tFilter[nChn+1][0].eEngine = AX_IVPS_ENGINE_TDP;
                stPipelineAttr.tFilter[nChn+1][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
                stPipelineAttr.nOutFifoDepth[nChn] = 1;
            }else{
                stPipelineAttr.tFilter[nChn+1][0].bEnable = AX_TRUE;
                stPipelineAttr.tFilter[nChn+1][0].tFRC.nSrcFrameRate = 30;
                stPipelineAttr.tFilter[nChn+1][0].tFRC.nDstFrameRate = 30;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicOffsetX0 = 0;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicOffsetY0 = 0;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicWidth = 720;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicHeight = 576;
                stPipelineAttr.tFilter[nChn+1][0].nDstPicStride = stPipelineAttr.tFilter[nChn+1][0].nDstPicWidth;
                stPipelineAttr.tFilter[nChn+1][0].nDstFrameWidth = 720;
                stPipelineAttr.tFilter[nChn+1][0].nDstFrameHeight = 576;
                stPipelineAttr.tFilter[nChn+1][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
                stPipelineAttr.tFilter[nChn+1][0].eEngine = AX_IVPS_ENGINE_TDP;
                stPipelineAttr.tFilter[nChn+1][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
                stPipelineAttr.nOutFifoDepth[nChn] = 1;
            }

            s32Ret = AX_IVPS_SetPipelineAttr(nGrp, &stPipelineAttr);
            if (0 != s32Ret)
            {
                SAMPLE_ERR_LOG("AX_IVPS_SetPipelineAttr failed,nGrp %d,s32Ret:0x%x\n",nGrp,s32Ret);
                return s32Ret;
            }

            s32Ret = AX_IVPS_EnableChn(nGrp, nChn);
            if (0 != s32Ret)
            {
                SAMPLE_ERR_LOG("AX_IVPS_EnableChn failed,nGrp %d,nChn %d,s32Ret:0x%x\n",nGrp,nChn,s32Ret);
                return s32Ret;
            }
        }

        s32Ret = AX_IVPS_StartGrp(nGrp);
        if (0 != s32Ret)
        {
            SAMPLE_ERR_LOG("AX_IVPS_StartGrp failed,nGrp %d,s32Ret:0x%x\n",nGrp,s32Ret);
            return s32Ret;
        }
    }
    return 0;
}

static AX_S32 SampleIvpsDeInit()
{
    AX_S32 s32Ret = 0,nGrp = 0,nChn = 0;

    for(nGrp = 0;nGrp < SAMPLE_IVPS_GROUP_NUM;nGrp++)
    {
        s32Ret = AX_IVPS_StopGrp(nGrp);
        if (0 != s32Ret){
            SAMPLE_ERR_LOG("AX_IVPS_StopGrp failed,nGrp %d,s32Ret:0x%x\n",nGrp,s32Ret);
            return s32Ret;
        }

        for(nChn = 0;nChn < SAMPLE_IVPS_CHN_NUM;nChn++)
        {
            s32Ret = AX_IVPS_DisableChn(nGrp, nChn);
            if (0 != s32Ret){
                SAMPLE_ERR_LOG("AX_IVPS_DisableChn failed,nGrp %d,nChn %d,s32Ret:0x%x\n",nGrp,nChn,s32Ret);
                return s32Ret;
            }
        }

        s32Ret = AX_IVPS_DestoryGrp(nGrp);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("AX_IVPS_DestoryGrp failed,nGrp %d,s32Ret:0x%x",nGrp,s32Ret);
            return s32Ret;
        }
    }

    s32Ret = AX_IVPS_Deinit();
    if (0 != s32Ret)
    {
        SAMPLE_ERR_LOG("AX_IVPS_Deinit failed,s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }

    return 0;
}
#endif

static void *IspRun(void *args)
{
    AX_U32 i = (AX_U32)args;

    SAMPLE_LOG("cam %d is running...\n", i);

    while (!g_isp_force_loop_exit)
    {
        if (!gCams[i].bOpen)
        {
            usleep(40 * 1000);
            continue;
        }

        AX_ISP_Run(gCams[i].nPipeId);
    }
    return NULL;
}

static AX_S32 SysRun()
{
    AX_S32 s32Ret = 0, i;

    g_isp_force_loop_exit = 0;
    for (i = 0; i < MAX_CAMERAS; i++)
    {
        if (gCams[i].bOpen)
            pthread_create(&gCams[i].tIspProcThread, NULL, IspRun, (AX_VOID*)i);
    }

    while (!gLoopExit)
    {
        sleep(1);
    }

    g_isp_force_loop_exit = 1;

    for (i = 0; i < MAX_CAMERAS; i++)
    {
        if (gCams[i].bOpen)
        {
            pthread_cancel(gCams[i].tIspProcThread);
            s32Ret = pthread_join(gCams[i].tIspProcThread, NULL);
            if (s32Ret < 0)
            {
                SAMPLE_ERR_LOG(" isp run thread exit failed,s32Ret:0x%x\n", s32Ret);
            }
        }
    }
    return 0;
}

static AX_VOID __sigint(int iSigNo)
{
    //SAMPLE_LOG("Catch signal %d!\n", iSigNo);
    gLoopExit = 1;
    return;
}

static AX_S32 SampleLinkInit()
{
    AX_S32 i = 0;

    /*
	VIN --> IVPS --> VENC
    (ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
    --------------------------------------------------
    (VIN        0       2) -> (IVPS     2       0)
    (VIN        0       1) -> (IVPS     1       0)
    (VIN        0       0) -> (IVPS     0       0)
    (IVPS       2       0) -> (VENC     0       2)
    (IVPS       1       0) -> (VENC     0       1)
    (IVPS       0       0) -> (VENC     0       0)

	VIN --> VENC
    (ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
    --------------------------------------------------
    (VIN        0       2) -> (VENC     0       2)
    (VIN        0       1) -> (VENC     0       1)
    (VIN        0       0) -> (VENC     0       0)
    */

    for(i = 0;i < 3; i++)
    {
        AX_MOD_INFO_S srcMod, dstMod;
        srcMod.enModId = AX_ID_VIN;
        srcMod.s32GrpId = 0;
        srcMod.s32ChnId = i;

#ifndef SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS
        dstMod.enModId = AX_ID_IVPS;
        dstMod.s32GrpId = i;
        dstMod.s32ChnId = 0;
        AX_SYS_Link(&srcMod, &dstMod);

        srcMod.enModId = AX_ID_IVPS;
        srcMod.s32GrpId = i;
        srcMod.s32ChnId = 0;
#endif
        dstMod.enModId = AX_ID_VENC;
        dstMod.s32GrpId = 0;
        dstMod.s32ChnId = i;
        AX_SYS_Link(&srcMod, &dstMod);
    }

    return 0;
}

static AX_S32 SampleLinkDeInit()
{
    AX_S32 i = 0;
	
    /*
	VIN --> IVPS --> VENC
    (ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
    --------------------------------------------------
    (VIN        0       2) -> (IVPS     2       0)
    (VIN        0       1) -> (IVPS     1       0)
    (VIN        0       0) -> (IVPS     0       0)
    (IVPS       2       0) -> (VENC     0       2)
    (IVPS       1       0) -> (VENC     0       1)
    (IVPS       0       0) -> (VENC     0       0)

	VIN --> VENC
    (ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
    --------------------------------------------------
    (VIN        0       2) -> (VENC     0       2)
    (VIN        0       1) -> (VENC     0       1)
    (VIN        0       0) -> (VENC     0       0)
    */

    for(i = 0;i < 3; i++)
    {
        AX_MOD_INFO_S srcMod, dstMod;
        srcMod.enModId = AX_ID_VIN;
        srcMod.s32GrpId = 0;
        srcMod.s32ChnId = i;

#ifndef SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS
        dstMod.enModId = AX_ID_IVPS;
        dstMod.s32GrpId = i;
        dstMod.s32ChnId = 0;
        AX_SYS_UnLink(&srcMod, &dstMod);

        srcMod.enModId = AX_ID_IVPS;
        srcMod.s32GrpId = i;
        srcMod.s32ChnId = 0;
#endif

        dstMod.enModId = AX_ID_VENC;
        dstMod.s32GrpId = 0;
        dstMod.s32ChnId = i;
        AX_SYS_UnLink(&srcMod, &dstMod);
    }

    return 0;
}

static AX_VOID PrintHelp(char *testApp)
{
    printf("Usage:%s -h for help\n\n", testApp);

    printf("\t-c: ISP Test Case:\n");
    printf("\t\t0: Single OS04A10\n");
    printf("\t\t1: Single IMX334\n");
    printf("\t\t2: Single GC4653\n");
    printf("\t\t3: DUAL OS04A10\n");
    printf("\t\t4: Single OS08A20\n");
    printf("\t\t5: Single OS04A10 Online\n");

    printf("\t-e: SDR/HDR Mode:\n");
    printf("\t\t1: SDR\n");
    printf("\t\t2: HDR 2DOL\n");

    exit(0);
}

int main(int argc, char *argv[])
{
    AX_S32 isExit = 0,i,ch;
    AX_S32 s32Ret = 0;
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_NONE;
    COMMON_SYS_ARGS_T tCommonArgs = {0};
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_LINEAR_MODE;
    SAMPLE_SNS_TYPE_E eSnsType = OMNIVISION_OS04A10;

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, __sigint);

    SAMPLE_LOG("sample_vin_ivps_venc begin\n\n");

    if (argc < 2) {
        PrintHelp(argv[0]);
        exit(0);
    }

    while ((ch = getopt(argc, argv, "c:e:h")) != -1) {
        isExit = 0;
        switch (ch) {
        case 'c':
            eSysCase = (COMMON_SYS_CASE_E)atoi(optarg);
            break;
        case 'e':
            eHdrMode = (AX_SNS_HDR_MODE_E)atoi(optarg);
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
        PrintHelp(argv[0]);
        exit(0);
    }

    SAMPLE_LOG("eSysCase=%d,eHdrMode=%d\n", eSysCase,eHdrMode);

    if (eSysCase == SYS_CASE_SINGLE_OS04A10) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE){
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Sdr) / sizeof(gtSysCommPoolSingleOs04a10Sdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10Sdr;
            gCams[0].stPipeAttr.ePipeDataSrc = AX_PIPE_SOURCE_DEV_ONLINE;
        } else if(eHdrMode == AX_SNS_HDR_2X_MODE){
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Hdr) / sizeof(gtSysCommPoolSingleOs04a10Hdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10Hdr;
        }
    } else if (eSysCase == SYS_CASE_SINGLE_OS04A10_ONLINE) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE){
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10OnlineSdr) / sizeof(gtSysCommPoolSingleOs04a10OnlineSdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10OnlineSdr;
        } else if(eHdrMode == AX_SNS_HDR_2X_MODE){
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10OnlineHdr) / sizeof(gtSysCommPoolSingleOs04a10OnlineHdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10OnlineHdr;
        }
        gCams[0].stPipeAttr.ePipeDataSrc = AX_PIPE_SOURCE_DEV_ONLINE;
        gCams[0].stChnAttr.tChnAttr[0].nDepth = 1;
        gCams[0].stChnAttr.tChnAttr[1].nDepth = 1;
        gCams[0].stChnAttr.tChnAttr[2].nDepth = 1;
    } else if (eSysCase == SYS_CASE_SINGLE_IMX334) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = SONY_IMX334;
        COMMON_ISP_GetSnsConfig(SONY_IMX334, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
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
        COMMON_ISP_GetSnsConfig(GALAXYCORE_GC4653, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
    } else if (eSysCase == SYS_CASE_DUAL_OS04A10) {
        tCommonArgs.nCamCnt = 2;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[1].stSnsAttr, &gCams[1].stSnsClkAttr, &gCams[1].stDevAttr, &gCams[1].stPipeAttr,
                                &gCams[1].stChnAttr);
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolDoubleOs04a10) / sizeof(gtSysCommPoolDoubleOs04a10[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolDoubleOs04a10;

        gCams[0].stSnsClkAttr.nSnsClkIdx = 0;    /* mclk0 only by AX DEMO board, User defined */
        gCams[1].stSnsClkAttr.nSnsClkIdx = 2;   /* mclk2 only by AX DEMO board, User defined */
    } else if (eSysCase == SYS_CASE_SINGLE_OS08A20) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS08A20;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS08A20, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE) {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs08a20Sdr) / sizeof(gtSysCommPoolSingleOs08a20Sdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs08a20Sdr;
            gCams[0].stSnsAttr.eRawType = AX_RT_RAW12;
            gCams[0].stDevAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
            gCams[0].stPipeAttr.ePixelFmt = AX_FORMAT_BAYER_RAW_12BPP;
        } else {
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs08a20Hdr) / sizeof(gtSysCommPoolSingleOs08a20Hdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs08a20Hdr;
        }
    } else {
        SAMPLE_ERR_LOG("Unsupported case eSysCase=%d\n",eSysCase);
        PrintHelp(argv[0]);
        exit(0);
    }

    /*step 1:sys init*/
    s32Ret = COMMON_SYS_Init(&tCommonArgs);
    if (s32Ret) {
        SAMPLE_ERR_LOG("COMMON_SYS_Init failed,s32Ret:0x%x\n", s32Ret);
        return -1;
    }

    /*step 2:link init*/
    s32Ret = SampleLinkInit();
    if (s32Ret) {
        SAMPLE_ERR_LOG("COMMON_SYS_Init failed,s32Ret:0x%x\n", s32Ret);
        goto EXIT_1;
    }

    /*step 3:npu init*/
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
        SAMPLE_ERR_LOG("AX_NPU_SDK_EX_Init_with_attr failed,s32Ret:0x%x\n", s32Ret);
        goto EXIT_2;
    }
 
    /*step 3:camera init*/
    s32Ret = COMMON_CAM_Init();
    if (0 != s32Ret) {
        SAMPLE_ERR_LOG("COMMON_CAM_Init failed,s32Ret:0x%x\n", s32Ret);
        goto EXIT_2;
    }

    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        s32Ret = COMMON_CAM_Open(&gCams[i]);
        if (s32Ret){
            SAMPLE_ERR_LOG("COMMON_CAM_Open failed,s32Ret:0x%x\n", s32Ret);
            goto EXIT_3;
		}
        gCams[i].bOpen = AX_TRUE;
        SAMPLE_LOG("camera %d is open\n", i);
    }

    /*step 4:ivps init*/
#ifndef SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS
    s32Ret = SampleIvpsInit();
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("SampleIvpsInit failed,s32Ret:0x%x\n", s32Ret);
        goto EXIT_4;
    }
#endif

    /*step 5:venc init*/
    s32Ret = SampleVencInit();
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("SampleVencInit failed,s32Ret:0x%x\n", s32Ret);
        goto EXIT_5;
    }

    /*step 6:sys run*/
    s32Ret = SysRun();
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("SysRun error,s32Ret:0x%x\n",s32Ret);
        goto EXIT_6;
    }

EXIT_6:
   SampleVencDeInit();

EXIT_5:
#ifndef SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS
    SampleIvpsDeInit();
#endif

EXIT_4:
    for (i = 0; i < tCommonArgs.nCamCnt; i++) {
        if (!gCams[i].bOpen)
            continue;
        COMMON_CAM_Close(&gCams[i]);
    }

EXIT_3:
    COMMON_CAM_Deinit();

EXIT_2:
    SampleLinkDeInit();

EXIT_1:
    COMMON_SYS_DeInit();

    SAMPLE_LOG("sample_vin_ivps_venc end\n");
	return 0;
}
