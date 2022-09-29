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
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <dirent.h>

#ifdef __linux
#include <sys/syscall.h>
#endif

#include "ax_sys_api.h"
#include "ax_venc_api.h"


// #include "ax_isp_api.h"
// #include "ax_mipi_api.h"
// #include "common_isp.h"
#include "sample_utils.h"


#define ENABLE_DPU_OSD 0
#if ENABLE_DPU_OSD
#include "ax_dpu_api.h"
#endif

#define JENC_NUM_COMM 64
/* set qp table by user */
//#define ENABLE_USER_QPTABLE

//#define ENABLE_RC_DYNAMIC

//#define ENABLE_THUMBNAIL_CODING

#define MULTI_THREAD_GET_STREAM

#define SAMPLE_NAME "Sample Jpegenc"

#define JENC_MAX_WIDTH 16384
#define JENC_MAX_HEIGHT 16384
#define Max_Input_File 100
//#define DYNAMIC_SET_HW_FREQUENCE
//#define ENABLE_BITRATEJAM_STRATEGY
//#define ENABLE_SETCHANATTR_CROP
//#define ENABLE_QUERY_STATUS_LOG

#define gettid() syscall(__NR_gettid)

#define ENABLE_DEBUG 0

//#define ENABLE_DYNAMIC_RC

