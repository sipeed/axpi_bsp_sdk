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

#define SAMPLE_VDEC_USERPIC_TEST
#ifdef SAMPLE_VDEC_USERPIC_TEST
#define USR_PIC_WIDTH 1920
#define USR_PIC_HEIGHT 1080
#endif
#define SIZE_ALIGN(x,align) ((((x)+(align)-1)/(align))*(align))
#ifndef ALIGN_UP
#define ALIGN_UP(x, align)      (((x) + ((align) - 1)) & ~((align) - 1))
#endif
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

static AX_S32 gChnNumber = 1;
static AX_S32 gLoopDecodeNumber = 1;
static AX_S32 gMilliSec = -1;
static AX_S32 gWriteFrames = 0;
static AX_CHAR *gdestMD5 = NULL;
static AX_S32 userPicTest = 0;
static AX_VDEC_GRP_ATTR_S gGrpAttr = {
    .enType = PT_JPEG,
    .enMode = VIDEO_MODE_FRAME,
    .u32PicWidth = 4096,
    .u32PicHeight = 4096,
    .u32StreamBufSize = 10 * 1024 * 1024,
    .u32FrameBufSize = 10 * 1024 * 1024,
    .u32FrameBufCnt = 1,
};
static AX_S32 gLoopExit = 0;
static struct timeval gTimebegin, gTimeend;

static SAMPLE_CHN_ARGS_S GrpChnArgs[AX_VDEC_MAX_GRP_SIZE];
static SAMPLE_ARGS_S GrpArgs[AX_VDEC_MAX_GRP_SIZE];

#if 0
AX_U32 MD5SumValidOnly(AX_U8 *p_lu, AX_U8 *p_ch,
                       AX_U32 coded_width, AX_U32 coded_height,
                       AX_U32 coded_width_ch, AX_U32 coded_height_ch,
                       AX_U32 pic_stride, AX_U32 pic_stride_ch,
                       AX_U32 planar, AX_U32 frame_number, char *md5_str)
{
    unsigned char digest[16];
    MD5_CTX ctx;
    int i = 0;
    MD5_Init(&ctx);
    AX_U8 *p_yuv = p_lu;
    if (p_yuv) {
        for (i = 0; i < coded_height; i++) {
            MD5_Update(&ctx, p_yuv, coded_width);
            p_yuv += pic_stride;
        }
    }
    p_yuv = p_ch;
    if (p_yuv) {
        if (!planar) {
            for (i = 0; i < coded_height_ch; i++) {
                MD5_Update(&ctx, p_yuv, coded_width_ch);
                p_yuv += pic_stride;
            }
        } else {
            for (i = 0; i < coded_height_ch; i++) {
                MD5_Update(&ctx, p_yuv, coded_width_ch / 2);
                p_yuv += pic_stride_ch;
            }
            for (i = 0; i < coded_height_ch; i++) {
                MD5_Update(&ctx, p_yuv, coded_width_ch / 2);
                p_yuv += pic_stride_ch;
            }
        }
    }
    MD5_Final(digest, &ctx);
    /*    fprintf(f_out, "FRAME %d: ", frame_number);*/
    for (i = 0; i < 16; i++) {
        snprintf(md5_str + i * 2, 2 + 1, "%02x", digest[i]);
    }
    return 0;
}

