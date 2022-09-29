#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include "ax_venc_api.h"
//#include "common_isp.h"
#include "ax_global_type.h"
#include "sample_ivps_venc.h"
#include "sample_ivps_object.h"

#define VENC_NUM 1

SAMPLE_IVPS_VENC_CHN_S tVencChnAttr[9] = {
    {
        .nSrcWidth = 1920,
        .nSrcHeight = 1080,
        .nSrcStride = 1920,
        .nDstWidth = 1920,
        .nDstHeight = 1080,
    },
    {
        .nSrcWidth = 1920,
        .nSrcHeight = 1080,
        .nSrcStride = 1920,
        .nDstWidth = 1920,
        .nDstHeight = 1080,
    },
};

#define VENC_MAX_WIDTH 4096
#define VENC_MAX_HEIGHT 4096

typedef enum _SAMPLE_RC
{
    VENC_RC_CBR = 0,
    VENC_RC_VBR,
    VENC_RC_AVBR,
    VENC_RC_QPMAP,
    VENC_RC_FIXQP
} SAMPLE_RC_E;

SAMPLE_RC_E gSampleRc = VENC_RC_CBR;

static AX_S32 gBitRate = 6000; // 6000kbps

AX_S32 IVPS_VencInit(AX_U8 pipe)
{
    AX_S32 s32Ret = -1;
    AX_U32 u32Gop = 30;
    AX_U32 u32FrameRate = 25;
    AX_VENC_MOD_ATTR_S tModAttr;
    tModAttr.enVencType = VENC_VIDEO_ENCODER;
    AX_VENC_Init(&tModAttr);
#if 0
    /* VENC link with IVPS */
    for (int i = 0; i < VENC_NUM; i++)
    {
        ivpsMod.enModId = AX_ID_IVPS;
        ivpsMod.s32GrpId = pipe;
        ivpsMod.s32ChnId = i;
        vencMod.enModId = AX_ID_VENC;
        vencMod.s32GrpId = 0;
        vencMod.s32ChnId = pipe * 3 + i; // max chn
        AX_SYS_Link(&ivpsMod, &vencMod);
    }
#endif
    for (int i = 0; i < VENC_NUM; i++)
    {
        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
        stVencChnAttr.stVencAttr.u32MaxPicWidth = 4096;
        stVencChnAttr.stVencAttr.u32MaxPicHeight = 2160;
        stVencChnAttr.stVencAttr.u32PicWidthSrc = tVencChnAttr[i].nSrcWidth;   /*the picture width*/
        stVencChnAttr.stVencAttr.u32PicHeightSrc = tVencChnAttr[i].nSrcHeight; /*the picture height*/

        stVencChnAttr.stVencAttr.u32CropOffsetX = 0;
        stVencChnAttr.stVencAttr.u32CropOffsetY = 0;
        stVencChnAttr.stVencAttr.u32CropWidth = tVencChnAttr[i].nDstWidth;   /*the encode output picture width*/
        stVencChnAttr.stVencAttr.u32CropHeight = tVencChnAttr[i].nDstHeight; /*the encode output picture height*/

        stVencChnAttr.stVencAttr.enLinkMode = AX_LINK_MODE;
        /* GOP setting */
        stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;

        stVencChnAttr.stVencAttr.enType = PT_H264;
        stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;

        /* stream buffer setting */
        stVencChnAttr.stVencAttr.u32BufSize = 2 * tVencChnAttr[i].nDstWidth * tVencChnAttr[i].nDstHeight;

        if (0 == (i % 2))
        {
            stVencChnAttr.stVencAttr.enProfile = VENC_H264_MAIN_PROFILE;
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
        }
        else
        {
            stVencChnAttr.stVencAttr.enType = PT_H265;
            stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_PROFILE;
            stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;
            stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;
        }

        /* RC GOP setting */
        switch (stVencChnAttr.stVencAttr.enType)
        {
        case PT_H265:
        {
            stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;
            if (gSampleRc == VENC_RC_CBR)
            {
                AX_VENC_H265_CBR_S stH265Cbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH265Cbr.u32Gop = u32Gop;
                stH265Cbr.u32SrcFrameRate = u32FrameRate;  /* input frame rate */
                stH265Cbr.fr32DstFrameRate = u32FrameRate; /* target frame rate */
                stH265Cbr.u32BitRate = 6000;               // kbps
                stH265Cbr.u32MinQp = 22;
                stH265Cbr.u32MaxQp = 51;
                stH265Cbr.u32MinIQp = 22;
                stH265Cbr.u32MaxIQp = 51;
                stH265Cbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH265Cbr, &stH265Cbr, sizeof(AX_VENC_H265_CBR_S));
            }
            else if (gSampleRc == VENC_RC_VBR)
            {
                AX_VENC_H265_VBR_S stH265Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH265Vbr.u32Gop = u32Gop;
                stH265Vbr.u32SrcFrameRate = u32FrameRate;
                stH265Vbr.fr32DstFrameRate = u32FrameRate;
                stH265Vbr.u32MaxBitRate = gBitRate;
                stH265Vbr.u32MinQp = 22;
                stH265Vbr.u32MaxQp = 51;
                stH265Vbr.u32MinIQp = 22;
                stH265Vbr.u32MaxIQp = 51;
                stH265Vbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH265Vbr, &stH265Vbr, sizeof(AX_VENC_H265_VBR_S));
            }
            else if (gSampleRc == VENC_RC_FIXQP)
            {
                AX_VENC_H265_FIXQP_S stH265FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                stH265FixQp.u32Gop = u32Gop;
                stH265FixQp.u32SrcFrameRate = u32FrameRate;
                stH265FixQp.fr32DstFrameRate = u32FrameRate;
                stH265FixQp.u32IQp = 25;
                stH265FixQp.u32PQp = 30;
                stH265FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH265FixQp, &stH265FixQp, sizeof(AX_VENC_H265_FIXQP_S));
            }
        }
        break;

        case PT_H264:
        {
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
            if (gSampleRc == VENC_RC_CBR)
            {
                AX_VENC_H264_CBR_S stH264Cbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH264Cbr.u32Gop = u32Gop;
                stH264Cbr.u32SrcFrameRate = u32FrameRate;  /* input frame rate */
                stH264Cbr.fr32DstFrameRate = u32FrameRate; /* target frame rate */
                if (i == 0)
                    stH264Cbr.u32BitRate = 20000;
                else if (i == 1)
                    stH264Cbr.u32BitRate = 4000;
                else
                    stH264Cbr.u32BitRate = 2000;
                stH264Cbr.u32MinQp = 22;
                stH264Cbr.u32MaxQp = 51;
                stH264Cbr.u32MinIQp = 22;
                stH264Cbr.u32MaxIQp = 51;
                stH264Cbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_S));
            }
            else if (gSampleRc == VENC_RC_VBR)
            {
                AX_VENC_H264_VBR_S stH264Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH264Vbr.u32Gop = u32Gop;
                stH264Vbr.u32SrcFrameRate = u32FrameRate;
                stH264Vbr.fr32DstFrameRate = u32FrameRate;
                stH264Vbr.u32MaxBitRate = gBitRate;
                stH264Vbr.u32MinQp = 22;
                stH264Vbr.u32MaxQp = 51;
                stH264Vbr.u32MinIQp = 22;
                stH264Vbr.u32MaxIQp = 51;
                stH264Vbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH264Vbr, &stH264Vbr, sizeof(AX_VENC_H264_VBR_S));
            }
            else if (gSampleRc == VENC_RC_FIXQP)
            {
                AX_VENC_H264_FIXQP_S stH264FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = u32Gop;
                stH264FixQp.u32SrcFrameRate = u32FrameRate;
                stH264FixQp.fr32DstFrameRate = u32FrameRate;
                stH264FixQp.u32IQp = 25;
                stH264FixQp.u32PQp = 30;
                stH264FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH264FixQp, &stH264FixQp, sizeof(AX_VENC_H264_FIXQP_S));
            }
        }
        break;

        default:
            break;
        }

        s32Ret = AX_VENC_CreateChn(pipe * 3 + i, &stVencChnAttr);
        if (AX_SUCCESS != s32Ret)
        {
            printf("AX_VENC_Create Chn [%d] failed! ret = %#x\n", i, s32Ret);
            return s32Ret;
        }
    }

    printf("%s: %d: exit.\n", __func__, __LINE__);
    return 0;
}

