/**********************************************************************************
 *
 * Copyright (c) 2022-2023 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "ax_sys_api.h"
#include "ax_venc_api.h"
#include "common_venc.h"

/* Max resolution: 5584x4188 */
#define VENC_MAX_WIDTH  5584
#define VENC_MAX_HEIGHT 4188
#define VENC_MIN_WIDTH  136
#define VENC_MIN_HEIGHT 136
/**************************************
* QPMAP
**************************************/
#define QpMapBufNum 10
#define HEVC_MAX_CU_SIZE 64
#define AVC_MAX_CU_SIZE 16
// #define MAX_CU_SIZE 64
#define ROIMAP_PREFETCH_EXT_SIZE 1536
#define REQUEST_IDR_INTERVAL 5
#define HEVC_STREAM 1 //0: HEVC, 1:264
// #define ENABLE_DYNAMIC_FRAMERATE 1
// #define ENABLE_DYNAMIC_BITRATE 1

//#define ENABLE_REQUEST_IDR
//#define GET_STREAM_SELECT
//#define ROI_ENABLE
//#define DYNAMIC_RC
//#define DYNAMIC_FRAMERATE
//#define DYNAMIC_SET_HW_FREQUENCE
//#define ENABLE_DYNAMIC_VUI
//#define ENABLE_START_STOP_TEST  // stopRecv immediately after startRevc
//#define DYNAMIC_VBR_PARAM
//#define ENABLE_BITRATEJAM_STRATEGY
//#define ENABLE_INSERT_USERDATA_SEI
//#define ENABLE_SUPERFRAME_STRATEGY
//#define ENABLE_QUERY_STATUS_LOG
//#define ENABLE_DYNAMIC_LEVEL_AND_PROFILE
//#define ENABLE_DYNAMIC_RcMode
static AX_S32 gSceneChangeEnable = 0;

#ifdef __linux
#include <sys/syscall.h>
#endif
#define gettid() syscall(__NR_gettid)

#define SAMPLE_NAME "SampleCommonVenc"
#define VENC_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"", SAMPLE_NAME, __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_LOG(str, arg...)    \
    do {    \
        printf("%s: %s:%d "str"\n", SAMPLE_NAME, __func__, __LINE__, ##arg); \
    } while(0)

#define SAMPLE_ERR_LOG(str, arg...)   \
    do{  \
        printf("%s: %s:%d Error! "str"\n", "common_venc.c", __func__, __LINE__, ##arg); \
    }while(0)

#define CLIP3(x, y, z)  ((z) < (x) ? (x) : ((z) > (y) ? (y) : (z)))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))

static AX_VENC_ROI_ATTR_S gGetRoiAttr[MAX_ROI_NUM];
/* roi map delta qp */
//static AX_VENC_QPMAP_QP_TYPE_E gEnableQpmap = VENC_QPMAP_DISABLE;
//static AX_VENC_RC_CTBRC_MODE_E gCtbRcType = VENC_RC_CTBRC_QUALITY;

/* hevc support four blocks, 0: 64x64 1: 32x32 2: 16x16 3: 8x8 */
/* h264 support three blocks, 0: 64x64 1: 32x32 2: 16x16 */
static AX_U32 gQpmapBlockUnit = 0;

static AX_S32 gLoopExit = 0;

static pthread_mutex_t gOutputMutex[MAX_VENC_NUM];
static pthread_cond_t gOutputCond[MAX_VENC_NUM];
static AX_BOOL gSaveOneFrameEnd[MAX_VENC_NUM] = {false};


static SAMPLE_VENC_GETSTREAM_PARA_T gGetStreamPara[MAX_VENC_NUM];
static pthread_t gGetStreamPid[MAX_VENC_NUM];

static SAMPLE_VENC_GETSTREAM_PARA_T gstGetStreamSelectPara = {.bThreadStart = AX_TRUE};
static pthread_t gstGetStreamSelectPid;

static SAMPLE_VENC_ENCODE_PARA_T gEncodePara[MAX_VENC_NUM];
static pthread_t gEncodePid[MAX_VENC_NUM];
static pthread_attr_t attr;  // used when GET_STREAM_SELECT open

static int SampleSetRoiAttr(AX_S32 VeChn, SAMPLE_VENC_CMD_PARA_T *pstArg);

#if 0
/* SVC-T Configure */
static AX_U32 gSvcTGopSize = 4;
/*SVC-T GOP4*/
static char *gSvcTCfg[] = {
    "Frame1:  P      1      0       0.4624        2        1           -1          1",
    "Frame2:  P      2      0       0.4624        1        1           -2          1",
    "Frame3:  P      3      0       0.4624        2        2           -1 -3       1 0",
    "Frame4:  P      4      0       0.4624        0        1           -4          1",
    NULL,
};
#endif

AX_U32 TimeDiff(struct timeval end, struct timeval start)
{
   return (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
}

void SetMaskSIGALRM()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGALRM);
    pthread_sigmask(SIG_SETMASK,&set,NULL);
    return;
}

/* one thread select all channel stream */
void *VencOneThreadGetAllStreamProc(void *arg)
{
    AX_S32 s32Ret = -1;
    SAMPLE_VENC_GETSTREAM_PARA_T *pstPara;
    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_T *)arg;
    AX_VENC_STREAM_S stStream[MAX_VENC_NUM];
    FILE *pStrm[MAX_VENC_NUM];
    AX_S8 esName[MAX_VENC_NUM][50];
    AX_CHN_STREAM_STATUS_S stChnStrmState;
    AX_U32 u32ChnEncodeFrms[MAX_VENC_NUM] = {0};
    SAMPLE_VENC_CMD_PARA_T *pCmdl = pstPara->ChnPara.pCmdl;
    AX_S32 chnNum = pCmdl->chnNum;

    SetMaskSIGALRM();
    for (int i = 0; i < chnNum; i++)
    {
        if (i % 2 == HEVC_STREAM)
            sprintf(esName[i], "enc_%d.265", i);
        else
            sprintf(esName[i], "enc_%d.264", i);

        pStrm[i] = fopen(esName[i], "wb");
        if (pStrm[i] == NULL)
        {
            VENC_LOG("enc %d: open output file error!\n", i);
            goto EXIT;
        }

        memset(&stStream[i], 0, sizeof(AX_VENC_STREAM_S));
    }

    while (AX_TRUE == pstPara->bThreadStart) {
        s32Ret = AX_VENC_SelectChn(&stChnStrmState, -1);
        if (AX_SUCCESS != s32Ret) {
            //VENC_LOG("ERR: select err, no stream.\n");
            continue;
        }

        for (int i = 0; i < stChnStrmState.u32TotalChnNum; i++) {
            int channelID = stChnStrmState.au32ChnIndex[i];
            s32Ret = AX_VENC_GetStream(channelID, &stStream[channelID], 0);
            if (AX_SUCCESS == s32Ret) {
                    fwrite(stStream[channelID].stPack.pu8Addr,
                            1,
                            stStream[channelID].stPack.u32Len,
                            pStrm[channelID]);

                    fflush(pStrm[channelID]);

                    u32ChnEncodeFrms[channelID]++;

                    s32Ret = AX_VENC_ReleaseStream(channelID, &stStream[channelID]);
                    if (AX_SUCCESS != s32Ret) {
                        VENC_LOG("AX_VENC_ReleaseStream failed!\n");
                        goto EXIT;
                    }
            }
        }
    }

EXIT:

    for (int i = 0; i < chnNum; i++) {
        if (pStrm[i] != NULL) {
            fclose(pStrm[i]);
            pStrm[i] = NULL;
        }

        VENC_LOG("venc %d: Total get %u encoded frames. getStream Exit!\n", i, u32ChnEncodeFrms[i]);
    }

    return (void *)(intptr_t)s32Ret;
}

static AX_U32 LoadFrameFromFile(FILE *pFileIn, AX_S32 widthSrc, AX_S32 strideSrc, AX_S32 heightSrc, AX_IMG_FORMAT_E eFmt, AX_VOID *pVaddr)
{
    AX_U32 i, rows, realRead, readSize;

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
            SAMPLE_ERR_LOG("Invalid format, eFmt = %d\n", eFmt);
    }

    return readSize;
}

/* encode task */
static void *VencEncodeProc(void *arg)
{
    AX_S32 s32Ret = -1;
    AX_BOOL bLoopEncode = AX_FALSE;
    AX_S32 encodeFrmNum = -1;
    AX_U64 totalInputFrames = 0;
    struct timeval timeFrameStart;
    struct timeval timeFrameEnd;
    AX_U64 totalEncodeTime = 0;
    AX_U16 frameFormat = 0;
    AX_BOOL enablePerf = AX_FALSE;
    AX_S8 * fileInput;
    AX_S32 syncType = -1;
    AX_U32 frameSize;
    AX_POOL s32UserPoolId;
    AX_BLK BlkId = AX_INVALID_BLOCKID;
    AX_BOOL bChangeClk = AX_FALSE;
    AX_U32 vui_signal_flag = 0;
    AX_U32 vui_color_description_flag = 0;
    AX_U32 realRead, readSize = 0;
    AX_U32 Ytotal_size = 0;
    AX_U32 UVtotal_size = 0;
    AX_BOOL bChangeUserData = AX_FALSE;

    SAMPLE_VENC_ENCODE_PARA_T *pEncodePara = NULL;
    pEncodePara = (SAMPLE_VENC_ENCODE_PARA_T *)arg;

    SAMPLE_VENC_CMD_PARA_T *pCmdl = pEncodePara->ChnPara.pCmdl;

    bLoopEncode = pCmdl->loopEncode;
    encodeFrmNum = pCmdl->frameNum;
    frameFormat = pCmdl->inputFormat;
    fileInput = pCmdl->input;
    syncType = pCmdl->syncType;
    frameSize = pEncodePara->frameSize;
    s32UserPoolId = pEncodePara->userPoolId;

    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    AX_VIDEO_FRAME_INFO_S stFrame;

    /* Rate Control */
    AX_VENC_RC_PARAM_S stRcParam;

    SetMaskSIGALRM();

    /* input yuv file */
    FILE *fFileIn = NULL;

    fFileIn = fopen(fileInput, "rb");
    if (fFileIn == NULL) {
        SAMPLE_ERR_LOG("Open input file error!\n");
        return NULL;
    }

    if (AX_VENC_GetRcParam(pEncodePara->VeChn, &stRcParam) != AX_SUCCESS)
    {
        SAMPLE_ERR_LOG("VeChn:%d AX_VENC_GetRcParam error!\n", pEncodePara->VeChn);
        goto EXIT;
    }

    /* set rate control */
    stRcParam.stSceneChangeDetect.bDetectSceneChange = gSceneChangeEnable; // enable scene chaneg detect
    if (AX_VENC_SetRcParam(pEncodePara->VeChn, &stRcParam) != AX_SUCCESS)
    {
        SAMPLE_ERR_LOG("VeChn:%d AX_VENC_SetRcParam error!\n", pEncodePara->VeChn);
        goto EXIT;
    }

#ifdef ENABLE_BITRATEJAM_STRATEGY
    AX_VENC_RATE_JAM_CFG_S stRateJamParam;
    if (AX_VENC_GetRateJamStrategy(pEncodePara->VeChn, &stRateJamParam) != AX_SUCCESS)
    {
        printf("%s %d: AX_VENC_GetRateJamStrategy error!\n", SAMPLE_NAME, pEncodePara->VeChn);
        goto EXIT;
    }
    stRateJamParam.bDropFrmEn = AX_TRUE;
    stRateJamParam.enDropFrmMode = DROPFRM_PSKIP;
    stRateJamParam.u32MaxApplyCount = 3;
    stRateJamParam.u32DropFrmThrBps = 64 * 1024; // 64k bps
    if (AX_VENC_SetRateJamStrategy(pEncodePara->VeChn, &stRateJamParam) != AX_SUCCESS)
    {
        printf("%s %d: AX_VENC_SetRateJamStrategy error!\n", SAMPLE_NAME, pEncodePara->VeChn);
        goto EXIT;
    }
#endif

#ifdef ENABLE_SUPERFRAME_STRATEGY
    AX_VENC_SUPERFRAME_CFG_S stSuperFrameCfg;
    memset(&stSuperFrameCfg, 0, sizeof(AX_VENC_SUPERFRAME_CFG_S));
    s32Ret = AX_VENC_GetSuperFrameStrategy(pEncodePara->VeChn, &stSuperFrameCfg);
    if (AX_SUCCESS != s32Ret)
    {
        printf("%s %d: AX_VENC_GetSuperFrameStrategy error!\n", SAMPLE_NAME, pEncodePara->VeChn);
        goto EXIT;
    }

    stSuperFrameCfg.bStrategyEn = AX_TRUE;
    stSuperFrameCfg.u32MaxReEncodeTimes = 3;
    stSuperFrameCfg.u32SuperIFrmBitsThr = 300000;
    stSuperFrameCfg.u32SuperPFrmBitsThr = 60000;
    stSuperFrameCfg.u32SuperBFrmBitsThr = 60000;
    
    s32Ret = AX_VENC_SetSuperFrameStrategy(pEncodePara->VeChn, &stSuperFrameCfg);
    if (AX_SUCCESS != s32Ret)
    {
        printf("%s %d: AX_VENC_SetSuperFrameStrategy error!\n", SAMPLE_NAME, pEncodePara->VeChn);
        goto EXIT;
    }
#endif

    s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("Encoder thread exit, ret=%x\n", s32Ret);
        goto EXIT;
    }