#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("tid:%ld sample_jpegenc.c %s:%d "str"\n", gettid(), __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)    \
    do{     \
        printf(" tid:%ld sample_jpegenc.c %s %d Error! "str"\n", gettid(), __func__, __LINE__, ##arg); \
    }while(0)

AX_OSD_BMP_ATTR_S gStBmp[JENC_NUM_COMM];

// static AX_S32 gFrameSize;

static AX_S32 gLoopExit = 0;

static AX_S32 gLoopEncode = 0;

// static AX_S32 gFixQpLevel = 22;
// static AX_S32 gMinQfactor = 0;
// static AX_S32 gMaxQfactor = 51;

// static AX_S32 gSrcFrameRate = 30;
// static AX_S32 gDstFrameRate = 30;

// static AX_S32 gBitRate = 20000000;

static AX_S32 gSyncType = -1;

static AX_CHAR *gFileInput;

static const AX_U8 QTableLuminance[64] = {
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1};

static const AX_U8 QTableChrominance[64] = {
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1};

/*In one case,encode thread may run faster than getStream thread,
  so getStream thread can not save data as soon as possible.
  Below three variables are just for two thread synchronization and non-needed,
  could be removed in the real product environment.*/
static pthread_mutex_t gOutputMutex[JENC_NUM_COMM];
static pthread_cond_t gOutputCond[JENC_NUM_COMM];
static AX_BOOL gSaveOneFrameEnd[JENC_NUM_COMM] = {false};

typedef struct _SAMPLE_JENC_ENCODE_PARA
{
    AX_BOOL bThreadStart;
    VENC_CHN VeChn;
    AX_S32 input_width;
    AX_S32 input_height;
    AX_S32 stride[3];
    AX_S32 FrameSize;
    AX_POOL userPoolId;
    AX_S16  s16OffsetTop;    /* top offset of show area */
    AX_S16  s16OffsetBottom; /* bottom offset of show area */
    AX_S16  s16OffsetLeft;   /* left offset of show area */
    AX_S16  s16OffsetRight;  /* right offset of show area */
    AX_IMG_FORMAT_E enFrameFormat;
    AX_PAYLOAD_TYPE_E enCodecFormat;
    SAMPLE_CMD_PARA_T *pCmdPara;
} SAMPLE_JENC_ENCODE_PARA_T;

typedef struct _SAMPLE_JENC_GETSTREAM_PARA
{
    AX_BOOL bThreadStart;
    VENC_CHN VeChn;
    AX_MOD_INFO_S ispMod;
    AX_MOD_INFO_S jencMod;
    AX_PAYLOAD_TYPE_E enCodecFormat;
    SAMPLE_CMD_PARA_T *pCmdPara;
} SAMPLE_JENC_GETSTREAM_PARA_T;



static SAMPLE_JENC_GETSTREAM_PARA_T gGetStreamPara[JENC_NUM_COMM];
static pthread_t gGetStreamPid[JENC_NUM_COMM];

static SAMPLE_JENC_ENCODE_PARA_T gEncodePara[JENC_NUM_COMM];
static pthread_t gEncodePid[JENC_NUM_COMM];


static unsigned int uTimeDiff(struct timeval end, struct timeval start)
{
   return (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
}

#if ENABLE_DPU_OSD
static AX_VOID LoadBmp(AX_OSD_BMP_ATTR_S *pstBmp)
{
  char path[] = "/opt/data/venc/OSD_754X70.ARGB1555";

    memset(pstBmp, 0, sizeof(*pstBmp));
    pstBmp->bEnable = AX_TRUE;
    pstBmp->u32Zindex = 0;
    pstBmp->enRgbFormat = AX_FORMAT_ARGB1555;
    pstBmp->u32ColorKey = 0x0;
    pstBmp->u32BgColorLo = 0xffffffff;
    pstBmp->u32BgColorHi = 0xffffffff;
    pstBmp->u32BmpWidth = 754;
    pstBmp->u32BmpHeight = 70;
    pstBmp->u32DstXoffset = 40;
    pstBmp->u32DstYoffset = 40;

    FILE *fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        AX_U32 nSize = ftell(fp);
        pstBmp->pBitmap = malloc(nSize);
        fseek(fp, 0, SEEK_SET);
        fread(pstBmp->pBitmap, 1, nSize, fp);
        fclose(fp);
    } else {
        printf("open %s fail\n", path);
    }
}
#endif

static int LoadFrameFromFile(FILE *pFileIn, AX_S32 widthSrc, AX_S32 *strideSrc,
                            AX_S32 heightSrc, AX_IMG_FORMAT_E eFmt, AX_VIDEO_FRAME_S stFrame)
{
    AX_U64 pVaddr[3] = {0};
    AX_S32 stride[3] = {0};
#if ENABLE_DEBUG
    SAMPLE_LOG("widthSrc=%d, strideSrc[0]=%d, strideSrc[1]=%d, strideSrc[2]=%d, heightSrc=%d",
               widthSrc, strideSrc[0], strideSrc[1], strideSrc[2],heightSrc);
#endif
    int i, j, rows, realRead, readSize;

    if (!pFileIn)
        return -1;
    if (!pVaddr)
        return -1;

    readSize = 0;
    for (i = 0; i < 3; i++) {
        pVaddr[i] = stFrame.u64VirAddr[i];
        stride[i] = strideSrc[i];
    }

    switch(eFmt) {
    case AX_YUV420_PLANAR:
        for (i = 0; i < 3; i++) {
            if (i == 0) {
                rows = heightSrc;
            } else {
                rows = heightSrc / 2;
            }
            for (j = 0; j < rows; j++) {
                realRead = fread(pVaddr[i], 1, stride[i], pFileIn);
                if (realRead < stride[i]) {
                    //printf("%s %d: Warning: real read size %d less than %d\n", __func__, __LINE__, realRead, widthSrc);
                    break;
                }
                readSize += realRead;
                pVaddr[i] += stride[i];
            }
        }
        break;
    case AX_YUV420_SEMIPLANAR:
    case AX_YUV420_SEMIPLANAR_VU:
        for (i = 0; i < 2; i++) {
            if (i == 0) {
                rows = heightSrc;
            } else {
                rows = heightSrc / 2;
            }
            for (j = 0; j < rows; j++) {
                realRead = fread(pVaddr[i], 1, stride[i], pFileIn);
                if (realRead < stride[i]) {
                    //printf("%s %d: Warning: real read size %d less than %d\n", __func__, __LINE__, realRead, widthSrc);
                    break;
                }
                readSize += realRead;
                pVaddr[i] += stride[i];
            }
        }
        break;
    case AX_YUV422_INTERLEAVED_YUYV:
    case AX_YUV422_INTERLEAVED_UYVY:
        rows = heightSrc;
        for (i = 0; i < rows; i++) {
            realRead = fread(pVaddr, 1, stride[0], pFileIn);
            if (realRead < widthSrc) {
                printf("%s %d: Warning: real read size less than %d\n", __func__, __LINE__, stride[0]);
                break;
            }
            readSize += realRead;
            pVaddr[0] += stride[0];
        }
        break;
    default:
        printf("%s invalid format, eFmt = %d\n", __func__, eFmt);
    }

    return readSize;
}

static void *MjpegencEncodeUsage1Proc(void *arg)
{
    AX_S32 s32Ret = 0;
    AX_S32 readSize = 0;
    AX_S32 encodeFrmNum = 1;
    AX_U64 totalEncodedFrames = 0;
    AX_S8 * fileInput = NULL;
    AX_S32 syncType = -1;
    AX_U32 frameSize = 0;
    struct timeval timeFrameStart;
    AX_BOOL bChangeClk = AX_FALSE;

    SAMPLE_JENC_ENCODE_PARA_T *pEncodePara;

    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    AX_VIDEO_FRAME_INFO_S stFrame;

    pEncodePara = (SAMPLE_JENC_ENCODE_PARA_T *)arg;

    FILE *fFileIn = NULL;

    if (pEncodePara->pCmdPara == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: pEncodePara->pCmdPara == NULL!\n", pEncodePara->VeChn);
        return NULL;
    }

    if (pEncodePara->pCmdPara->input == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: pEncodePara->pCmdPara->input == NULL!\n", pEncodePara->VeChn);
        return NULL;
    }

    fileInput = pEncodePara->pCmdPara->input;

    fFileIn = fopen(fileInput, "rb");

    if (fFileIn == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: open input file error!\n", pEncodePara->VeChn);
        return NULL;
    }

    frameSize = pEncodePara->FrameSize;

    if (pEncodePara->pCmdPara->frameNum > 1) {
        encodeFrmNum = pEncodePara->pCmdPara->frameNum;
    }

    AX_POOL s32UserPoolId = pEncodePara->userPoolId;
    AX_BLK BlkId;

    if (pEncodePara->enCodecFormat == PT_MJPEG)
    {
        AX_VENC_RATE_JAM_CFG_S stRateJamParam;
        if (AX_VENC_GetRateJamStrategy(pEncodePara->VeChn, &stRateJamParam) != AX_SUCCESS)
        {
            printf("%s %d: AX_VENC_GetRateJamStrategy error!\n", SAMPLE_NAME, pEncodePara->VeChn);
            goto EXIT;
        }
        stRateJamParam.bDropFrmEn = AX_TRUE;
        stRateJamParam.enDropFrmMode = DROPFRM_NORMAL;
        stRateJamParam.u32MaxApplyCount = 3;
        stRateJamParam.u32DropFrmThrBps = 64 * 1024; // 64k bps
        if (AX_VENC_SetRateJamStrategy(pEncodePara->VeChn, &stRateJamParam) != AX_SUCCESS)
        {
            printf("%s %d: AX_VENC_SetRateJamStrategy error!\n", SAMPLE_NAME, pEncodePara->VeChn);
            goto EXIT;
        }
    }

    s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("Encoder thread exit, ret=%x\n", s32Ret);
        goto EXIT;
    }

    memset(&stFrame, 0, sizeof(stFrame));
    while (pEncodePara->bThreadStart) {
        stFrame.stVFrame.enCompressMode = pEncodePara->pCmdPara->eCompressMode;

        BlkId = AX_POOL_GetBlock(s32UserPoolId, frameSize, NULL);
        if (AX_INVALID_POOLID == BlkId) {
            usleep(5*1000);
            continue;
        }
        stFrame.stVFrame.enImgFormat = pEncodePara->enFrameFormat;
        stFrame.stVFrame.u32Width = pEncodePara->input_width;
        stFrame.stVFrame.u32Height = pEncodePara->input_height;
        for (int i = 0; i < 3; i++) {
            stFrame.stVFrame.u32PicStride[i] = pEncodePara->stride[i];
        }

        stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);

        stFrame.stVFrame.u32BlkId[0] = BlkId;
        stFrame.stVFrame.u32BlkId[1] = 0;//must set 0 if not used
        stFrame.stVFrame.u32BlkId[2] = 0;//must set 0 if not used
        switch (pEncodePara->enFrameFormat) {
        case AX_YUV420_PLANAR:
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64PhyAddr[2] = stFrame.stVFrame.u64PhyAddr[1] + pEncodePara->stride[1] * pEncodePara->input_height / 2;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64VirAddr[2] = stFrame.stVFrame.u64VirAddr[1] + pEncodePara->stride[1] * pEncodePara->input_height / 2;
            break;
        case AX_YUV420_SEMIPLANAR:
        case AX_YUV420_SEMIPLANAR_VU:
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64PhyAddr[2] = 0;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64VirAddr[2] = 0;
            break;
        case AX_YUV422_INTERLEAVED_YUYV:
        case AX_YUV422_INTERLEAVED_UYVY:
            stFrame.stVFrame.u64PhyAddr[1] = 0;
            stFrame.stVFrame.u64PhyAddr[2] = 0;
            stFrame.stVFrame.u64VirAddr[1] = 0;
            stFrame.stVFrame.u64VirAddr[2] = 0;
            break;
        default:
            break;
        }

        SAMPLE_LOG("eCompressMode %d.\n", pEncodePara->pCmdPara->eCompressMode);
        if (pEncodePara->pCmdPara->eCompressMode) {
            int realRead, readSize = 0;
            int Ytotal_size = pEncodePara->pCmdPara->YheaderSize + pEncodePara->pCmdPara->YpayloadSize;

            realRead = fread(stFrame.stVFrame.u64VirAddr[0], 1, Ytotal_size, fFileIn);
            // SAMPLE_LOG(" realRead %d, Ytotal %d.\n", realRead, Ytotal_size);
            if (realRead < Ytotal_size) {
                //SAMPLE_LOG("Warning: real read size %d less than %d\n", realRead, widthSrc);
                break;
            }

            stFrame.stVFrame.u64VirAddr[0] += Ytotal_size;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0];

            int UVtotal_size = pEncodePara->pCmdPara->UVheaderSize + pEncodePara->pCmdPara->UVpayloadSize;
            realRead = fread(stFrame.stVFrame.u64VirAddr[1], 1, UVtotal_size, fFileIn);
            //SAMPLE_LOG("realRead %d, UVtotal %d.\n", realRead, UVtotal_size);

            if (realRead < UVtotal_size) {
                //SAMPLE_LOG("Warning: real read size %d less than %d\n", realRead, widthSrc);
                break;
            }

            stFrame.stVFrame.u64VirAddr[1] += UVtotal_size;
            readSize = Ytotal_size + UVtotal_size;

            stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId) + pEncodePara->pCmdPara->YheaderSize;
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->pCmdPara->YpayloadSize + pEncodePara->pCmdPara->UVheaderSize;

            SAMPLE_LOG("encodering Ydata Phyaddr is %llx,UV=%llx.\n", stFrame.stVFrame.u64PhyAddr[0], stFrame.stVFrame.u64PhyAddr[1]);
            SAMPLE_LOG("encodering Ydata Viraddr is %llx,UV=%llx.\n", stFrame.stVFrame.u64VirAddr[0], stFrame.stVFrame.u64VirAddr[1]);
            SAMPLE_LOG("AX_POOL_Handle2PhysAddr(BlkId) %x,AX_POOL_GetBlockVirAddr(BlkId)=%x.\n",
                AX_POOL_Handle2PhysAddr(BlkId), AX_POOL_GetBlockVirAddr(BlkId));
            SAMPLE_LOG("YheaderSize %d, YpayloadSize %d, UVheaderSize %d.\n",
                pEncodePara->pCmdPara->YheaderSize, pEncodePara->pCmdPara->YpayloadSize,
                pEncodePara->pCmdPara->UVheaderSize);
        } else{
            /* read frame data from yuv file */
            readSize = LoadFrameFromFile(fFileIn,
                                        pEncodePara->input_width,
                                        pEncodePara->stride,
                                        pEncodePara->input_height,
                                        pEncodePara->enFrameFormat,
                                        stFrame.stVFrame);
            if (!gLoopEncode && (readSize < (pEncodePara->input_width * pEncodePara->input_height))) {
    #if ENABLE_DEBUG
                SAMPLE_LOG("ChnId:%d: Warning: read frame size : %d less than %d\n",
                    pEncodePara->VeChn, readSize, pEncodePara->input_width * pEncodePara->input_height);
    #endif
            }
        }

        if (feof(fFileIn)) {
            if (gLoopEncode) {
                fseek(fFileIn, 0, SEEK_SET);
                readSize = LoadFrameFromFile(fFileIn,
                                        pEncodePara->input_width,
                                        pEncodePara->stride,
                                        pEncodePara->input_height,
                                        pEncodePara->enFrameFormat,
                                        stFrame.stVFrame);
            } else {
                SAMPLE_LOG("ChnId:%d: End of input file!\n", pEncodePara->VeChn);
                goto EXIT;
            }
        }

        /* the input frame sequence number */
        stFrame.stVFrame.u64SeqNum = totalEncodedFrames + 1;

        /* crop parameters setting */
        stFrame.stVFrame.s16OffsetTop = pEncodePara->s16OffsetTop;
        stFrame.stVFrame.s16OffsetBottom = pEncodePara->s16OffsetBottom;
        stFrame.stVFrame.s16OffsetLeft = pEncodePara->s16OffsetLeft;
        stFrame.stVFrame.s16OffsetRight = pEncodePara->s16OffsetRight;

        s32Ret = AX_VENC_SendFrame(pEncodePara->VeChn, &stFrame, syncType);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_SendFrame failed, ret=0x%x\n", s32Ret);

            s32Ret = AX_POOL_ReleaseBlock(stFrame.stVFrame.u32BlkId[0]);
            if (s32Ret){
                SAMPLE_ERR_LOG("ChnId:%d: AX_POOL_ReleaseBlock failed, ret=0x%x\n", pEncodePara->VeChn,s32Ret);
            }
            continue;
        }

        totalEncodedFrames++;
        SAMPLE_LOG(" encode %llu frames \n", totalEncodedFrames);

        if (totalEncodedFrames >= encodeFrmNum && encodeFrmNum > 0) {
            SAMPLE_LOG("Want to encode %llu frames, exit!\n", encodeFrmNum);
            goto EXIT;
        }
    }

