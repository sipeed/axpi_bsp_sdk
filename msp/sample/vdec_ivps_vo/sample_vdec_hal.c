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
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

//#include "openssl/md5.h"
#include "ax_vdec_api.h"
#include "ax_base_type.h"
#include "ax_sys_api.h"
#include "ax_buffer_tool.h"
#include "sample_vdec_hal.h"



typedef struct axSAMPLE_CHN_ARGS_S {
    AX_VDEC_GRP VdecGrp;
    AX_U8 *pu8StreamMem;
    AX_S32 s32StreamLen;
} SAMPLE_CHN_ARGS_S;

typedef struct _SAMPLE_BSPARSER {
    FILE *fInput;
    AX_S32 sSize;
} SAMPLE_BSPARSER_T;

typedef struct _SAMPLE_VDEC_FUNPARAM {
    AX_VDEC_GRP VdecGrp;
    AX_VIDEO_FRAME_INFO_S *pstUsrPic;
    AX_CHAR *sFile;
} SAMPLE_ARGS_S;


typedef enum _SAMPLE_BSBOUNDAR_YTYPE {
    BSPARSER_NO_BOUNDARY = 0,
    BSPARSER_BOUNDARY = 1,
    BSPARSER_BOUNDARY_NON_SLICE_NAL = 2
} SAMPLE_BSBOUNDAR_YTYPE_E;

extern AX_S32 gLoopDecodeNumber;
extern AX_S32 gLoopExit;
AX_S32 gWriteFrames;
static AX_VDEC_GRP_ATTR_S gGrpAttr = {
    .enType = PT_JPEG,
    .enMode = VIDEO_MODE_FRAME,
    .u32PicWidth = 4096,
    .u32PicHeight = 4096,
    .u32StreamBufSize = 10 * 1024 * 1024,
    .u32FrameBufSize = 10 * 1024 * 1024,
    .u32FrameBufCnt = 1,
};

AX_POOL GrpPoolId[AX_VDEC_MAX_GRP_SIZE];


AX_S32 LoadFileToMem(const AX_CHAR *ps8File, AX_U8 **ppu8Mem, AX_S32 *ps32Len)
{
    /* Reading input file */
    FILE *f_in = fopen(ps8File, "rb");
    if (f_in == NULL) {
        printf("Unable to open input file\n");
        return -1;
    }

    /* file i/o pointer to full */
    fseek(f_in, 0L, SEEK_END);
    *ps32Len = ftell(f_in);
    rewind(f_in);

    *ppu8Mem = malloc(sizeof(AX_U8) * (*ps32Len));
    assert(*ppu8Mem != NULL);
    printf("alloc decoder addr %p\n", *ppu8Mem);

    /* read input stream from file to buffer and close input file */
    if (fread(*ppu8Mem, sizeof(AX_U8), *ps32Len, f_in) != *ps32Len) {
        printf("fread error\n");
        free(*ppu8Mem);
        *ppu8Mem = NULL;
        fclose(f_in);
        return -1;
    }

    fclose(f_in);
    return 0;
}


/*here for h264 stream read frame by frame*/
#define NAL_CODED_SLICE_CRA  21
#define NAL_CODED_SLICE_IDR  5
#define BUFFER_SIZE  3 * 1024 * 1024

static AX_S32 FindNextStartCode(SAMPLE_BSPARSER_T *tBsInfo, AX_U32 *uZeroCount)
{
    AX_S32 i;
    AX_S32 sStart = ftello(tBsInfo->fInput);
    *uZeroCount = 0;

    /* Scan for the beginning of the packet. */
    for (i = 0; i < tBsInfo->sSize && i < tBsInfo->sSize - sStart; i++) {
        AX_U8 byte;
        AX_S32 ret_val = fgetc(tBsInfo->fInput);
        if (ret_val == EOF) return ftello(tBsInfo->fInput);
        byte = (unsigned char)ret_val;
        switch (byte) {
        case 0:
            *uZeroCount = *uZeroCount + 1;
            break;
        case 1:
            /* If there's more than three leading zeros, consider only three
             * of them to be part of this packet and the rest to be part of
             * the previous packet. */
            if (*uZeroCount > 3) *uZeroCount = 3;
            if (*uZeroCount >= 2) {
                return ftello(tBsInfo->fInput) - *uZeroCount - 1;
            }
            *uZeroCount = 0;
            break;
        default:
            *uZeroCount = 0;
            break;
        }
    }
    return ftello(tBsInfo->fInput);
}


