/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_COMM_VENC_RC_H__
#define __AX_COMM_VENC_RC_H__

#include "ax_base_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* For RC, in kbps */
#define VENC_MIN_BITRATE             (3)
#define VENC_MAX_BITRATE             (60*1000)
#define VENC_BITRATE_RATIO          (1000)

/* IDR interval */
#define VENC_MIN_IDR_INTERVAL         (0)
#define VENC_MAX_IDR_INTERVAL         (65536)

/* qp range */
#define VENC_MIN_QP                   (0)
#define VENC_MAX_QP                   (51)

/* frame rate range */
#define VENC_MIN_FRAME_RATE             (1)
#define VENC_MAX_FRAME_RATE             (240)

/* qpmap block unit */
#define VENC_MIN_QPMAP_BLOCK_UNIT       (0)
#define VENC_MAX_H264_QPMAP_BLOCK_UNIT  (2)
#define VENC_MAX_H265_QPMAP_BLOCK_UNIT  (3)

/* qpmap qp type */
#define VENC_MIN_QPMAP_QPTYPE           (0)
#define VENC_MAX_QPMAP_QPTYPE           (3)

/* ctbrc type */
#define VENC_MIN_CTBRC_TYPE             (0)
#define VENC_MAX_CTBRC_TYPE             (3)

#define MAX_EXTRA_BITRATE   1000*1024

/* rc mode */
typedef enum
{
    VENC_RC_MODE_H264CBR = 1,
    VENC_RC_MODE_H264VBR,
    VENC_RC_MODE_H264AVBR,
    VENC_RC_MODE_H264QVBR,
    VENC_RC_MODE_H264CVBR,
    VENC_RC_MODE_H264FIXQP,
    VENC_RC_MODE_H264QPMAP,

    VENC_RC_MODE_MJPEGCBR,
    VENC_RC_MODE_MJPEGVBR,
    VENC_RC_MODE_MJPEGFIXQP,

    VENC_RC_MODE_H265CBR,
    VENC_RC_MODE_H265VBR,
    VENC_RC_MODE_H265AVBR,
    VENC_RC_MODE_H265QVBR,
    VENC_RC_MODE_H265CVBR,
    VENC_RC_MODE_H265FIXQP,
    VENC_RC_MODE_H265QPMAP,

    VENC_RC_MODE_BUTT,

} AX_VENC_RC_MODE_E;

/* qpmap mode*/
typedef enum
{
    VENC_RC_QPMAP_MODE_MEANQP= 0,
    VENC_RC_QPMAP_MODE_MINQP ,
    VENC_RC_QPMAP_MODE_MAXQP,

    VENC_RC_QPMAP_MODE_BUTT,
} AX_VENC_RC_QPMAP_MODE_E;

/* the attribute of h264e fixqp*/
typedef struct axVENC_H264_FIXQP_S
{
    AX_U32      u32Gop;                    /* RW; Range:[1, 65536]; the interval of ISLICE. */
    AX_U32      u32SrcFrameRate;           /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate;         /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32IQp;                    /* RW; Range:[0, 51]; qp of the i frame */
    AX_U32      u32PQp;                    /* RW; Range:[0, 51]; qp of the p frame */
    AX_U32      u32BQp;                    /* RW; Range:[0, 51]; qp of the b frame */
} AX_VENC_H264_FIXQP_S;

typedef enum
{
    VENC_RC_CTBRC_DISABLE = 0,
    VENC_RC_CTBRC_QUALITY,
    VENC_RC_CTBRC_RATE,
    VENC_RC_CTBRC_QUALITY_RATE,
} AX_VENC_RC_CTBRC_MODE_E;

typedef enum
{
    VENC_QPMAP_DISABLE = 0,
    VENC_QPMAP_ENABLE_DELTAQP = 1,          /* qp rang in [-31, 32] */
    VENC_QPMAP_ENABLE_ABSQP = 2,            /* qp range in [0, 51] */
    VENC_QPMAP_ENABLE_IPCM = 3,
} AX_VENC_QPMAP_QP_TYPE_E;