EXIT:

    if (NULL != fFileIn) {
        fclose(fFileIn);
        fFileIn = NULL;
    }

    SAMPLE_LOG("Chn %d - Total encode %llu frames, Encoder exit!\n", pEncodePara->VeChn, totalEncodedFrames);

    return (void *)(intptr_t)s32Ret;
}

/* encode thread */
static void *JpegencEncodeUsage1Proc(void *arg)
{
    AX_S32 s32Ret = 0;
    AX_S32 readSize = 0;
    AX_S32 encodeFrmNum = 1;
    AX_U64 totalEncodedFrames = 0;
    AX_S8 * fileInput = NULL;
    AX_S32 syncType = -1;
    AX_U32 frameSize = 0;
    struct timeval timeFrameStart;
    AX_BOOL bChangeClk = AX_FALSE;

    SAMPLE_JENC_ENCODE_PARA_T *pEncodePara;

    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    AX_VIDEO_FRAME_INFO_S stFrame;

    pEncodePara = (SAMPLE_JENC_ENCODE_PARA_T *)arg;

    FILE *fFileIn = NULL;

    if (pEncodePara->pCmdPara == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: pEncodePara->pCmdPara == NULL!\n", pEncodePara->VeChn);
        return NULL;
    }

    if (pEncodePara->pCmdPara->input == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: pEncodePara->pCmdPara->input == NULL!\n", pEncodePara->VeChn);
        return NULL;
    }

    fileInput = pEncodePara->pCmdPara->input;

    fFileIn = fopen(fileInput, "rb");

    if (fFileIn == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: open input file error!\n", pEncodePara->VeChn);
        return NULL;
    }

    frameSize = pEncodePara->FrameSize;

    if (pEncodePara->pCmdPara->frameNum > 1) {
        encodeFrmNum = pEncodePara->pCmdPara->frameNum;
    }

    AX_POOL s32UserPoolId = pEncodePara->userPoolId;
    AX_BLK BlkId;

    s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("Encoder thread exit, ret=%x\n", s32Ret);
        goto EXIT;
    }

    memset(&stFrame, 0, sizeof(stFrame));
    while (pEncodePara->bThreadStart) {
        stFrame.stVFrame.enCompressMode = pEncodePara->pCmdPara->eCompressMode;

        BlkId = AX_POOL_GetBlock(s32UserPoolId, frameSize, NULL);
        if (AX_INVALID_POOLID == BlkId) {
            SAMPLE_ERR_LOG("Get block fail.\n");
            goto EXIT;
        }
        stFrame.stVFrame.enImgFormat = pEncodePara->enFrameFormat;
        stFrame.stVFrame.u32Width = pEncodePara->input_width;
        stFrame.stVFrame.u32Height = pEncodePara->input_height;
        for (int i = 0; i < 3; i++) {
            stFrame.stVFrame.u32PicStride[i] = pEncodePara->stride[i];
        }

        stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);

        stFrame.stVFrame.u32BlkId[0] = BlkId;
        switch (pEncodePara->enFrameFormat) {
        case AX_YUV420_PLANAR:
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64PhyAddr[2] = stFrame.stVFrame.u64PhyAddr[1] + pEncodePara->stride[1] * pEncodePara->input_height / 2;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64VirAddr[2] = stFrame.stVFrame.u64VirAddr[1] + pEncodePara->stride[1] * pEncodePara->input_height / 2;
            break;
        case AX_YUV420_SEMIPLANAR:
        case AX_YUV420_SEMIPLANAR_VU:
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64PhyAddr[2] = 0;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
            stFrame.stVFrame.u64VirAddr[2] = 0;
            break;
        case AX_YUV422_INTERLEAVED_YUYV:
        case AX_YUV422_INTERLEAVED_UYVY:
            stFrame.stVFrame.u64PhyAddr[1] = 0;
            stFrame.stVFrame.u64PhyAddr[2] = 0;
            stFrame.stVFrame.u64VirAddr[1] = 0;
            stFrame.stVFrame.u64VirAddr[2] = 0;
            break;
        default:
            break;
        }

        SAMPLE_LOG("eCompressMode %d.\n", pEncodePara->pCmdPara->eCompressMode);
        if (pEncodePara->pCmdPara->eCompressMode) {
            int realRead, readSize = 0;
            int Ytotal_size = pEncodePara->pCmdPara->YheaderSize + pEncodePara->pCmdPara->YpayloadSize;

            realRead = fread(stFrame.stVFrame.u64VirAddr[0], 1, Ytotal_size, fFileIn);
            // SAMPLE_LOG(" realRead %d, Ytotal %d.\n", realRead, Ytotal_size);
            if (realRead < Ytotal_size) {
                //SAMPLE_LOG("Warning: real read size %d less than %d\n", realRead, widthSrc);
                break;
            }

            stFrame.stVFrame.u64VirAddr[0] += Ytotal_size;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0];

            int UVtotal_size = pEncodePara->pCmdPara->UVheaderSize + pEncodePara->pCmdPara->UVpayloadSize;
            realRead = fread(stFrame.stVFrame.u64VirAddr[1], 1, UVtotal_size, fFileIn);
            //SAMPLE_LOG("realRead %d, UVtotal %d.\n", realRead, UVtotal_size);

            if (realRead < UVtotal_size) {
                //SAMPLE_LOG("Warning: real read size %d less than %d\n", realRead, widthSrc);
                break;
            }

            stFrame.stVFrame.u64VirAddr[1] += UVtotal_size;
            readSize = Ytotal_size + UVtotal_size;

            stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId) + pEncodePara->pCmdPara->YheaderSize;
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->pCmdPara->YpayloadSize + pEncodePara->pCmdPara->UVheaderSize;

            SAMPLE_LOG("encodering Ydata Phyaddr is %llx,UV=%llx.\n", stFrame.stVFrame.u64PhyAddr[0], stFrame.stVFrame.u64PhyAddr[1]);
            SAMPLE_LOG("encodering Ydata Viraddr is %llx,UV=%llx.\n", stFrame.stVFrame.u64VirAddr[0], stFrame.stVFrame.u64VirAddr[1]);
            SAMPLE_LOG("AX_POOL_Handle2PhysAddr(BlkId) %x,AX_POOL_GetBlockVirAddr(BlkId)=%x.\n",
                AX_POOL_Handle2PhysAddr(BlkId), AX_POOL_GetBlockVirAddr(BlkId));
            SAMPLE_LOG("YheaderSize %d, YpayloadSize %d, UVheaderSize %d.\n",
                pEncodePara->pCmdPara->YheaderSize, pEncodePara->pCmdPara->YpayloadSize,
                pEncodePara->pCmdPara->UVheaderSize);
        } else{
            /* read frame data from yuv file */
            readSize = LoadFrameFromFile(fFileIn,
                                        pEncodePara->input_width,
                                        pEncodePara->stride,
                                        pEncodePara->input_height,
                                        pEncodePara->enFrameFormat,
                                        stFrame.stVFrame);
            if (!gLoopEncode && (readSize < (pEncodePara->input_width * pEncodePara->input_height))) {
    #if ENABLE_DEBUG
                SAMPLE_LOG("ChnId:%d: Warning: read frame size : %d less than %d\n",
                    pEncodePara->VeChn, readSize, pEncodePara->input_width * pEncodePara->input_height);
    #endif
            }
        }

        if (feof(fFileIn)) {
            if (gLoopEncode) {
                fseek(fFileIn, 0, SEEK_SET);
            } else {
                SAMPLE_LOG("ChnId:%d: End of input file!\n", pEncodePara->VeChn);
                goto EXIT;
            }
        }

        /* the input frame sequence number */
        stFrame.stVFrame.u64SeqNum = totalEncodedFrames + 1;

        pthread_mutex_lock(&gOutputMutex[pEncodePara->VeChn]);
        gettimeofday(&timeFrameStart, NULL);
        stFrame.stVFrame.u64PTS = timeFrameStart.tv_sec * 1000000 + timeFrameStart.tv_usec;

        /* crop parameters setting */