AX_S32 IVPS_VencInitEx(AX_U8 pipe)
{
    AX_S32 s32Ret = -1;
    AX_U32 u32Gop = 30;
    AX_U32 u32FrameRate = 25;

    for (int i = 0; i < VENC_NUM; i++)
    {
        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
        stVencChnAttr.stVencAttr.u32MaxPicWidth = 4096;
        stVencChnAttr.stVencAttr.u32MaxPicHeight = 2160;
        stVencChnAttr.stVencAttr.u32PicWidthSrc = 704;  /*the picture width*/
        stVencChnAttr.stVencAttr.u32PicHeightSrc = 576; /*the picture height*/

        stVencChnAttr.stVencAttr.u32CropOffsetX = 0;
        stVencChnAttr.stVencAttr.u32CropOffsetY = 0;
        stVencChnAttr.stVencAttr.u32CropWidth = 704;  /*the encode output picture width*/
        stVencChnAttr.stVencAttr.u32CropHeight = 576; /*the encode output picture height*/

        stVencChnAttr.stVencAttr.enLinkMode = AX_LINK_MODE;
        /* GOP setting */
        stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;

        stVencChnAttr.stVencAttr.enType = PT_H264;
        stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;

        /* stream buffer setting */
        stVencChnAttr.stVencAttr.u32BufSize = 2 * 704 * 576;

        if (0 == (i % 2))
        {
            stVencChnAttr.stVencAttr.enProfile = VENC_H264_MAIN_PROFILE;
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
        }
        else
        {
            stVencChnAttr.stVencAttr.enType = PT_H265;
            stVencChnAttr.stVencAttr.enProfile = VENC_HEVC_MAIN_PROFILE;
            stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;
            stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;
        }

        /* RC GOP setting */
        switch (stVencChnAttr.stVencAttr.enType)
        {
        case PT_H265:
        {
            stVencChnAttr.stVencAttr.enLevel = VENC_HEVC_LEVEL_6;
            if (gSampleRc == VENC_RC_CBR)
            {
                AX_VENC_H265_CBR_S stH265Cbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH265Cbr.u32Gop = u32Gop;
                stH265Cbr.u32SrcFrameRate = u32FrameRate;  /* input frame rate */
                stH265Cbr.fr32DstFrameRate = u32FrameRate; /* target frame rate */
                stH265Cbr.u32BitRate = 6000;               // kbps
                stH265Cbr.u32MinQp = 22;
                stH265Cbr.u32MaxQp = 51;
                stH265Cbr.u32MinIQp = 22;
                stH265Cbr.u32MaxIQp = 51;
                stH265Cbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH265Cbr, &stH265Cbr, sizeof(AX_VENC_H265_CBR_S));
            }
            else if (gSampleRc == VENC_RC_VBR)
            {
                AX_VENC_H265_VBR_S stH265Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH265Vbr.u32Gop = u32Gop;
                stH265Vbr.u32SrcFrameRate = u32FrameRate;
                stH265Vbr.fr32DstFrameRate = u32FrameRate;
                stH265Vbr.u32MaxBitRate = gBitRate;
                stH265Vbr.u32MinQp = 22;
                stH265Vbr.u32MaxQp = 51;
                stH265Vbr.u32MinIQp = 22;
                stH265Vbr.u32MaxIQp = 51;
                stH265Vbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH265Vbr, &stH265Vbr, sizeof(AX_VENC_H265_VBR_S));
            }
            else if (gSampleRc == VENC_RC_FIXQP)
            {
                AX_VENC_H265_FIXQP_S stH265FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
                stH265FixQp.u32Gop = u32Gop;
                stH265FixQp.u32SrcFrameRate = u32FrameRate;
                stH265FixQp.fr32DstFrameRate = u32FrameRate;
                stH265FixQp.u32IQp = 25;
                stH265FixQp.u32PQp = 30;
                stH265FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH265FixQp, &stH265FixQp, sizeof(AX_VENC_H265_FIXQP_S));
            }
        }
        break;

        case PT_H264:
        {
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
            if (gSampleRc == VENC_RC_CBR)
            {
                AX_VENC_H264_CBR_S stH264Cbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH264Cbr.u32Gop = u32Gop;
                stH264Cbr.u32SrcFrameRate = u32FrameRate;  /* input frame rate */
                stH264Cbr.fr32DstFrameRate = u32FrameRate; /* target frame rate */
                if (i == 0)
                    stH264Cbr.u32BitRate = 20000;
                else if (i == 1)
                    stH264Cbr.u32BitRate = 4000;
                else
                    stH264Cbr.u32BitRate = 2000;
                stH264Cbr.u32MinQp = 22;
                stH264Cbr.u32MaxQp = 51;
                stH264Cbr.u32MinIQp = 22;
                stH264Cbr.u32MaxIQp = 51;
                stH264Cbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH264Cbr, &stH264Cbr, sizeof(AX_VENC_H264_CBR_S));
            }
            else if (gSampleRc == VENC_RC_VBR)
            {
                AX_VENC_H264_VBR_S stH264Vbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stVencChnAttr.stRcAttr.s32FirstFrameStartQp = -1;
                stH264Vbr.u32Gop = u32Gop;
                stH264Vbr.u32SrcFrameRate = u32FrameRate;
                stH264Vbr.fr32DstFrameRate = u32FrameRate;
                stH264Vbr.u32MaxBitRate = gBitRate;
                stH264Vbr.u32MinQp = 22;
                stH264Vbr.u32MaxQp = 51;
                stH264Vbr.u32MinIQp = 22;
                stH264Vbr.u32MaxIQp = 51;
                stH264Vbr.s32IntraQpDelta = -2;
                memcpy(&stVencChnAttr.stRcAttr.stH264Vbr, &stH264Vbr, sizeof(AX_VENC_H264_VBR_S));
            }
            else if (gSampleRc == VENC_RC_FIXQP)
            {
                AX_VENC_H264_FIXQP_S stH264FixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = u32Gop;
                stH264FixQp.u32SrcFrameRate = u32FrameRate;
                stH264FixQp.fr32DstFrameRate = u32FrameRate;
                stH264FixQp.u32IQp = 25;
                stH264FixQp.u32PQp = 30;
                stH264FixQp.u32BQp = 32;
                memcpy(&stVencChnAttr.stRcAttr.stH264FixQp, &stH264FixQp, sizeof(AX_VENC_H264_FIXQP_S));
            }
        }
        break;

        default:
            break;
        }

        s32Ret = AX_VENC_CreateChn(pipe * 3 + i, &stVencChnAttr);
        if (AX_SUCCESS != s32Ret)
        {
            printf("AX_VENC_Create Chn [%d] failed! ret = %#x\n", i, s32Ret);
            return s32Ret;
        }
        AX_VENC_RECV_PIC_PARAM_S pstRecvParam;
        AX_VENC_StartRecvFrame(0, &pstRecvParam);
    }

    printf("%s: %d: exit.\n", __func__, __LINE__);
    return 0;
}

