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
#include "sample_ivps_jenc.h"
#include "sample_ivps_object.h"

#define VENC_NUM 1

SAMPLE_IVPS_JENC_CHN_S tJencChnAttr[9] = {
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

typedef enum _SAMPLE_JPEGENC_RCMODE
{
    JPEGENC_RC_NONE,
    JPEGENC_CBR = 1,
    JPEGENC_VBR = 2,
    JPEGENC_FIXQP,
    JPEGENC_RC_BUTT
} SAMPLE_JPEGENC_RCMODE;

SAMPLE_JPEGENC_RCMODE jSampleRc = JPEGENC_CBR;

static AX_S32 gBitRate = 6000; // 6000kbps

AX_S32 IVPS_JencInit(AX_U8 pipe)
{
    AX_S32 s32Ret = -1;
    AX_U32 u32Gop = 30;
    AX_U32 u32FrameRate = 25;
    AX_VENC_MOD_ATTR_S tModAttr;
    tModAttr.enVencType = VENC_JPEG_ENCODER;
    AX_MOD_INFO_S dstMod = {0};
    AX_MOD_INFO_S ivpsMod = {0};
    AX_VENC_Init(&tModAttr);

    /* VENC link with IVPS */
    for (int i = 0; i < VENC_NUM; i++)
    {
        ivpsMod.enModId = AX_ID_IVPS;
        ivpsMod.s32GrpId = pipe;
        ivpsMod.s32ChnId = i;
        dstMod.enModId = AX_ID_JENC;
        dstMod.s32GrpId = 0;
        dstMod.s32ChnId = pipe * 3 + i; // max chn
        AX_SYS_Link(&ivpsMod, &dstMod);
    }

    for (int i = 0; i < VENC_NUM; i++)
    {
        AX_VENC_CHN_ATTR_S stVencChnAttr;
        memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));
        stVencChnAttr.stVencAttr.u32MaxPicWidth = 4096;
        stVencChnAttr.stVencAttr.u32MaxPicHeight = 2160;
        stVencChnAttr.stVencAttr.u32PicWidthSrc = tJencChnAttr[i].nSrcWidth;   /*the picture width*/
        stVencChnAttr.stVencAttr.u32PicHeightSrc = tJencChnAttr[i].nSrcHeight; /*the picture height*/

        stVencChnAttr.stVencAttr.enLinkMode = AX_LINK_MODE;
        /* GOP setting */
        stVencChnAttr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;

        // stVencChnAttr.stVencAttr.enType = PT_JPEG;
        if (i % 2 == 0) {
            stVencChnAttr.stVencAttr.enType = PT_JPEG;
        } else {
            stVencChnAttr.stVencAttr.enType = PT_MJPEG;
        }

        stVencChnAttr.stVencAttr.enTier = VENC_HEVC_MAIN_TIER;

        /* stream buffer setting */
        stVencChnAttr.stVencAttr.u32BufSize = 2 * tJencChnAttr[i].nDstWidth * tJencChnAttr[i].nDstHeight;

        /* RC GOP setting */
        switch (stVencChnAttr.stVencAttr.enType)
        {
        case PT_JPEG:
        {
        }
        break;

        case PT_MJPEG:
        {
            stVencChnAttr.stVencAttr.enLevel = VENC_H264_LEVEL_5_2;
            if (jSampleRc == JPEGENC_CBR)
            {
                AX_VENC_MJPEG_CBR_S stMjpegCbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stMjpegCbr.u32StatTime = 1;
                stMjpegCbr.u32SrcFrameRate = u32FrameRate;;
                stMjpegCbr.fr32DstFrameRate = u32FrameRate; 
                stMjpegCbr.u32BitRate = 4000;
                stMjpegCbr.u32MinQp = 22;
                stMjpegCbr.u32MaxQp = 51;
                memcpy(&stVencChnAttr.stRcAttr.stMjpegCbr, &stMjpegCbr, sizeof(AX_VENC_MJPEG_CBR_S));
            }
            else if (jSampleRc == JPEGENC_VBR)
            {
                AX_VENC_MJPEG_VBR_S stMjpegVbr;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stMjpegVbr.u32StatTime = 1;
                stMjpegVbr.u32SrcFrameRate = u32FrameRate;
                stMjpegVbr.fr32DstFrameRate = u32FrameRate; //pCmdl->frameRateNum / pCmdl->frameRateDenom;
                stMjpegVbr.u32MaxBitRate = 4000;
                stMjpegVbr.u32MinQp = 22;
                stMjpegVbr.u32MaxQp = 51;
                memcpy(&stVencChnAttr.stRcAttr.stMjpegVbr, &stMjpegVbr, sizeof(AX_VENC_MJPEG_VBR_S));
            }
            else if (jSampleRc == JPEGENC_FIXQP)
            {
                AX_VENC_MJPEG_FIXQP_S stMjpegFixQp;

                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegFixQp.u32SrcFrameRate = u32FrameRate;
                stMjpegFixQp.fr32DstFrameRate = u32FrameRate; //pCmdl->frameRateNum / pCmdl->frameRateDenom;
                stMjpegFixQp.s32FixedQp = 45;
                memcpy(&stVencChnAttr.stRcAttr.stMjpegFixQp, &stMjpegFixQp, sizeof(AX_VENC_MJPEG_FIXQP_S));
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

AX_S32 IVPS_JencDeInit(AX_U8 pipe)
{
    /* release venc*/
    /* VENC release */
    AX_MOD_INFO_S jencMod = {0};
    AX_MOD_INFO_S ivpsMod = {0};
    for (int i = 0; i < VENC_NUM; i++)
    {
        /* VENC release */
        AX_VENC_DestroyChn(i);
    }
    for (int i = 0; i < VENC_NUM; i++)
    {
        ivpsMod.enModId = AX_ID_IVPS;
        ivpsMod.s32GrpId = pipe;
        ivpsMod.s32ChnId = i;
        jencMod.enModId = AX_ID_JENC;
        jencMod.s32GrpId = 0;
        jencMod.s32ChnId = pipe * 3 + i;
        AX_SYS_UnLink(&ivpsMod, &jencMod);
    }

    return 0;
}

static AX_VOID *IVPS_JencThread(AX_VOID *arg)
{
    if (NULL == arg)
        pthread_exit(NULL);

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
        sprintf(esName, "Jenc_%d.jpg", VeChn);
    }
    else
    {
        sprintf(esName, "Jenc_%d.mjpeg", VeChn);
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
            goto EXIT;
        }
    }

EXIT:

    printf("NT: venc exit.\n");
    pthread_exit(NULL);
}

AX_S32 IVPS_JencThreadStart(AX_VOID *p)
{
    pthread_t tid = 0;

    if (0 != pthread_create(&tid, NULL, IVPS_JencThread, p))
    {
        return -1;
    }
    pthread_detach(tid);

    return 0;
}
