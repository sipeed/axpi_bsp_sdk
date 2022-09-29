/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_COMMON_VDEC_H_
#define _AX_COMMON_VDEC_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_comm_codec.h"
#include "ax_pool_type.h"
#include "ax_sys_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AX_VDEC_MAX_GRP_SIZE         16
#define AX_VDEC_ALIGN_NUM            8

#define H264_MAX_WIDTH               1920
#define H264_MAX_HEIGHT              1088
#define H264_MIN_HEIGHT              48
#define H264_MIN_WIDTH               48
#define JPEG_MAX_WIDTH               16384
#define JPEG_MAX_HEIGHT              16384
#define JPEG_MIN_HEIGHT              48
#define JPEG_MIN_WIDTH               48




#define ATTRIBUTE __attribute__((aligned (AX_VDEC_ALIGN_NUM)))

typedef AX_S32 AX_VDEC_GRP;

typedef enum {
    AX_ID_VDEC_NULL    = 0x01,

    AX_ID_VDEC_BUTT,
} AX_VDEC_SUB_ID_E;

typedef enum axVIDEO_MODE_E
{
    /* send by stream */
    VIDEO_MODE_STREAM = 0,
    /* send by frame  */
    VIDEO_MODE_FRAME,
    /* One frame supports multiple packets sending. The current frame is
    considered to end when bEndOfFrame is equal to HI_TRUE */
    VIDEO_MODE_COMPAT,
    VIDEO_MODE_BUTT
}AX_VDEC_MODE_E;

typedef enum axVIDEO_OUTPUT_ORDER_E
{
    VIDEO_OUTPUT_ORDER_DISP = 0,
    VIDEO_OUTPUT_ORDER_DEC,
    VIDEO_OUTPUT_ORDER_BUTT
}AX_VDEC_OUTPUT_ORDER_E;

typedef struct axVDEC_ATTR_VIDEO_S
{
    /* RW, Range: [0, 16]; reference frame num. */
    AX_U32       u32RefFrameNum;
    /* RW; specifies whether temporal motion vector predictors
    can be used for inter prediction */
    AX_BOOL      bTemporalMvpEnable;
    /* RW; tmv buffer size(Byte) */
    AX_U32       u32TmvBufSize;
    AX_VDEC_OUTPUT_ORDER_E eOutOrder;
}AX_VDEC_ATTR_VIDEO_S;


typedef struct axVDEC_GRP_ATTR_S
{
    /* RW; video type(H.264/IPEG) to be decoded   */
    AX_PAYLOAD_TYPE_E enType;
    /* RW; send by stream or by frame */
    AX_VDEC_MODE_E   enMode;
    /* RW; frame to next module mode */
    AX_LINK_MODE_E enLinkMode;
    /* RW; effective pic width (pixels) */
    AX_U32         u32PicWidth;
    /* RW; effective pic height (lines) */
    AX_U32         u32PicHeight;
    /* RW; frame height = pic height + padding size(lines) */
    AX_U32         u32FrameHeight;
    /* RW; stream buffer size(Byte) */
    AX_U32         u32StreamBufSize;
    /* RW; frame buffer size(Byte) */
    AX_U32         u32FrameBufSize;
    /* RW; frame buffer number*/
    AX_U32         u32FrameBufCnt;
    /* RW; wait for output queue empty before executing destroy */
    AX_S32         s32DestroyTimeout;
    /* structure with video ( h264/h265) */
    union
    {
        AX_VDEC_ATTR_VIDEO_S stVdecVideoAttr;
    };
}AX_VDEC_GRP_ATTR_S;


typedef struct axVDEC_STREAM_S
{
    /* W; stream len */
    AX_U32  u32Len;
    /* W; time stamp */
    AX_U64  u64PTS;
    /* W; is the end of a frame */
    AX_BOOL bEndOfFrame;
    /* W; is the end of all stream */
    AX_BOOL bEndOfStream;
    /* W; is the current frame displayed. only valid by VIDEO_MODE_FRAME */
    AX_BOOL bDisplay;
    /* W; stream address */
    AX_U8* ATTRIBUTE pu8Addr;
}AX_VDEC_STREAM_S;

typedef struct axVDEC_USERDATA_S
{
    /* R; userdata data phy address */
    AX_U64           u64PhyAddr;
    /* R; userdata data len */
    AX_U32           u32Len;
    AX_BOOL          bValid;
    /* R; userdata data vir address */
    AX_U8* ATTRIBUTE pu8Addr;
}AX_VDEC_USERDATA_S;