AX_U32 CheckAccessUnitBoundaryH264(FILE *fInput, AX_S32 sNalBegin)
{
    AX_U32 uBoundary = BSPARSER_NO_BOUNDARY;
    AX_U32 uNalType, uVal;

    AX_S32 sStart = ftello(fInput);

    fseeko(fInput, sNalBegin, SEEK_SET);
    uNalType = (getc(fInput) & 0x1F);

    if (uNalType > NAL_CODED_SLICE_IDR)
        uBoundary = BSPARSER_BOUNDARY_NON_SLICE_NAL;
    else {
        uVal = getc(fInput);
        /* Check if first mb in slice is 0(ue(v)). */
        if (uVal & 0x80) uBoundary = BSPARSER_BOUNDARY;
    }

    fseeko(fInput, sStart, SEEK_SET);
    return uBoundary;
}

AX_S32 StreamParserReadFrameH264(SAMPLE_BSPARSER_T *tBsInfo, AX_U8 *sBuffer,
                                 AX_S32 *sSize)
{
    AX_S32 sBegin, sEnd, sStrmLen;
    AX_U32 sReadLen;
    AX_U32 uZeroCount = 0;

    AX_U32 uTmp = 0;
    AX_S32 sNalBegin;
    /* TODO(min): to extract exact one frame instead of a NALU */

    sBegin = FindNextStartCode(tBsInfo, &uZeroCount);
    sNalBegin = sBegin + uZeroCount + 1;
    uTmp = CheckAccessUnitBoundaryH264(tBsInfo->fInput, sNalBegin);
    sEnd = sNalBegin = FindNextStartCode(tBsInfo, &uZeroCount);

    if (sEnd != sBegin && uTmp != BSPARSER_BOUNDARY_NON_SLICE_NAL) {
        do {
            sEnd = sNalBegin;
            sNalBegin += uZeroCount + 1;

            /* Check access unit boundary for next NAL */
            uTmp = CheckAccessUnitBoundaryH264(tBsInfo->fInput, sNalBegin);
            if (uTmp == BSPARSER_NO_BOUNDARY) {
                sNalBegin = FindNextStartCode(tBsInfo, &uZeroCount);
            } else if (uTmp == BSPARSER_BOUNDARY_NON_SLICE_NAL) {
                do {
                    sNalBegin = FindNextStartCode(tBsInfo, &uZeroCount);
                    if (sEnd == sNalBegin) break;
                    sEnd = sNalBegin;
                    sNalBegin += uZeroCount + 1;
                    uTmp = CheckAccessUnitBoundaryH264(tBsInfo->fInput, sNalBegin);
                } while (uTmp == BSPARSER_BOUNDARY_NON_SLICE_NAL);

                if (sEnd == sNalBegin) {
                    break;
                } else if (uTmp == BSPARSER_NO_BOUNDARY) {
                    sNalBegin = FindNextStartCode(tBsInfo, &uZeroCount);
                }
            }
        } while (uTmp != BSPARSER_BOUNDARY);
    }

    if (sEnd == sBegin) {
        return AX_SUCCESS; /* End of stream */
    }
    fseeko(tBsInfo->fInput, sBegin, SEEK_SET);
    if (*sSize < sEnd - sBegin) {
        *sSize = sEnd - sBegin;
        return AX_ERR_VDEC_RUN_ERROR; /* Insufficient buffer size */
    }

    sStrmLen = sEnd - sBegin;
    sReadLen = fread(sBuffer, 1, sStrmLen, tBsInfo->fInput);

    return sReadLen;
}
/*end read*/