void CheckMD5(AX_VIDEO_FRAME_INFO_S *frameInfo, char *md5_str)
{
    AX_S32 s32Ret;
    AX_VOID *pLumaVirAddr;
    AX_VOID *pChromaVirAddr;
    AX_U32 lumaMapSize;
    AX_U32 chromaMapSize;

    lumaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16);

    pLumaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[0], lumaMapSize);

    if (!pLumaVirAddr) {
        printf("CheckMD5:AX_SYS_Mmap luma failed\n");
        goto END;
    } else {
        printf("CheckMD5:AX_SYS_Mmap luma success,pLumaVirAddr=%p,lumaMapSize=%d\n", pLumaVirAddr, lumaMapSize);
    }

    chromaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16) / 2;
    pChromaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[1], chromaMapSize);

    if (!pChromaVirAddr) {
        printf("CheckMD5:AX_SYS_Mmap chroma failed\n");
        goto END;
    } else {
        printf("CheckMD5:AX_SYS_Mmap chroma success,pChromaVirAddr=%p,chromaMapSize=%d\n", pChromaVirAddr, chromaMapSize);
    }

    AX_VOID *p_lu = pLumaVirAddr;
    AX_VOID *p_ch = pChromaVirAddr;
    AX_U32 coded_width = frameInfo->stVFrame.u32Width;
    AX_U32 coded_height = frameInfo->stVFrame.u32Height;
    AX_U32 pic_stride = frameInfo->stVFrame.u32PicStride[0];
    AX_U32 coded_width_ch = frameInfo->stVFrame.u32Width;
    AX_U32 coded_h_ch = frameInfo->stVFrame.u32Height / 2;
    AX_U32 pic_stride_ch = frameInfo->stVFrame.u32PicStride[1];
    MD5SumValidOnly(p_lu, p_ch, coded_width, coded_height, coded_width_ch, coded_h_ch, pic_stride, pic_stride_ch, 0, 0,
                    md5_str);

END:
    if (pLumaVirAddr) {
        s32Ret = AX_SYS_Munmap(pLumaVirAddr, lumaMapSize);

        if (s32Ret) {
            printf("CheckMD5:AX_SYS_Munmap luma failed,s32Ret=0x%x\n", s32Ret);
        } else {
            printf("CheckMD5:AX_SYS_Munmap luma success,pLumaVirAddr=%p,lumaMapSize=%d\n", pLumaVirAddr, lumaMapSize);
        }
    }

    if (pChromaVirAddr) {
        s32Ret = AX_SYS_Munmap(pChromaVirAddr, chromaMapSize);

        if (s32Ret) {
            printf("CheckMD5:AX_SYS_Munmap chroma failed,s32Ret=0x%x\n", s32Ret);
        } else {
            printf("CheckMD5:AX_SYS_Munmap chroma success,pChromaVirAddr=%p,chromaMapSize=%d\n", pChromaVirAddr, chromaMapSize);
        }
    }
}
#endif
int LoadFileToMem(const AX_CHAR *ps8File, AX_U8 **ppu8Mem, AX_S32 *ps32Len)
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