typedef struct axVENC_QPMAP_META_S
{
    AX_VENC_RC_CTBRC_MODE_E enCtbRcMode;    /* RW: Ctb rc mode */
    AX_VENC_QPMAP_QP_TYPE_E enQpmapQpType;  /* RW: The qp type of qpmap */
    AX_U32  u32QpmapBlockUnit;              /* RW: Range:[0,3]; 0: 64x64, 1: 32x32, 2: 16x16, 3: 8x8.(h264 not support 8x8)*/
} AX_VENC_QPMAP_META_S;

/* the attribute of h264e cbr*/
typedef struct axVENC_H264_CBR_S
{
    AX_U32      u32Gop;                    /* RW; Range:[1, 65536]; the interval of I Frame. */
    AX_U32      u32StatTime;               /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;           /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate ;         /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32BitRate;                /* RW; average bitrate(kbps) */

    AX_U32      u32MaxQp;                  /* RW; Range:[0, 51]; the max QP value */
    AX_U32      u32MinQp;                  /* RW: Range:[0, 51]; the min QP value */
    AX_U32      u32MaxIQp;                 /* RW: Range:[0, 51]; the max I qp */
    AX_U32      u32MinIQp;                 /* RW: Range:[0, 51]; the min I qp */
    AX_U32      u32MaxIprop;               /* RW: Range:[1, 100]; the max I P size ratio */
    AX_U32      u32MinIprop;               /* RW: Range:[1, u32MaxIprop]; the min I P size ratio */
    AX_S32      s32IntraQpDelta;           /* RW: Range:[-51, 51]; QP difference between target QP and intra frame QP */
    AX_VENC_QPMAP_META_S stQpmapInfo;      /* RW: Qpmap related info */
} AX_VENC_H264_CBR_S;

/* the attribute of h264e vbr*/
typedef struct axVENC_H264_VBR_S
{
    AX_U32      u32Gop;                     /* RW; Range:[1, 65536]; the interval of ISLICE. */
    AX_U32      u32StatTime;                /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;            /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate ;          /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32MaxBitRate;              /* RW; the max bitrate(kbps) */

    AX_U32      u32MaxQp;                  /* RW; Range:[0, 51]; the max QP value */
    AX_U32      u32MinQp;                  /* RW: Range:[0, 51]; the min QP value */
    AX_U32      u32MaxIQp;                 /* RW: Range:[0, 51]; the max I qp */
    AX_U32      u32MinIQp;                 /* RW: Range:[0, 51]; the min I qp */
    AX_S32      s32IntraQpDelta;           /* RW: Range:[-51, 51]; QP difference between target QP and intra frame QP */
    AX_VENC_QPMAP_META_S stQpmapInfo;      /* RW: Qpmap related info */
} AX_VENC_H264_VBR_S;

/* the attribute of h264e cvbr*/
typedef struct axVENC_H264_CVBR_S
{
    AX_U32      u32Gop;                     /* RW; Range:[1, 65536]; the interval of ISLICE. */
    AX_U32      u32StatTime;                /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;            /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate ;          /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32MaxBitRate;              /* RW; the max bitrate, the unit is kbps */

    AX_U32      u32ShortTermStatTime;       /* RW; Range:[1, 120]; the long-term rate statistic time, the unit is second (s)*/
    AX_U32      u32LongTermStatTime;        /* RW; Range:[1, 1440]; the long-term rate statistic time, the unit is u32LongTermStatTimeUnit*/
    AX_U32      u32LongTermMaxBitrate;     /* RW; Range:[2, 614400];the long-term target max bitrate, can not be larger than u32MaxBitRate,the unit is kbps */
    AX_U32      u32LongTermMinBitrate;     /* RW; Range:[0, 614400];the long-term target min bitrate,  can not be larger than u32LongTermMaxBitrate,the unit is kbps */

} AX_VENC_H264_CVBR_S;