AX_S32 IVPS_VencDeInit(AX_U8 pipe)
{
    /* release venc*/
    /* VENC release */
    for (int i = 0; i < VENC_NUM; i++)
    {
        /* VENC release */
        AX_VENC_StopRecvFrame(i);
        AX_VENC_DestroyChn(i);
    }

#if 0
    for (int i = 0; i < VENC_NUM; i++)
    {
        ivpsMod.enModId = AX_ID_IVPS;
        ivpsMod.s32GrpId = pipe;
        ivpsMod.s32ChnId = i;
        vencMod.enModId = AX_ID_VENC;
        vencMod.s32GrpId = 0;
        vencMod.s32ChnId = pipe * 3 + i;
        AX_SYS_UnLink(&ivpsMod, &vencMod);
    }
#endif
    return 0;
}

static AX_VOID *IVPS_VencThread(AX_VOID *arg)
{

    int retval = -1;
    AX_VENC_RECV_PIC_PARAM_S recv_param = {0};
    AX_VENC_STREAM_S stStream;
    FILE *pStrm = NULL;
    AX_U32 totalGetStream = 0;
    char esName[50];
    AX_S32 VeChn = 0;
    pthread_detach(pthread_self());
    printf("--- %s %d threadid=%lu \n", __func__, __LINE__, pthread_self());

    recv_param.s32RecvPicNum = -1;

    retval = AX_VENC_StartRecvFrame(0, &recv_param);
    if (AX_SUCCESS != retval)
    {
        printf("%s:%d AX_VENC_StartRecvFrame error.", __func__, __LINE__);
        return NULL;
    }

    if (VeChn % 2 == 0)
    {
        sprintf(esName, "Venc_%d.h264", VeChn);
    }
    else
    {
        sprintf(esName, "Venc_%d.hevc", VeChn);
    }
    pStrm = fopen(esName, "wb");

    if (pStrm == NULL)
    {
        printf("Open output file error!\n");
        return NULL;
    }

    memset(&stStream, 0, sizeof(AX_VENC_STREAM_S));
    while (!ThreadLoopStateGet())
    {
        retval = AX_VENC_GetStream(0, &stStream, -1);

        printf("retval:%d len:%d\n", retval, stStream.stPack.u32Len);
        switch (retval)
        {
        case AX_SUCCESS:
        {

            if (retval < 0)
            {
                AX_VENC_ReleaseStream(0, &stStream);
                goto EXIT;
            }

            fwrite(stStream.stPack.pu8Addr, 1, stStream.stPack.u32Len, pStrm);
            fflush(pStrm);
            totalGetStream++;
            if (totalGetStream == 60)
            {
                printf("VENC 60 frame\n");
                totalGetStream = 0;
            }
            retval = AX_VENC_ReleaseStream(0, &stStream);
            if (AX_SUCCESS != retval)
            {
                printf("AX_VENC_ReleaseStream failed 0!\n");
                goto EXIT;
            }
        }
        break;

        default:
            printf("%s %d venc get stream unknow error!\n", __func__, __LINE__);
            // goto EXIT;
            continue;
        }
    }

EXIT:

    printf("NT: venc exit.\n");
    pthread_exit(NULL);
}

AX_S32 IVPS_VencThreadStart(AX_VOID *p)
{
    pthread_t tid = 0;

    if (0 != pthread_create(&tid, NULL, IVPS_VencThread, p))
    {
        return -1;
    }
    pthread_detach(tid);

    return 0;
}