#if 1
void SaveYUV(AX_VIDEO_FRAME_INFO_S *frameInfo, FILE *fp_out)
{
    AX_U32 i;
    AX_VOID *p_lu = NULL;
    AX_VOID *p_ch = NULL;
    AX_U64 lu_buss = 0;
    AX_U64 ch_buss = 0;
    AX_S32 s32Ret = 0;
    AX_VOID *pLumaVirAddr = NULL;
    AX_VOID *pChromaVirAddr = NULL;
    AX_U32 lumaMapSize = 0;
    AX_U32 chromaMapSize = 0;

    if(NULL == frameInfo || NULL == fp_out){
        printf("SaveYUV: Pointer is NULL\n");
        return;
    }
    lumaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16);
    pLumaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[0], lumaMapSize);

    if (!pLumaVirAddr) {
        printf("SaveYUV:AX_SYS_Mmap luma failed\n");
        return;
    } else {
        printf("SaveYUV:AX_SYS_Mmap luma success,pLumaVirAddr=%p,lumaMapSize=%d\n", pLumaVirAddr, lumaMapSize);
    }

    chromaMapSize = frameInfo->stVFrame.u32PicStride[0] * SIZE_ALIGN(frameInfo->stVFrame.u32Height, 16) / 2;
    pChromaVirAddr = AX_SYS_Mmap(frameInfo->stVFrame.u64PhyAddr[1], chromaMapSize);

    if (!pChromaVirAddr) {
        printf("SaveYUV:AX_SYS_Mmap chroma failed\n");
        goto END;
    } else {
        printf("SaveYUV:AX_SYS_Mmap chroma success,pChromaVirAddr=%p,chromaMapSize=%d\n", pChromaVirAddr, chromaMapSize);
    }

    p_lu = pLumaVirAddr;
    lu_buss = frameInfo->stVFrame.u64PhyAddr[0];
    p_ch = pChromaVirAddr;
    ch_buss = frameInfo->stVFrame.u64PhyAddr[1];

    printf("p_lu: %p\n", p_lu);
    printf("lu_buss: 0x%llx\n", lu_buss);
    printf("p_ch: %p\n", p_ch);
    printf("ch_buss: 0x%llx\n", ch_buss);

    AX_U32 coded_width = frameInfo->stVFrame.u32Width;
    AX_U32 coded_height = frameInfo->stVFrame.u32Height;
    AX_U32 pic_stride = frameInfo->stVFrame.u32PicStride[0];
    AX_U32 coded_width_ch = frameInfo->stVFrame.u32Width;
    AX_U32 coded_h_ch = frameInfo->stVFrame.u32Height / 2;
    AX_U32 pic_stride_ch = frameInfo->stVFrame.u32PicStride[1];
    printf("SaveYUV:p_lu: %p, p_ch: %p, coded_width: %u, coded_height: %u, pic_stride: %u, "
           "coded_width_ch: %u, coded_h_ch: %u, pic_stride_ch: %u, pixel_bytes: %u\n",
           p_lu, p_ch, coded_width, coded_height, pic_stride, coded_width_ch, coded_h_ch, pic_stride_ch, 1);


    printf("write Y\n");
    for (i = 0; i < coded_height; i++) {
        fwrite(p_lu, 1, coded_width, fp_out);
        p_lu += pic_stride;
    }

    printf("write UV\n");
    for (i = 0; i < coded_h_ch; i++) {
        fwrite(p_ch, 1, coded_width_ch, fp_out);
        p_ch += pic_stride_ch;
    }