/* the attribute of h264e avbr*/
typedef struct axVENC_H264_AVBR_S
{
    AX_U32      u32Gop;                  /* RW; Range:[1, 65536]; the interval of ISLICE. */
    AX_U32      u32StatTime;             /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;         /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate ;       /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32MaxBitRate;           /* RW; the max bitrate, the unit is kbps */
    
    AX_U32      u32MaxQp;                  /* RW; Range:[0, 51]; the max QP value */
    AX_U32      u32MinQp;                  /* RW: Range:[0, 51]; the min QP value */
    AX_U32      u32MaxIQp;                 /* RW: Rangeb:[0, 51]; the max I qp */
    AX_U32      u32MinIQp;                 /* RW: Range:[0, 51]; the min I qp */
    AX_S32      s32IntraQpDelta;           /* RW: Range:[-51, 51]; QP difference between target QP and intra frame QP */
    AX_VENC_QPMAP_META_S stQpmapInfo;      /* RW: Qpmap related info */
} AX_VENC_H264_AVBR_S;

/* the attribute of h264e qpmap*/
typedef struct axVENC_H264_QPMAP_S
{
    AX_U32      u32Gop;                     /* RW; Range:[1, 65536]; the interval of ISLICE. */
    AX_U32      u32StatTime;                /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;            /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32     fr32DstFrameRate;            /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_VENC_RC_CTBRC_MODE_E enCtbRcMode;    /* RW: CTB QP adjustment mode for Rate Control and Subjective Quality */
    AX_VENC_QPMAP_QP_TYPE_E enQpmapQpType;  /* RW: The qp type of qpmap */
    AX_U32      u32QpmapBlockUnit;          /* Range:[0,2]; 0: 64x64, 1: 32x32, 2: 16x16 */
    AX_U32      u32TargetBitRate;           /* the target bitrate, the unit is kbps */
} AX_VENC_H264_QPMAP_S;


typedef struct axVENC_H264_QVBR_S
{
    AX_U32      u32Gop;                   /* RW; Range:[1, 65536];the interval of ISLICE. */
    AX_U32      u32StatTime;              /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;          /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate ;        /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32TargetBitRate;         /* RW; the target bitrate, the unit is kbps */
} AX_VENC_H264_QVBR_S;


/* the attribute of h265e qpmap*/
typedef struct axVENC_H265_QPMAP_S
{
    AX_U32      u32Gop;                     /* RW; Range:[1, 65536]; the interval of ISLICE. */
    AX_U32      u32StatTime;                /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;            /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate;           /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_VENC_RC_CTBRC_MODE_E enCtbRcMode;    /* RW: CTB QP adjustment mode for Rate Control and Subjective Quality */
    AX_VENC_QPMAP_QP_TYPE_E enQpmapQpType;  /* RW: The qp type of qpmap */
    AX_U32      u32QpmapBlockUnit;          /* Range:[0,3]; 0: 64x64, 1: 32x32, 2: 16x16, 3: 8x8 */
    AX_U32      u32TargetBitRate;           /* the target bitrate, the unit is kbps */
} AX_VENC_H265_QPMAP_S;

typedef struct axVENC_H264_CBR_S   AX_VENC_H265_CBR_S;
typedef struct axVENC_H264_VBR_S   AX_VENC_H265_VBR_S;
typedef struct axVENC_H264_AVBR_S  AX_VENC_H265_AVBR_S;
typedef struct axVENC_H264_FIXQP_S AX_VENC_H265_FIXQP_S;
typedef struct axVENC_H264_QVBR_S  AX_VENC_H265_QVBR_S;
typedef struct axVENC_H264_CVBR_S  AX_VENC_H265_CVBR_S;