#ifdef ENABLE_START_STOP_TEST
    goto EXIT;
#endif
    //MUST clear frame struct before use
    memset(&stFrame, 0, sizeof(AX_VIDEO_FRAME_INFO_S));

    while (pEncodePara->bThreadStart) {
        BlkId = AX_POOL_GetBlock(s32UserPoolId, frameSize, NULL);
        if (AX_INVALID_BLOCKID == BlkId) {
            //SAMPLE_ERR_LOG("VeChn:%d,Get block fail.s32UserPoolId=%d,frameSize=0x%x\n",pEncodePara->VeChn,s32UserPoolId,frameSize);
            usleep(5*1000);
            continue;
        }

        stFrame.stVFrame.u32FrameSize = frameSize;
        stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);
        stFrame.stVFrame.u32BlkId[0] = BlkId;
        stFrame.stVFrame.u32BlkId[1] = 0;//must set 0 if not used
        stFrame.stVFrame.u32BlkId[2] = 0;//must set 0 if not used

        if (pCmdl->enCompressMode) {
            SAMPLE_LOG("eCompressMode %d.\n", pCmdl->enCompressMode);

            Ytotal_size = pCmdl->YheaderSize + pCmdl->YpayloadSize;
            UVtotal_size = pCmdl->UVheaderSize + pCmdl->UVpayloadSize;

            realRead = fread(stFrame.stVFrame.u64VirAddr[0], 1, Ytotal_size, fFileIn);
            // SAMPLE_LOG(" realRead %d, Ytotal %d.\n", realRead, Ytotal_size);
            if (realRead < Ytotal_size) {
                //printf("%s %d: Warning: real read size %d less than %d\n", __func__, __LINE__, realRead, widthSrc);
                break;
            }

            stFrame.stVFrame.u64VirAddr[0] += Ytotal_size;
            stFrame.stVFrame.u64VirAddr[1] = stFrame.stVFrame.u64VirAddr[0];

            SAMPLE_LOG("frameSize:%d realRead:%d, Ytotal_size:%d UVtotal:%d.\n", frameSize, realRead, Ytotal_size, UVtotal_size);
            SAMPLE_LOG(".u64VirAddr[1]:0x%llx.\n", stFrame.stVFrame.u64VirAddr[1]);

            realRead = fread(stFrame.stVFrame.u64VirAddr[1], 1, UVtotal_size, fFileIn);
            if (realRead < UVtotal_size) {
                //printf("%s %d: Warning: real read size %d less than %d\n", __func__, __LINE__, realRead, widthSrc);
                break;
            }

            stFrame.stVFrame.u64VirAddr[1] += UVtotal_size;
            readSize = Ytotal_size + UVtotal_size;

            stFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId) + pCmdl->YheaderSize;
            stFrame.stVFrame.u64PhyAddr[1] = stFrame.stVFrame.u64PhyAddr[0]
                                                + pCmdl->YpayloadSize
                                                + pCmdl->UVheaderSize;

            SAMPLE_LOG("encodering Ydata Phyaddr is %llx, UV=%llx.\n", stFrame.stVFrame.u64PhyAddr[0], stFrame.stVFrame.u64PhyAddr[1]);
            SAMPLE_LOG("encodering Ydata Viraddr is %llx, UV=%llx.\n", stFrame.stVFrame.u64VirAddr[0], stFrame.stVFrame.u64VirAddr[1]);
            SAMPLE_LOG("AX_POOL_Handle2PhysAddr(BlkId) %llx, AX_POOL_GetBlockVirAddr(BlkId)=%p.\n",
                AX_POOL_Handle2PhysAddr(BlkId), AX_POOL_GetBlockVirAddr(BlkId));
            SAMPLE_LOG("YheaderSize %d, YpayloadSize %d, UVheaderSize %d.\n",
                pCmdl->YheaderSize, pCmdl->YpayloadSize, pCmdl->UVheaderSize);
        } else{
            /* read frame data from yuv file */
            readSize = LoadFrameFromFile(fFileIn,
                                        pEncodePara->width,
                                        pEncodePara->stride,
                                        pEncodePara->height,
                                        frameFormat,
                                        (void *)stFrame.stVFrame.u64VirAddr[0]);

            if (!bLoopEncode && (readSize <= 0)) {
                VENC_LOG("Warning: read frame size : %d less than %d\n", readSize, frameSize);
            }
        }

        if (feof(fFileIn)) {
            if (bLoopEncode) {
                fseek(fFileIn, 0, SEEK_SET);
                LoadFrameFromFile(fFileIn, pEncodePara->width, pEncodePara->stride,
                                    pEncodePara->height, frameFormat, (void *)stFrame.stVFrame.u64VirAddr[0]);
            } else {
                VENC_LOG("End of input file!\n");
                /* no more frames, stop encoder */
                goto EXIT;
            }
        }

#if ENABLE_DYNAMIC_FRAMERATE
        if (totalInputFrames == 60)
        {
            AX_VENC_RC_PARAM_S stRcParam;
			s32Ret = AX_VENC_GetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
            {
                SAMPLE_ERR_LOG("VeChn:%d AX_VENC_GetRcParam error!s32Ret=0x%x\n", pEncodePara->VeChn,s32Ret);
                goto EXIT;
            }
            
            // SAMPLE_LOG("VeChn:%d Get old dst frame rate %d\n", pEncodePara->VeChn,stRcParam.stRateCtrl.fr32DstFrameRate);
            SAMPLE_LOG("VeChn:%d ------- enRcMode:%d,Get old dst frame rate %d ,\n",  pEncodePara->VeChn, stRcParam.enRcMode, stRcParam.stH264Cbr.fr32DstFrameRate);
            SAMPLE_LOG("VeChn:%d ------- enRcMode:%d,Get old src frame rate %d ,\n",  pEncodePara->VeChn, stRcParam.enRcMode, stRcParam.stH264Cbr.u32SrcFrameRate);

            /* set dst frame rate */
            // stRcParam.stRateCtrl.fr32DstFrameRate = 10;//fps
            stRcParam.stH264Cbr.fr32DstFrameRate = 10;
            stRcParam.stH264Cbr.u32SrcFrameRate = 15;
			s32Ret = AX_VENC_SetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
            {
                SAMPLE_ERR_LOG("VeChn:%d AX_VENC_SetRcParam error!s32Ret=0x%x\n", pEncodePara->VeChn,s32Ret);
                goto EXIT;
            }
            // SAMPLE_LOG("VeChn:%d Set new dst frame rate %d\n", pEncodePara->VeChn,stRcParam.stRateCtrl.fr32DstFrameRate);
            SAMPLE_LOG("VeChn:%d ===== enRcMode:%d,new dst frame rate %d ,\n",  pEncodePara->VeChn, stRcParam.enRcMode, stRcParam.stH264Cbr.fr32DstFrameRate);
            SAMPLE_LOG("VeChn:%d ===== enRcMode:%d,new src frame rate %d ,\n",  pEncodePara->VeChn, stRcParam.enRcMode, stRcParam.stH264Cbr.u32SrcFrameRate);

        }
#endif

#if ENABLE_DYNAMIC_BITRATE
        if (totalInputFrames == 60)
        {
            AX_VENC_RC_PARAM_S stRcParam;
			s32Ret = AX_VENC_GetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
            {
                SAMPLE_ERR_LOG("VeChn:%d AX_VENC_GetRcParam error!s32Ret=0x%x\n", pEncodePara->VeChn,s32Ret);
                goto EXIT;
            }
            // SAMPLE_LOG("VeChn:%d Get old bitrate %d\n", pEncodePara->VeChn,stRcParam.stRateCtrl.u32BitPerSecond);
            SAMPLE_LOG("VeChn:%d ------- enRcMode:%d,Get old bitrate %d ,\n",  pEncodePara->VeChn, stRcParam.enRcMode, stRcParam.stH264Cbr.u32BitRate );


            /* set dst frame rate */
            // stRcParam.stRateCtrl.u32BitPerSecond = 1024000;//fps
            stRcParam.stH264Cbr.u32BitRate = 10240;//fps

			s32Ret = AX_VENC_SetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
            {
                SAMPLE_ERR_LOG("VeChn:%d AX_VENC_SetRcParam error!s32Ret=0x%x\n", pEncodePara->VeChn,s32Ret);
                goto EXIT;
            }
            // SAMPLE_LOG("VeChn:%d Set new bitrate %d\n", pEncodePara->VeChn,stRcParam.stRateCtrl.u32BitPerSecond);
             SAMPLE_LOG("VeChn:%d ------- enRcMode:%d,Set new bitrate %d ,\n",  pEncodePara->VeChn, stRcParam.enRcMode, stRcParam.stH264Cbr.u32BitRate );

        }