END:
    if (pLumaVirAddr) {
        s32Ret = AX_SYS_Munmap(pLumaVirAddr, lumaMapSize);

        if (s32Ret) {
            printf("SaveYUV:AX_SYS_Munmap luma failed,s32Ret=0x%x\n", s32Ret);
        } else {
            printf("SaveYUV:AX_SYS_Munmap luma success,pLumaVirAddr=%p,lumaMapSize=%d\n", pLumaVirAddr, lumaMapSize);
        }
    }

    if (pChromaVirAddr) {
        s32Ret = AX_SYS_Munmap(pChromaVirAddr, chromaMapSize);

        if (s32Ret) {
            printf("SaveYUV:AX_SYS_Munmap chroma failed,s32Ret=0x%x\n", s32Ret);
        } else {
            printf("SaveYUV:AX_SYS_Munmap chroma success,pChromaVirAddr=%p,chromaMapSize=%d\n", pChromaVirAddr, chromaMapSize);
        }
    }

    printf("write end\n");
}
#else
void SaveYUV(AX_VIDEO_FRAME_INFO_S *frameInfo, FILE *fp_out)
{
    AX_U32 i;

    AX_U8 *p_lu = (AX_U8 *)(AX_U32)frameInfo->stVFrame.u64VirAddr[0];
    AX_U8 *p_lu_buss = (AX_U8 *)(AX_U32)frameInfo->stVFrame.u64PhyAddr[0];
    //AX_U8 *p_ch = (AX_U8 *)frameInfo->stVFrame.u64VirAddr[0] +
    //              frameInfo->stVFrame.u32PicStride[0] * frameInfo->stVFrame.u32Height;
    AX_U8 *p_ch = (AX_U8 *)(AX_U32)frameInfo->stVFrame.u64VirAddr[1];
    AX_U8 *p_ch_buss = (AX_U8 *)(AX_U32)frameInfo->stVFrame.u64PhyAddr[1];

    printf("p_lu: %p\n", p_lu);
    printf("p_lu_buss: %p\n", p_lu_buss);
    printf("p_ch: %p\n", p_ch);
    printf("p_ch_buss: %p\n", p_ch_buss);

    AX_U32 coded_width = frameInfo->stVFrame.u32Width;
    AX_U32 coded_height = frameInfo->stVFrame.u32Height;
    AX_U32 pic_stride = frameInfo->stVFrame.u32PicStride[0];
    AX_U32 coded_width_ch = frameInfo->stVFrame.u32Width;
    AX_U32 coded_h_ch = frameInfo->stVFrame.u32Height / 2;
    AX_U32 pic_stride_ch = frameInfo->stVFrame.u32PicStride[1];
    printf("SaveYUV:p_lu: %p, p_ch: %p, coded_width: %u, coded_height: %u, pic_stride: %u, "
           "coded_width_ch: %u, coded_h_ch: %u, pic_stride_ch: %u, pixel_bytes: %u\n",
           p_lu, p_ch, coded_width, coded_height, pic_stride, coded_width_ch, coded_h_ch, pic_stride_ch, 1);

    if (gGrpAttr.enType == PT_JPEG) {
        printf("write Y\n");
        for (i = 0; i < coded_height; i++) {
            fwrite(p_lu, 1, coded_width, fp_out);
            p_lu += pic_stride;
        }

        printf("write UV\n");
        for (i = 0; i < coded_h_ch; i++) {
            fwrite(p_ch, 1, coded_width_ch, fp_out);
            p_ch += pic_stride_ch;
        }
    } else {
        printf("write Y\n");
        fwrite(p_lu, 1, coded_width * coded_height, fp_out);
        printf("write UV\n");
        fwrite(p_ch, 1, coded_width * coded_height / 2, fp_out);
    }
}
#endif

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

