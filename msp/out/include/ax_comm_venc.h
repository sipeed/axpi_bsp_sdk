/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AX_COMM_VENC_H__
#define __AX_COMM_VENC_H__

#include "ax_base_type.h"
#include "ax_global_type.h"

#include "ax_comm_venc_rc.h"
#include "ax_comm_codec.h"


#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


#define MAX_OSD_NUM 6
#define AX_FAILURE (-1)
#define AX_SUCCESS 0
#define VENC_QP_HISGRM_NUM 52

#define MAX_VENC_NUM 64
#define MAX_ROI_NUM 8

#define VENC_MAX_NALU_NUM (8)

#define MAX_VENC_PIC_WIDTH  (5584)
#define MAX_VENC_PIC_HEIGHT (4188)

#define MIN_VENC_PIC_WIDTH  (136)
#define MIN_VENC_PIC_HEIGHT (136)

#define MAX_JENC_PIC_WIDTH  (32768)
#define MAX_JENC_PIC_HEIGHT (32768)

#define MIN_JENC_PIC_WIDTH  (2)
#define MIN_JENC_PIC_HEIGHT (2)

typedef AX_S32 VENC_CHN;

typedef enum
{
    AX_JPEGENC_OK = 0,
    AX_JPEGENC_ERROR = -1
} AX_JPEGENC_RET;

/*the attribute of h264e*/
typedef struct axVENC_ATTR_H264_S
{
    AX_BOOL bRcnRefShareBuf; /* RW; Range:[0, 1]; Whether to enable the Share Buf of Rcn and Ref .*/
    //reserved
} AX_VENC_ATTR_H264_S;

/*the attribute of h265e*/
typedef struct axVENC_ATTR_H265_S
{
    AX_BOOL bRcnRefShareBuf; /* RW; Range:[0, 1]; Whether to enable the Share Buf of Rcn and Ref .*/
    //reserved
} AX_VENC_ATTR_H265_S;

/* the param of the jpege */
typedef struct axVENC_JPEG_PARAM_S
{
    AX_U32 u32Qfactor;                     /* RW; Range:[1,99]; Qfactor value */
    AX_U8  u8YQt[64];                      /* RW; Range:[1, 255]; Y quantization table */
    AX_U8  u8CbCrQt[64];                     /* RW; Range:[1, 255]; CbCr quantization table */
    AX_U32 u32MCUPerECS;                   /* RW; the max MCU number is (picwidth + 15) >> 4 x (picheight +
                                            15) >> 4 x 2]; MCU number of one ECS*/

} AX_VENC_JPEG_PARAM_S;

/* the param of the mjpege */
typedef struct axVENC_MJPEG_PARAM_S
{
    AX_U8 u8YQt[64];                       /* RW; Range:[1, 255]; Y quantization table */
    AX_U8 u8CbQt[64];                      /* RW; Range:[1, 255]; Cb quantization table */
    AX_U8 u8CrQt[64];                      /* RW; Range:[1, 255]; Cr quantization table */
    AX_U32 u32MCUPerECS;                   /* RW; the max MCU number is (picwidth + 15) >> 4 x (picheight +
                                            15) >> 4 x 2]; MCU number of one ECS*/
} AX_VENC_MJPEG_PARAM_S;