#endif
#ifdef ENABLE_DYNAMIC_VUI
        if (totalInputFrames % pCmdl->gopLength == 0)
        {
            AX_VENC_SPS_VUI_PARAM_S stGetSpsVuiParam;
            if (AX_VENC_GetSpsVuiParam(pEncodePara->VeChn, &stGetSpsVuiParam) != AX_SUCCESS)
            {
                printf("%s %d: AX_VENC_GetSpsVuiParam error!\n", SAMPLE_NAME, pEncodePara->VeChn);
                goto EXIT;
            }

            stGetSpsVuiParam.u32VideoSignalTypePresentFlag = vui_signal_flag;
            stGetSpsVuiParam.u32ColourDescriptionPresentFlag = vui_color_description_flag;
            stGetSpsVuiParam.u32ColourPrimaries = 9;
            stGetSpsVuiParam.u32MatrixCoefficients = 9;
            stGetSpsVuiParam.u32TransferCharacteristics = 0;

            if (AX_VENC_SetSpsVuiParam(pEncodePara->VeChn, &stGetSpsVuiParam) != AX_SUCCESS)
            {
                printf("%s %d: AX_VENC_SetSpsVuiParam error!\n", SAMPLE_NAME, pEncodePara->VeChn);
                goto EXIT;
            }
            vui_color_description_flag = !vui_color_description_flag;
            vui_signal_flag = !vui_signal_flag;
        }
#endif

#ifdef ENABLE_INSERT_USERDATA_SEI
        if (totalInputFrames % pCmdl->gopLength == 0)
        {
            if (!bChangeUserData)
            {
                AX_U8 userData[] = "test01: this is test for insert sei userdata.";
                s32Ret = AX_VENC_InsertUserData(pEncodePara->VeChn, userData, sizeof(userData));
            }
            else
            {
                AX_U8 userData[] = "test02: this is test for insert sei userdata.";
                s32Ret = AX_VENC_InsertUserData(pEncodePara->VeChn, userData, sizeof(userData));
            }
            if (s32Ret != AX_SUCCESS)
            {
                printf("%s %d: AX_VENC_InsertUserData error!\n", SAMPLE_NAME, pEncodePara->VeChn);
                goto EXIT;
            }

            bChangeUserData = !bChangeUserData;
        }
#endif
#ifdef ENABLE_DYNAMIC_LEVEL_AND_PROFILE
        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
        if (totalInputFrames == 100)
        {       
            s32Ret = AX_VENC_StopRecvFrame(pEncodePara->VeChn);
            if (AX_SUCCESS != s32Ret)
            {
                SAMPLE_ERR_LOG(" %d:  AX_VENC_StopRecvFrame failed, ret=%x\n", pEncodePara->VeChn, s32Ret);
            }
            s32Ret = AX_VENC_GetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (0 != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetChnAttr(chn: %d) failed, s32Ret=0x%x", pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }

            SAMPLE_LOG("VeChn:%d type: %d AX_VENC_GetChnAttr Get old enProfile %d ,\n",  pEncodePara->VeChn, 
                stVencChnAttr.stVencAttr.enType, stVencChnAttr.stVencAttr.enProfile);
            SAMPLE_LOG("VeChn:%d type: %d AX_VENC_GetChnAttr Get old level %d ,\n",  pEncodePara->VeChn, 
                stVencChnAttr.stVencAttr.enType, stVencChnAttr.stVencAttr.enLevel); 

            if (stVencChnAttr.stVencAttr.enType == PT_H264) {
                stVencChnAttr.stVencAttr.enProfile = VENC_H264_HIGH_10_PROFILE;
                stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_4_2;
            }
            else if (stVencChnAttr.stVencAttr.enType == PT_H265) {
                stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_10_PROFILE;
                stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6_1;
            }
            
        
            s32Ret = AX_VENC_SetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (0 != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_SetChnAttr(chn: %d) failed, s32Ret=0x%x", pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }

            memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
            s32Ret = AX_VENC_GetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (0 != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetChnAttr(chn: %d) failed, s32Ret=0x%x", pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }
            SAMPLE_LOG("VeChn:%d type: %d AX_VENC_GetChnAttr Get new enProfile %d ,\n",  pEncodePara->VeChn, 
                stVencChnAttr.stVencAttr.enType, stVencChnAttr.stVencAttr.enProfile);
            SAMPLE_LOG("VeChn:%d type: %d AX_VENC_GetChnAttr Get new level %d ,\n",  pEncodePara->VeChn, 
                stVencChnAttr.stVencAttr.enType, stVencChnAttr.stVencAttr.enLevel); 

            s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_StartRecvFrame fail, ret=%x\n", s32Ret);
                goto EXIT;
            }
        }
#endif
#ifdef ENABLE_DYNAMIC_RcMode
        /* RC setting */
        if (totalInputFrames == 30) {
            AX_VENC_CHN_ATTR_S stVencChnAttr;
            memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
            s32Ret = AX_VENC_StopRecvFrame(pEncodePara->VeChn);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG(" %d:  AX_VENC_StopRecvFrame failed, ret=%x\n", pEncodePara->VeChn, s32Ret);
            }
            s32Ret = AX_VENC_GetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (0 != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetChnAttr(chn: %d) failed, s32Ret=0x%x", pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }

            SAMPLE_LOG("VeChn:%d type: %d AX_VENC_GetChnAttr Get old enRcMode %d ,\n",  pEncodePara->VeChn,
                      stVencChnAttr.stVencAttr.enType, stVencChnAttr.stRcAttr.enRcMode);
            switch (stVencChnAttr.stVencAttr.enType) {
            case PT_H265:
            {
                AX_VENC_H265_FIXQP_S stH265FixQp;
                memset(&stH265FixQp, 0, sizeof(AX_VENC_H265_FIXQP_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                stH265FixQp.u32Gop = 30;
                stH265FixQp.u32SrcFrameRate = 30;
                stH265FixQp.fr32DstFrameRate = 30;
                stH265FixQp.u32IQp = 25;
                stH265FixQp.u32PQp = 30;
                stH265FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH265FixQp, &stH265FixQp, sizeof(AX_VENC_H265_FIXQP_S));
            }
            break;

            case PT_H264:
            {
                AX_VENC_H264_FIXQP_S stH264FixQp;
                memset(&stH264FixQp, 0, sizeof(AX_VENC_H264_FIXQP_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = 30;
                stH264FixQp.u32SrcFrameRate = 30;
                stH264FixQp.fr32DstFrameRate = 30;
                stH264FixQp.u32IQp = 25;
                stH264FixQp.u32PQp = 30;
                stH264FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH264FixQp, &stH264FixQp, sizeof(AX_VENC_H264_FIXQP_S));
            }
            break;

            default:
            {
                SAMPLE_ERR_LOG("Invalid Codec Format.\n");
                s32Ret = -1;
            }
            break;

            }

            s32Ret = AX_VENC_SetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (0 != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_SetChnAttr(chn: %d) failed, s32Ret=0x%x", pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }

            memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
            s32Ret = AX_VENC_GetChnAttr(pEncodePara->VeChn, &stVencChnAttr);
            if (0 != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetChnAttr(chn: %d) failed, s32Ret=0x%x", pEncodePara->VeChn, s32Ret);
                goto EXIT;
            }
            SAMPLE_LOG("VeChn:%d type: %d AX_VENC_GetChnAttr Get new enRcMode %d ,\n",  pEncodePara->VeChn,
                      stVencChnAttr.stVencAttr.enType, stVencChnAttr.stRcAttr.enRcMode);

            s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_StartRecvFrame fail, ret=%x\n", s32Ret);
                goto EXIT;
            }
        }
#endif
        /* the input frame sequence number */
        stFrame.stVFrame.u64SeqNum = totalInputFrames + 1;

        stFrame.stVFrame.enImgFormat = frameFormat;
        stFrame.stVFrame.u32Width = pEncodePara->width;
        stFrame.stVFrame.u32Height = pEncodePara->height;
        stFrame.stVFrame.u32PicStride[0] = pEncodePara->stride;
        stFrame.stVFrame.enCompressMode = pCmdl->enCompressMode;
        // SAMPLE_LOG(".enCompressMode:%d\n", stFrame.stVFrame.enCompressMode);

        if (frameFormat == AX_YUV420_PLANAR) {
            stFrame.stVFrame.u32PicStride[1] = pEncodePara->stride / 2;
            stFrame.stVFrame.u32PicStride[2] = pEncodePara->stride / 2;
        } else if (frameFormat == AX_YUV420_SEMIPLANAR || frameFormat == AX_YUV420_SEMIPLANAR_VU) {
            stFrame.stVFrame.u32PicStride[1] = pEncodePara->stride;
            stFrame.stVFrame.u32PicStride[2] = 0;
        } else if (frameFormat == AX_YUV422_INTERLEAVED_YUYV || frameFormat == AX_YUV422_INTERLEAVED_YUYV) {
            stFrame.stVFrame.u32PicStride[1] = 0;
            stFrame.stVFrame.u32PicStride[2] = 0;
        }

        stFrame.stVFrame.u64VirAddr[0] = 0;
        stFrame.stVFrame.u64VirAddr[1] = 0;
        stFrame.stVFrame.u64VirAddr[2] = 0;

        s32Ret = AX_VENC_SendFrame(pEncodePara->VeChn, &stFrame, syncType);
        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_SendFrame failed, ret=%x\n", s32Ret);
            s32Ret = AX_POOL_ReleaseBlock(stFrame.stVFrame.u32BlkId[0]);
            if (s32Ret)
                SAMPLE_ERR_LOG("release block err.\n");
            continue;
        }

        #ifdef ENABLE_REQUEST_IDR
        if (totalInputFrames % REQUEST_IDR_INTERVAL == 0)
        {
            AX_VENC_RequestIDR(pEncodePara->VeChn, true);
        }
        #endif
        totalInputFrames++;

#ifdef DYNAMIC_FRAMERATE
        AX_VENC_RC_PARAM_S stRcParam;
        if (totalInputFrames == 60) {
            AX_S32 s32Ret = -1;
            s32Ret = AX_VENC_GetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
                SAMPLE_ERR_LOG("AX_VENC_GetRcParam error.\n");

            if (pEncodePara->VeChn % 2 == HEVC_STREAM) {
                stRcParam.stH265Cbr.fr32DstFrameRate = 15;//fps
            } else {
                stRcParam.stH264Cbr.fr32DstFrameRate = 15;
            }

            s32Ret = AX_VENC_SetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
                SAMPLE_ERR_LOG("AX_VENC_SetRcParam error.\n");
        }
#endif

        #ifdef DYNAMIC_VBR_PARAM
        AX_VENC_RC_PARAM_S stRcParam;
        if (totalInputFrames == 120) {
            AX_S32 s32Ret = -1;
            s32Ret = AX_VENC_GetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
                SAMPLE_ERR_LOG("AX_VENC_GetRcParam error.\n");
            if (pEncodePara->VeChn % 2 == 0) {
                stRcParam.stH265Vbr.s32IntraQpDelta = -8;//fps
            } else {
                stRcParam.stH264Vbr.s32IntraQpDelta = -8;
            }

            SAMPLE_LOG("DYNAMIC_VBR_PARAM set intraQpDelta -8.\n");

            AX_VENC_SetRcParam(pEncodePara->VeChn, &stRcParam);
            if (AX_SUCCESS != s32Ret)
                SAMPLE_ERR_LOG("AX_VENC_SetRcParam error.\n");
        }
        #endif


        #ifdef DYNAMIC_SET_HW_FREQUENCE
        if (totalInputFrames % pCmdl->gopLength == 0)
        {
            AX_VENC_MOD_PARAM_S stModParam;
            AX_VENC_ENCODER_TYPE_E enVencType = VENC_VIDEO_ENCODER;
            memset(&stModParam, 0, sizeof(AX_VENC_MOD_PARAM_S));
            s32Ret = AX_VENC_GetModParam(enVencType, &stModParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_GetModParam failed, ret=%x\n", s32Ret);
                goto EXIT;
            }

            printf("get venc clk frequence: %d\n", stModParam.enVencHwClk);

            stModParam.enVencHwClk = bChangeClk ? VENC_MOD_CLK_FREQUENCY_624M : VENC_MOD_CLK_FREQUENCY_312M;
            s32Ret = AX_VENC_SetModParam(enVencType, &stModParam);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_SetModParam failed, ret=%x\n", s32Ret);
                goto EXIT;
            }
            bChangeClk = !bChangeClk;
        }
        #endif

        if (totalInputFrames == encodeFrmNum) {
            VENC_LOG("Want to encode %llu frames, exit!\n", totalInputFrames);
            goto EXIT;
        }
    }


EXIT:

    if(AX_INVALID_BLOCKID != BlkId){
        AX_POOL_ReleaseBlock(BlkId);
    }

    if (NULL != fFileIn) {
        fclose(fFileIn);
        fFileIn = NULL;
    }

    VENC_LOG("venc %d - Total input %llu frames, Encoder exit!\n", pEncodePara->VeChn, totalInputFrames);

    return (void *)(intptr_t)s32Ret;
}