#ifndef ENABLE_SETCHANATTR_CROP
        stFrame.stVFrame.s16OffsetTop = pEncodePara->s16OffsetTop;
        stFrame.stVFrame.s16OffsetBottom = pEncodePara->s16OffsetBottom;
        stFrame.stVFrame.s16OffsetLeft = pEncodePara->s16OffsetLeft;
        stFrame.stVFrame.s16OffsetRight = pEncodePara->s16OffsetRight;
#else
        AX_VENC_CHN_ATTR_S chnAttr;
        s32Ret = AX_VENC_GetChnAttr(pEncodePara->VeChn, &chnAttr);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_GetChnAttr failed, ret=0x%x\n", s32Ret);
        }
        chnAttr.stVencAttr.u32CropWidth = pEncodePara->s16OffsetRight - pEncodePara->s16OffsetLeft;
        chnAttr.stVencAttr.u32CropHeight = pEncodePara->s16OffsetBottom - pEncodePara->s16OffsetTop;
        chnAttr.stVencAttr.u32CropOffsetX = pEncodePara->s16OffsetLeft;
        chnAttr.stVencAttr.u32CropOffsetY = pEncodePara->s16OffsetTop;
        s32Ret = AX_VENC_SetChnAttr(pEncodePara->VeChn, &chnAttr);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_SetChnAttr failed, ret=0x%x\n", s32Ret);
        }
#endif

        stFrame.stVFrame.u64VirAddr[0] = 0;
        stFrame.stVFrame.u64VirAddr[1] = 0;
        stFrame.stVFrame.u64VirAddr[2] = 0;

        s32Ret = AX_VENC_SendFrame(pEncodePara->VeChn, &stFrame, syncType);

        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_SendFrame failed, ret=0x%x\n", s32Ret);

            s32Ret = AX_POOL_ReleaseBlock(stFrame.stVFrame.u32BlkId[0]);
            if (s32Ret){
                SAMPLE_ERR_LOG("ChnId:%d: AX_POOL_ReleaseBlock failed, ret=0x%x\n", pEncodePara->VeChn,s32Ret);
            }
            pthread_mutex_unlock(&gOutputMutex[pEncodePara->VeChn]);
            goto EXIT;
        }

#ifdef DYNAMIC_SET_HW_FREQUENCE
        if (totalEncodedFrames % 20 == 0)
        {
            AX_VENC_MOD_PARAM_S stModParam;
            AX_VENC_ENCODER_TYPE_E enVencType = VENC_JPEG_ENCODER;
            memset(&stModParam, 0, sizeof(AX_VENC_MOD_PARAM_S));
            s32Ret = AX_VENC_GetModParam(enVencType, &stModParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetModParam failed, ret=%x\n", s32Ret);
                goto EXIT;
            }
            
            printf("get jenc clk frequence: %d\n", stModParam.enJencHwClk);

            stModParam.enJencHwClk = bChangeClk ? JENC_MOD_CLK_FREQUENCY_624M : JENC_MOD_CLK_FREQUENCY_312M;
            s32Ret = AX_VENC_SetModParam(enVencType, &stModParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_SetModParam failed, ret=%x\n", s32Ret);
                goto EXIT;
            }
            bChangeClk = !bChangeClk;
        }
#endif

        while (gSaveOneFrameEnd[pEncodePara->VeChn] == false) {
            //SAMPLE_LOG(" begin pthread_cond_wait");
            pthread_cond_wait(&gOutputCond[pEncodePara->VeChn], &gOutputMutex[pEncodePara->VeChn]);
            //SAMPLE_LOG(" exit pthread_cond_wait");
        }

        gSaveOneFrameEnd[pEncodePara->VeChn] = false;
        pthread_mutex_unlock(&gOutputMutex[pEncodePara->VeChn]);
        //SAMPLE_LOG(" exit pthread_mutex_unlock");
        totalEncodedFrames++;
        //SAMPLE_LOG(" encode %llu frames \n", totalEncodedFrames);

        if (totalEncodedFrames >= encodeFrmNum && encodeFrmNum > 0) {
            SAMPLE_LOG("Want to encode %llu frames, exit!\n", encodeFrmNum);
            goto EXIT;
        }
    }

EXIT:
    if (gStBmp[pEncodePara->VeChn].pBitmap) {
        free(gStBmp[pEncodePara->VeChn].pBitmap);
        gStBmp[pEncodePara->VeChn].pBitmap = NULL;
    }

    if (NULL != fFileIn) {
        fclose(fFileIn);
        fFileIn = NULL;
    }

    SAMPLE_LOG("Chn %d - Total encode %llu frames, Encoder exit!\n", pEncodePara->VeChn, totalEncodedFrames);

    return (void *)(intptr_t)s32Ret;
}

static void ParseFileInfo(AX_S8 * fileInputName, SAMPLE_JENC_ENCODE_PARA_T *pEncodePara)
{
/* file list input file name should be widthxheight_format.yuv format:i420,nv12,nv21*/
    AX_U32 count = 0;
    AX_U32 inputWidth = 0;
    AX_U32 inputHeight = 0;
    AX_U32 inputFormat = 0;

    for (int i = 0; fileInputName[i] != '\0'; ++i) {
        if (fileInputName[i] >= '0' && fileInputName[i] <= '9') {
            count = count * 10 + (fileInputName[i] - '0');
            continue;
        }
        if (!inputWidth && count) {
            inputWidth = count;
            pEncodePara->input_width = inputWidth;
            pEncodePara->stride[0] = inputWidth;
            count = 0;
        }
        if (!inputHeight && count) {
            inputHeight = count;
            pEncodePara->input_height = inputHeight;
            count = 0;
        }
        if (!inputFormat && count) {
            inputFormat = count;
            switch (inputFormat) {
            case 420:
                pEncodePara->enFrameFormat = AX_YUV420_PLANAR;
                pEncodePara->stride[1] = pEncodePara->stride[0] / 2;
                pEncodePara->stride[2] = pEncodePara->stride[0] / 2;
                break;
            case 12:
                pEncodePara->enFrameFormat = AX_YUV420_SEMIPLANAR;
                pEncodePara->stride[1] = pEncodePara->stride[0];
                pEncodePara->stride[2] = 0;
                break;
            case 21:
                pEncodePara->enFrameFormat = AX_YUV420_SEMIPLANAR_VU;
                pEncodePara->stride[1] = pEncodePara->stride[0];
                pEncodePara->stride[2] = 0;
                break;
            default:
                break;
            }
        }
    }
#if ENABLE_DEBUG
    SAMPLE_LOG("Chn %d, update new frame info: wxh [%dx%d], stride: [%d,%d,%d], imageFormat: %d",
        pEncodePara->VeChn, pEncodePara->input_width, pEncodePara->input_height,
        pEncodePara->stride[0], pEncodePara->stride[1], pEncodePara->stride[2], pEncodePara->enFrameFormat);
#endif
}