void *VdecRecvThread(void *arg)
{
    SAMPLE_CHN_ARGS_S *GrpArgs = (SAMPLE_CHN_ARGS_S *)arg;
    AX_VDEC_GRP VdecGrp = GrpArgs->VdecGrp;

    FILE *fp_out = NULL;
    if (gWriteFrames) {
        AX_CHAR file_path[128];
        snprintf(file_path, 128, "group%d.yuv", VdecGrp);
        printf("Write YUV to: %s\n", file_path);
        fp_out = fopen(file_path, "w");
        assert(fp_out != NULL);
    }

    AX_S32 decNumber = 0;
    while (!gLoopExit) {
        AX_VIDEO_FRAME_INFO_S frameInfo;
        AX_S32 ret = AX_VDEC_GetFrame(VdecGrp, &frameInfo, gMilliSec);

        if (ret == AX_SUCCESS) {
            if (frameInfo.bEof == AX_FALSE) {
                if (gdestMD5) {
                    AX_CHAR md5_str[33];

                    memset(md5_str, 0, sizeof(md5_str));
                    //CheckMD5(&frameInfo, md5_str);
                    if (strcmp(md5_str, gdestMD5)) {
                        printf("vdecChn: %d, decNumber: %d, md5 not match, dest_md5: %s, md5_str: %s\n", VdecGrp, decNumber, gdestMD5, md5_str);
                        if (gWriteFrames)
                            SaveYUV(&frameInfo, fp_out);
                    }
                } else {
                    if (gWriteFrames)
                        SaveYUV(&frameInfo, fp_out);
                }
                printf("AX_VDEC_ReleaseFrame %lld, BlkId[0] = %u, BlkId[1] = %u\n",
                    frameInfo.stVFrame.u64PhyAddr[0], frameInfo.stVFrame.u32BlkId[0], frameInfo.stVFrame.u32BlkId[1]);
                AX_VDEC_ReleaseFrame(VdecGrp, &frameInfo);
                decNumber++;
            } else {
                //gettimeofday(&gTimeend, NULL);

                printf("end of frame GRP=%d\n", VdecGrp);
                break;
            }
        } else if (ret == AX_ERR_VDEC_BUF_EMPTY) {
            /* no data in unblock mode or timeout mode */
            printf("AX_VDEC_GetFrame: AX_ERR_VDEC_BUF_EMPTY\n");
            usleep(20 * 1000);
        }  else if ((ret == AX_ERR_VDEC_UNEXIST) || (ret == AX_ERR_VDEC_FLOW_END)) {
            //gettimeofday(&gTimeend, NULL);
            printf("AX_VDEC_GetFrame channel unexist or stream end: 0x%x, GRP=%d\n", ret, VdecGrp);
            break;
        } else {
            printf("AX_VDEC_GetFrame error: 0x%x\n", ret);
        }
    }

    if (gWriteFrames && fp_out) {
        fclose(fp_out);
        fp_out = NULL;
    }

    printf("Grp %d RecvThread exit, Total decode %d frames\n", VdecGrp, decNumber);
    return NULL;
}

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
AX_S32 PoolUserPicInit(AX_VIDEO_FRAME_INFO_S *pstUserPic)
{
    AX_S32 ret = 0;
    AX_POOL_CONFIG_T PoolConfig;
    AX_POOL UserPoolId;
    AX_S32 uiWidthAlign = ALIGN_UP(USR_PIC_WIDTH,16);
    AX_S32 uiHeightAlign = ALIGN_UP(USR_PIC_HEIGHT,16);

    memset(&PoolConfig, 0, sizeof(AX_POOL_CONFIG_T));
    PoolConfig.MetaSize= 8 * 1024;
    PoolConfig.BlkSize = (uiWidthAlign * uiHeightAlign) * 3/2;
    PoolConfig.BlkCnt  = 1;
    PoolConfig.CacheMode = POOL_CACHE_MODE_NONCACHE;
    memset(PoolConfig.PartitionName, 0, sizeof(PoolConfig.PartitionName));
    strcpy((AX_CHAR *)PoolConfig.PartitionName, "anonymous");

    UserPoolId = AX_POOL_CreatePool(&PoolConfig);

    if (AX_INVALID_POOLID == UserPoolId){
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
    if (fpUserYUV == NULL){
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
    AX_POOL PoolId;
    AX_U32 num = 25;
    AX_S32 id = 0;

    AX_VDEC_GRP VdGrp = tFunPara->VdecGrp;

    AX_VDEC_DISPLAY_MODE_E eDispMod;
    AX_VDEC_DISPLAY_MODE_E eGetDispMod;
    AX_CHAR *sFile = tFunPara->sFile;
    AX_U8 *sStreamMem = NULL;
    SAMPLE_BSPARSER_T tStreamInfo = {0};

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    AX_VIDEO_FRAME_INFO_S *pstUsrPic = tFunPara->pstUsrPic;
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
    tVdecAttr.u32FrameBufCnt = 8;

    /*GROUP CREATE FOR 16 PATH*/
    sRet = AX_VDEC_CreateGrp(VdGrp, &tVdecAttr);
    if (sRet != AX_SUCCESS) {
        printf("Create instance failed! %x\n", sRet);
        goto end;
    }
    ///////////////////////////////////////////////////////
    /*create thread for get frame*/
    pthread_t recvTid;
    if (pthread_create(&recvTid, NULL, VdecRecvThread, tFunPara) != 0) {
        printf("__read_thread error!\n");
        goto end1;
    }

    FrameSize = AX_VDEC_GetPicBufferSize(1920, 1088, PT_H264); //3655712;
    printf("Get pool mem size is %d\n", FrameSize);
    sRet = FramePoolInit(VdGrp, FrameSize, &PoolId);
    if (sRet != AX_SUCCESS) {
        printf("FramePoolInit failed! Error:%x\n", sRet);
        goto end1;
    }
    /////////////////////////////////////

    /////////////////////////////////////
    /*VDEC DISPALY MODE*/
    eDispMod = VIDEO_DISPLAY_MODE_PLAYBACK;
    AX_VDEC_SetDisplayMode(VdGrp, eDispMod);
    AX_VDEC_GetDisplayMode(VdGrp, &eGetDispMod);
    printf("Vdec Get Display Mode is %d\n", eGetDispMod);

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest)
    {
        AX_VDEC_DisableUserPic(VdGrp);
        AX_VDEC_SetUserPic(VdGrp, pstUsrPic);
    }
    #endif

    sRet = AX_VDEC_StartRecvStream(VdGrp);
    if (sRet != AX_SUCCESS) {
        printf("Create instance failed!\n");
        goto end2;
    }

    tStreamInfo.fInput = fInput;
    tStreamInfo.sSize = sLen;
    AX_S32 decNumber = 0;

    printf("begin to decoder sLen=%d\n", sLen);
    gettimeofday(&gTimebegin, NULL);

    do {
        printf("frame id = %d \n", id);
        id++;
        sReadLen = StreamParserReadFrameH264(&tStreamInfo, sStreamMem, &sSize);

        if (sReadLen) {

            tStrInfo.pu8Addr = sStreamMem;
            tStrInfo.u32Len = sReadLen;  /*stream len*/
            tStrInfo.bEndOfFrame = AX_TRUE;
            tStrInfo.bEndOfStream = AX_FALSE;
            tStrInfo.u64PTS += 1;
            decNumber ++;
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

        #ifdef SAMPLE_VDEC_USERPIC_TEST
        if(userPicTest && id >=11)
        {
            AX_VDEC_StopRecvStream(VdGrp);
            usleep(30000);
            sleep(2);
            AX_VDEC_EnableUserPic(VdGrp, 0);
            sleep(2);
            AX_VDEC_DestroyGrp(VdGrp);
            goto end2;
        }
        #endif
    } while (sReadLen);
    gettimeofday(&gTimeend, NULL);
    AX_U32 total_usec = 1000000 * (gTimeend.tv_sec - gTimebegin.tv_sec) + gTimeend.tv_usec - gTimebegin.tv_usec;
    float total_msec = (float)total_usec / 1000.f;
    float msec_per_frame = total_msec / (float)decNumber;
    printf("msec per frame: %.1f\n", msec_per_frame);
    printf("AVG FPS: %.1f\n", 1000.f / msec_per_frame);

    pthread_join(recvTid, NULL);

    AX_VDEC_StopRecvStream(VdGrp);

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest)
        AX_VDEC_DisableUserPic(VdGrp);
    #endif
    AX_VDEC_DetachPool(VdGrp);

    AX_VDEC_DestroyGrp(VdGrp);

retry:
    sRet = AX_POOL_MarkDestroyPool(PoolId);
    if (sRet) {
        num --;
        if (num) {
            usleep(40000);
            goto retry;
        }
    }
    printf("AX_POOL_MarkDestroyPool successful\n");

    if (sStreamMem) {
        free(sStreamMem);
    }
    if (fInput) {
        fclose(fInput);
    }

    return (void *)AX_SUCCESS;

end2:
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

void *VdecThread(void *arg)
{
    SAMPLE_CHN_ARGS_S *tFunPara = (SAMPLE_CHN_ARGS_S *)arg;
    AX_VDEC_GRP VdecGrp = tFunPara->VdecGrp;
    AX_U32 FrameSize = 0;
    AX_POOL PoolId;
    AX_S32 sRet;

    /*jdec used pool memory,for 1920*1080*/
    FrameSize = AX_VDEC_GetPicBufferSize(1920, 1088, PT_JPEG); //3655712;
    printf("Get pool mem size is %d\n", FrameSize);
    gGrpAttr.u32FrameBufSize = FrameSize;

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
    pthread_t recvTid;
    pthread_create(&recvTid, NULL, VdecRecvThread, (void *)tFunPara);
    gettimeofday(&gTimebegin, NULL);
    while (1) {
        AX_VDEC_STREAM_S stream;
        stream.u32Len = tFunPara->s32StreamLen;
        stream.pu8Addr = tFunPara->pu8StreamMem;
        stream.bEndOfStream = AX_FALSE;
        AX_VDEC_SendStream(VdecGrp, &stream, -1);
        loopDecNumber++;
        if (((gLoopDecodeNumber > 0) && (loopDecNumber >= gLoopDecodeNumber)) || gLoopExit) {
            stream.u32Len = 0;
            stream.pu8Addr = NULL;
            stream.bEndOfStream = AX_TRUE;
            gettimeofday(&gTimeend, NULL);
            AX_U32 total_usec = 1000000 * (gTimeend.tv_sec - gTimebegin.tv_sec) + gTimeend.tv_usec - gTimebegin.tv_usec;
            float total_msec = (float)total_usec / 1000.f;
            float msec_per_frame = total_msec / (float)loopDecNumber;
            printf("msec per frame: %.1f\n", msec_per_frame);
            printf("AVG FPS: %.1f\n", 1000.f / msec_per_frame);
            AX_VDEC_SendStream(VdecGrp, &stream, -1);
            break;
        }
    }

    pthread_join(recvTid, NULL);
    AX_VDEC_StopRecvStream(VdecGrp);
    AX_VDEC_DetachPool(VdecGrp);
    AX_VDEC_DestroyGrp(VdecGrp);
    sRet = AX_POOL_MarkDestroyPool(PoolId);
    if (sRet) {
        printf("AX_POOL_MarkDestroyPool fail!!Error Code:0x%X\n", sRet);
    }

    printf("Chn %d exit\n", VdecGrp);
    return NULL;
}

static void SigInt(int sigNo)
{
    printf("Catch signal %d\n", sigNo);
    gLoopExit = 1;
}


static void PrintHelp()
{
    printf("usage: sample_vdec streamFile <args>\n");
    printf("args:\n");
    printf("  -c:       channel number.             (1-16), default: 1\n");
    printf("  -L:       loop decode number.         (int), default: 1\n");
    printf("  -t:       receive timeout flag.       (-1: block, 0: unblock, > 0: msec), default: -1\n");
    printf("  -w:       write YUV frame to file.    (0: not write, others: write), default: 0\n");
    printf("  -m:       check MD5 every frame.      (char*), default: NULL\n");
    printf("  -M:       video mode.                 (0: stream, 1: frame), default: 1\n");
    printf("  -T:       video type.                 (96: PT_H264, 26: PT_JPEG), default: 26 (PT_JPEG)\n");
    printf("  -u:       flag of userPic test.       (0: not test userPic, 1: test userPic), default: 0\n");
}

int main(int argc, char *argv[])
{
    extern int optind;
    AX_S32 c;
    AX_S32 isExit = 0;
    AX_S32 type = 26;
    AX_S32 s32Ret = -1;

    signal(SIGINT, SigInt);


    while ((c = getopt(argc, argv, "c:L:t:w:m:M:T:u:h")) != -1) {
        isExit = 0;
        switch (c) {
        case 'c':
            gChnNumber = atoi(optarg);
            break;
        case 'L':
            gLoopDecodeNumber = atoi(optarg);
            break;
        case 't':
            gMilliSec = atoi(optarg);
            break;
        case 'w':
            gWriteFrames = atoi(optarg);
            break;
        case 'm':
            gdestMD5 = (AX_CHAR *)optarg;
            break;
        case 'M':
            gGrpAttr.enMode = atoi(optarg);
            break;
        case 'T':
            type = atoi(optarg);
            break;
        case 'u':
            userPicTest = 1;
            break;
        case 'h':
            isExit = 1;
            break;
        default:
            isExit = 1;
            break;
        }
    }
    if (isExit || optind >= argc) {
        PrintHelp();
        exit(0);
    }

    if (gChnNumber < 1 || gChnNumber > AX_VDEC_MAX_GRP_SIZE) {
        printf("Invalid group number\n");
        return -1;
    }
    if (gGrpAttr.enMode != VIDEO_MODE_STREAM && gGrpAttr.enMode != VIDEO_MODE_FRAME) {
        printf("Invalid decode mode\n");
        return -1;
    }

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    AX_VIDEO_FRAME_INFO_S tUserPic[gChnNumber];
    AX_VIDEO_FRAME_INFO_S *pstUserPic[gChnNumber];
    #endif
    AX_SYS_Init();

    s32Ret = AX_POOL_Exit();
    if (s32Ret) {
        printf("VDEC AX_POOL_Exit fail!!Error Code:0x%X\n", s32Ret);
        return -1;
    }

    AX_VDEC_Init();
    printf("main get type %d\n", gGrpAttr.enType);

    AX_CHAR *ps8StreamFile = (AX_CHAR *)argv[optind];
    AX_U8 *pu8StreamMem = NULL;
    AX_S32 s32StreamLen = 0;

    if (type == 96) {
        gGrpAttr.enType = PT_H264;
    } else if (type == 26) {
        gGrpAttr.enType = PT_JPEG;
    } else {
        printf("Invalid decode type, can not supported\n");
        return -1;
    }

    if (gGrpAttr.enType == PT_JPEG) {
        if (LoadFileToMem(ps8StreamFile, &pu8StreamMem, &s32StreamLen)) {
            printf("LoadFileToMem error\n");
            return -1;
        }
    }

    pthread_t chnTids[AX_VDEC_MAX_GRP_SIZE];
    AX_S32 i;

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest) {
        for(i = 0; i< gChnNumber;i++)
        {
            memset(&tUserPic[i], 0 , sizeof(AX_VIDEO_FRAME_INFO_S));
            pstUserPic[i] = &tUserPic[i];
            s32Ret = PoolUserPicInit(pstUserPic[i]);
            if (AX_SUCCESS != s32Ret)
            {
                printf("PoolUserPicInit Failed!\n");
                return -1;
            }

            s32Ret = PoolUserPicDisplay(pstUserPic[i]);
            if(AX_SUCCESS != s32Ret)
            {
                printf("PoolUserPicInit Failed!\n");
                return -1;
            }
        }
    }
    #endif
    if (gGrpAttr.enType == PT_JPEG) {
        for (i = 0; i < gChnNumber; i++) {
            GrpChnArgs[i].VdecGrp = i;
            GrpChnArgs[i].pu8StreamMem = pu8StreamMem;
            GrpChnArgs[i].s32StreamLen = s32StreamLen;
            pthread_create(&chnTids[i], NULL, VdecThread, (void *)&GrpChnArgs[i]);
        }
    } else {
        for (i = 0; i < gChnNumber; i++) {
            GrpArgs[i].VdecGrp = i;
            GrpArgs[i].sFile = ps8StreamFile;
            GrpArgs[i].pstUsrPic = pstUserPic[i];
            pthread_create(&chnTids[i], NULL, H264DecFrameFunc, (void *)&GrpArgs[i]);
        }
    }

    for (i = 0; i < gChnNumber; i++)
        pthread_join(chnTids[i], NULL);

    if (pu8StreamMem) {
        free(pu8StreamMem);
    }

    #ifdef SAMPLE_VDEC_USERPIC_TEST
    if(userPicTest) {
        for(i = 0; i < gChnNumber; i++)
        {
            PoolUserPicDeinit(pstUserPic[i]);
        }
    }
    #endif
    AX_VDEC_DeInit();
    AX_SYS_Deinit();

    return 0;
}