void writeQpValue2Memory(AX_S8 qpDelta, AX_S8* memory, AX_U16 column, AX_U16 row, AX_U16 blockunit,
                            AX_U16 width, AX_U16 ctb_size, AX_U32 ctb_per_row, AX_U32 ctb_per_column,
                            AX_VENC_QPMAP_QP_TYPE_E enQpmapType)
{
  AX_U32 blks_per_ctb = ctb_size / 8;
  AX_U32 blks_per_unit = 1 << (3 - blockunit);
  AX_U32 ctb_row_number    = row * blks_per_unit / blks_per_ctb;
  AX_U32 ctb_column_number = column * blks_per_unit / blks_per_ctb;
  AX_U32 ctb_row_stride = ctb_per_row * blks_per_ctb * blks_per_ctb;
  AX_U32 xoffset = (column * blks_per_unit) % blks_per_ctb;
  AX_U32 yoffset = (row * blks_per_unit) % blks_per_ctb;
  AX_U32 stride = blks_per_ctb;
  AX_U32 columns, rows, r, c;

  rows = columns = blks_per_unit;
  if(blks_per_ctb < blks_per_unit)
  {
    rows = MIN(rows, ctb_per_column * blks_per_ctb - row * blks_per_unit);
    columns = MIN(columns, ctb_per_row * blks_per_ctb - column * blks_per_unit);
    rows /= blks_per_ctb;
    columns *= blks_per_ctb;
    stride = ctb_row_stride;
  }

  // ctb addr --> blk addr
  memory += ctb_row_number * ctb_row_stride + ctb_column_number * (blks_per_ctb * blks_per_ctb);
  memory += yoffset * stride + xoffset;
  for(r = 0; r < rows; r++)
  {
    AX_S8 *dst = memory + r * stride;
    for(c = 0; c < columns; c++)
    {
        if (VENC_QPMAP_ENABLE_DELTAQP == enQpmapType || VENC_QPMAP_ENABLE_ABSQP == enQpmapType)
            *dst++ = qpDelta;
    }
  }
}

static void CopyQPDelta2Memory(int w, int h, int maxCuSize, AX_S8 *QpmapAddr, AX_VENC_QPMAP_QP_TYPE_E enQpmapType)
{
    AX_U32 ctb_per_row = (w + maxCuSize - 1) / maxCuSize;
    AX_U32 ctb_per_column = (h + maxCuSize - 1) / maxCuSize;
    AX_U32 block_unit = 0;
    AX_U32 width, height;
    AX_S32 qpDelta = 0;
    AX_U32 blockunit = 0;

    switch (gQpmapBlockUnit)
    {
        case 0:
            block_unit = 64;
            blockunit = 0;
            break;
        case 1:
            block_unit = 32;
            blockunit = 1;
            break;
        case 2:
            block_unit = 16;
            blockunit = 2;
            break;
        case 3:
            block_unit = 8;
            blockunit = 3;
            break;
        default:
            block_unit = 64;
            blockunit = 0;
            break;
    }

    width = (((w + maxCuSize- 1) & (~(maxCuSize - 1))) + block_unit - 1) / block_unit;
    height = (((h + maxCuSize - 1) & (~(maxCuSize - 1))) + block_unit - 1) / block_unit;

    //blockunit = gQpmapBlockUnit;
    AX_U32 ctb_size = maxCuSize;

    for (int line_idx = 0; line_idx < height; line_idx++)
    {
        // delta qp,rang in [-31, 32]
        if (VENC_QPMAP_ENABLE_DELTAQP == enQpmapType) {
            if (line_idx % 2 == 0)
                qpDelta = 10;
            else
                qpDelta = 0;

            qpDelta = CLIP3(-31, 32, qpDelta);
            qpDelta = -qpDelta;
            qpDelta &= 0x3f;
            qpDelta = (qpDelta << 1) | 0;
        } else if (VENC_QPMAP_ENABLE_ABSQP == enQpmapType) {//absolute qp, rang in [0, 51]
            if (line_idx % 2 == 0)
                qpDelta = 20;
            else
                qpDelta = 35;

            qpDelta = CLIP3(0, 51, qpDelta);
            qpDelta &= 0x3f;
            qpDelta = (qpDelta << 1) | 1;
        }

        for (int i = 0; i < width; i++)
        {
            writeQpValue2Memory((AX_S8)qpDelta, QpmapAddr, i, line_idx, blockunit, width,
                                    ctb_size, ctb_per_row, ctb_per_column, enQpmapType);
        }
    }
}

/* IPCM map setting */
static void WriteFlags2Memory(char flag, AX_S8* memory, AX_U16 column, AX_U16 row,
    AX_U16 blockunit, AX_U16 width, AX_U16 ctb_size, AX_U32 ctb_per_row, AX_U32 ctb_per_column)
{
    AX_U32 blks_per_ctb = ctb_size / 8;
    AX_U32 blks_per_unit = 1 << (3 - blockunit);
    AX_U32 ctb_row_number = row * blks_per_unit / blks_per_ctb;
    AX_U32 ctb_column_number = column * blks_per_unit / blks_per_ctb;
    AX_U32 ctb_row_stride = ctb_per_row * blks_per_ctb * blks_per_ctb;
    AX_U32 xoffset = (column * blks_per_unit) % blks_per_ctb;
    AX_U32 yoffset = (row * blks_per_unit) % blks_per_ctb;
    AX_U32 stride = blks_per_ctb;
    AX_U32 columns, rows, r, c;

    rows = columns = blks_per_unit;
    if (blks_per_ctb < blks_per_unit) {
        rows = MIN(rows, ctb_per_column * blks_per_ctb - row * blks_per_unit);
        columns = MIN(columns, ctb_per_row * blks_per_ctb - column * blks_per_unit);
        rows /= blks_per_ctb;
        columns *= blks_per_ctb;
        stride = ctb_row_stride;
    }

    // ctb addr --> blk addr
    memory += ctb_row_number * ctb_row_stride + ctb_column_number * (blks_per_ctb * blks_per_ctb);
    memory += yoffset * stride + xoffset;
    for (r = 0; r < rows; r++) {
        AX_S8* dst = memory + r * stride;
        AX_U8 val;
        for (c = 0; c < columns; c++) {
            val = *dst;
            *dst++ = (val & 0x7f) | (flag << 7);
        }
    }
}

static int CopyFlagsMap2Memory(int w, int h, int maxCuSize, AX_S8* QpmapAddr, AX_VENC_QPMAP_QP_TYPE_E enQpmapType)
{
    AX_U32 ctb_per_row = ((w + maxCuSize - 1) / (maxCuSize));
    AX_U32 ctb_per_column = ((h + maxCuSize - 1) / (maxCuSize));
    AX_U16 ctb_size = maxCuSize;
    AX_U16 blockunit;
    AX_U32 i;
    AX_U32 flag = 0;

    AX_U16 width, height, block_unit_size;
    blockunit = (ctb_size == 64 ? 0 : 2);

    block_unit_size = 8 << (3 - blockunit);
    width = (((w + maxCuSize - 1) & (~(maxCuSize - 1))) + block_unit_size - 1) / block_unit_size;
    height = (((h + maxCuSize - 1) & (~(maxCuSize - 1))) + block_unit_size - 1) / block_unit_size;

    /* qpDelta map is not setting */
    if (VENC_QPMAP_ENABLE_IPCM == enQpmapType) {
        AX_S32 block_size = ((w + maxCuSize - 1) & (~(maxCuSize - 1))) * ((h + maxCuSize - 1) & (~(maxCuSize - 1))) / (8 * 8);
        memset(QpmapAddr, 0, block_size);
    }

    for (int line_idx = 0; line_idx < height; line_idx++) {
        /* ipcm map setting, could change by user */
        if (line_idx % 8 == 0)
            flag = 1; //enable ipcm map
        else
            flag = 0; //disable ipcm map

        for (i = 0; i < width; i++)
            WriteFlags2Memory((char)flag, QpmapAddr, i, line_idx, blockunit, width, ctb_size, ctb_per_row, ctb_per_column);
    }

    return 0;
}