static void *JpegencEncodeFileListProc(void *arg)
{
    AX_S32 s32Ret;
    AX_S32 readSize = 0;
    AX_U64 totalEncodedFrames = 0;
    AX_U64 encodedFramesPerFile = 0;
    AX_S8 fileInput[256];
    AX_S8 * fileInputPath = NULL;
    AX_U32 fileInputIndex = 0;
    AX_S32 syncType = -1;
    AX_U32 frameSize = 0;
    struct timeval timeFrameStart;
    struct dirent *pstInputFile;
    DIR *pstInputPath;

    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    AX_VIDEO_FRAME_INFO_S stFrame;

    FILE *fFileIn = NULL;

    SAMPLE_JENC_ENCODE_PARA_T *pEncodePara;
    pEncodePara = (SAMPLE_JENC_ENCODE_PARA_T *)arg;

    if (pEncodePara->pCmdPara == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: pEncodePara->pCmdPara == NULL!\n", pEncodePara->VeChn);
        return NULL;
    }

    if (pEncodePara->pCmdPara->inputPath == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: pEncodePara->pCmdPara->inputPath == NULL!\n", pEncodePara->VeChn);
        return NULL;
    }

    AX_POOL s32UserPoolId = pEncodePara->userPoolId;
    AX_BLK BlkId;


    fileInputPath = pEncodePara->pCmdPara->inputPath;
    if (fileInputPath != NULL) {
        frameSize = pEncodePara->FrameSize;

        s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("Encoder thread exit, ret=%x\n", s32Ret);
            goto EXIT;
        }

        pstInputPath = opendir(fileInputPath);
        while ((pstInputFile = readdir(pstInputPath)) != NULL) {
            if (!(strcmp(pstInputFile->d_name, ".") && (strcmp(pstInputFile->d_name, "..")))) {
                continue;
            }
            sprintf(fileInput, "%s%s", fileInputPath, pstInputFile->d_name);
#if ENABLE_DEBUG
            SAMPLE_LOG("File list: input file name %s", fileInput);
#endif
            fFileIn = fopen(fileInput, "rb");
            if (fFileIn == NULL) {
                SAMPLE_ERR_LOG("ChnId:%d: open input file error!\n", pEncodePara->VeChn);
                return NULL;
            }

            ParseFileInfo(fileInput, pEncodePara);
            encodedFramesPerFile = 0;

            memset(&stFrame, 0, sizeof(stFrame));
            stFrame.stVFrame.enImgFormat = pEncodePara->enFrameFormat;
            stFrame.stVFrame.u32Width = pEncodePara->input_width;
            stFrame.stVFrame.u32Height = pEncodePara->input_height;
            for (int i = 0; i < 3; i++) {
                stFrame.stVFrame.u32PicStride[i] = pEncodePara->stride[i];
            }
            stFrame.stVFrame.u64UserData = fileInputIndex;
            fileInputIndex++;

            while (1) {
                BlkId = AX_POOL_GetBlock(s32UserPoolId, frameSize, NULL);
                if (AX_INVALID_POOLID == BlkId) {
                    SAMPLE_ERR_LOG("Get block fail.\n");
                    goto EXIT;
                }

                stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
                stFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);

                stFrame.stVFrame.u32BlkId[0] = BlkId;
                stFrame.stVFrame.u32BlkId[1] = 0;//must set 0 if not used
                stFrame.stVFrame.u32BlkId[2] = 0;//must set 0 if not used
                switch (pEncodePara->enFrameFormat) {
                case AX_YUV420_PLANAR:
                    stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
                    stFrame.stVFrame.u64PhyAddr[2] = stFrame.stVFrame.u64PhyAddr[1] + pEncodePara->stride[1] * pEncodePara->input_height / 2;
                    stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
                    stFrame.stVFrame.u64VirAddr[2] = stFrame.stVFrame.u64VirAddr[1] + pEncodePara->stride[1] * pEncodePara->input_height / 2;
                    break;
                case AX_YUV420_SEMIPLANAR:
                case AX_YUV420_SEMIPLANAR_VU:
                    stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
                    stFrame.stVFrame.u64PhyAddr[2] = 0;
                    stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0] + pEncodePara->stride[0] * pEncodePara->input_height;
                    stFrame.stVFrame.u64VirAddr[2] = 0;
                    break;
                case AX_YUV422_INTERLEAVED_YUYV:
                case AX_YUV422_INTERLEAVED_UYVY:
                    stFrame.stVFrame.u64PhyAddr[1] = 0;
                    stFrame.stVFrame.u64PhyAddr[2] = 0;
                    stFrame.stVFrame.u64VirAddr[1] = 0;
                    stFrame.stVFrame.u64VirAddr[2] = 0;
                    break;
                default:
                    break;
                }

                /* read frame data from yuv file */
                readSize = LoadFrameFromFile(fFileIn,
                                            pEncodePara->input_width,
                                            pEncodePara->stride,
                                            pEncodePara->input_height,
                                            pEncodePara->enFrameFormat,
                                            stFrame.stVFrame);

                if (readSize < (pEncodePara->input_width * pEncodePara->input_height)) {
#if ENABLE_DEBUG
                    SAMPLE_LOG("ChnId:%d: Warning: read frame size : %d less than %d\n",
                        pEncodePara->VeChn, readSize, pEncodePara->input_width * pEncodePara->input_height);
#endif
                }
                if (feof(fFileIn)) {
                    SAMPLE_LOG("ChnId:%d: End of input file %s!\n", pEncodePara->VeChn, fileInput);
                    break;
                }

                /* the input frame sequence number */
                stFrame.stVFrame.u64SeqNum = encodedFramesPerFile + 1;

                gettimeofday(&timeFrameStart, NULL);
                stFrame.stVFrame.u64PTS = timeFrameStart.tv_sec * 1000000 + timeFrameStart.tv_usec;

                /* crop parameters setting */
                stFrame.stVFrame.s16OffsetTop = pEncodePara->s16OffsetTop;
                stFrame.stVFrame.s16OffsetBottom = pEncodePara->s16OffsetBottom;
                stFrame.stVFrame.s16OffsetLeft = pEncodePara->s16OffsetLeft;
                stFrame.stVFrame.s16OffsetRight = pEncodePara->s16OffsetRight;

                stFrame.stVFrame.u64VirAddr[0] = 0;
                stFrame.stVFrame.u64VirAddr[1] = 0;
                stFrame.stVFrame.u64VirAddr[2] = 0;

                s32Ret = AX_VENC_SendFrame(pEncodePara->VeChn, &stFrame, syncType);
                if (AX_SUCCESS != s32Ret) {
                    SAMPLE_ERR_LOG("AX_VENC_SendFrame failed, ret=%x\n", s32Ret);
                    s32Ret = AX_POOL_ReleaseBlock(stFrame.stVFrame.u32BlkId[0]);
                    if (s32Ret){
                        SAMPLE_ERR_LOG("ChnId:%d: AX_POOL_ReleaseBlock failed, ret=0x%x\n", pEncodePara->VeChn,s32Ret);
                    }
                    goto EXIT;
                }

                encodedFramesPerFile++;
                totalEncodedFrames++;

                if (!(pEncodePara->pCmdPara->frameNum >= 1 && encodedFramesPerFile < pEncodePara->pCmdPara->frameNum)) {
                    break;
                }

                /* when set bThreadStart to false, encode eos and exit */
                if (AX_FALSE == pEncodePara->bThreadStart) {
                    goto EXIT;
                }
            }
        }
    }

EXIT:
    if (gStBmp[pEncodePara->VeChn].pBitmap) {
        free(gStBmp[pEncodePara->VeChn].pBitmap);
        gStBmp[pEncodePara->VeChn].pBitmap = NULL;
    }

    if (NULL != fFileIn) {
        fclose(fFileIn);
        fFileIn = NULL;
    }

    SAMPLE_LOG("Chn %d - Total encode %llu frames, Encoder exit!\n", pEncodePara->VeChn, totalEncodedFrames);

    return (void *)(intptr_t)s32Ret;
}


static AX_S32 MemPrint(AX_U8 *addr, AX_S32 len)
{

    return 0;
}

static void *MjpegencGetStreamProc(void *arg)
{
    AX_S32 s32Ret = -1;
    SAMPLE_JENC_GETSTREAM_PARA_T *pstPara;
    pstPara = (SAMPLE_JENC_GETSTREAM_PARA_T *)arg;
    AX_VENC_STREAM_S stStream;
    AX_U32 totalGetStream = 0;
    AX_CHAR esName[50] = {0};

    sprintf(esName, "jenc_%d.mjpeg", pstPara->VeChn); //Mjpeg stream

    FILE *fOut = fopen(esName, "wb");
    if (fOut == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: open output file error!\n", pstPara->VeChn);
        return NULL;
    }

// pstPara->pCmdPara->output;

    memset(&stStream, 0, sizeof(AX_VENC_STREAM_S));

    while (AX_TRUE == pstPara->bThreadStart) {

        s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, gSyncType);
        if (AX_SUCCESS == s32Ret)
        {
            fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, fOut);

            fflush(fOut);

            totalGetStream++;

            SAMPLE_LOG("=== Encoded mjpeg get stream num: %d, seq %lld", totalGetStream, stStream.stPack.u64SeqNum);

            s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("ChnId:%d: AX_JENC_ReleaseStream failed!\n", pstPara->VeChn);
                goto EXIT;
            }
        }
    }