typedef struct ax_VDEC_DECODE_ERROR_S
{
    /* R; format error. eg: do not support filed */
    AX_S32 s32FormatErr;
    /* R; picture width or height is larger than chnnel width or height*/
    AX_S32 s32PicSizeErrSet;
    /* R; unsupport the stream specification */
    AX_S32 s32StreamUnsprt;
    /* R; stream package error */
    AX_S32 s32PackErr;
    /* R; protocol num is not enough. eg: slice, pps, sps */
    AX_S32 s32PrtclNumErrSet;
    /* R; refrence num is not enough */
    AX_S32 s32RefErrSet;
    /* R; the buffer size of picture is not enough */
    AX_S32 s32PicBufSizeErrSet;
    /* R; the stream size is too big and and force discard stream */
    AX_S32 s32StreamSizeOver;
    /* R; the stream not released for too long time */
    AX_S32 s32VdecStreamNotRelease;
}AX_VDEC_DECODE_ERROR_S;


typedef struct axVDEC_GRP_STATUS_S
{
    /* R; video type to be decoded */
    AX_PAYLOAD_TYPE_E enType;
    /* R; left stream bytes waiting for decode */
    AX_U32  u32LeftStreamBytes;
    /* R; left frames waiting for decode,only valid for VIDEO_MODE_FRAME*/
    AX_U32  u32LeftStreamFrames;
    /* R; pics waiting for output */
    AX_U32  u32LeftPics;
    /* R; had started recv stream? */
    AX_BOOL bStartRecvStream;
    /* R; how many frames of stream has been received. valid when send by frame. */
    AX_U32  u32RecvStreamFrames;
    /* R; how many frames of stream has been decoded. valid when send by frame. */
    AX_U32  u32DecodeStreamFrames;
    /* R; information about decode error */
    AX_VDEC_DECODE_ERROR_S stVdecDecErr;
}AX_VDEC_GRP_STATUS_S;



typedef enum axVIDEO_DEC_MODE_E
{
    VIDEO_DEC_MODE_IPB = 0,
    VIDEO_DEC_MODE_IP,
    VIDEO_DEC_MODE_I,
    VIDEO_DEC_MODE_BUTT
}AX_VDEC_DEC_MODE_E;


typedef struct axVDEC_PARAM_VIDEO_S
{
    /* RW, Range: [0, 100]; threshold for stream error process,
    0: discard with any error, 100 : keep data with any error */
    AX_S32               s32ErrThreshold;
    /* RW; decode mode , 0: deocde IPB frames,
    1: only decode I frame & P frame , 2: only decode I frame */
    AX_VDEC_DEC_MODE_E     enDecMode;
    /* RW; frames output order ,0: the same with display order,
    1: the same width decoder order */
    AX_VDEC_OUTPUT_ORDER_E enOutputOrder;
    /* RW; compress mode */
    AX_COMPRESS_MODE_E   enCompressMode;
    /* RW; video format */
    AX_VSCAN_FORMAT_E       enVideoFormat;
}AX_VDEC_PARAM_VIDEO_S;


typedef struct axVDEC_PARAM_PICTURE_S
{
   /* RW; out put pixel format */
    AX_IMG_FORMAT_E   enPixelFormat;
   /* RW, Range: [0, 255]; value 0 is transparent.
   [0 ,127]   is deemed to transparent when enPixelFormat is ARGB1555 or ABGR1555
   [128 ,256] is deemed to non-transparent when enPixelFormat is ARGB1555 or ABGR1555 */
    AX_U32            u32Alpha;
}AX_VDEC_PARAM_PICTURE_S;


typedef struct axVDEC_GRP_PARAM_S
{
    /* RW; video type to be decoded   */
    AX_PAYLOAD_TYPE_E enType;
    /* RW, Range: [0, 16]; display frame num */
    AX_U32  u32DisplayFrameNum;
    union
    {
        /* structure with video ( h265/h264) */
        AX_VDEC_PARAM_VIDEO_S stVdecVideoParam;
        /* structure with picture (jpeg/mjpeg )*/
        AX_VDEC_PARAM_PICTURE_S stVdecPictureParam;
    };

}AX_VDEC_GRP_PARAM_S;



typedef struct axH264_PRTCL_PARAM_S
{
    /* RW; max slice num support */
    AX_S32  s32MaxSliceNum;
    /* RW; max sps num support */
    AX_S32  s32MaxSpsNum;
    /* RW; max pps num support */
    AX_S32  s32MaxPpsNum;
}AX_VDEC_H264_PRTCL_PARAM_S;

typedef struct axH265_PRTCL_PARAM_S
{
    /* RW; max slice segmnet num support */
    AX_S32  s32MaxSliceSegmentNum;
    /* RW; max vps num support */
    AX_S32  s32MaxVpsNum;
    /* RW; max sps num support */
    AX_S32  s32MaxSpsNum;
    /* RW; max pps num support */
    AX_S32  s32MaxPpsNum;
}AX_VDEC_H265_PRTCL_PARAM_S;

