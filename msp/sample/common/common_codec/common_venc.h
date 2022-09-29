#ifndef __COMMON_VENC_H__
#define __COMMON_VENC_H__


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_sys_api.h"
#include "ax_comm_venc.h"


#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


#define MAX_MOSAIC_NUM 12

#define VENC_CHN_NUM_MAX    (64)
#define VENC_CHN_NUM        (2)
#define CHN_NUM             VENC_CHN_NUM
#define INVALID_DEFAULT     (-255)
#define MAX_CU_SIZE         (64)
#define CLIP3(x, y, z)      ((z) < (x) ? (x) : ((z) > (y) ? (y) : (z)))
#define MIN(a, b)           ((a) < (b) ? (a) : (b))

/** Video Codec Format */
typedef enum
{
    VENC_CODEC_NONE=0,
    VENC_CODEC_HEVC=1,
    VENC_CODEC_H264=2,
} SAMPLE_VIDEO_CODEC_FORMAT_E;

typedef enum _SAMPLE_RC
{
    VENC_RC_NONE = -1,
    VENC_RC_CBR = 0,
    VENC_RC_VBR,
    VENC_RC_AVBR,
    VENC_RC_QPMAP,
    VENC_RC_FIXQP,
    VENC_RC_BUTT
} SAMPLE_VENC_RC_E;

/* Structure for command line options */
typedef struct __SAMPLE_VENC_CMD_PARA
{
    AX_COMPRESS_MODE_E  enCompressMode;
    AX_U32 UVheaderSize;
    AX_U32 UVpayloadSize;
    AX_U32 YheaderSize;
    AX_U32 YpayloadSize;
    AX_U32 CropX;
    AX_U32 CropY;

    AX_S8 *input;
    AX_S8 *output;

    AX_S32 outputFrameRate;      /* Output frame rate */
    AX_S32 inputFrameRate;      /* Input frame rate */

    AX_S32 nSrcWidth;
    AX_S32 nSrcHeight;
    AX_S32 picStride[3];

    AX_S32 inputFormat;
    SAMPLE_VIDEO_CODEC_FORMAT_E codecFormat;

    AX_S32 picture_cnt;
    AX_S32 chnNum;          /* encoder channel number */
    AX_S32 syncType;

    AX_S32 enableCabac;      /* [0,1] H.264 entropy coding mode, 0 for CAVLC, 1 for CABAC */

    AX_S32 intraAreaEnable;
    AX_S32 intraAreaTop;
    AX_S32 intraAreaLeft;
    AX_S32 intraAreaBottom;
    AX_S32 intraAreaRight;

    AX_S32 ipcm1AreaTop;
    AX_S32 ipcm1AreaLeft;
    AX_S32 ipcm1AreaBottom;
    AX_S32 ipcm1AreaRight;

    AX_S32 ipcm2AreaTop;
    AX_S32 ipcm2AreaLeft;
    AX_S32 ipcm2AreaBottom;
    AX_S32 ipcm2AreaRight;

    AX_S32 ipcm3AreaTop;
    AX_S32 ipcm3AreaLeft;
    AX_S32 ipcm3AreaBottom;
    AX_S32 ipcm3AreaRight;

    AX_S32 ipcm4AreaTop;
    AX_S32 ipcm4AreaLeft;
    AX_S32 ipcm4AreaBottom;
    AX_S32 ipcm4AreaRight;

    AX_S32 ipcm5AreaTop;
    AX_S32 ipcm5AreaLeft;
    AX_S32 ipcm5AreaBottom;
    AX_S32 ipcm5AreaRight;

    AX_S32 ipcm6AreaTop;
    AX_S32 ipcm6AreaLeft;
    AX_S32 ipcm6AreaBottom;
    AX_S32 ipcm6AreaRight;

    AX_S32 ipcm7AreaTop;
    AX_S32 ipcm7AreaLeft;
    AX_S32 ipcm7AreaBottom;
    AX_S32 ipcm7AreaRight;

    AX_S32 ipcm8AreaTop;
    AX_S32 ipcm8AreaLeft;
    AX_S32 ipcm8AreaBottom;
    AX_S32 ipcm8AreaRight;
    AX_S32 ipcmMapEnable;
    AX_S8 *ipcmMapFile;

    AX_S8 *skipMapFile;
    AX_S32 skipMapEnable;
    AX_S32 skipMapBlockUnit;

    AX_VENC_ROI_ATTR_S roiAttr[MAX_ROI_NUM];

    /* Rate control parameters */
    AX_S32 hrdConformance;
    AX_S32 cpbSize;
    AX_S32 gopLength;   /* IDR interval */

    SAMPLE_VENC_RC_E rcMode;
    AX_S32 qpHdr;
    AX_S32 qpMin;
    AX_S32 qpMax;
    AX_S32 qpMinI;
    AX_S32 qpMaxI;
    AX_S32 bitRate;

    AX_BOOL loopEncode;

    AX_S32 picRc;
    AX_S32 ctbRc;
    AX_S32 blockRCSize;
    AX_U32 rcQpDeltaRange;
    AX_U32 rcBaseMBComplexity;
    AX_S32 picSkip;
    AX_S32 picQpDeltaMin;
    AX_S32 picQpDeltaMax;
    AX_S32 ctbRcRowQpStep;

    AX_F32 tolCtbRcInter;
    AX_F32 tolCtbRcIntra;

    AX_S32 bitrateWindow;
    AX_S32 intraQpDelta;

    AX_S32 disableDeblocking;

    AX_S32 enableSao;

    AX_BOOL enablePerfTest;
    AX_S32 frameNum;

    AX_S32 profile;              /*main profile or main still picture profile*/
    AX_S32 tier;               /*main tier or high tier*/
    AX_S32 level;              /*main profile level*/

    AX_S32 sliceSize;

    AX_S32 rotation;

    AX_BOOL enableCrop;
    AX_S32 cropWidth;
    AX_S32 cropHeight;
    AX_S32 horOffsetSrc;
    AX_S32 verOffsetSrc;

    AX_S32 enableDeblockOverride;
    AX_S32 deblockOverride;

    AX_S32 fieldOrder;
    AX_S32 videoRange;
    AX_S32 sei;
    AX_S8 *userData;
    AX_VENC_GOP_MODE_E gopType;

    AX_S32 gdrDuration;
    AX_U32 roiMapDeltaQpBlockUnit;
    AX_U32 roiMapDeltaQpEnable;
    AX_S8 *roiMapDeltaQpFile;
    AX_S8 *roiMapDeltaQpBinFile;
    AX_S8 *roiMapInfoBinFile;
    AX_S8 *RoimapCuCtrlInfoBinFile;
    AX_S8 *RoimapCuCtrlIndexBinFile;
    AX_U32 RoiCuCtrlVer;
    AX_U32 RoiQpDeltaVer;

    AX_S8 **argv;      /* Command line parameter... */
    AX_S32 argc;

    /* constant chroma control */
    AX_S32 constChromaEn;
    AX_U32 constCb;
    AX_U32 constCr;

    /*for skip frame encoding ctr*/
    AX_S32 skip_frame_enabled_flag;
    AX_S32 skip_frame_poc;

    /* Mosaic parameters */
    AX_U32 mosaicEnables;
    AX_U32 mosXoffset[MAX_MOSAIC_NUM];
    AX_U32 mosYoffset[MAX_MOSAIC_NUM];
    AX_U32 mosWidth[MAX_MOSAIC_NUM];
    AX_U32 mosHeight[MAX_MOSAIC_NUM];
} SAMPLE_VENC_CMD_PARA_T;