AX_S32 FramePoolInit(AX_VDEC_GRP VdGrp, AX_U32 FrameSize, AX_POOL *PoolId)
{
    AX_S32 s32Ret = AX_SUCCESS;
    /* vdec use pool to alloc output buffer */
    AX_POOL_CONFIG_T stPoolConfig = {0};
    AX_POOL s32PoolId;

    memset(&stPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    stPoolConfig.MetaSize = 512;
    stPoolConfig.BlkCnt = 10;
    stPoolConfig.BlkSize = FrameSize;
    stPoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(stPoolConfig.PartitionName, 0, sizeof(stPoolConfig.PartitionName));
    strcpy((AX_CHAR *)stPoolConfig.PartitionName, "anonymous");

    s32PoolId = AX_POOL_CreatePool(&stPoolConfig);
    if (AX_INVALID_POOLID == s32PoolId) {
        printf("Create pool err.\n");
        return AX_ERR_VDEC_NULL_PTR;
    }

    *PoolId = s32PoolId;

    s32Ret = AX_VDEC_AttachPool(VdGrp, s32PoolId);
    if (s32Ret != AX_SUCCESS) {
        AX_POOL_MarkDestroyPool(s32PoolId);
        printf("Attach pool err. %x\n", s32Ret);
    }

    printf("FramePoolInit successfully! %d\n", s32PoolId);

    return s32Ret;
}

#ifdef SAMPLE_VDEC_USERPIC_TEST
//inout
AX_S32 PoolUserPicInit(AX_VIDEO_FRAME_INFO_S *pstUserPic)
{
    AX_S32 ret = 0;
    AX_POOL_CONFIG_T PoolConfig;
    AX_POOL UserPoolId;
    AX_S32 uiWidthAlign = ALIGN_UP(USR_PIC_WIDTH,16);
    AX_S32 uiHeightAlign = ALIGN_UP(USR_PIC_HEIGHT,16);

    memset(&PoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    PoolConfig.MetaSize= 8 * 1024;//8k
    PoolConfig.BlkSize = (uiWidthAlign * uiHeightAlign) * 3/2;
    PoolConfig.BlkCnt  = 1;
    PoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(PoolConfig.PartitionName, 0, sizeof(PoolConfig.PartitionName));
    strcpy((AX_CHAR *)PoolConfig.PartitionName, "anonymous");

    UserPoolId = AX_POOL_CreatePool(&PoolConfig);

    if (AX_INVALID_POOLID == UserPoolId)
    {
        printf("AX_POOL_CreatePool error!!!\n");
        goto ERROR;
    }else{
        printf("AX_POOL_CreatePool[%d] success\n",UserPoolId);
    }

    pstUserPic->u32PoolId = UserPoolId;
    printf("%s:line:%d: pstUserPic->uPoolId = %d\n",__func__, __LINE__, pstUserPic->u32PoolId);

     return AX_SUCCESS;
ERROR:
     printf("\n\n");
     ret = AX_POOL_Exit();
     if(ret){
          printf("AX_POOL_Exit fail!!Error Code:0x%X\n",ret);
          return -1;
     }else{
          printf("AX_POOL_Exit success!\n");
     }
     return -1;
}

AX_S32 PoolUserPicDisplay(AX_VIDEO_FRAME_INFO_S *pstUserPic)
{
    AX_S32 iRet = AX_SUCCESS;
    AX_S32 i = 0;
    AX_S32 j = 0;
    AX_U32 uiWidthAlign = 0;
    AX_U32 uiHeightAlign = 0;
    AX_U32 uiPicSize = 0;
    AX_U32 u32YStride = 0;
    AX_U8 *pu8yuv = NULL;
    AX_BLK blk_id = AX_INVALID_BLOCKID;

    FILE *fpUserYUV = NULL;

    char pFileName[] = "/opt/data/vdec/UserPic_nv12.yuv";

    uiWidthAlign = ALIGN_UP(USR_PIC_WIDTH,16);
    uiHeightAlign = ALIGN_UP(USR_PIC_HEIGHT,16);
    uiPicSize = (uiWidthAlign * uiHeightAlign) * 3/2;

    blk_id = AX_POOL_GetBlock(pstUserPic->u32PoolId, uiPicSize, NULL);
    if (AX_INVALID_BLOCKID == blk_id) {
        printf("AX_POOL_GetBlock AX_POOL_GetBlockfailed! \n");
        return -1;
    }

    pstUserPic->bEof = AX_TRUE;
    pstUserPic->enModId = AX_ID_VDEC;
    pstUserPic->stVFrame.u32BlkId[0] = blk_id;
    pstUserPic->stVFrame.u32Width = USR_PIC_WIDTH;
    pstUserPic->stVFrame.u32Height = USR_PIC_HEIGHT;
    pstUserPic->stVFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
    pstUserPic->stVFrame.enVscanFormat = AX_VSCAN_FORMAT_RASTER;
    pstUserPic->stVFrame.enCompressMode = AX_COMPRESS_MODE_NONE;
    pstUserPic->stVFrame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(blk_id);
    pstUserPic->stVFrame.u64VirAddr[0] = AX_POOL_GetBlockVirAddr(blk_id);
    pstUserPic->stVFrame.u32PicStride[0] = uiWidthAlign;
    pstUserPic->stVFrame.u64PhyAddr[1] = pstUserPic->stVFrame.u64PhyAddr[0] + pstUserPic->stVFrame.u32PicStride[0] * pstUserPic->stVFrame.u32Height;
    pstUserPic->stVFrame.u64PhyAddr[2] = 0;
    pstUserPic->stVFrame.u64VirAddr[1] = pstUserPic->stVFrame.u64VirAddr[0] + pstUserPic->stVFrame.u32PicStride[0] * pstUserPic->stVFrame.u32Height;
    pstUserPic->stVFrame.u64VirAddr[2] = 0;
    pstUserPic->u32PoolId = AX_POOL_Handle2PoolId(blk_id);

    pu8yuv = (AX_U8 *)pstUserPic->stVFrame.u64VirAddr[0];
    u32YStride = (pstUserPic->stVFrame.u32Width + 15) & ( ~(15) );
    fpUserYUV = fopen(pFileName,"rb");
    if (fpUserYUV == NULL)
    {
        printf("can't open file %s in VDEC_PREPARE_USERPIC.\n", pFileName);
        return -1;
    }

    
    /* read the data of YUV*/
    fread(pu8yuv, 1, (pstUserPic->stVFrame.u32Height * u32YStride * 3 / 2), fpUserYUV);
    fclose(fpUserYUV);

    printf("PoolUserPicDisplay %llx %llx %llx %llx\n", pstUserPic->stVFrame.u64PhyAddr[0], pstUserPic->stVFrame.u64VirAddr[0],
            pstUserPic->stVFrame.u64PhyAddr[1], pstUserPic->stVFrame.u64VirAddr[1]);
    printf("PoolUserPicDisplay sucess\n");
    pstUserPic->stVFrame.u64PTS = 0;
    return AX_SUCCESS;
}

AX_S32 PoolUserPicDeinit(AX_VIDEO_FRAME_INFO_S *pstUserPic)
{
    AX_S32 ret = AX_SUCCESS;
    AX_U32 PoolId = 0;
    AX_BLK BlkId = 0;

    PoolId = pstUserPic->u32PoolId;

    BlkId = pstUserPic->stVFrame.u32BlkId[0];
    ret = AX_POOL_ReleaseBlock(BlkId);
    if(ret){
        printf("AX_POOL_ReleaseBlock fail!Error Code:0x%X\n",ret);
        goto ERROR;
    }

    ret = AX_POOL_MunmapPool(PoolId);
    if(ret){
        printf("AX_POOL_MunmapPool fail!!Error Code:0x%X\n", ret);
        goto ERROR;
    }

    ret = AX_POOL_MarkDestroyPool(PoolId);
    if(ret){
        printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n", ret);
        goto ERROR;
    }

    return 0;

ERROR:
    printf("\nsample_pool test fail!\n");
    ret = AX_POOL_Exit();
    if(ret){
        printf("AX_POOL_Exit fail!!Error Code:0x%X\n", ret);
    }

    return -1;
}
#endif
void *H264DecFrameFunc(void *arg)
{
    SAMPLE_ARGS_S *tFunPara = (SAMPLE_ARGS_S *)arg;
    AX_S32 sRet = 0;
    AX_VDEC_GRP_ATTR_S tVdecAttr = {0};
    AX_VDEC_STREAM_S tStrInfo = {0};
    AX_U32 sReadLen = 0;
    AX_U32 FrameSize = 0;
    AX_U32 id = 0;
    AX_POOL PoolId;

    AX_VDEC_GRP VdGrp = tFunPara->VdecGrp;

    AX_VDEC_DISPLAY_MODE_E eDispMod;
    AX_VDEC_DISPLAY_MODE_E eGetDispMod;
    AX_CHAR *sFile = tFunPara->sFile;
    AX_U8 *sStreamMem = NULL;
    SAMPLE_BSPARSER_T tStreamInfo = {0};
    #ifdef SAMPLE_VDEC_USERPIC_TEST
    AX_VIDEO_FRAME_INFO_S *pstUsrPic = tFunPara->pstUsrPic;
    AX_U32 userPicFlag= 0;
    #endif
    AX_S32 sSize = BUFFER_SIZE;

    printf("DecoderFrameFunc Creat Grp is %d\n", VdGrp);

    FILE *fInput = NULL;

    fInput = fopen(sFile, "rb");
    if (fInput == NULL) {
        printf("Unable to open input file\n");
        return NULL;
    }

    fseek(fInput, 0L, SEEK_END);
    AX_S32 sLen = ftell(fInput);
    rewind(fInput);

    ///////////////////////////////////////////////////////
    /*begin to read frame stream*/

    sStreamMem = (AX_U8 *)malloc(sizeof(AX_U8) * BUFFER_SIZE);
    assert(sStreamMem != NULL);
    /////////////////////////////////////
    /*GROUP VDEC ATTR*/
    tVdecAttr.enType = PT_H264;
    //tVdecAttr.stVdecVideoAttr.eDecMode = VIDEO_DEC_NODE_IPB;
    tVdecAttr.stVdecVideoAttr.eOutOrder = VIDEO_OUTPUT_ORDER_DISP;
    tVdecAttr.u32PicWidth = 1920;  /*Max pic width*/
    tVdecAttr.u32PicHeight = 1080;  /*Max pic height*/
    tVdecAttr.u32StreamBufSize = 8 * 1024 * 1024;
    tVdecAttr.u32FrameBufCnt = 10;
    tVdecAttr.enLinkMode = AX_LINK_MODE;
    

    /*GROUP CREATE FOR 16 PATH*/
    sRet = AX_VDEC_CreateGrp(VdGrp, &tVdecAttr);
    if (sRet != AX_SUCCESS) {
        printf("Create instance failed! %x\n", sRet);
        goto end;
    }

    FrameSize = AX_VDEC_GetPicBufferSize(1920, 1088, PT_H264); //3655712;
    printf("Get pool mem size is %d\n", FrameSize);
    sRet = FramePoolInit(VdGrp, FrameSize, &PoolId);
    if (sRet != AX_SUCCESS) {
        printf("FramePoolInit failed! Error:%x\n", sRet);
        goto end1;
    }
    /////////////////////////////////////
    GrpPoolId[VdGrp] = PoolId;

    /////////////////////////////////////
    /*VDEC DISPALY MODE*/
    eDispMod = VIDEO_DISPLAY_MODE_PLAYBACK;
    AX_VDEC_SetDisplayMode(VdGrp, eDispMod);
    AX_VDEC_GetDisplayMode(VdGrp, &eGetDispMod);
    printf("Vdec Get Display Mode is %d\n", eGetDispMod);

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest){
        AX_VDEC_DisableUserPic(VdGrp);
        AX_VDEC_SetUserPic(VdGrp, pstUsrPic);
        userPicFlag = 1;
    }
    #endif
    sRet = AX_VDEC_StartRecvStream(VdGrp);
    if (sRet != AX_SUCCESS) {
        printf("Create instance failed!\n");
        goto end2;
    }

    tStreamInfo.fInput = fInput;
    tStreamInfo.sSize = sLen;

    do {
        sReadLen = StreamParserReadFrameH264(&tStreamInfo, sStreamMem, &sSize);
        printf("begin to decoder sLen=%d id %d\n", sReadLen, id);
        id++;

        if (sReadLen) {

            tStrInfo.pu8Addr = sStreamMem;
            tStrInfo.u32Len = sReadLen;  /*stream len*/
            tStrInfo.bEndOfFrame = AX_TRUE;
            tStrInfo.bEndOfStream = AX_FALSE;
            tStrInfo.u64PTS += 20;
        } else {
            /*init video stream info*/
            tStrInfo.pu8Addr = NULL;
            tStrInfo.u32Len = 0x0;  /*stream len*/
            tStrInfo.bEndOfFrame = AX_TRUE;
            tStrInfo.bEndOfStream = AX_TRUE;
        }

        sRet = AX_VDEC_SendStream(VdGrp, &tStrInfo, -1);
        usleep(10 *1000);
        /*add CTRL+C*/
        if (gLoopExit) {
            /*init video stream info*/
            tStrInfo.pu8Addr = NULL;
            tStrInfo.u32Len = 0x0;  /*stream len*/
            tStrInfo.bEndOfFrame = AX_TRUE;
            tStrInfo.bEndOfStream = AX_TRUE;

            sRet = AX_VDEC_SendStream(VdGrp, &tStrInfo, -1);
            break;
        }

        #ifdef SAMPLE_VDEC_USERPIC_TEST
        if(userPicFlag && userPicTest && (id >= gWriteFrames + 2 || id >=11))
        {
            userPicFlag = 0;
            AX_VDEC_StopRecvStream(VdGrp);
            usleep(30000);
            sleep(2);
            printf("AX_VDEC_EnableUserPic VdGrp %d\n", VdGrp);
            AX_VDEC_EnableUserPic(VdGrp, 0);
        }
        #endif
    } while (sReadLen);


    if (sStreamMem) {
        free(sStreamMem);
    }
    if (fInput) {
        fclose(fInput);
    }

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest)
        AX_VDEC_DisableUserPic(VdGrp);
    #endif
    return (void *)AX_SUCCESS;

end2:
    if (sStreamMem) {
        free(sStreamMem);
    }
    if (fInput) {
        fclose(fInput);
    }
    AX_VDEC_DetachPool(VdGrp);
    AX_VDEC_DestroyGrp(VdGrp);
    AX_POOL_MarkDestroyPool(PoolId);
    return NULL;
end1:
    AX_VDEC_DestroyGrp(VdGrp);
end:
    if (sStreamMem) {
        free(sStreamMem);
    }
    if (fInput) {
        fclose(fInput);
    }
    return NULL;
}