/* the attribute of mjpege fixqp*/
typedef struct axVENC_MJPEG_FIXQP_S
{
    AX_U32      u32SrcFrameRate;          /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate;         /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_S32      s32FixedQp;               /* Range:[-1, 51]; Fixed qp for every frame.
                                                            -1 : disable fixed qp mode.
                                                            0~51 : value of fixed qp. */
} AX_VENC_MJPEG_FIXQP_S;

/* the attribute of mjpege cbr*/
typedef struct axVENC_MJPEG_CBR_S
{
    AX_U32      u32StatTime;              /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;          /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate;         /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32BitRate;               /* RW; average bitrate ,the unit is kbps*/
    AX_U32      u32MaxQp;                 /* RW; Range:[0, 51]; the max Qfactor value*/
    AX_U32      u32MinQp;                 /* RW; Range:[0, 51]; the min Qfactor value ,can not be larger than u32MaxQfactor */
} AX_VENC_MJPEG_CBR_S;

/* the attribute of mjpege vbr*/
typedef struct axVENC_MJPEG_VBR_S
{
    AX_U32      u32StatTime;              /* RW; Range:[1, 60]; the rate statistic time, the unit is senconds(s) */
    AX_U32      u32SrcFrameRate;          /* RW; Range:[1, 240]; the input frame rate of the venc chnnel */
    AX_U32      fr32DstFrameRate;         /* RW; Range:[0.015625, 240]; the target frame rate of the venc chnnel,can not be larger than u32SrcFrameRate */
    AX_U32      u32MaxBitRate;            /* RW; the max bitrate ,the unit is kbps*/
    AX_U32      u32MaxQp;                 /* RW; Range:[0, 51]; max image quailty allowed */
    AX_U32      u32MinQp;                 /* RW; Range:[0, 51]; min image quality allowed ,can not be larger than u32MaxQfactor*/
} AX_VENC_MJPEG_VBR_S;

/* the attribute of rc*/
typedef struct axVENC_RC_ATTR_S
{
    AX_VENC_RC_MODE_E enRcMode;               /* RW; the type of rc*/
    AX_S32 s32FirstFrameStartQp;              /* RW; Range[-1, 51]; Start QP value of the first frame
                                               **    -1: Encoder calculates initial QP.
                                               */
    union
    {
        AX_VENC_H264_CBR_S    stH264Cbr;
        AX_VENC_H264_VBR_S    stH264Vbr;
        AX_VENC_H264_AVBR_S   stH264AVbr;
        AX_VENC_H264_QVBR_S   stH264QVbr;
        AX_VENC_H264_CVBR_S   stH264CVbr;
        AX_VENC_H264_FIXQP_S  stH264FixQp;
        AX_VENC_H264_QPMAP_S  stH264QpMap;

        AX_VENC_H265_CBR_S    stH265Cbr;
        AX_VENC_H265_VBR_S    stH265Vbr;
        AX_VENC_H265_AVBR_S   stH265AVbr;
        AX_VENC_H265_QVBR_S   stH265QVbr;
        AX_VENC_H265_CVBR_S   stH265CVbr;
        AX_VENC_H265_FIXQP_S  stH265FixQp;
        AX_VENC_H265_QPMAP_S  stH265QpMap;

        AX_VENC_MJPEG_CBR_S   stMjpegCbr;
        AX_VENC_MJPEG_VBR_S   stMjpegVbr;
        AX_VENC_MJPEG_FIXQP_S stMjpegFixQp;
    };
} AX_VENC_RC_ATTR_S;

typedef struct axVENC_SCENE_CHANGE_DETECT_S
{
    AX_BOOL bDetectSceneChange;         /* RW; Range:[0, 1]; enable detect scene change.*/
    AX_BOOL bAdaptiveInsertIDRFrame;    /* RW; Range:[0, 1]; enable a daptive insertIDR frame.*/
} AX_VENC_SCENE_CHANGE_DETECT_S;