/* qpmap task */
static void *VencEncodeQpmapProc(void *arg)
{
    AX_S32 s32Ret = -1;
    AX_S32 readSize;
    AX_BOOL bLoopEncode = AX_FALSE;
    AX_S32 encodeFrmNum = -1;
    AX_U64 totalInputFrames = 0;
    struct timeval timeFrameStart;
    struct timeval timeFrameEnd;
    AX_U64 totalEncodeTime = 0;
    AX_U16 frameFormat = 0;
    AX_BOOL enablePerf = AX_FALSE;
    AX_S8 * fileInput;
    AX_S32 syncType = -1;
    AX_U32 frameSize;
    AX_POOL s32UserPoolId;
    AX_BLK BlkId = AX_INVALID_BLOCKID;
    AX_VENC_QPMAP_QP_TYPE_E enQpmapType = VENC_QPMAP_DISABLE;

    SetMaskSIGALRM();

    SAMPLE_VENC_ENCODE_PARA_T *pEncodePara = NULL;
    pEncodePara = (SAMPLE_VENC_ENCODE_PARA_T *)arg;

    SAMPLE_VENC_CMD_PARA_T *pCmdl = pEncodePara->ChnPara.pCmdl;

    bLoopEncode = pCmdl->loopEncode;
    encodeFrmNum = pCmdl->frameNum;
    frameFormat = pCmdl->inputFormat;
    fileInput = pCmdl->input;
    syncType = pCmdl->syncType;
    frameSize = pEncodePara->frameSize;
    s32UserPoolId = pEncodePara->userPoolId;

    AX_VENC_RECV_PIC_PARAM_S stRecvParam;
    AX_USER_FRAME_INFO_S stFrame;

    /* Rate Control */
    AX_VENC_RC_PARAM_S stRcParam;

    /* use pool to alloc qpmap buffer */
    AX_U32 u32QpMapSize;
    AX_U64 u64QpMapPhyAddr;
    AX_U64 u64QpMapVirAddr;
    AX_S8 *pVirAddrTemp;
    AX_POOL_CONFIG_T stQpmapPoolConfig;
	AX_POOL s32QpmapPoolId;
    AX_BLK QpmapBlkId;

    memset(&stQpmapPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    u32QpMapSize = ((pEncodePara->width + MAX_CU_SIZE -1)&(~(MAX_CU_SIZE -1))) *
                        ((pEncodePara->height + MAX_CU_SIZE -1)&(~(MAX_CU_SIZE -1))) / (8 * 8);
    u32QpMapSize = (u32QpMapSize + MAX_CU_SIZE -1) & (~(MAX_CU_SIZE-1));
    stQpmapPoolConfig.MetaSize = 512;
    stQpmapPoolConfig.BlkCnt = 10;
    stQpmapPoolConfig.BlkSize = u32QpMapSize;
    stQpmapPoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(stQpmapPoolConfig.PartitionName, 0, sizeof(stQpmapPoolConfig.PartitionName));
    strcpy(stQpmapPoolConfig.PartitionName, "anonymous");

    s32QpmapPoolId = AX_POOL_CreatePool(&stQpmapPoolConfig);
    if (AX_INVALID_POOLID == s32QpmapPoolId) {
        SAMPLE_ERR_LOG("Create pool err.\n");
        goto EXIT;
    }

    /* input yuv file */
    FILE *fFileIn = NULL;

    fFileIn = fopen(fileInput, "rb");
    if (fFileIn == NULL) {
        SAMPLE_ERR_LOG("Open input file error!\n");
        return NULL;
    }

    s32Ret = AX_VENC_StartRecvFrame(pEncodePara->VeChn, &stRecvParam);
    if (AX_SUCCESS != s32Ret) {
        SAMPLE_ERR_LOG("Encoder thread exit, ret=%x\n", s32Ret);
        goto EXIT;
    }

    memset(&stFrame, 0, sizeof(AX_USER_FRAME_INFO_S));

    while (pEncodePara->bThreadStart)
    {
        BlkId = AX_POOL_GetBlock(s32UserPoolId, frameSize, NULL);
        if (AX_INVALID_BLOCKID == BlkId) {
            //VENC_LOG("Get block fail.\n");
            usleep(5*1000);
            continue;
        }

        while(AX_INVALID_BLOCKID == (QpmapBlkId = AX_POOL_GetBlock(s32QpmapPoolId, u32QpMapSize, NULL)))
        {
            usleep(5 * 1000);
            continue;
        }

        stFrame.stUserFrame.stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(BlkId);
        stFrame.stUserFrame.stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(BlkId);
        stFrame.stUserFrame.stVFrame.u32BlkId[0] = BlkId;
        stFrame.stUserFrame.stVFrame.u32BlkId[1] = 0;//must set 0 if not used
        stFrame.stUserFrame.stVFrame.u32BlkId[2] = 0;//must set 0 if not used

        u64QpMapPhyAddr = AX_POOL_Handle2PhysAddr(QpmapBlkId);
        u64QpMapVirAddr = AX_POOL_GetBlockVirAddr(QpmapBlkId);
        stFrame.stUserRcInfo.u64QpMapPhyAddr = u64QpMapPhyAddr;
        stFrame.stUserRcInfo.pQpMapVirAddr = (AX_S8 *)u64QpMapVirAddr;
        stFrame.stUserRcInfo.u32RoiMapDeltaSize = u32QpMapSize;

        /* read frame data from yuv file */
        readSize = LoadFrameFromFile(fFileIn,
                                    pEncodePara->width,
                                    pEncodePara->stride,
                                    pEncodePara->height,
                                    frameFormat,
                                    (void *)stFrame.stUserFrame.stVFrame.u64VirAddr[0]);
        if (!bLoopEncode && (readSize <= 0)) {
            VENC_LOG("Warning: read frame size : %d less than %d\n", readSize, frameSize);
        }

        if (feof(fFileIn)) {
            if (bLoopEncode) {
                fseek(fFileIn, 0, SEEK_SET);
                LoadFrameFromFile(fFileIn, pEncodePara->width, pEncodePara->stride,
                                    pEncodePara->height, frameFormat, (void *)stFrame.stUserFrame.stVFrame.u64VirAddr[0]);
            } else {
                SAMPLE_ERR_LOG("End of input file!\n");
                /* no more frames, stop encoder */
                goto EXIT;
            }
        }

        /* qpDelta map setting */
        pVirAddrTemp = (AX_S8 *)u64QpMapVirAddr;
        memset(pVirAddrTemp, 0x0, u32QpMapSize);

        if (VENC_QPMAP_ENABLE_DELTAQP == enQpmapType || VENC_QPMAP_ENABLE_ABSQP == enQpmapType)
            CopyQPDelta2Memory(pEncodePara->width, pEncodePara->height,
                pEncodePara->maxCuSize, pVirAddrTemp, enQpmapType);

        /* IPCM Map setting */
        pVirAddrTemp = (AX_S8 *)u64QpMapVirAddr;
        if (VENC_QPMAP_ENABLE_IPCM == enQpmapType)
            CopyFlagsMap2Memory(pEncodePara->width, pEncodePara->height,
                pEncodePara->maxCuSize, (AX_S8 *)pVirAddrTemp, enQpmapType);

        /* the input frame sequence number */
        stFrame.stUserFrame.stVFrame.u64SeqNum = totalInputFrames + 1;

        stFrame.stUserFrame.stVFrame.u32Width = pEncodePara->width;
        stFrame.stUserFrame.stVFrame.u32Height = pEncodePara->height;
        stFrame.stUserFrame.stVFrame.u64UserData = QpmapBlkId;  // transform qpmapBlkId by u64UserData

        stFrame.stUserFrame.stVFrame.enImgFormat = frameFormat;
        stFrame.stUserFrame.stVFrame.u32PicStride[0] = pEncodePara->stride;

        if (frameFormat == AX_YUV420_PLANAR) {
            stFrame.stUserFrame.stVFrame.u32PicStride[1] = pEncodePara->stride / 2;
            stFrame.stUserFrame.stVFrame.u32PicStride[2] = pEncodePara->stride / 2;
        } else if (frameFormat == AX_YUV420_SEMIPLANAR || frameFormat == AX_YUV420_SEMIPLANAR_VU) {
            stFrame.stUserFrame.stVFrame.u32PicStride[1] = pEncodePara->stride;
            stFrame.stUserFrame.stVFrame.u32PicStride[2] = 0;
        } else if (frameFormat == AX_YUV422_INTERLEAVED_YUYV || frameFormat == AX_YUV422_INTERLEAVED_YUYV) {
            stFrame.stUserFrame.stVFrame.u32PicStride[1] = 0;
            stFrame.stUserFrame.stVFrame.u32PicStride[2] = 0;
        }

        stFrame.stUserFrame.stVFrame.u64VirAddr[0] = 0;
        stFrame.stUserFrame.stVFrame.u64VirAddr[1] = 0;
        stFrame.stUserFrame.stVFrame.u64VirAddr[2] = 0;

        s32Ret = AX_VENC_SendFrameEx(pEncodePara->VeChn, &stFrame, 0);
        if (AX_SUCCESS != s32Ret)
        {
            SAMPLE_ERR_LOG("AX_VENC_SendFrameEx failed, ret=%x\n", s32Ret);
            s32Ret = AX_POOL_ReleaseBlock(stFrame.stUserFrame.stVFrame.u32BlkId[0]);
            if (s32Ret)
                SAMPLE_ERR_LOG("release block err.\n");
            continue;
        }

#ifdef ENABLE_REQUEST_IDR
        if (totalInputFrames % REQUEST_IDR_INTERVAL == 0)
        {
            AX_VENC_RequestIDR(pEncodePara->VeChn, true);
        }
#endif
        totalInputFrames++;

        if (totalInputFrames == encodeFrmNum) {
            VENC_LOG("Want to encode %llu frames, exit!\n", totalInputFrames);
            goto EXIT;
        }
    }

EXIT:

    if(AX_INVALID_BLOCKID != BlkId){
        AX_POOL_ReleaseBlock(BlkId);
    }

    if (NULL != fFileIn) {
        fclose(fFileIn);
        fFileIn = NULL;
    }

    VENC_LOG("venc %d - Total input %llu frames, Encoder exit!\n", pEncodePara->VeChn, totalInputFrames);

    return (void *)(intptr_t)s32Ret;
}

static int __AX_VENC_GetStreamBufInfo_Test(VENC_CHN VeChn)
{
    return 0;
    int ret;
    AX_VENC_STREAM_BUF_INFO_T StreamBufInfo;
    ret = AX_VENC_GetStreamBufInfo(VeChn, &StreamBufInfo);
    if (ret) {
        SAMPLE_ERR_LOG("AX_VENC_GetStreamBufInfo failed, VeChn:%d", VeChn);
        return -1;
    }

    SAMPLE_LOG(".u64PhyAddr:0x%llx, .pUserAddr:%p, .u32BufSize:0x%x",
        StreamBufInfo.u64PhyAddr, StreamBufInfo.pUserAddr, StreamBufInfo.u32BufSize);

    return 0;
}

/* get stream task */
static void *VencGetStreamProc(void *arg)
{
    AX_S32 s32Ret = -1;
    SAMPLE_VENC_GETSTREAM_PARA_T *pstPara;
    pstPara = (SAMPLE_VENC_GETSTREAM_PARA_T *)arg;
    AX_VENC_STREAM_S stStream;
    AX_S16 syncType = -1;
    FILE *pStrm = NULL;
    AX_S8 esName[150];
    AX_U32 totalGetStream = 0;
    AX_VENC_QPMAP_QP_TYPE_E enQpmapType = VENC_QPMAP_DISABLE;
    time_t ti_s = time(NULL);

    SAMPLE_VENC_CMD_PARA_T *pCmdl = pstPara->ChnPara.pCmdl;
    syncType = pCmdl->syncType;
    memset(esName, 0, 150);
    
    SetMaskSIGALRM();

    SAMPLE_LOG("VENC %d pCmdl->codecFormat:%d \n", pstPara->VeChn, pCmdl->codecFormat);

    if (pCmdl->codecFormat) {
        if (NULL != pCmdl->output) {
            if (pCmdl->codecFormat == VENC_CODEC_H264) {
                sprintf(esName, "%s_venc_%d_%ld.264", pCmdl->output, pstPara->VeChn, ti_s);
            } else if (pCmdl->codecFormat == VENC_CODEC_HEVC) {
                sprintf(esName, "%s_venc_%d_%ld.265", pCmdl->output, pstPara->VeChn, ti_s);
            }
        } else {
            if (pCmdl->codecFormat == VENC_CODEC_H264) {
                sprintf(esName, "venc_%d_%ld.264", pstPara->VeChn, ti_s);
            } else if (pCmdl->codecFormat == VENC_CODEC_HEVC) {
                sprintf(esName, "venc_%d_%ld.265", pstPara->VeChn, ti_s);
            }
        }

        SAMPLE_LOG("VENC %d Open output file name:%s pCmdl->codecFormat:%d\n",
            pstPara->VeChn, esName, pCmdl->codecFormat);
        pStrm = fopen(esName, "wb");
    } else {
        if (pstPara->VeChn % 2 == HEVC_STREAM)
            sprintf(esName, "enc_%d.265", pstPara->VeChn);
        else
            sprintf(esName, "enc_%d.264", pstPara->VeChn);

        pStrm = fopen(esName, "wb");
    }

    if (pStrm == NULL) {
        SAMPLE_ERR_LOG("Open output file error!\n");
        return NULL;
    }


#ifdef DYNAMIC_FRAMERATE
    FILE *pStrmex = NULL;
    if (pstPara->VeChn % 2 == HEVC_STREAM)
        sprintf(esName, "enc_%d_ex.265", pstPara->VeChn);
    else
        sprintf(esName, "enc_%d_ex.264", pstPara->VeChn);
    pStrmex = fopen(esName, "wb");
    if (pStrmex == NULL) {
        SAMPLE_ERR_LOG("Open output file error!\n");
        return NULL;
    }
#endif

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

        s32Ret = AX_VENC_GetStream(pstPara->VeChn, &stStream, syncType);
        if (AX_SUCCESS == s32Ret) {
            __AX_VENC_GetStreamBufInfo_Test(pstPara->VeChn);

            totalGetStream++;
#ifdef DYNAMIC_FRAMERATE
        if (totalGetStream > 60) {
            fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrmex);
            fflush(pStrm);
        } else {
            fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
            fflush(pStrm);
        }
#else
        if (s32Ret)
            SAMPLE_ERR_LOG("release block err.\n");
        fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
        fflush(pStrm);
#endif
        if (enQpmapType != VENC_QPMAP_DISABLE)
            s32Ret = AX_POOL_ReleaseBlock(stStream.stPack.u64UserData);

#ifdef ROI_ENABLE
            if (totalGetStream == 10) {
                s32Ret = SampleSetRoiAttr(0, pstPara->ChnPara.pCmdl);
                if (0 != s32Ret) {
                    SAMPLE_ERR_LOG("SampleSetRoiAttr err.\n");
                    return -1;
                }

            }
#endif

#ifdef DYNAMIC_RC
            AX_VENC_RC_PARAM_S stRcParam;
            if (totalGetStream == 60) {
                AX_S32 s32Ret = -1;
                s32Ret = AX_VENC_GetRcParam(pstPara->VeChn, &stRcParam);
                if (AX_SUCCESS != s32Ret)
                    SAMPLE_ERR_LOG("AX_VENC_GetRcParam error.\n");

                if (pstPara->VeChn % 2 == HEVC_STREAM)
                {
                    stRcParam.stH265Cbr.u32Gop = 60;
                    stRcParam.stH265Cbr.u32BitRate = 4000; //kbps
                    stRcParam.stH265Cbr.u32MinQp = 20;
                    stRcParam.stH265Cbr.u32MaxQp = 30;
                    stRcParam.stH265Cbr.u32MinIQp = 30;
                    stRcParam.stH265Cbr.u32MaxIQp = 40;
                }
                else
                {
                    stRcParam.stH264Cbr.u32Gop = 60;
                    stRcParam.stH264Cbr.u32BitRate = 3000; //kbps
                    stRcParam.stH264Cbr.u32MinQp = 20;
                    stRcParam.stH264Cbr.u32MaxQp = 30;
                    stRcParam.stH264Cbr.u32MinIQp = 30;
                    stRcParam.stH264Cbr.u32MaxIQp = 40;
                }

                s32Ret = AX_VENC_SetRcParam(pstPara->VeChn, &stRcParam);
                if (AX_SUCCESS != s32Ret)
                    SAMPLE_ERR_LOG("AX_VENC_SetRcParam error.\n");
            }
#endif

            s32Ret = AX_VENC_ReleaseStream(pstPara->VeChn, &stStream);
            if (AX_SUCCESS != s32Ret) {
                SAMPLE_ERR_LOG("AX_VENC_ReleaseStream failed!\n");
                goto EXIT;
            }
        }
    }