typedef struct axSIZE_S
{
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_SIZE_S;

/*the size of array is 2,that is the maximum*/
typedef struct axVENC_MPF_CFG_S
{
    AX_U8   u8LargeThumbNailNum;      /* RW; Range:[0,2]; the large thumbnail pic num of the MPF */
    AX_SIZE_S  astLargeThumbNailSize[2]; /* RW; The resolution of large ThumbNail*/
} AX_VENC_MPF_CFG_S;

typedef enum
{
    VENC_PIC_RECEIVE_SINGLE = 0,
    VENC_PIC_RECEIVE_MULTI,
    VENC_PIC_RECEIVE_BUTT
} AX_VENC_PIC_RECEIVE_MODE_E;

/*the attribute of jpege*/
typedef struct axVENC_ATTR_JPEG_S
{
//    AX_VENC_PIC_RECEIVE_MODE_E  	enReceiveMode;  /*RW; Config the receive mode*/ not support now
} AX_VENC_ATTR_JPEG_S;

/*the attribute of mjpege*/
typedef struct axVENC_ATTR_MJPEG_S
{
//    AX_VENC_PIC_RECEIVE_MODE_E  	enReceiveMode;  /*RW; Config the receive mode*/ not support now
} AX_VENC_ATTR_MJPEG_S;

  /* Profile for initialization */
typedef enum
{
    VENC_HEVC_MAIN_PROFILE = 0,
    VENC_HEVC_MAIN_STILL_PICTURE_PROFILE = 1,
    VENC_HEVC_MAIN_10_PROFILE = 2,
    VENC_HEVC_MAINREXT = 3,
    /* H264 Defination*/
    VENC_H264_BASE_PROFILE = 9,
    VENC_H264_MAIN_PROFILE = 10,
    VENC_H264_HIGH_PROFILE = 11,
    VENC_H264_HIGH_10_PROFILE = 12
} AX_VENC_PROFILE_E;

 /* Level for initialization */
typedef enum
{
    VENC_HEVC_LEVEL_1 = 30,
    VENC_HEVC_LEVEL_2 = 60,
    VENC_HEVC_LEVEL_2_1 = 63,
    VENC_HEVC_LEVEL_3 = 90,
    VENC_HEVC_LEVEL_3_1 = 93,
    VENC_HEVC_LEVEL_4 = 120,
    VENC_HEVC_LEVEL_4_1 = 123,
    VENC_HEVC_LEVEL_5 = 150,
    VENC_HEVC_LEVEL_5_1 = 153,
    VENC_HEVC_LEVEL_5_2 = 156,
    VENC_HEVC_LEVEL_6 = 180,
    VENC_HEVC_LEVEL_6_1 = 183,
    VENC_HEVC_LEVEL_6_2 = 186,

    /* H264 Defination*/
    VENC_H264_LEVEL_1 = 10,
    VENC_H264_LEVEL_1_b = 99,
    VENC_H264_LEVEL_1_1 = 11,
    VENC_H264_LEVEL_1_2 = 12,
    VENC_H264_LEVEL_1_3 = 13,
    VENC_H264_LEVEL_2 = 20,
    VENC_H264_LEVEL_2_1 = 21,
    VENC_H264_LEVEL_2_2 = 22,
    VENC_H264_LEVEL_3 = 30,
    VENC_H264_LEVEL_3_1 = 31,
    VENC_H264_LEVEL_3_2 = 32,
    VENC_H264_LEVEL_4 = 40,
    VENC_H264_LEVEL_4_1 = 41,
    VENC_H264_LEVEL_4_2 = 42,
    VENC_H264_LEVEL_5 = 50,
    VENC_H264_LEVEL_5_1 = 51,
    VENC_H264_LEVEL_5_2 = 52,
    VENC_H264_LEVEL_6 = 60,
    VENC_H264_LEVEL_6_1 = 61,
    VENC_H264_LEVEL_6_2 = 62
} AX_VENC_LEVEL_E;

  /* Tier for initialization */
typedef enum
{
    VENC_HEVC_MAIN_TIER = 0,
    VENC_HEVC_HIGH_TIER = 1,
} AX_VENC_TIER_E;

typedef enum
{
    /* enable h264/hevc encoder */
    VENC_VIDEO_ENCODER = 1,
    /* enable jpeg/mjpeg encoder */
    VENC_JPEG_ENCODER = 2,
    /* enable h264/h265/jpeg/mjpeg encoder */
    VENC_MULTI_ENCODER = 3
} AX_VENC_ENCODER_TYPE_E;

typedef struct axVENC_MOD_ATTR_S
{
    AX_VENC_ENCODER_TYPE_E enVencType;
} AX_VENC_MOD_ATTR_S;

typedef struct axVENC_USERDATA_QUEUE_ATTR_S
{
    AX_U32  u32UserDataBufferCount;
    AX_U32  u32UserDataBufferSize;
} AX_VENC_USERDATA_QUEUE_ATTR_S;
/* the attribute of the Venc*/
typedef struct axVENC_ATTR_S
{
    AX_PAYLOAD_TYPE_E enType; /* RW; the type of payload*/

    /* VENC: RW; Max input image width [8192], in pixel*/
    /* JENC: RW; Max input image width [32768], in pixel*/
    AX_U32 u32MaxPicWidth;
    /* VENC: RW; Max input image height [8192], in pixel*/
    /* JENC: RW; Max input image height [32768], in pixel*/
    AX_U32 u32MaxPicHeight;

    AX_MEMORY_SOURCE_E enMemSource ; /* RW; memory source of stream buffer */
    AX_U32 u32BufSize;      /* RW; stream buffer size*/
    AX_VENC_PROFILE_E enProfile;      /* RW; H.264:   9: Baseline; 10: Main; 11: High; 12: High 10;
                                   H.265:   0: Main; 1: Main Still Picture; 2: Main 10; */
    AX_VENC_LEVEL_E enLevel;        /* RW; HEVC level: 180 = level 6.0 * 30; H264 level: 51 = Level 5.1*/
    AX_VENC_TIER_E  enTier;         /* RW; HEVC: 0: Main tier、 1: High tier*/
    AX_U32 u32MbLinesPerSlice; /* RW; 0: slice contain one frame; [1, align_up(picHeight)/BLK_SIZE]: a slice should contain how many MCU/MB/CTU lines. */

    /* VENC: RW; Range:[136, 5584];width of source image,must be even, in pixel*/
    /* JENC: RW; Range:[2, 32768];width of source image,must be even, in pixel*/
    AX_U32 u32PicWidthSrc;
    /* VENC: RW; Range:[136, 4188];height of source image,must be even, in pixel*/
    /* JENC: RW; Range:[2, 32768];height of source image,must be even, in pixel*/
    AX_U32 u32PicHeightSrc;

    /* VENC: RW; Range:[0, 8192];Output image horizontal cropping offset, must be even, in pixel*/
    /* JENC: RW; Range:[0, 32768];Output image horizontal cropping offset, must be even, in pixel*/
    AX_U32 u32CropOffsetX;
    /* VENC: RW; Range:[0, 8192];Output image vertical cropping offset, must be even, in pixel*/
    /* JENC: RW; Range:[0, 32768];Output image vertical cropping offset, must be even, in pixel*/
    AX_U32 u32CropOffsetY;
    /* VENC: RW; Range:[136, 5584];Width of encoded image, must be even,in pixel*/
    /* JENC: RW; Range:[1, 32768];Width of encoded image, must be even,in pixel*/
    AX_U32 u32CropWidth;
    /* VENC: RW; Range:[136, 4188];Height of encoded image, must be even, in pixel*/
    /* JENC: RW; Range:[1, 32768];Height of encoded image, must be even, in pixel*/
    AX_U32 u32CropHeight;

    AX_LINK_MODE_E enLinkMode;
    AX_U32 u32GdrDuration; /* how many frames it will take to do GDR, 0: disable GDR >0: enable GDR */

    AX_U32 u32VideoRange; /* 0: Narrow Range(NR), Y[16,235], Cb/Cr[16,240]; 1: Full Range(FR), Y/Cb/Cr[0,255] */

    /* whether flush output queue immediately when try to destroy channel,
        -1: wait until output queue empty; 0: not wait (default); >0: wait some time, in millisecond */
    AX_S32 s32StopWaitTime;

    AX_U8 u8InFifoDepth;  /* RW; depth of input fifo */
    AX_U8 u8OutFifoDepth; /* RW; depth of output fifo */
    union {
        AX_VENC_ATTR_H264_S stAttrH264e; /* attributes of H264e */
        AX_VENC_ATTR_H265_S stAttrH265e; /* attributes of H265e */
        AX_VENC_ATTR_MJPEG_S stAttrMjpege;  /* attributes of Mjpeg */
        AX_VENC_ATTR_JPEG_S  stAttrJpege;   /* attributes of jpeg  */
    };
} AX_VENC_ATTR_S;

/* the gop mode */
typedef enum axVENC_GOP_MODE_E
{
    VENC_GOPMODE_NORMALP    = 0,     /* NORMALP */
    VENC_GOPMODE_ONELTR     = 1,     /* ONELTR */
    VENC_GOPMODE_SVC_T      = 2,     /* SVC-T */

    VENC_GOPMODE_BUTT
} AX_VENC_GOP_MODE_E;

/* QPFactor (quality preference) will be used in rate distorition optimization, higher value
       mean lower quality and less bits. Typical suggested range is between 0.3 and 1 */
typedef struct ax_VENC_GOP_PIC_CONFIG_S
{
    AX_S32 s32QpOffset;     /*  QP offset will be added to the QP parameter to set the final QP */
    AX_F32 f32QpFactor;
} AX_VENC_GOP_PIC_CONFIG_S;

/* QPFactor (quality preference) will be used in rate distorition optimization, higher value
       mean lower quality and less bits. Typical suggested range is between 0.3 and 1 */
typedef struct ax_VENC_GOP_PIC_SPECIAL_CONFIG_S
{
    AX_S32 s32QpOffset;     /*  QP offset will be added to the QP parameter to set the final QP */
    AX_F32 f32QpFactor;

    /* interval between two pictures using LTR as reference picture or
       interval between two pictures coded as special frame */
    AX_S32 s32Interval;
} AX_VENC_GOP_PIC_SPECIAL_CONFIG_S;

/* the attribute of the normalp*/
typedef struct  axVENC_GOP_NORMALP_S
{
    AX_VENC_GOP_PIC_CONFIG_S stPicConfig;   /* normal P frame config */
} AX_VENC_GOP_NORMALP_S;

/* the attribute of the one long-term reference frame */
typedef struct axVENC_GOP_ONE_LTR_S
{
    AX_VENC_GOP_PIC_CONFIG_S stPicConfig;   /* normal P frame config */
    AX_VENC_GOP_PIC_SPECIAL_CONFIG_S stPicSpecialConfig; /* one long-term reference frame config */
} AX_VENC_GOP_ONE_LTR_S;

/* the attribute of the one long-term reference frame */
typedef struct axVENC_GOP_SVC_T_S
{
    AX_S8 **s8SvcTCfg;
    AX_U32 u32GopSize;
} AX_VENC_GOP_SVC_T_S;

/* the attribute of the gop*/
typedef struct axVENC_GOP_ATTR_S
{
    AX_VENC_GOP_MODE_E enGopMode;                   /* RW; Encoding GOP type */
    union
    {
        AX_VENC_GOP_NORMALP_S   stNormalP;          /*attributes of normal P*/
        AX_VENC_GOP_ONE_LTR_S   stOneLTR;           /*attributes of one long-term reference frame */
        AX_VENC_GOP_SVC_T_S     stSvcT;             /*attributes of svc-t */
    };

} AX_VENC_GOP_ATTR_S;

/* the attribute of the venc chnl*/
typedef struct axVENC_CHN_ATTR_S
{
    AX_VENC_ATTR_S stVencAttr;                       /*the attribute of video encoder channel */
    AX_VENC_RC_ATTR_S  stRcAttr;                     /*the attribute of rate  ctrl */
    AX_VENC_GOP_ATTR_S stGopAttr;                    /*the attribute of gop */
} AX_VENC_CHN_ATTR_S;

typedef enum
{
    VENC_MOD_CLK_FREQUENCY_624M = 0,    /* VENC hw mod clk frequency 624M*/
    VENC_MOD_CLK_FREQUENCY_500M = 1,    /* VENC hw mod clk frequency 500M*/
    VENC_MOD_CLK_FREQUENCY_312M = 2,    /* VENC hw mod clk frequency 312M*/
    JENC_MOD_CLK_FREQUENCY_624M = 3,    /* JENC hw mod clk frequency 624M*/
    JENC_MOD_CLK_FREQUENCY_500M = 4,    /* JENC hw mod clk frequency 500M*/
    JENC_MOD_CLK_FREQUENCY_312M = 5,    /* JENC hw mod clk frequency 312M*/
    VENC_MOD_CLK_FREQUENCY_BUTT
} AX_VENC_HW_CLK_E;

/* the param of the venc hw mod */
typedef struct axVENC_MOD_PARAM_S
{
    AX_VENC_HW_CLK_E        enVencHwClk;     /* VENC Hw mod clk frequency */
    AX_VENC_HW_CLK_E        enJencHwClk;     /* JENC Hw mod clk frequency */
} AX_VENC_MOD_PARAM_S;

/* the param of vui */
typedef struct axVENC_SPS_VUI_PARAM_S
{
    AX_U32 u32VideoSignalTypePresentFlag;       /* RW; see h264/hevc spec. Range:[0, 1], default 0 */
    AX_U32 u32ColourDescriptionPresentFlag;     /* RW; see h264/hevc spec. Range:[0, 1], default 0 */
    AX_U32 u32ColourPrimaries;                  /* RW; see h264/hevc spec. Range:[0, 255], default 9 */
    AX_U32 u32TransferCharacteristics;          /* RW; see h264/hevc spec. Range:[0, 255], default 0 */
    AX_U32 u32MatrixCoefficients;               /* RW; see h264/hevc spec. Range:[0, 255], default 9 */
} AX_VENC_SPS_VUI_PARAM_S;
/*the nalu type of H264E*/
typedef enum
{
    H264E_NALU_BSLICE = 0,   /*B SLICE types*/
    H264E_NALU_PSLICE = 1,   /*P SLICE types*/
    H264E_NALU_ISLICE = 2,   /*I SLICE types*/
    H264E_NALU_IDRSLICE = 5, /*IDR SLICE types*/
    H264E_NALU_SEI = 6,      /*SEI types*/
    H264E_NALU_SPS = 7,      /*SPS types*/
    H264E_NALU_PPS = 8,      /*PPS types*/

    H264E_NALU_PREFIX_14 = 14,  /*Prefix NAL unit */

    H264E_NALU_BUTT
} AX_H264E_NALU_TYPE_E;

/*the nalu type of H265E*/
typedef enum
{
    H265E_NALU_BSLICE = 0,    /*B SLICE types*/
    H265E_NALU_PSLICE = 1,    /*P SLICE types*/
    H265E_NALU_ISLICE = 2,    /*I SLICE types*/

    H265E_NALU_TSA_R = 3,

    H265E_NALU_IDRSLICE = 19, /*IDR SLICE types*/
    H265E_NALU_VPS = 32,      /*VPS types*/
    H265E_NALU_SPS = 33,      /*SPS types*/
    H265E_NALU_PPS = 34,      /*PPS types*/
    H265E_NALU_SEI = 39,      /*SEI types*/

    H265E_NALU_BUTT
} AX_H265E_NALU_TYPE_E;

/* Picture type for encoding */
typedef enum
{
    VENC_INTRA_FRAME = 0, /* I Frame */
    VENC_PREDICTED_FRAME = 1, /* P Frame */
    VENC_BIDIR_PREDICTED_FRAME = 2, /* B Frame */
    VENC_VIRTUAL_INTRA_FRAME = 3, /* virtual I frame */
    VENC_NOTCODED_FRAME /* Used just as a return value */
} AX_VENC_PICTURE_CODING_TYPE_E;

/*the pack type of JPEGE*/
typedef enum
{
    JPEGE_PACK_ECS = 5,                            /*ECS types*/
    JPEGE_PACK_APP = 6,                            /*APP types*/
    JPEGE_PACK_VDO = 7,                            /*VDO types*/
    JPEGE_PACK_PIC = 8,                            /*PIC types*/
    JPEGE_PACK_DCF = 9,                            /*DCF types*/
    JPEGE_PACK_DCF_PIC = 10,                       /*DCF PIC types*/
    JPEGE_PACK_BUTT
} AX_JPEGE_PACK_TYPE_E;

/*the data type of VENC*/
typedef union
{
    AX_H264E_NALU_TYPE_E enH264EType; /* R; H264E NALU types*/
    AX_JPEGE_PACK_TYPE_E enJPEGEType; /* R; JPEGE pack types*/
    AX_H265E_NALU_TYPE_E enH265EType; /* R; H264E NALU types*/
} AX_VENC_DATA_TYPE_U;

/*the pack info of VENC*/
typedef struct axVENC_NALU_INFO_S
{
    AX_VENC_DATA_TYPE_U unNaluType; /* R; the nalu type*/
    AX_U32 u32NaluOffset;
    AX_U32 u32NaluLength;
} AX_VENC_NALU_INFO_S;

typedef struct axCHN_STREAM_STATUS_S
{
    AX_U32 u32TotalChnNum; /* Range:[0, MAX_VENC_NUM], how many channels have stream. */
    AX_U32 au32ChnIndex[MAX_VENC_NUM]; /* the channel id set of venc channel that has stream */
    AX_PAYLOAD_TYPE_E aenChnCodecType[MAX_VENC_NUM]; /* channel payload type */
} AX_CHN_STREAM_STATUS_S;

/*Defines a stream packet*/
typedef struct axVENC_PACK_S
{
    AX_U64 ulPhyAddr; /* R; the physics address of stream */
    AX_U8 *pu8Addr;       /* R; the virtual address of stream */
    AX_U32 u32Len;        /* R; the length of stream */

    AX_U64 u64PTS;        /* R; PTS */
    AX_U64 u64SeqNum;     /* sequence number of input frame */
    AX_U64 u64UserData;

    AX_PAYLOAD_TYPE_E enType; /* RW; the type of payload*/
    AX_VENC_PICTURE_CODING_TYPE_E enCodingType; /* stream type */
    AX_U32 u32TemporalID; /* svc-t, layer id*/

    AX_U32 u32NaluNum;                 /* R; the stream nalus num */
    AX_VENC_NALU_INFO_S stNaluInfo[VENC_MAX_NALU_NUM]; /* R; the stream nalu Information */
} AX_VENC_PACK_S;

/*Defines the frame type and reference attributes of the H.264 frame skipping reference streams*/
typedef enum
{
    BASE_IDRSLICE = 0,           /* the Idr frame at Base layer*/
    BASE_PSLICE_REFTOIDR,        /* the P frame at Base layer, referenced by other frames at Base layer and reference to Idr frame*/
    BASE_PSLICE_REFBYBASE,       /* the P frame at Base layer, referenced by other frames at Base layer*/
    BASE_PSLICE_REFBYENHANCE,    /* the P frame at Base layer, referenced by other frames at Enhance layer*/
    ENHANCE_PSLICE_REFBYENHANCE, /* the P frame at Enhance layer, referenced by other frames at Enhance layer*/
    ENHANCE_PSLICE_NOTFORREF,    /* the P frame at Enhance layer ,not referenced*/
    ENHANCE_PSLICE_BUTT
} AX_H264E_REF_TYPE_E;

typedef AX_H264E_REF_TYPE_E AX_H265E_REF_TYPE_E;

/*Defines the features of an H.264 stream*/
typedef struct axVENC_STREAM_INFO_H264_S
{
    AX_U32 u32PicBytesNum;     /* R; the coded picture stream byte number */
    AX_U32 u32Inter16x16MbNum; /* R; the inter16x16 macroblock num */
    AX_U32 u32Inter8x8MbNum;   /* R; the inter8x8 macroblock num */
    AX_U32 u32Intra16MbNum;    /* R; the intra16x16 macroblock num */
    AX_U32 u32Intra8MbNum;     /* R; the intra8x8 macroblock num */
    AX_U32 u32Intra4MbNum;     /* R; the inter4x4 macroblock num */

    AX_H264E_REF_TYPE_E enRefType; /* R; Type of encoded frames in advanced frame skipping reference mode*/
    AX_U32 u32UpdateAttrCnt;       /* R; Number of times that channel attributes or parameters (including RC parameters) are set*/
    AX_U32 u32StartQp;             /* R; the start Qp of encoded frames*/
    AX_U32 u32MeanQp;              /* R; the mean Qp of encoded frames*/
    AX_BOOL bPSkip;
} AX_VENC_STREAM_INFO_H264_S;

/*Defines the features of an H.265 stream*/
typedef struct axVENC_STREAM_INFO_H265_S
{
    AX_U32 u32PicBytesNum;     /* R; the coded picture stream byte number */
    AX_U32 u32Inter64x64CuNum; /* R; the inter64x64 cu num  */
    AX_U32 u32Inter32x32CuNum; /* R; the inter32x32 cu num  */
    AX_U32 u32Inter16x16CuNum; /* R; the inter16x16 cu num  */
    AX_U32 u32Inter8x8CuNum;   /* R; the inter8x8   cu num  */
    AX_U32 u32Intra32x32CuNum; /* R; the Intra32x32 cu num  */
    AX_U32 u32Intra16x16CuNum; /* R; the Intra16x16 cu num  */
    AX_U32 u32Intra8x8CuNum;   /* R; the Intra8x8   cu num  */
    AX_U32 u32Intra4x4CuNum;   /* R; the Intra4x4   cu num  */

    AX_H265E_REF_TYPE_E enRefType; /* R; Type of encoded frames in advanced frame skipping reference mode*/
    AX_U32 u32UpdateAttrCnt;       /* R; Number of times that channel attributes or parameters (including RC parameters) are set*/
    AX_U32 u32StartQp;             /* R; the start Qp of encoded frames*/
    AX_U32 u32MeanQp;              /* R; the mean Qp of encoded frames*/
    AX_BOOL bPSkip;
} AX_VENC_STREAM_INFO_H265_S;

/* the sse info*/
typedef struct axVENC_SSE_INFO_S
{
    AX_BOOL bSSEEn; /* RW; Range:[0,1]; Region SSE enable */
    AX_U32 u32SSEVal;  /* R; Region SSE value */
} AX_VENC_SSE_INFO_S;

/* the advance information of the h264e */
typedef struct axVENC_STREAM_ADVANCE_INFO_H264_S
{
    AX_U32 u32ResidualBitNum;               /* R; the residual num */
    AX_U32 u32HeadBitNum;                   /* R; the head bit num */
    AX_U32 u32MadiVal;                      /* R; the madi value */
    AX_U32 u32MadpVal;                      /* R; the madp value */
    AX_F64 f64PSNRVal;                     /* R; the PSNR value */
    AX_U32 u32MseLcuCnt;                    /* R; the lcu cnt of the mse */
    AX_U32 u32MseSum;                       /* R; the sum of the mse */
    AX_VENC_SSE_INFO_S stSSEInfo[8];        /* R; the information of the sse */
    AX_U32 u32QpHstgrm[VENC_QP_HISGRM_NUM]; /* R; the Qp histogram value */
    AX_U32 u32MoveScene16x16Num;            /* R; the 16x16 cu num of the move scene*/
    AX_U32 u32MoveSceneBits;                /* R; the stream bit num of the move scene */
} AX_VENC_STREAM_ADVANCE_INFO_H264_S;

/* the advance information of the h265e */
typedef struct axVENC_STREAM_ADVANCE_INFO_H265_S
{
    AX_U32 u32ResidualBitNum;               /* R; the residual num */
    AX_U32 u32HeadBitNum;                   /* R; the head bit num */
    AX_U32 u32MadiVal;                      /* R; the madi value */
    AX_U32 u32MadpVal;                      /* R; the madp value */
    AX_F64 f64PSNRVal;                     /* R; the PSNR value */
    AX_U32 u32MseLcuCnt;                    /* R; the lcu cnt of the mse */
    AX_U32 u32MseSum;                       /* R; the sum of the mse */
    AX_VENC_SSE_INFO_S stSSEInfo[8];        /* R; the information of the sse */
    AX_U32 u32QpHstgrm[VENC_QP_HISGRM_NUM]; /* R; the Qp histogram value */
    AX_U32 u32MoveScene32x32Num;            /* R; the 32x32 cu num of the move scene*/
    AX_U32 u32MoveSceneBits;                /* R; the stream bit num of the move scene */
} AX_VENC_STREAM_ADVANCE_INFO_H265_S;

/*Defines the features of an jpege stream*/
typedef struct axVENC_STREAM_INFO_JPEG_S
{
    AX_U32 u32PicBytesNum;                      /* R; the coded picture stream byte number */
    AX_U32 u32UpdateAttrCnt;                    /* R; Number of times that channel attributes or parameters (including RC parameters) are set*/
    AX_U32 u32Qfactor;                          /* R; image quality */
} AX_VENC_STREAM_INFO_JPEG_S;

/* the advance information of the Jpege */
typedef struct axVENC_STREAM_ADVANCE_INFO_JPEG_S
{
    // AX_U32 u32Reserved;
} AX_VENC_STREAM_ADVANCE_INFO_JPEG_S;

/*Defines the features of an strAX_AX_*/
typedef struct axVENC_STREAM_S
{
    AX_VENC_PACK_S stPack; /* R; stream pack attribute*/

    union {
        AX_VENC_STREAM_INFO_H264_S stH264Info; /* R; the stream info of h264*/
        AX_VENC_STREAM_INFO_JPEG_S   stJpegInfo;                        /* R; the stream info of jpeg*/
        AX_VENC_STREAM_INFO_H265_S stH265Info; /* R; the stream info of h265*/
    };

    union {
        AX_VENC_STREAM_ADVANCE_INFO_H264_S stAdvanceH264Info; /* R; the stream info of h264*/
        AX_VENC_STREAM_ADVANCE_INFO_JPEG_S   stAdvanceJpegInfo;         /* R; the stream info of jpeg*/
        AX_VENC_STREAM_ADVANCE_INFO_H265_S stAdvanceH265Info; /* R; the stream info of h265*/
    };
} AX_VENC_STREAM_S;

/* the param of receive picture */
typedef struct axVENC_RECV_PIC_PARAM_S
{
    AX_S32 s32RecvPicNum;                         /* RW; Range:[-1, 2147483647]; Number of frames received and encoded by the encoding channel,0 is not supported*/
} AX_VENC_RECV_PIC_PARAM_S;


typedef struct axVENC_STREAM_BUF_INFO_T
{
    AX_U64 u64PhyAddr;
    AX_VOID * pUserAddr;
    AX_U32 u32BufSize;
} AX_VENC_STREAM_BUF_INFO_T;


#if 0
typedef struct axVENC_OSD_AREA_ATTR_S
{
    /* Overlay, support 6 osd regions at present */
    AX_U32 u32OverlayEnables;
    AX_U32 u32Format[MAX_OSD_NUM];
    AX_U32 u32Alpha[MAX_OSD_NUM];
    AX_U32 u32Xoffset[MAX_OSD_NUM];
    AX_U32 u32CropXoffset[MAX_OSD_NUM];
    AX_U32 u32Yoffset[MAX_OSD_NUM];
    AX_U32 u32CropYoffset[MAX_OSD_NUM];
    AX_U32 u32Width[MAX_OSD_NUM];
    AX_U32 u32CropWidth[MAX_OSD_NUM];
    AX_U32 u32Height[MAX_OSD_NUM];
    AX_U32 u32CropHeight[MAX_OSD_NUM];
    AX_U32 u32YStride[MAX_OSD_NUM];
    AX_U32 u32UVStride[MAX_OSD_NUM];
    AX_U32 u32BitmapY[MAX_OSD_NUM];
    AX_U32 u32BitmapU[MAX_OSD_NUM];
    AX_U32 u32BitmapV[MAX_OSD_NUM];
    AX_ADDR ulInputYAddr[MAX_OSD_NUM];
    AX_ADDR ulInputUAddr[MAX_OSD_NUM];
    AX_ADDR ulInputVAddr[MAX_OSD_NUM];
} AX_VENC_OSD_AREA_ATTR_S;
#endif
/* the attribute of the roi */
typedef struct axRECT_S
{
    AX_U32 u32X;
    AX_U32 u32Y;
    AX_U32 u32Width;
    AX_U32 u32Height;
} AX_RECT_S;

typedef struct axVENC_ROI_ATTR_S
{
    AX_U32     u32Index;                     /* RW; Range:[0, 7]; Index of an ROI. The system supports indexes ranging from 0 to 7 */
    AX_BOOL    bEnable;                      /* RW; Range:[0, 1]; Whether to enable this ROI */
    AX_BOOL    bAbsQp;                       /* RW; Range:[0, 1]; QP mode of an ROI. 0: relative QP. 1: absolute QP.*/
    AX_S32     s32RoiQp;                     /* RW; Range: [-51, 51] when bAbsQp==0; [0, 51] when bAbsQp==1; */
    AX_RECT_S  stRoiArea;                    /* RW; Region of an ROI*/
} AX_VENC_ROI_ATTR_S;

/* ROI struct */
typedef struct axVENC_ROI_ATTR_EX_S
{
    AX_U32  u32Index;                     /* RW; Range:[0, 7]; Index of an ROI. The system supports indexes ranging from 0 to 7 */
    AX_BOOL bEnable[3];                   /* RW; Range:[0, 1]; Subscript of array   0: I Frame; 1: P/B Frame; 2: VI Frame; other params are the same. */
    AX_BOOL bAbsQp[3];                    /* RW; Range:[0, 1]; QP mode of an ROI.HI_FALSE: relative QP.HI_TURE: absolute QP.*/
    AX_S32  s32Qp[3];                     /* RW; Range:[-51, 51]; QP value,only relative mode can QP value less than 0. */
    AX_RECT_S  stRect[3];                    /* RW;Region of an ROI*/
} AX_VENC_ROI_ATTR_EX_S;

/* VENC CHANNEL STATUS struct */
typedef struct axVENC_CHN_STATUS_S
{
    AX_U32 u32LeftPics;                     /* R; Number of frames yet to encode (until fifo empty) */
    AX_U32 u32LeftStreamBytes;              /* R; Number of bytes remaining in the bitstream buffer */
    AX_U32 u32LeftStreamFrames;             /* R; Number of frames remaining in the bitstream buffer */
    AX_U32 u32CurPacks;                     /* R; Number of current stream packets. not support now */ 
    AX_U32 u32LeftRecvPics;                 /* R; Number of frames yet to recieve (total number specified at start). not support now */
    AX_U32 u32LeftEncPics;                  /* R; Number of frames yet to encode (total number specified at start). not support now */
    AX_U32 u32Reserved;                     /* R; Reserved */
} AX_VENC_CHN_STATUS_S;

/* the information of the user rc*/
typedef struct axUSER_RC_INFO_S
{
    AX_BOOL bQpMapValid;          /* RW; Range:[0,1]; Indicates whether the QpMap mode is valid for the current frame*/
    AX_BOOL bIPCMMapValid;     /* RW; Range:[0,1]; Indicates whether the IpcmMap mode is valid for the current frame*/
    AX_U32  u32BlkStartQp;        /* RW; Range:[0,51];QP value of the first 16 x 16 block in QpMap mode */
    AX_U64  u64QpMapPhyAddr;      /* RW; Physical address of the QP table in QpMap mode*/
    AX_S8*  pQpMapVirAddr;          /* RW: virtaul address of the qpMap */
    AX_U64  u64IpcmMapPhyAddr;       /* RW; Physical address of the IPCM table in QpMap mode*/
    AX_FRAME_TYPE_E enFrameType;  /* RW; Encoding frame type of the current frame */
    AX_U32  u32RoiMapDeltaSize;   /* size of QpDelta map (per frame) */
} AX_USER_RC_INFO_S;

/* the information of the user frame*/
typedef struct axUSER_FRAME_INFO_S
{
    AX_VIDEO_FRAME_INFO_S stUserFrame;
    AX_USER_RC_INFO_S     stUserRcInfo;
} AX_USER_FRAME_INFO_S;

/* error code */
typedef enum
{
    AX_ID_VENC_INNER    = 0x01,
    AX_ID_VENC_COMMON  = 0x02,
    /* reserved */
} AX_ENC_SUB_ID_E;

typedef enum
{
    AX_ERR_ENC_CREATE_CHAN_ERR          = 0x80, // create encoder channel failed
    AX_ERR_ENC_SET_PRIORITY_FAIL        = 0x81, // set encoder thread priority failed
} AX_ENC_ERR_CODE_E;

/* video encoder error code by inner sub_module*/
#define AX_ERR_VENC_CREATE_CHAN_ERR         AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_CREATE_CHAN_ERR)
#define AX_ERR_VENC_SET_PRIORITY_FAIL       AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ENC_SET_PRIORITY_FAIL)

/* video encoder error code by common sub_module*/
#define AX_ERR_VENC_NULL_PTR                AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NULL_PTR)
#define AX_ERR_VENC_ILLEGAL_PARAM           AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_VENC_BAD_ADDR                AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BAD_ADDR)

#define AX_ERR_VENC_NOT_SUPPORT             AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_SUPPORT)

#define AX_ERR_VENC_NOT_INIT                AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_INIT)

#define AX_ERR_VENC_BUF_EMPTY               AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BUF_EMPTY)
#define AX_ERR_VENC_BUF_FULL                AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_BUF_FULL)
#define AX_ERR_VENC_QUEUE_EMPTY             AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_QUEUE_EMPTY)
#define AX_ERR_VENC_QUEUE_FULL              AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_QUEUE_FULL)

#define AX_ERR_VENC_EXIST                   AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_EXIST)
#define AX_ERR_VENC_UNEXIST                 AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_UNEXIST)

#define AX_ERR_VENC_NOT_PERMIT              AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_PERM)
#define AX_ERR_VENC_UNKNOWN                 AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_UNKNOWN)
#define AX_ERR_VENC_TIMEOUT                 AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_TIMED_OUT)
#define AX_ERR_VENC_FLOW_END                AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_FLOW_END)

#define AX_ERR_VENC_ATTR_NOT_CFG            AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_CONFIG)

#define AX_ERR_VENC_SYS_NOTREADY            AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_SYS_NOTREADY)
#define AX_ERR_VENC_INVALID_CHNID           AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_INVALID_CHNID)
#define AX_ERR_VENC_NOMEM                   AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOMEM)
#define AX_ERR_VENC_NOT_MATCH               AX_DEF_ERR(AX_ID_VENC, AX_ID_VENC_COMMON, AX_ERR_NOT_MATCH)


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of __AX_COMM_ENC_H__ */