AX_S32 H264DecExitFunc(AX_VDEC_GRP VdGrp)
{
    AX_S32 sRet = 0;

    AX_VDEC_StopRecvStream(VdGrp);

    AX_VDEC_DetachPool(VdGrp);

    AX_VDEC_DestroyGrp(VdGrp);

    sRet = AX_POOL_MarkDestroyPool(GrpPoolId[VdGrp]);
    if (sRet) {
        printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n", sRet);
        return AX_ERR_VDEC_UNKNOWN;
    }

    return AX_SUCCESS;
}

void *VdecThread(void *arg)
{
    SAMPLE_CHN_ARGS_S *tFunPara = (SAMPLE_CHN_ARGS_S *)arg;
    AX_VDEC_GRP VdecGrp = tFunPara->VdecGrp;
    AX_U32 FrameSize = 0;
    AX_POOL PoolId;

    /*jdec used pool memory,for 1920*1080*/
    FrameSize = AX_VDEC_GetPicBufferSize(1920, 1088, PT_H264); //3655712;
    printf("Get pool mem size is %d\n", FrameSize);
    gGrpAttr.u32FrameBufSize = FrameSize;
    gGrpAttr.enLinkMode = AX_LINK_MODE;

    AX_S32 ret = AX_VDEC_CreateGrp(VdecGrp, &gGrpAttr);
    if (ret != AX_SUCCESS) {
        printf("AX_VDEC_CreateChn error: %d\n", ret);
        return NULL;
    }

    /*init pool memory for jdec*/
    ret = FramePoolInit(VdecGrp, gGrpAttr.u32FrameBufSize, &PoolId);
    if (ret != AX_SUCCESS) {
        printf("AX_VDEC_StartRecvStream error: %d\n", ret);
        AX_VDEC_DestroyGrp(VdecGrp);
        return NULL;
    }

    ret = AX_VDEC_StartRecvStream(VdecGrp);
    if (ret != AX_SUCCESS) {
        printf("AX_VDEC_StartRecvStream error: %d\n", ret);
        AX_VDEC_DestroyGrp(VdecGrp);
        return NULL;
    }

    AX_S32 loopDecNumber = 0;

    while (1) {
        AX_VDEC_STREAM_S stream;
        stream.u32Len = tFunPara->s32StreamLen;
        stream.pu8Addr = tFunPara->pu8StreamMem;
        printf("decoder addr %p, DecodeNumber %d\n", tFunPara->pu8StreamMem, gLoopDecodeNumber);
        stream.bEndOfStream = AX_FALSE;
        AX_VDEC_SendStream(VdecGrp, &stream, -1);
        loopDecNumber++;
        if (((gLoopDecodeNumber > 0) && (loopDecNumber >= gLoopDecodeNumber)) || gLoopExit) {
            stream.u32Len = 0;
            stream.pu8Addr = NULL;
            stream.bEndOfStream = AX_TRUE;
            AX_VDEC_SendStream(VdecGrp, &stream, -1);
            break;
        }
    }

    printf("Jpeg Chn send data %d exit\n", VdecGrp);
    return NULL;
}

AX_S32 JpegDecExitFunc(AX_VDEC_GRP VdGrp)
{
    AX_S32 sRet = 0;

    AX_VDEC_StopRecvStream(VdGrp);

    AX_VDEC_DetachPool(VdGrp);

    AX_VDEC_DestroyGrp(VdGrp);

    sRet = AX_POOL_MarkDestroyPool(GrpPoolId[VdGrp]);
    if (sRet) {
        printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n", sRet);
        return AX_ERR_VDEC_UNKNOWN;
    }
    printf("Jpeg Chn %d exit\n", VdGrp);

    return AX_SUCCESS;
}

