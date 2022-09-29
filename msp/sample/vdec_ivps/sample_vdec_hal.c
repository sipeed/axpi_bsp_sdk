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
    AX_CHAR *sFile;
} SAMPLE_ARGS_S;


typedef enum _SAMPLE_BSBOUNDAR_YTYPE {
    BSPARSER_NO_BOUNDARY = 0,
    BSPARSER_BOUNDARY = 1,
    BSPARSER_BOUNDARY_NON_SLICE_NAL = 2
} SAMPLE_BSBOUNDAR_YTYPE_E;

static AX_S32 gLoopDecodeNumber = 1;
extern AX_S32 gLoopExit;
extern AX_S32 gChnNumber;


static AX_VDEC_GRP_ATTR_S gGrpAttr = {
    .enType = PT_JPEG,
    .enMode = VIDEO_MODE_FRAME,
    .u32PicWidth = 4096,
    .u32PicHeight = 4096,
    .u32StreamBufSize = 10 * 1024 * 1024,
    .u32FrameBufSize = 10 * 1024 * 1024,
    .u32FrameBufCnt = 1,
};

AX_POOL GrpPoolId;


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

AX_S32 SampleVdecInit(AX_PAYLOAD_TYPE_E enType)
{
    /* vdec use pool to alloc output buffer */
    AX_POOL_CONFIG_T stPoolConfig = {0};
    AX_POOL s32PoolId;
    AX_U32 FrameSize = 0;

    FrameSize = AX_VDEC_GetPicBufferSize(1920, 1088, enType);
    printf("Get pool mem size is %d\n", FrameSize);

    memset(&stPoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    stPoolConfig.MetaSize = 1024;
    stPoolConfig.BlkCnt = 10 * gChnNumber;
    stPoolConfig.BlkSize = FrameSize;
    stPoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(stPoolConfig.PartitionName, 0, sizeof(stPoolConfig.PartitionName));
    strcpy((AX_CHAR *)stPoolConfig.PartitionName, "anonymous");

    s32PoolId = AX_POOL_CreatePool(&stPoolConfig);
    if (AX_INVALID_POOLID == s32PoolId) {
        printf("Create pool err.\n");
        return AX_ERR_VDEC_NULL_PTR;
    }

    GrpPoolId = s32PoolId;

    printf("FramePoolInit successfully! %d\n", s32PoolId);

    return AX_SUCCESS;
}

void *H264DecFrameFunc(void *arg)
{
    SAMPLE_ARGS_S *tFunPara = (SAMPLE_ARGS_S *)arg;
    AX_S32 sRet = 0;
    AX_VDEC_GRP_ATTR_S tVdecAttr = {0};
    AX_VDEC_STREAM_S tStrInfo = {0};
    AX_U32 sReadLen = 0;

    AX_VDEC_GRP VdGrp = tFunPara->VdecGrp;

    AX_VDEC_DISPLAY_MODE_E eDispMod;
    AX_VDEC_DISPLAY_MODE_E eGetDispMod;
    AX_CHAR *sFile = tFunPara->sFile;
    AX_U8 *sStreamMem = NULL;
    SAMPLE_BSPARSER_T tStreamInfo = {0};

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

    /////////////////////////////////////
    sRet = AX_VDEC_AttachPool(VdGrp, GrpPoolId);
    if (sRet != AX_SUCCESS) {
        printf("Attach pool err. %x\n", sRet);
    }

    /////////////////////////////////////
    /*VDEC DISPALY MODE*/
    eDispMod = VIDEO_DISPLAY_MODE_PLAYBACK;
    AX_VDEC_SetDisplayMode(VdGrp, eDispMod);
    AX_VDEC_GetDisplayMode(VdGrp, &eGetDispMod);
    printf("Vdec Get Display Mode is %d\n", eGetDispMod);

    sRet = AX_VDEC_StartRecvStream(VdGrp);
    if (sRet != AX_SUCCESS) {
        printf("Create instance failed!\n");
        goto end1;
    }

    tStreamInfo.fInput = fInput;
    tStreamInfo.sSize = sLen;

    do {
        sReadLen = StreamParserReadFrameH264(&tStreamInfo, sStreamMem, &sSize);
        printf("begin to decoder sLen=%d\n", sReadLen);

        if (sReadLen) {

            tStrInfo.pu8Addr = sStreamMem;
            tStrInfo.u32Len = sReadLen;  /*stream len*/
            tStrInfo.bEndOfFrame = AX_TRUE;
            tStrInfo.bEndOfStream = AX_FALSE;
            tStrInfo.u64PTS += 1;
        } else {
            /*init video stream info*/
            tStrInfo.pu8Addr = NULL;
            tStrInfo.u32Len = 0x0;  /*stream len*/
            tStrInfo.bEndOfFrame = AX_TRUE;
            tStrInfo.bEndOfStream = AX_TRUE;
        }

        sRet = AX_VDEC_SendStream(VdGrp, &tStrInfo, -1);

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
    } while (sReadLen);

    if (sStreamMem) {
        free(sStreamMem);
    }
    if (fInput) {
        fclose(fInput);
    }

    return (void *)AX_SUCCESS;

end1:
    if (sStreamMem) {
        free(sStreamMem);
    }
    if (fInput) {
        fclose(fInput);
    }
    AX_VDEC_DetachPool(VdGrp);
    AX_VDEC_DestroyGrp(VdGrp);
    return NULL;
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
    AX_VDEC_StopRecvStream(VdGrp);

    AX_VDEC_DetachPool(VdGrp);

    AX_VDEC_DestroyGrp(VdGrp);

    printf("H264DecExitFunc successful GRP:%d\n", VdGrp);

    return AX_SUCCESS;
}

AX_S32 SampleVdecExit()
{
    AX_S32 sRet = 0;
    AX_U32 num = 25;

retry:
    sRet = AX_POOL_MarkDestroyPool(GrpPoolId);
    if (sRet) {
        num --;
        if (num) {
            usleep(40000);
            goto retry;
        }
    }
    printf("AX_POOL_MarkDestroyPool successful\n");

    return AX_SUCCESS;
}

void *VdecThread(void *arg)
{
    SAMPLE_CHN_ARGS_S *tFunPara = (SAMPLE_CHN_ARGS_S *)arg;
    AX_VDEC_GRP VdecGrp = tFunPara->VdecGrp;
    AX_U32 FrameSize = 0;

    /*jdec used pool memory,for 1920*1080*/
    FrameSize = AX_VDEC_GetPicBufferSize(1920, 1088, PT_JPEG);

    gGrpAttr.u32FrameBufSize = FrameSize;
    gGrpAttr.enLinkMode = AX_LINK_MODE;

    AX_S32 ret = AX_VDEC_CreateGrp(VdecGrp, &gGrpAttr);
    if (ret != AX_SUCCESS) {
        printf("AX_VDEC_CreateChn error: %d\n", ret);
        return NULL;
    }

    /*init pool memory for jdec*/
    ret = AX_VDEC_AttachPool(VdecGrp, GrpPoolId);
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
        printf("decoder addr %p\n", tFunPara->pu8StreamMem);
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

    AX_VDEC_StopRecvStream(VdGrp);

    AX_VDEC_DetachPool(VdGrp);

    AX_VDEC_DestroyGrp(VdGrp);

    printf("Jpeg Chn %d exit\n", VdGrp);

    return AX_SUCCESS;
}