typedef struct
{
    AX_U32 nQpmapBlockUnit;

    AX_VENC_QPMAP_QP_TYPE_E enQpmapType;
    AX_VENC_RC_CTBRC_MODE_E enCtbRcType;
    AX_IMG_FORMAT_E eImgFormat;
    AX_LINK_MODE_E enLinkMode;
    SAMPLE_VENC_RC_E enRcType;

    SAMPLE_VENC_CMD_PARA_T *pCmdl;
} SAMPLE_VENC_CHN_PARAM_S;


typedef struct
{
    SAMPLE_VENC_CHN_PARAM_S tDevAttr[VENC_CHN_NUM_MAX];
    AX_U32 nChnNum;
} SAMPLE_VENC_ATTR_S;


typedef struct _SAMPLE_VENC_ENCODE_PARA
{
    AX_BOOL bThreadStart;
    VENC_CHN VeChn;
    AX_S32 width;
    AX_S32 height;
    AX_S32 stride;
    AX_U32 maxCuSize;
    AX_U32 frameSize;
    AX_POOL userPoolId;
    SAMPLE_VENC_CHN_PARAM_S ChnPara;
} SAMPLE_VENC_ENCODE_PARA_T;

typedef struct _SAMPLE_VENC_GETSTREAM_PARA
{
    AX_BOOL bThreadStart;
    VENC_CHN VeChn;
    SAMPLE_VENC_CHN_PARAM_S ChnPara;
} SAMPLE_VENC_GETSTREAM_PARA_T;




AX_S32 SampleVencStart(SAMPLE_VENC_ATTR_S *pChnAttr);
AX_S32 SampleVencStop(SAMPLE_VENC_ATTR_S *pChnAttr);


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /*  */