EXIT:

    if (pStrm != NULL) {
        fclose(pStrm);
        pStrm = NULL;
    }

    VENC_LOG("venc %d: Total get %u encoded frames. getStream Exit!\n", pstPara->VeChn, totalGetStream);
    return (void *)(intptr_t)s32Ret;
}


static AX_POOL VencPoolInit(AX_U32 frameSize, VENC_CHN VeChn)
{
    /* use pool to alloc buffer */
    AX_POOL_CONFIG_T stPoolConfig;
    AX_POOL s32UserPoolId;

    if (0 == frameSize) {
        SAMPLE_ERR_LOG("VeChn:%d Forbid 0 == frameSize.\n", VeChn);
        return AX_INVALID_POOLID;
    }

    memset(&stPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    stPoolConfig.MetaSize = 512;
    stPoolConfig.BlkCnt = 5;
    stPoolConfig.BlkSize = frameSize;
    stPoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(stPoolConfig.PartitionName, 0, sizeof(stPoolConfig.PartitionName));
    strcpy(stPoolConfig.PartitionName, "anonymous");

    s32UserPoolId = AX_POOL_CreatePool(&stPoolConfig);
    if (AX_INVALID_POOLID == s32UserPoolId) {
        SAMPLE_ERR_LOG("VeChn:%d Create pool err! frameSize:0x%x .BlkSize:%lld .BlkCnt:%d.\n",
            VeChn, frameSize, stPoolConfig.BlkSize, stPoolConfig.BlkCnt);
    }

    SAMPLE_LOG("VeChn:%d frameSize:0x%x .BlkSize:%lld .BlkCnt:%d s32UserPoolId:%d.\n",
        VeChn, frameSize, stPoolConfig.BlkSize, stPoolConfig.BlkCnt, s32UserPoolId);
    return s32UserPoolId;
}

static AX_S32 CalculateFrameSize(SAMPLE_VENC_CMD_PARA_T * pstArg)
{
    AX_S32 widthSrc;
    AX_S32 strideSrc;
    AX_S32 heightSrc;
    AX_S32 frameFormat = 0;
    AX_U32 frameSize;

    widthSrc = pstArg->nSrcWidth;
    heightSrc = pstArg->nSrcHeight;
    strideSrc = pstArg->picStride[0];
    frameFormat = pstArg->inputFormat;

    if (!strideSrc) {
        strideSrc = widthSrc;
        if (frameFormat > AX_YUV420_SEMIPLANAR_VU)
            strideSrc *= 2;
    }

    switch (frameFormat)
    {
    case 0:
        frameSize = strideSrc * heightSrc * 3 / 2;
        break;
    case 1:
        frameSize = strideSrc * heightSrc * 3 / 2;
        break;
    case 2:
        frameSize = strideSrc * heightSrc * 3 / 2;
        break;
    case 3:
        frameSize = strideSrc * heightSrc * 2;
        break;
    case 4:
        frameSize = strideSrc * heightSrc * 2;
        break;
    default:
        SAMPLE_ERR_LOG("Invalid frame format!\n");
        return 0;
    }

    if (pstArg->enCompressMode) {
        AX_U32 Ytotal_size = pstArg->YheaderSize + pstArg->YpayloadSize;
        AX_U32 UVtotal_size = pstArg->UVheaderSize + pstArg->UVpayloadSize;
        frameSize = Ytotal_size + UVtotal_size;
        SAMPLE_LOG("frameSize:%d, ->YheaderSize:%d, ->YpayloadSize:%d, ->UVheaderSize:%d, ->UVpayloadSize:%d\n",
            frameSize, pstArg->YheaderSize, pstArg->YpayloadSize, pstArg->UVheaderSize, pstArg->UVpayloadSize);
    }

    return frameSize;
}

static AX_S32 VencCommPoolInit(AX_U32 frameSize)
{
    AX_POOL_FLOORPLAN_T PoolFloorPlan;
    AX_S32 ret;

    if (0 == frameSize) {
        SAMPLE_ERR_LOG("Invalid frameSize=0.\n");
        return -1;
    }

    memset(&PoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));
    PoolFloorPlan.CommPool[0].MetaSize   = 0x1000;
    PoolFloorPlan.CommPool[0].BlkSize   = frameSize;
    PoolFloorPlan.CommPool[0].BlkCnt    = 2;
    PoolFloorPlan.CommPool[0].CacheMode = POOL_CACHE_MODE_NONCACHE;

    memset(PoolFloorPlan.CommPool[0].PartitionName,0,sizeof(PoolFloorPlan.CommPool[0].PartitionName));
    strcpy((char *)PoolFloorPlan.CommPool[0].PartitionName,"anonymous");

    ret = AX_POOL_SetConfig(&PoolFloorPlan);
    if (ret) {
        SAMPLE_ERR_LOG("Create common pool err! BlkSize:%lld,BlkCnt:%d.\n",
            PoolFloorPlan.CommPool[0].BlkSize,PoolFloorPlan.CommPool[0].BlkCnt);
        return -1;
    }

    ret = AX_POOL_Init();
    if(ret){
        SAMPLE_ERR_LOG("AX_POOL_Init fail!!ret:0x%X\n",ret);
        return -1;
    }

    SAMPLE_LOG("Create common pool success! BlkSize:%lld,BlkCnt:%d.\n",
        PoolFloorPlan.CommPool[0].BlkSize,PoolFloorPlan.CommPool[0].BlkCnt);
    return 0;
}