EXIT:
    if (fOut != NULL) {
        fclose(fOut);
        fOut = NULL;
    }

    return (void *)(intptr_t)s32Ret;
}

/* get stream thread */
static void *JpegencGetStreamProc(void *arg)
{
    AX_S32 s32Ret = -1;
    SAMPLE_JENC_GETSTREAM_PARA_T *pstPara;
    pstPara = (SAMPLE_JENC_GETSTREAM_PARA_T *)arg;
    AX_VENC_STREAM_S stStream;
    AX_U64 timeEndFrame = 0;
    struct timeval timeFrameEnd;

    AX_CHAR esName[50] = {0};
    if (pstPara->enCodecFormat == PT_JPEG)
        sprintf(esName, "jenc_%d.jpg", pstPara->VeChn);
    else if (pstPara->enCodecFormat == PT_MJPEG)
        sprintf(esName, "jenc_%d.mjpeg", pstPara->VeChn); //Mjpeg stream

    FILE *fOut = fopen(esName, "wb");
    if (fOut == NULL) {
        SAMPLE_ERR_LOG("ChnId:%d: open output file error!\n", pstPara->VeChn);
        return NULL;
    }

// pstPara->pCmdPara->output;

    memset(&stStream, 0, sizeof(AX_VENC_STREAM_S));
    AX_VENC_CHN_STATUS_S venc_status = {0};

    while (AX_TRUE == pstPara->bThreadStart) {
        s32Ret = AX_VENC_QueryStatus(pstPara->VeChn, &venc_status);
        if (AX_SUCCESS == s32Ret) {
#ifdef ENABLE_QUERY_STATUS_LOG
            SAMPLE_LOG("Chn %d: get status leftPics %d, leftStreamBytes %d, leftStreamFrames %d",
                        pstPara->VeChn,
                        venc_status.u32LeftPics,
                        venc_status.u32LeftStreamBytes,
                        venc_status.u32LeftStreamFrames);
#endif
        }

        s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, gSyncType);
        switch (s32Ret) {
        case AX_SUCCESS:
            pthread_mutex_lock(&gOutputMutex[pstPara->VeChn]);
            gettimeofday(&timeFrameEnd, NULL);
            //SAMPLE_LOG("AX_VENC_GetStream pu8Addr:0x%llx u32Len:%d ",
            //    stStream.pstPack->pu8Addr, stStream.pstPack->u32Len);
            /* save encode data */
            fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, fOut);

            fflush(fOut);

            gSaveOneFrameEnd[pstPara->VeChn] = true;
            pthread_cond_signal(&gOutputCond[pstPara->VeChn]);
            pthread_mutex_unlock(&gOutputMutex[pstPara->VeChn]);
            timeEndFrame = timeFrameEnd.tv_sec * 1000000 + timeFrameEnd.tv_usec;

            SAMPLE_LOG("=== Encoded jpeg Time(us %lld HW +SW), pts %lld, seq %lld",
                timeEndFrame - stStream.stPack.u64PTS, stStream.stPack.u64PTS, stStream.stPack.u64SeqNum);

            MemPrint(stStream.stPack.pu8Addr, stStream.stPack.u32Len);

            s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("ChnId:%d: AX_JENC_ReleaseStream failed!\n", pstPara->VeChn);
                break;
            }
            break;
        case AX_ERR_VENC_FLOW_END:
            SAMPLE_LOG("ChnId:%d: getStream thread exit.\n", pstPara->VeChn);
            /* end of stream, getStream exit. */
            goto EXIT;
            break;
        case AX_ERR_VENC_BUF_EMPTY:
        case AX_ERR_VENC_TIMEOUT:
            SAMPLE_ERR_LOG("ChnId:%d: error type 0x%x.\n", pstPara->VeChn, s32Ret);
            break;
        default:
            SAMPLE_ERR_LOG("ChnId:%d: error type 0x%x.\n", pstPara->VeChn, s32Ret);
            goto EXIT;
        }
    }

EXIT:
    if (fOut != NULL) {
        fclose(fOut);
        fOut = NULL;
    }
#if ENABLE_DEBUG
    SAMPLE_LOG("GetStreamProc exit!, s32Ret:%d", s32Ret);
#endif
    return (void *)(intptr_t)s32Ret;
}

/* get stream thread */
static void *JpegencGetStreamListProc(void *arg)
{
    AX_S32 s32Ret = -1;
    SAMPLE_JENC_GETSTREAM_PARA_T *pstPara;
    pstPara = (SAMPLE_JENC_GETSTREAM_PARA_T *)arg;
    AX_VENC_STREAM_S stStream;
    AX_U64 timeEndFrame = 0;
    struct timeval timeFrameEnd;
    AX_U64 FileSeq = -1;

    AX_CHAR esName[50];
    FILE *fOut = NULL;

    if (pstPara->enCodecFormat == PT_JPEG)
        sprintf(esName, "jenc_%d_0.jpg", pstPara->VeChn);
    else if (pstPara->enCodecFormat == PT_MJPEG)
        sprintf(esName, "jenc_%d_0.mjpeg", pstPara->VeChn); //Mjpeg stream

    memset(&stStream, 0, sizeof(AX_VENC_STREAM_S));

    while (AX_TRUE == pstPara->bThreadStart) {

        s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, gSyncType);
        switch (s32Ret) {
        case AX_SUCCESS:
            if (stStream.stPack.u64UserData != FileSeq) {
                if (fOut != NULL) {
                    fclose(fOut);
                    fOut = NULL;
                }

                if (pstPara->enCodecFormat == PT_JPEG) {
                    sprintf(esName, "jenc_%d_%lld.jpg", pstPara->VeChn, stStream.stPack.u64UserData);
                } else if (pstPara->enCodecFormat == PT_MJPEG) {
                    sprintf(esName, "jenc_%d_%lld.mjpeg", pstPara->VeChn, stStream.stPack.u64UserData); //Mjpeg stream
                }
#if ENABLE_DEBUG
                SAMPLE_LOG("File list: output file name %s\n", esName);
#endif
                fOut = fopen(esName, "wb");
                if (fOut == NULL) {
                    SAMPLE_ERR_LOG("ChnId:%d: open output file error!\n", pstPara->VeChn);
                    return NULL;
                }
                FileSeq = stStream.stPack.u64UserData;
            }

            gettimeofday(&timeFrameEnd, NULL);
            if (fOut == NULL)
            {
                SAMPLE_ERR_LOG("ChnId:%d: output file null!\n", pstPara->VeChn);
                return NULL;
            }

            /* save encode data */
            fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, fOut);
            fflush(fOut);

            timeEndFrame = timeFrameEnd.tv_sec * 1000000 + timeFrameEnd.tv_usec;

            SAMPLE_LOG("=== Encoded jpeg Time(us %lld HW +SW), pts %lld, seq %lld",
                timeEndFrame - stStream.stPack.u64PTS, stStream.stPack.u64PTS, stStream.stPack.u64SeqNum);

            MemPrint(stStream.stPack.pu8Addr, stStream.stPack.u32Len);

            s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("ChnId:%d: AX_JENC_ReleaseStream failed!\n", pstPara->VeChn);
                break;
            }
            break;
        case AX_ERR_VENC_FLOW_END:
            SAMPLE_LOG("ChnId:%d: getStream thread exit.\n", pstPara->VeChn);
            /* end of stream, getStream exit. */
            goto EXIT;
            break;
        case AX_ERR_VENC_BUF_EMPTY:
        case AX_ERR_VENC_TIMEOUT:
            SAMPLE_ERR_LOG("ChnId:%d: error type %d.\n", pstPara->VeChn, s32Ret);
            break;
        default:
            SAMPLE_ERR_LOG("ChnId:%d: error type %d.\n", pstPara->VeChn, s32Ret);
            goto EXIT;
        }
    }