typedef struct axVDEC_PRTCL_PARAM_S
{
    /* RW; video type to be decoded, only h264 and h265 supported*/
    AX_PAYLOAD_TYPE_E enType;
    union
    {
        /* protocol param structure for h264 */
        AX_VDEC_H264_PRTCL_PARAM_S stH264PrtclParam;
        /* protocol param structure for h265 */
        AX_VDEC_H265_PRTCL_PARAM_S stH265PrtclParam;
    };
}AX_VDEC_PRTCL_PARAM_S;

typedef enum axVDEC_EVNT_E
{
    VDEC_EVNT_STREAM_ERR = 1,
    VDEC_EVNT_UNSUPPORT,
    VDEC_EVNT_OVER_REFTHR,
    VDEC_EVNT_REF_NUM_OVER,
    VDEC_EVNT_SLICE_NUM_OVER,
    VDEC_EVNT_SPS_NUM_OVER,
    VDEC_EVNT_PPS_NUM_OVER,
    VDEC_EVNT_PICBUF_SIZE_ERR,
    VDEC_EVNT_SIZE_OVER,
    VDEC_EVNT_IMG_SIZE_CHANGE,
    VDEC_EVNT_VPS_NUM_OVER,
    VDEC_EVNT_BUTT
} AX_VDEC_EVNT_E;

typedef struct axVDEC_VIDEO_MOD_PARAM_S
{
    AX_U32  u32MaxPicWidth;
    AX_U32  u32MaxPicHeight;
    AX_U32  u32MaxSliceNum;
    AX_U32  u32VdhMsgNum;
    AX_U32  u32VdhBinSize;
    AX_U32  u32VdhExtMemLevel;
} AX_VDEC_VIDEO_MOD_PARAM_S;

typedef enum axVIDEO_DISPLAY_MODE_E
{
    VIDEO_DISPLAY_MODE_PREVIEW  = 0x0,
    VIDEO_DISPLAY_MODE_PLAYBACK = 0x1,

    VIDEO_DISPLAY_MODE_BUTT
} AX_VDEC_DISPLAY_MODE_E;


/************************************************************************************************************************/

#ifndef AX_SUCCESS
#define AX_SUCCESS                          0
#endif
/* invlalid channel ID */
#define AX_ERR_VDEC_INVALID_CHNID     AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x4)
/* at lease one parameter is illegal ,eg, out of range  */
#define AX_ERR_VDEC_ILLEGAL_PARAM     AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0xA)
/* using a NULL pointer */
#define AX_ERR_VDEC_NULL_PTR          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0xB)
/* bad address,  eg. used for copy_from_user & copy_to_user   */
#define AX_ERR_VDEC_BAD_ADDR          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0xC)

/* system is not ready, not properly initialized or loaded driver */
#define AX_ERR_VDEC_SYS_NOTREADY      AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x10)
/*system busy, cannot share use or destroy */
#define AX_ERR_VDEC_BUSY              AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x11)
/* init error or not init at all */
#define AX_ERR_VDEC_NOT_INIT          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x12)
/* configuration required before using a system/device/channel */
#define AX_ERR_VDEC_NOT_CONFIG        AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x13)
/* operation is not (yet) supported on this platform */
#define AX_ERR_VDEC_NOT_SUPPORT       AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x14)
/* operation is not permitted, eg, try to change a static attribute, or start without init */
#define AX_ERR_VDEC_NOT_PERM          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x15)
/* channel already exists */
#define AX_ERR_VDEC_EXIST             AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x16)
/* the channle is not existing  */
#define AX_ERR_VDEC_UNEXIST           AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x17)
/* failure caused by malloc memory from heap*/
#define AX_ERR_VDEC_NOMEM             AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x18)
/* failure caused by borrow buffer from pool*/
#define AX_ERR_VDEC_NOBUF             AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x19)

/* no data in buffer */
#define AX_ERR_VDEC_BUF_EMPTY         AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x20)
/* no buffer for new data */
#define AX_ERR_VDEC_BUF_FULL          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x21)
/* wait timeout failed */
#define AX_ERR_VDEC_TIMED_OUT         AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x27)
/* process termination */
#define AX_ERR_VDEC_FLOW_END          AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x28)
/*for vdec unknown error*/
#define AX_ERR_VDEC_UNKNOWN           AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x29)

/* run error */
#define AX_ERR_VDEC_RUN_ERROR         AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x80)
/* stream error */
#define AX_ERR_VDEC_STRM_ERROR        AX_DEF_ERR(AX_ID_VDEC, AX_ID_VDEC_NULL, 0x81)


#ifdef __cplusplus
}
#endif

#endif