AX_S32 SampleVencStart(SAMPLE_VENC_ATTR_S *pChnAttr)
{
    AX_S32 s32Ret = -1;
    AX_U32 u32StartTime = 1;
    AX_S32 widthSrc;
    AX_S32 strideSrc;
    AX_S32 heightSrc;
    AX_S32 frameFormat = 0;
    SAMPLE_VENC_RC_E rcMode = VENC_RC_CBR;
    AX_U16 chnNum = 2;
    AX_U16 gdrDuration = 0;
    AX_U32 gopLen = 30;
    AX_U16 inputFrameRate = 30;
    AX_U16 outputFrameRate = 30;

    AX_BOOL enableCrop = AX_FALSE;
    AX_U32 bitRate = 2000;//kbps
    AX_U16 qpMin = 22;
    AX_U16 qpMax = 51;
    AX_U16 qpMinI= 22;
    AX_U16 qpMaxI = 51;
    AX_S16 firstFrameQp = -1;
    AX_VENC_GOP_MODE_E gopType = VENC_GOPMODE_NORMALP;
    AX_S32 intraQpDelta = -2;
    AX_U32 frameSize;
    AX_POOL userPoolId;
    AX_VENC_QPMAP_QP_TYPE_E enQpmapType = VENC_QPMAP_DISABLE;
    AX_U32 nQpmapBlockUnit = 0;
    AX_VENC_RC_CTBRC_MODE_E enCtbRcType = VENC_RC_CTBRC_QUALITY;
    AX_S32 i;
    AX_VENC_RECV_PIC_PARAM_S stVencRecvParam;

    /* SVC-T Configure */
    AX_U32 sSvcTGopSize = 4;
    /*SVC-T GOP4*/
    char *stSvcTCfg[] = {
        "Frame1:  P      1      0       0.4624        2        1           -1          1",
        "Frame2:  P      2      0       0.4624        1        1           -2          1",
        "Frame3:  P      3      0       0.4624        2        2           -1 -3       1 0",
        "Frame4:  P      4      0       0.4624        0        1           -4          1",
        NULL,
    };

    chnNum = pChnAttr->nChnNum;
    stVencRecvParam.s32RecvPicNum = -1;

    //init common pool
    frameSize = CalculateFrameSize(pChnAttr->tDevAttr[0].pCmdl);

    s32Ret = VencCommPoolInit(frameSize);
    if(s32Ret){
       SAMPLE_ERR_LOG("VencCommPoolInit failed\n");
       return -1;
    }

    for (i = 0; i < chnNum; i++)
    {
        SAMPLE_LOG(" i:%d, chnNum:%d\n", i, chnNum);

        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));

        SAMPLE_VENC_CMD_PARA_T * pstArg = pChnAttr->tDevAttr[i].pCmdl;

        widthSrc = pstArg->nSrcWidth;
        heightSrc = pstArg->nSrcHeight;
        strideSrc = pstArg->picStride[0];
        SAMPLE_LOG("VENC %d widthSrc:%d, heightSrc:%d, strideSrc:%d", i, widthSrc, heightSrc, strideSrc);

        frameFormat = pstArg->inputFormat;
        rcMode = pstArg->rcMode;

        gdrDuration = pstArg->gdrDuration;
        gopLen = pstArg->gopLength;
        inputFrameRate = pstArg->inputFrameRate;
        outputFrameRate = pstArg->outputFrameRate;
        bitRate = pstArg->bitRate;
        qpMin = pstArg->qpMin;
        qpMax = pstArg->qpMax;
        qpMinI = pstArg->qpMinI;
        qpMaxI = pstArg->qpMaxI;
        firstFrameQp = pstArg->qpHdr;
        gopType = pstArg->gopType;
        intraQpDelta = pstArg->intraQpDelta;

        /* set codec format */
        if (pstArg->codecFormat) {
            if (VENC_CODEC_HEVC == pstArg->codecFormat) {
                stVencChnAttr.stVencAttr.enType = PT_H265;
                gEncodePara[i].maxCuSize = 64;
            } else if (VENC_CODEC_H264 == pstArg->codecFormat) {
                stVencChnAttr.stVencAttr.enType = PT_H264;
                gEncodePara[i].maxCuSize = 16;
            }
        } else {
            if (HEVC_STREAM == (i % 2)) {
                stVencChnAttr.stVencAttr.enType = PT_H265;
                gEncodePara[i].maxCuSize = 64;
            }
            else {
                stVencChnAttr.stVencAttr.enType = PT_H264;
                gEncodePara[i].maxCuSize = 16;
            }
        }

        stVencChnAttr.stVencAttr.u32PicWidthSrc = widthSrc;   /*the input picture width*/
        stVencChnAttr.stVencAttr.u32PicHeightSrc = heightSrc; /*the input picture height*/
        stVencChnAttr.stVencAttr.u32MbLinesPerSlice = pstArg->sliceSize;

	    //crop setting
        if (pstArg->enableCrop) {
            stVencChnAttr.stVencAttr.u32CropOffsetX = pstArg->horOffsetSrc;
            stVencChnAttr.stVencAttr.u32CropOffsetY = pstArg->verOffsetSrc;
            stVencChnAttr.stVencAttr.u32CropWidth = pstArg->cropWidth;
            stVencChnAttr.stVencAttr.u32CropHeight = pstArg->cropHeight;
        }

        if (!strideSrc) {
            strideSrc = widthSrc;
            if (frameFormat > AX_YUV420_SEMIPLANAR_VU)
                strideSrc *= 2;
        }

        if (stVencChnAttr.stVencAttr.enType == PT_H264) {
            if (INVALID_DEFAULT != pstArg->profile)
                stVencChnAttr.stVencAttr.enProfile = pstArg->profile;
            else
                stVencChnAttr.stVencAttr.enProfile = VENC_H264_MAIN_PROFILE;                  //main profile

            if (INVALID_DEFAULT != pstArg->level)
                stVencChnAttr.stVencAttr.enLevel = pstArg->level;
            else
                stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
        }
        else if (stVencChnAttr.stVencAttr.enType == PT_H265) {
            if (INVALID_DEFAULT != pstArg->profile)
                stVencChnAttr.stVencAttr.enProfile = pstArg->profile;
            else
                stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_PROFILE;                  //main profile

            if (INVALID_DEFAULT != pstArg->level)
                stVencChnAttr.stVencAttr.enLevel = pstArg->level;
            else
                stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;

            if (INVALID_DEFAULT != pstArg->tier)
                stVencChnAttr.stVencAttr.enTier = pstArg->tier;
            else
                stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;
        } else {
            SAMPLE_ERR_LOG("Invalid codec format!\n");
            s32Ret = -1;
            goto END;
        }

        stVencChnAttr.stVencAttr.enLinkMode = AX_NONLINK_MODE;
        /* how many frames take to do GDR */
        stVencChnAttr.stVencAttr.u32GdrDuration = gdrDuration;
        /* stream buffer configure */
        stVencChnAttr.stVencAttr.u32BufSize = 2 * strideSrc * heightSrc;

        //stVencChnAttr.stVencAttr.enMemSource = AX_MEMORY_SOURCE_POOL;

        switch (frameFormat)
        {
        case 0:
            frameSize = strideSrc * heightSrc * 3 / 2;
            break;
        case 1:
            frameSize = strideSrc * heightSrc * 3 / 2;
            break;
        case 2:
            frameSize = strideSrc * heightSrc * 3 / 2;
            break;
        case 3:
            frameSize = strideSrc * heightSrc * 2;
            break;
        case 4:
            frameSize = strideSrc * heightSrc * 2;
            break;
        default:
            VENC_LOG("Invalid frame format!\n");
            goto END;
        }

        if (pstArg->enCompressMode) {
            AX_U32 Ytotal_size = pstArg->YheaderSize + pstArg->YpayloadSize;
            AX_U32 UVtotal_size = pstArg->UVheaderSize + pstArg->UVpayloadSize;
            frameSize = Ytotal_size + UVtotal_size;
            SAMPLE_LOG("frameSize:%d, ->YheaderSize:%d, ->YpayloadSize:%d, ->UVheaderSize:%d, ->UVpayloadSize:%d\n",
                frameSize, pstArg->YheaderSize, pstArg->YpayloadSize, pstArg->UVheaderSize, pstArg->UVpayloadSize);
        }

        if (rcMode <= VENC_RC_NONE || rcMode >= VENC_RC_BUTT) {
            SAMPLE_ERR_LOG("Invalid RC Mode!\n");
            s32Ret = -1;
            goto END;
        }
        /* RC setting */
        switch (stVencChnAttr.stVencAttr.enType)
        {
        case PT_H265:
        {
            if (rcMode == VENC_RC_CBR) {
                AX_VENC_H265_CBR_S stH265Cbr;
                memset(&stH265Cbr, 0, sizeof(AX_VENC_H265_CBR_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH265Cbr.u32Gop = gopLen;
                stH265Cbr.u32SrcFrameRate = inputFrameRate;  /* input frame rate */
                stH265Cbr.fr32DstFrameRate = outputFrameRate; /* target frame rate */
                stH265Cbr.u32BitRate = bitRate;
                stH265Cbr.u32MinQp = qpMin;
                stH265Cbr.u32MaxQp = qpMax;
                stH265Cbr.u32MinIQp = qpMinI;
                stH265Cbr.u32MaxIQp = qpMaxI;
                stH265Cbr.s32IntraQpDelta = intraQpDelta;

                stH265Cbr.stQpmapInfo.enQpmapQpType = enQpmapType;
                stH265Cbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH265Cbr.stQpmapInfo.enCtbRcMode = enCtbRcType;
                memcpy(&stVencChnAttr.stRcAttr.stH265Cbr, &stH265Cbr, sizeof(AX_VENC_H265_CBR_S));
            }
            else if (rcMode == VENC_RC_VBR) {
                AX_VENC_H265_VBR_S stH265Vbr;
                memset(&stH265Vbr, 0, sizeof(AX_VENC_H265_VBR_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH265Vbr.u32Gop = gopLen;
                stH265Vbr.u32SrcFrameRate = inputFrameRate;
                stH265Vbr.fr32DstFrameRate = outputFrameRate;
                stH265Vbr.u32MaxBitRate = bitRate;
                stH265Vbr.u32MinQp = qpMin;
                stH265Vbr.u32MaxQp = qpMax;
                stH265Vbr.u32MinIQp = qpMinI;
                stH265Vbr.u32MaxIQp = qpMaxI;
                stH265Vbr.s32IntraQpDelta = intraQpDelta;

                stH265Vbr.stQpmapInfo.enQpmapQpType = enQpmapType;
                stH265Vbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH265Vbr.stQpmapInfo.enCtbRcMode = enCtbRcType;
                memcpy(&stVencChnAttr.stRcAttr.stH265Vbr, &stH265Vbr, sizeof(AX_VENC_H265_VBR_S));
            }
            else if (rcMode == VENC_RC_AVBR) {
                AX_VENC_H265_VBR_S stH265AVbr;
                memset(&stH265AVbr, 0, sizeof(AX_VENC_H265_AVBR_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265AVBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH265AVbr.u32Gop = gopLen;
                stH265AVbr.u32SrcFrameRate = inputFrameRate;
                stH265AVbr.fr32DstFrameRate = outputFrameRate;
                stH265AVbr.u32MaxBitRate = bitRate;
                stH265AVbr.u32MinQp = qpMin;
                stH265AVbr.u32MaxQp = qpMax;
                stH265AVbr.u32MinIQp = qpMinI;
                stH265AVbr.u32MaxIQp = qpMaxI;
                stH265AVbr.s32IntraQpDelta = intraQpDelta;

                stH265AVbr.stQpmapInfo.enQpmapQpType = enQpmapType;
                stH265AVbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH265AVbr.stQpmapInfo.enCtbRcMode = enCtbRcType;
                memcpy(&stVencChnAttr.stRcAttr.stH264AVbr, &stH265AVbr, sizeof(AX_VENC_H265_AVBR_S));
            }
            else if (rcMode == VENC_RC_FIXQP) {
                AX_VENC_H265_FIXQP_S stH265FixQp;
                memset(&stH265FixQp, 0, sizeof(AX_VENC_H265_FIXQP_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                stH265FixQp.u32Gop = gopLen;
                stH265FixQp.u32SrcFrameRate = inputFrameRate;
                stH265FixQp.fr32DstFrameRate = outputFrameRate;
                stH265FixQp.u32IQp = 25;
                stH265FixQp.u32PQp = 30;
                stH265FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH265FixQp, &stH265FixQp, sizeof(AX_VENC_H265_FIXQP_S));
            }
            else if (rcMode == VENC_RC_QPMAP) {
                AX_VENC_H265_QPMAP_S stH265QpMap;
                memset(&stH265QpMap, 0, sizeof(AX_VENC_H265_QPMAP_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265QPMAP;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH265QpMap.enQpmapQpType = enQpmapType;
                stH265QpMap.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH265QpMap.enCtbRcMode = enCtbRcType;

                stH265QpMap.u32Gop = gopLen;
                stH265QpMap.u32SrcFrameRate = inputFrameRate;
                stH265QpMap.fr32DstFrameRate = outputFrameRate;
                stH265QpMap.u32TargetBitRate = bitRate;
                memcpy(&stVencChnAttr.stRcAttr.stH265QpMap, &stH265QpMap, sizeof(AX_VENC_H265_QPMAP_S));
            }
        }
        break;

        case PT_H264:
        {
            if (rcMode == VENC_RC_CBR) {
                AX_VENC_H264_CBR_S stH264Cbr;
                memset(&stH264Cbr, 0, sizeof(AX_VENC_H264_CBR_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH264Cbr.u32Gop = gopLen;
                stH264Cbr.u32SrcFrameRate = inputFrameRate;  /* input frame rate */
                stH264Cbr.fr32DstFrameRate = outputFrameRate; /* target frame rate */
                stH264Cbr.u32BitRate = bitRate;
                stH264Cbr.u32MinQp = qpMin;
                stH264Cbr.u32MaxQp = qpMax;
                stH264Cbr.u32MinIQp = qpMinI;
                stH264Cbr.u32MaxIQp = qpMaxI;
                stH264Cbr.s32IntraQpDelta = intraQpDelta;

                stH264Cbr.stQpmapInfo.enQpmapQpType = enQpmapType;
                stH264Cbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH264Cbr.stQpmapInfo.enCtbRcMode = enCtbRcType;
                memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_S));
            }
            else if (rcMode == VENC_RC_VBR) {
                AX_VENC_H264_VBR_S stH264Vbr;
                memset(&stH264Vbr, 0, sizeof(AX_VENC_H264_VBR_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH264Vbr.u32Gop = gopLen;
                stH264Vbr.u32SrcFrameRate = inputFrameRate;
                stH264Vbr.fr32DstFrameRate = outputFrameRate;
                stH264Vbr.u32MaxBitRate = bitRate;
                stH264Vbr.u32MinQp = qpMin;
                stH264Vbr.u32MaxQp = qpMax;
                stH264Vbr.u32MinIQp = qpMinI;
                stH264Vbr.u32MaxIQp = qpMaxI;
                stH264Vbr.s32IntraQpDelta = intraQpDelta;

                stH264Vbr.stQpmapInfo.enQpmapQpType = enQpmapType;
                stH264Vbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH264Vbr.stQpmapInfo.enCtbRcMode = enCtbRcType;
                memcpy(&stVencChnAttr.stRcAttr.stH264Vbr, &stH264Vbr, sizeof(AX_VENC_H264_VBR_S));
            }
            else if (rcMode == VENC_RC_AVBR) {
                AX_VENC_H264_AVBR_S stH264AVbr;
                memset(&stH264AVbr, 0, sizeof(AX_VENC_H264_AVBR_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH264AVbr.u32Gop = gopLen;
                stH264AVbr.u32SrcFrameRate = inputFrameRate;
                stH264AVbr.fr32DstFrameRate = outputFrameRate;
                stH264AVbr.u32MaxBitRate = bitRate;
                stH264AVbr.u32MinQp = qpMin;
                stH264AVbr.u32MaxQp = qpMax;
                stH264AVbr.u32MinIQp = qpMinI;
                stH264AVbr.u32MaxIQp = qpMaxI;
                stH264AVbr.s32IntraQpDelta = intraQpDelta;

                stH264AVbr.stQpmapInfo.enQpmapQpType = enQpmapType;
                stH264AVbr.stQpmapInfo.u32QpmapBlockUnit = gQpmapBlockUnit;
                stH264AVbr.stQpmapInfo.enCtbRcMode = enCtbRcType;
                memcpy(&stVencChnAttr.stRcAttr.stH264AVbr, &stH264AVbr, sizeof(AX_VENC_H264_AVBR_S));
            }
            else if (rcMode == VENC_RC_FIXQP) {
                AX_VENC_H264_FIXQP_S stH264FixQp;
                memset(&stH264FixQp, 0, sizeof(AX_VENC_H264_FIXQP_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = gopLen;
                stH264FixQp.u32SrcFrameRate = inputFrameRate;
                stH264FixQp.fr32DstFrameRate = outputFrameRate;
                stH264FixQp.u32IQp = 25;
                stH264FixQp.u32PQp = 30;
                stH264FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH264FixQp, &stH264FixQp, sizeof(AX_VENC_H264_FIXQP_S));
            }
            else if (rcMode == VENC_RC_QPMAP) {
                AX_VENC_H264_QPMAP_S stH264QpMap;
                memset(&stH264QpMap, 0, sizeof(AX_VENC_H264_QPMAP_S));
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264QPMAP;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = firstFrameQp;
                stH264QpMap.enQpmapQpType = enQpmapType;
                stH264QpMap.enCtbRcMode = enCtbRcType;
                stH264QpMap.u32QpmapBlockUnit = gQpmapBlockUnit;

                stH264QpMap.u32Gop = gopLen;
                stH264QpMap.u32SrcFrameRate = inputFrameRate;
                stH264QpMap.fr32DstFrameRate = outputFrameRate;
                stH264QpMap.u32TargetBitRate = bitRate;
                memcpy(&stVencChnAttr.stRcAttr.stH264QpMap, &stH264QpMap, sizeof(AX_VENC_H264_QPMAP_S));
            }
        }
        break;

        default:
            SAMPLE_ERR_LOG("Invalid Codec Format.\n");
            s32Ret = -1;
            goto END;
        }

        /* GOP table setting */
        switch (gopType) {
        case VENC_GOPMODE_NORMALP:
        {
            stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
        }
        break;
        case VENC_GOPMODE_ONELTR:
        {
            /* Normal frame configures */
            stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_ONELTR;
            stVencChnAttr.stGopAttr.stOneLTR.stPicConfig.s32QpOffset = 0;
            stVencChnAttr.stGopAttr.stOneLTR.stPicConfig.f32QpFactor = 0.4624;
            /* long-term reference and special frame configure */
            stVencChnAttr.stGopAttr.stOneLTR.stPicSpecialConfig.s32Interval = gopLen / 2;
            stVencChnAttr.stGopAttr.stOneLTR.stPicSpecialConfig.s32QpOffset = -2;
            stVencChnAttr.stGopAttr.stOneLTR.stPicSpecialConfig.f32QpFactor = 0.4624;
        }
        break;
        case VENC_GOPMODE_SVC_T:
        {
            stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_SVC_T;
            stVencChnAttr.stGopAttr.stSvcT.u32GopSize = sSvcTGopSize;
            stVencChnAttr.stGopAttr.stSvcT.s8SvcTCfg = stSvcTCfg;
        }
        break;

        default:
            SAMPLE_ERR_LOG("Invalid gop type(%d).\n", gopType);
            s32Ret = -1;
            goto END;
        }

        //userPoolId = VencPoolInit(frameSize, i);
        //if(userPoolId == AX_INVALID_POOLID)
        //{
        //    SAMPLE_ERR_LOG("VeChn %d VencPoolInit failed,CMM memory may not enough!\n", i);
        //    s32Ret = -1;
        //    goto END;
        //}

        /* init frame/stream buffer parameters */
        gEncodePara[i].width = widthSrc;
        gEncodePara[i].height = heightSrc;
        gEncodePara[i].stride = strideSrc;
        gEncodePara[i].frameSize = frameSize;
        gEncodePara[i].userPoolId = AX_INVALID_POOLID;//userPoolId;
        gEncodePara[i].VeChn = i;

        gEncodePara[i].ChnPara.pCmdl = pstArg;

        gGetStreamPara[i].VeChn = i;
        gGetStreamPara[i].ChnPara.pCmdl = pstArg;

        s32Ret = AX_VENC_CreateChn(i, &stVencChnAttr);

        if (AX_SUCCESS != s32Ret) {
            SAMPLE_ERR_LOG("VeChn %d AX_VENC_CreateChn failed, ret=0x%x\n", i,s32Ret);
            goto END;
        }

#ifdef ROI_ENABLE
        s32Ret = SampleSetRoiAttr(i, pstArg);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("VeChn %d SampleSetRoiAttr err, ret=0x%x\n",i,s32Ret);
            goto END;
        }
#endif

#ifndef GET_STREAM_SELECT
        /* create get output stream thread */
        gGetStreamPara[i].bThreadStart = AX_TRUE;
        pthread_create(&gGetStreamPid[i], NULL, VencGetStreamProc, (void *)&gGetStreamPara[i]);
        sleep(1);
#endif

        gEncodePara[i].bThreadStart = AX_TRUE;
        if (VENC_QPMAP_DISABLE  == enQpmapType)
        {
            pthread_create(&gEncodePid[i], NULL, VencEncodeProc, (void *)&gEncodePara[i]);
        }
        else
        {
            pthread_create(&gEncodePid[i], NULL, VencEncodeQpmapProc, (void *)&gEncodePara[i]);
        }
    }

#ifdef GET_STREAM_SELECT
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
        gstGetStreamSelectPara.ChnPara.pCmdl = pChnAttr->tDevAttr[0].pCmdl;
        gstGetStreamSelectPara.ChnPara.pCmdl->chnNum = pChnAttr->nChnNum;
        pthread_create(&gstGetStreamSelectPid, &attr, VencOneThreadGetAllStreamProc, (void *)&gstGetStreamSelectPara);
#endif

    return 0;

END:
    return s32Ret;
}


AX_S32 SampleVencStop(SAMPLE_VENC_ATTR_S *pChnAttr)
{
    AX_S32 s32Ret = -1;
    AX_U32 chnNum = pChnAttr->nChnNum;

#ifdef GET_STREAM_SELECT
    for (int i = 0; i < chnNum; i++) {
        if (AX_TRUE == gEncodePara[i].bThreadStart) {
            gEncodePara[i].bThreadStart = AX_FALSE;
            pthread_join(gEncodePid[i], NULL);
        }
    }

    for (int i = 0; i < chnNum; i++) {
        s32Ret = AX_VENC_StopRecvFrame(i);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_StopRecvFrame Chn:%d failed, ret=0x%x.\n", i, s32Ret);
            // return -1;
        }

        /* VENC release */
        s32Ret = AX_VENC_DestroyChn(i);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_DestroyChn Chn:%d failed, ret=0x%x.\n", i, s32Ret);
            // return -1;
        }
    }

    if (AX_TRUE == gstGetStreamSelectPara.bThreadStart) {
        gstGetStreamSelectPara.bThreadStart = AX_FALSE;
        pthread_attr_destroy(&attr);
    }
#else
    for (int i = 0; i < chnNum; i++) {
        if (AX_TRUE == gEncodePara[i].bThreadStart) {
            gEncodePara[i].bThreadStart = AX_FALSE;
            pthread_join(gEncodePid[i], NULL);
        }else{
            continue;
        }

        s32Ret = AX_VENC_StopRecvFrame(i);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_StopRecvFrame Chn:%d failed, ret=0x%x.\n", i, s32Ret);
            // return -1;
        }

        /* VENC release */
        s32Ret = AX_VENC_DestroyChn(i);
        if (0 != s32Ret) {
            SAMPLE_ERR_LOG("AX_VENC_DestroyChn Chn:%d failed, ret=0x%x.\n", i, s32Ret);
            // return -1;
        }

        if (AX_TRUE == gGetStreamPara[i].bThreadStart) {
            gGetStreamPara[i].bThreadStart = AX_FALSE;
            pthread_join(gGetStreamPid[i], NULL);
        }

    }
#endif
    return 0;
}


static int SampleSetRoiAttr(AX_S32 VeChn, SAMPLE_VENC_CMD_PARA_T *pstArg)
{
    AX_S32 s32Ret = -1;

    for (int i = 0; i < MAX_ROI_NUM; i++) {
        s32Ret = AX_VENC_SetRoiAttr(VeChn, &pstArg->roiAttr[i]);
        if (AX_SUCCESS != s32Ret) {
            VENC_LOG("Chn %d: AX_VENC_SetRoiAttr %d failed, ret=%x\n", VeChn, i, s32Ret);
            return -1;
        }
    }

    return 0;
}