EXIT:
    if (fOut != NULL) {
        fclose(fOut);
        fOut = NULL;
    }
#if ENABLE_DEBUG
    SAMPLE_LOG("GetStreamProc exit!, s32Ret:%d", s32Ret);
#endif
    return (void *)(intptr_t)s32Ret;
}

static AX_BOOL JencPoolInit(AX_U32 frameSize,VENC_CHN VeChn)
{
    /* use pool to alloc buffer */
    AX_POOL_CONFIG_T stPoolConfig;
    AX_POOL s32UserPoolId;

    memset(&stPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    stPoolConfig.MetaSize = 512;
    stPoolConfig.BlkSize = frameSize;
    if(frameSize >= (MAX_WIDTH_DEFAULT*MAX_HEIGHT_DEFAULT*3/2))
        stPoolConfig.BlkCnt = 2;
    else
        stPoolConfig.BlkCnt = 4;
    stPoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(stPoolConfig.PartitionName, 0, sizeof(stPoolConfig.PartitionName));
    strcpy((char *)stPoolConfig.PartitionName, "anonymous");

    s32UserPoolId = AX_POOL_CreatePool(&stPoolConfig);
    if (AX_INVALID_POOLID == s32UserPoolId) {
        SAMPLE_ERR_LOG("VeChn:%d Create pool err.\n",VeChn);
    }
    return s32UserPoolId;
}

static AX_S32 SampleJencStart(SAMPLE_CMD_PARA_T *Para)
{
    AX_S32 s32Ret;
    SAMPLE_CMD_PARA_T * pCmdl = Para;
    AX_U32 input_width = pCmdl->lumWidthSrc;
    AX_U32 input_height = pCmdl->lumHeightSrc;
    AX_U32 stride[3] = {0};
    AX_U32 output_width = pCmdl->lumWidthSrc;
    AX_U32 output_height = pCmdl->lumHeightSrc;
    AX_IMG_FORMAT_E enFrameFormat = pCmdl->enFrameFormat;
    AX_S32 ChnNum = pCmdl->EncChnNum;
    AX_S32 FrameSize = 0;
    AX_U32 cid;
    AX_POOL userPoolId;

    gFileInput = pCmdl->input;

    for (cid = 0; cid < ChnNum; cid++) {
        pthread_mutex_init(&gOutputMutex[cid], NULL);
        pthread_cond_init(&gOutputCond[cid], NULL);

        /* JENC channel arams */
        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(stVencChnAttr));

        stVencChnAttr.stVencAttr.u32MaxPicHeight = pCmdl->maxHeight;
        stVencChnAttr.stVencAttr.u32MaxPicWidth = pCmdl->maxWidth;


        if (!pCmdl->picStride[0]) {
            stride[0] = input_width;

            if (enFrameFormat == AX_YUV420_PLANAR) {
                stride[1] = stride[0] / 2;
                stride[2] = stride[0] / 2;
            } else if (enFrameFormat == AX_YUV420_SEMIPLANAR || enFrameFormat == AX_YUV420_SEMIPLANAR_VU) {
                stride[1] = stride[0];
                stride[2] = 0;
            }
        } else {
            for (int i = 0; i < 3; i++) {
                stride[i] = pCmdl->picStride[i];
            }
        }

        stVencChnAttr.stVencAttr.u32PicWidthSrc = input_width;
        stVencChnAttr.stVencAttr.u32PicHeightSrc = input_height;

        stVencChnAttr.stVencAttr.u32BufSize = input_width * input_height * 2;

        if (pCmdl->eCompressMode) {
            stVencChnAttr.stVencAttr.u32BufSize = pCmdl->YheaderSize + pCmdl->YpayloadSize + pCmdl->UVheaderSize + pCmdl->UVpayloadSize;
        }

        /* crop setting */
        if ((pCmdl->output_width != INVALID_DEFAULT) && (pCmdl->output_height != INVALID_DEFAULT)) {
            SAMPLE_LOG(":user crop setting:horOffsetSrc:%d,verOffsetSrc:%d,output_width:%d,output_height:%d",
                       pCmdl->horOffsetSrc,pCmdl->verOffsetSrc,pCmdl->output_width,pCmdl->output_height);
            /* calculate offset of the output picture */
            gEncodePara[cid].s16OffsetTop = pCmdl->verOffsetSrc;
            gEncodePara[cid].s16OffsetBottom = pCmdl->output_height + pCmdl->verOffsetSrc;
            gEncodePara[cid].s16OffsetLeft = pCmdl->horOffsetSrc;
            gEncodePara[cid].s16OffsetRight = pCmdl->output_width + pCmdl->horOffsetSrc;
        }else{
            gEncodePara[cid].s16OffsetTop = 0;
            gEncodePara[cid].s16OffsetBottom = 0;
            gEncodePara[cid].s16OffsetLeft = 0;
            gEncodePara[cid].s16OffsetRight = 0;
        }

#if ENABLE_DEBUG
        SAMPLE_LOG(".u32PicWidthSrc:%d .u32PicHeightSrc:%d",
             stVencChnAttr.stVencAttr.u32PicWidthSrc, stVencChnAttr.stVencAttr.u32PicHeightSrc);
        for (int i = 0; i < 3; i++) {
            SAMPLE_LOG("stride[%d], %d", i, stride[i]);
        }
#endif

        stVencChnAttr.stVencAttr.enLinkMode = AX_NONLINK_MODE;

        switch (enFrameFormat) {
        case AX_YUV420_PLANAR:
            FrameSize = stride[0] * input_height +
                stride[1] * input_height / 2 +
                stride[2] * input_height / 2;
            break;
        case AX_YUV420_SEMIPLANAR:
        case AX_YUV420_SEMIPLANAR_VU:
            FrameSize = stride[0] * input_height +
                stride[1] * input_height / 2;
            break;
        default:
            SAMPLE_ERR_LOG("%s ChnId:%d: unknow input type:%d",
                SAMPLE_NAME, cid, enFrameFormat);
            goto END;
        }

        if (pCmdl->eCompressMode) {
            FrameSize = pCmdl->YheaderSize + pCmdl->YpayloadSize + pCmdl->UVheaderSize + pCmdl->UVpayloadSize;
        }

#ifndef ENABLE_BITRATEJAM_STRATEGY
        if (cid % 2 == 0) {
            stVencChnAttr.stVencAttr.enType = PT_JPEG;
            gEncodePara[cid].enCodecFormat = PT_JPEG;
            gGetStreamPara[cid].enCodecFormat = PT_JPEG;
        } else {
            stVencChnAttr.stVencAttr.enType = PT_MJPEG;
            gEncodePara[cid].enCodecFormat = PT_MJPEG;
            gGetStreamPara[cid].enCodecFormat = PT_MJPEG;
        }
#else
        stVencChnAttr.stVencAttr.enType = PT_MJPEG;
        gEncodePara[cid].enCodecFormat = PT_MJPEG;
        gGetStreamPara[cid].enCodecFormat = PT_MJPEG;
#endif

        switch (stVencChnAttr.stVencAttr.enType) {
            case PT_JPEG:
            {
            }
            break;
        case PT_MJPEG:
            if (pCmdl->rcMode == JPEGENC_CBR) {
                AX_VENC_MJPEG_CBR_S stMjpegCbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stMjpegCbr.u32StatTime = 1;
                stMjpegCbr.u32SrcFrameRate = pCmdl->srcFrameRate;;
                stMjpegCbr.fr32DstFrameRate = pCmdl->frameRateNum; //pCmdl->frameRateNum / pCmdl->frameRateDenom;
                stMjpegCbr.u32BitRate = pCmdl->bitRate;
                stMjpegCbr.u32MinQp = pCmdl->qpmin;
                stMjpegCbr.u32MaxQp = pCmdl->qpmax;
                memcpy(&stVencChnAttr.stRcAttr.stMjpegCbr, &stMjpegCbr, sizeof(AX_VENC_MJPEG_CBR_S));
            } else if (pCmdl->rcMode == JPEGENC_VBR) {
                AX_VENC_MJPEG_VBR_S stMjpegVbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stMjpegVbr.u32StatTime = 1;
                stMjpegVbr.u32SrcFrameRate = pCmdl->srcFrameRate;
                stMjpegVbr.fr32DstFrameRate = pCmdl->frameRateNum; //pCmdl->frameRateNum / pCmdl->frameRateDenom;
                stMjpegVbr.u32MaxBitRate = pCmdl->bitRate;
                stMjpegVbr.u32MinQp = pCmdl->qpmin;
                stMjpegVbr.u32MaxQp = pCmdl->qpmax;
                memcpy(&stVencChnAttr.stRcAttr.stMjpegVbr, &stMjpegVbr, sizeof(AX_VENC_MJPEG_VBR_S));
            } else if (pCmdl->rcMode == JPEGENC_FIXQP) {
                AX_VENC_MJPEG_FIXQP_S stMjpegFixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegFixQp.u32SrcFrameRate = pCmdl->srcFrameRate;
                stMjpegFixQp.fr32DstFrameRate = pCmdl->frameRateNum; //pCmdl->frameRateNum / pCmdl->frameRateDenom;
                stMjpegFixQp.s32FixedQp = pCmdl->fixedQP;
                memcpy(&stVencChnAttr.stRcAttr.stMjpegFixQp, &stMjpegFixQp, sizeof(AX_VENC_MJPEG_FIXQP_S));
            }
            break;
        default:
            SAMPLE_ERR_LOG(" unknow codec format:%d !\n", stVencChnAttr.stVencAttr.enType);
            goto END;
        }

        userPoolId = JencPoolInit(FrameSize,cid);

        if(userPoolId == AX_INVALID_POOLID)
        {
            SAMPLE_ERR_LOG("VeChn %d JencPoolInit failed\n", cid);
            goto END;
        }

        s32Ret = AX_VENC_CreateChn(cid, &stVencChnAttr);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_JENC_CreateChn:%d failed!s32Ret=0x%x\n", cid,s32Ret);
            goto END;
        }

        AX_VENC_JPEG_PARAM_S stJpegParam;
        memset(&stJpegParam, 0, sizeof(stJpegParam));
        s32Ret = AX_VENC_GetJpegParam(cid, &stJpegParam);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_GetJpegParam:%d failed!\n", cid);
            goto END;
        }

        stJpegParam.u32Qfactor = pCmdl->qFactor;
        /* Use user set qtable. Qtable example */
        if (pCmdl->qTableEnable) {
            memcpy(stJpegParam.u8YQt, QTableLuminance, sizeof(QTableLuminance));
            memcpy(stJpegParam.u8CbCrQt, QTableChrominance, sizeof(QTableChrominance));
        }

        s32Ret = AX_VENC_SetJpegParam(cid, &stJpegParam);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_SetJpegParam:%d failed!\n", cid);
            goto END;
        }