/* The param of rc*/
typedef struct axVENC_RC_PARAM_S
{
    AX_U32 u32RowQpDelta;                   /* RW; Range:[0, 10];the start QP value of each macroblock row relative to the start QP value */
    AX_S32 s32FirstFrameStartQp;            /* RW; Range:[-1, 51];Start QP value of the first frame*/
    AX_VENC_SCENE_CHANGE_DETECT_S stSceneChangeDetect;/* RW;*/
    AX_VENC_RC_MODE_E enRcMode;               /* RW; the type of rc*/
    union
    {
        AX_VENC_H264_CBR_S    stH264Cbr;
        AX_VENC_H264_VBR_S    stH264Vbr;
        AX_VENC_H264_AVBR_S   stH264AVbr;
        AX_VENC_H264_QVBR_S   stH264QVbr;
        AX_VENC_H264_CVBR_S   stH264CVbr;
        AX_VENC_H264_FIXQP_S  stH264FixQp;
        AX_VENC_H264_QPMAP_S  stH264QpMap;

        AX_VENC_H265_CBR_S    stH265Cbr;
        AX_VENC_H265_VBR_S    stH265Vbr;
        AX_VENC_H265_AVBR_S   stH265AVbr;
        AX_VENC_H265_QVBR_S   stH265QVbr;
        AX_VENC_H265_CVBR_S   stH265CVbr;
        AX_VENC_H265_FIXQP_S  stH265FixQp;
        AX_VENC_H265_QPMAP_S  stH265QpMap;

        AX_VENC_MJPEG_CBR_S   stMjpegCbr;
        AX_VENC_MJPEG_VBR_S   stMjpegVbr;
        AX_VENC_MJPEG_FIXQP_S stMjpegFixQp;
    };
} AX_VENC_RC_PARAM_S;

/* the frame lost mode*/
typedef enum
{
    DROPFRM_NORMAL=0,                               /*normal mode*/
    DROPFRM_PSKIP,                                  /*pskip*/
    DROPFRM_BUTT,
} AX_VENC_DROPFRAME_MODE_E;

/* The param of the rate jam mode*/
typedef struct axVENC_RATE_JAM_S
{
    AX_BOOL                     bDropFrmEn;             /* RW; Range:[0,1];Indicates whether to enable rate jam stragety */
    AX_U32                      u32DropFrmThrBps;       /* RW; Range:[64k, 163840k];the instant bit rate threshold */
    AX_VENC_DROPFRAME_MODE_E    enDropFrmMode;          /* RW; drop frame mode */
    AX_U32                      u32MaxApplyCount;       /* RW; Range:[0,65535]; the max frame number of apply this startegy */
} AX_VENC_RATE_JAM_CFG_S;

/* the rc priority*/
typedef enum
{
    VENC_RC_PRIORITY_BITRATE_FIRST = 1,           /* bitrate first */
    VENC_RC_PRIORITY_FRAMEBITS_FIRST,             /* framebits first*/

    VENC_RC_PRIORITY_BUTT,
} AX_VENC_RC_PRIORITY_E;

/* the config of the superframe */
typedef struct axVENC_SUPERFRAME_CFG_S
{
    AX_BOOL                 bStrategyEn;                /* RW: super frame strategy enable */
    AX_U32                  u32MaxReEncodeTimes;        /* RW; Range:[0, 65535]; Indicate the max re-encode times */
    AX_U32                  u32SuperIFrmBitsThr;        /* RW; Range:[0, 4294967295];Indicate the threshold of the super I frame for enabling the super frame processing mode */
    AX_U32                  u32SuperPFrmBitsThr;        /* RW; Range:[0, 4294967295];Indicate the threshold of the super P frame for enabling the super frame processing mode */
    AX_U32                  u32SuperBFrmBitsThr;        /* RW; Range:[0, 4294967295];Indicate the threshold of the super B frame for enabling the super frame processing mode */
} AX_VENC_SUPERFRAME_CFG_S;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __AX_COMM_RC_H__ */