#ifdef ENABLE_DYNAMIC_RC
        AX_VENC_RC_PARAM_S stRcParam;
        if (stVencChnAttr.stVencAttr.enType == PT_MJPEG) {

            s32Ret = AX_VENC_GetRcParam(cid, &stRcParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetRcParam:%d failed!\n",cid);
                goto END;
            }

            if (pCmdl->rcMode == JPEGENC_CBR) {
                stRcParam.stMjpegCbr.u32BitRate = 4000;//kbps
                stRcParam.stMjpegCbr.u32MinQp  = 20;
                stRcParam.stMjpegCbr.u32MaxQp  = 30;
            } else if (pCmdl->rcMode == JPEGENC_VBR) {
                stRcParam.stMjpegVbr.u32MaxBitRate  = 4000;//kbps
                stRcParam.stMjpegVbr.u32MinQp  = 20;
                stRcParam.stMjpegVbr.u32MaxQp  = 30;
            } else if (pCmdl->rcMode == JPEGENC_FIXQP) {
                stRcParam.stMjpegFixQp.s32FixedQp = 22;
            }

            s32Ret = AX_VENC_SetRcParam(cid, &stRcParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_SetRcParam:%d failed!\n",cid);
                goto END;
            }

        }
#endif

        /* init frame/stream buffer parameters */
        gEncodePara[cid].input_width = input_width;
        gEncodePara[cid].input_height = input_height;
        gEncodePara[cid].bThreadStart = AX_TRUE;
        gEncodePara[cid].VeChn = cid;
        gEncodePara[cid].enFrameFormat = enFrameFormat;
        gEncodePara[cid].FrameSize = FrameSize;
        gEncodePara[cid].userPoolId = userPoolId;
        gEncodePara[cid].pCmdPara = pCmdl;


        for (int i = 0; i < 3; i++) {
            gEncodePara[cid].stride[i] = stride[i];
        }

        gGetStreamPara[cid].bThreadStart = AX_TRUE;
        gGetStreamPara[cid].VeChn = cid;
        gGetStreamPara[cid].pCmdPara = pCmdl;

        /* create get output stream thread */
        if (pCmdl->inputPath == NULL) {
#ifndef ENABLE_BITRATEJAM_STRATEGY
            pthread_create(&gGetStreamPid[cid], NULL, JpegencGetStreamProc, (void *)&gGetStreamPara[cid]);
            pthread_create(&gEncodePid[cid], NULL, JpegencEncodeUsage1Proc, (void *)&gEncodePara[cid]);
#else
            pthread_create(&gGetStreamPid[cid], NULL, MjpegencGetStreamProc, (void *)&gGetStreamPara[cid]);
            pthread_create(&gEncodePid[cid], NULL, MjpegencEncodeUsage1Proc, (void *)&gEncodePara[cid]);
#endif
        } else {
            pthread_create(&gGetStreamPid[cid], NULL, JpegencGetStreamListProc, (void *)&gGetStreamPara[cid]);
            pthread_create(&gEncodePid[cid], NULL, JpegencEncodeFileListProc, (void *)&gEncodePara[cid]);
        }

    }

    while (!gLoopExit) {
        sleep(2);
    }

END:
    /* release encoder resource */
    for (cid = 0; cid < ChnNum; cid++) {
        if (AX_TRUE == gEncodePara[cid].bThreadStart) {
            gEncodePara[cid].bThreadStart = AX_FALSE;
            pthread_join(gEncodePid[cid], NULL);
        }

        AX_VENC_StopRecvFrame(cid);
        AX_VENC_DestroyChn(cid);

        pthread_mutex_destroy(&gOutputMutex[cid]);
        pthread_cond_destroy(&gOutputCond[cid]);

        printf("%s %d exit!\n", SAMPLE_NAME, cid);
    }

    gLoopExit = 0;
    return AX_SUCCESS;
}

static void SigInt(AX_S32 sigNo)
{
    printf("Catch signal %d\n", sigNo);
    gLoopExit = 1;
}


int main(int argc, char *argv[])
{
    AX_S32 s32Ret = -1;
    SAMPLE_CMD_PARA_T cmdLinePara;
    AX_VENC_MOD_ATTR_S stModAttr = {.enVencType = VENC_JPEG_ENCODER};

    printf("Jpegenc Sample. Build at %s %s\n", __DATE__, __TIME__);

    signal(SIGINT, SigInt);

    JencSetDefaultParameter(&cmdLinePara);

    s32Ret = JencParameterGet(argc, argv, &cmdLinePara);
    if (s32Ret) {
        SAMPLE_ERR_LOG("Invalid input argument!\n");
        return -1;
    }

    /* SYS global init */
    s32Ret = AX_SYS_Init();
	if(s32Ret) {
		SAMPLE_ERR_LOG("AX_SYS_Init failed! ret:0x%x\n", s32Ret);
		goto SYS_DEINIT;
    }

	s32Ret = AX_POOL_Exit();
	if(s32Ret) {
		SAMPLE_ERR_LOG("AX_POOL_Exit failed! ret:0x%x\n", s32Ret);
		goto SYS_DEINIT;
    }

    s32Ret = AX_VENC_Init(&stModAttr);
    if (s32Ret) {
        SAMPLE_ERR_LOG("AX_VENC_Init failed! ret:0x%x\n", s32Ret);
        goto POOL_EXIT;
    }

    s32Ret = SampleJencStart(&cmdLinePara);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("SampleJencStart failed! ret:0x%x\n", s32Ret);
        goto JENC_DEINIT;
    }

    s32Ret = AX_VENC_Deinit();
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("AX_VENC_Deinit failed! ret:0x%x\n", s32Ret);
    }

    s32Ret = AX_SYS_Deinit();
    if(s32Ret) {
        SAMPLE_ERR_LOG("AX_SYS_Deinit failed! ret:0x%x\n", s32Ret);
    }

    printf("sample_jpegenc test end success.\n");
    return 0;

JENC_DEINIT:
    AX_VENC_Deinit();

POOL_EXIT:
    AX_POOL_Exit();

SYS_DEINIT:
    AX_SYS_Deinit();

    return -1;
}
