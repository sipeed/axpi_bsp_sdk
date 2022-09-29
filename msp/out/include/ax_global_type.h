/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_GLOBAL_TYPE_H_
#define _AX_GLOBAL_TYPE_H_
#include "ax_base_type.h"

#define DEF_ALL_MOD_GRP_MAX         (16)
#define DEF_ALL_MOD_CHN_MAX         (128)
#define AX_LINK_DEST_MAXNUM         (4)

/* don't change this */
typedef enum {
    SYS_LOG_MIN         = -1,
    SYS_LOG_EMERGENCY   = 0,
    SYS_LOG_ALERT       = 1,
    SYS_LOG_CRITICAL    = 2,
    SYS_LOG_ERROR       = 3,
    SYS_LOG_WARN        = 4,
    SYS_LOG_NOTICE      = 5,
    SYS_LOG_INFO        = 6,
    SYS_LOG_DEBUG       = 7,
    SYS_LOG_MAX
} AX_LOG_LEVEL_E;

typedef enum {
    SYS_LOG_TARGET_MIN = 0,
    SYS_LOG_TARGET_STDERR = 1,
    SYS_LOG_TARGET_SYSLOG = 2,
    SYS_LOG_TARGET_NULL   = 3,
    SYS_LOG_TARGET_MAX
} AX_LOG_TARGET_E;

typedef enum {
    AX_NONLINK_MODE = 0,
    AX_LINK_MODE = 1,
} AX_LINK_MODE_E;

typedef enum {
    AX_MEMORY_SOURCE_CMM  = 0,
    AX_MEMORY_SOURCE_POOL = 1,
} AX_MEMORY_SOURCE_E;

typedef enum {
    AX_FORMAT_INVALID = -1,
    /* YUV */
    AX_YUV420_PLANAR = 0x0,           /* YYYY... UUUU... VVVV...  */
    AX_YUV420_SEMIPLANAR = 0x1,       /* YYYY... UVUVUV...        */
    AX_YUV420_SEMIPLANAR_VU = 0x2,    /* YYYY... VUVUVU...        */
    AX_YUV422_INTERLEAVED_YUYV = 0x3, /* YUYVYUYV...              */
    AX_YUV422_INTERLEAVED_UYVY = 0x4, /* UYVYUYVY...              */
    AX_YUV444_PACKED = 0x05,          /* YUV YUV YUV ...          */

    /* BAYER RAW */
    AX_FORMAT_BAYER_RAW_8BPP = 0x10,
    AX_FORMAT_BAYER_RAW_10BPP = 0x11,
    AX_FORMAT_BAYER_RAW_12BPP = 0x12,
    AX_FORMAT_BAYER_RAW_14BPP = 0x13,
    AX_FORMAT_BAYER_RAW_16BPP = 0x14,

    /* RGB Format */
    AX_FORMAT_RGB565 = 0x40,
    AX_FORMAT_RGB888 = 0x41,        /**<  RGB888 24bpp */

    AX_FORMAT_KRGB444 = 0x42,       /**<  RGB444 16bpp */
    AX_FORMAT_KRGB555 = 0x43,       /**<  RGB555 16bpp */
    AX_FORMAT_KRGB888 = 0x44,       /**<  RGB888 32bpp */

    AX_FORMAT_ARGB4444 = 0x45,      /**< ARGB4444 */
    AX_FORMAT_ARGB1555 = 0x46,      /**< ARGB1555 */
    AX_FORMAT_ARGB8888 = 0x47,      /**< ARGB8888 */
    AX_FORMAT_ARGB8565 = 0x48,      /**< ARGB8565 */
    AX_FORMAT_RGBA8888 = 0x49,      /**< RGBA8888 */
    AX_FORMAT_RGBA4444 = 0x50,      /**< RGBA4444 */
    AX_FORMAT_RGBA5551 = 0x51,      /**< RGBA5551 */
    AX_FORMAT_BITMAP = 0x52,      /**< BITMAP */
    AX_FORMAT_BGR888 = 0x53,        /**< BGR888 24bpp */

    AX_FORMAT_MAX
} AX_IMG_FORMAT_E;

typedef enum {
    AX_ID_MIN      = 0x00,
    AX_ID_ISP      = 0x01,
    AX_ID_CE       = 0x02,
    AX_ID_VO       = 0x03,
    AX_ID_VDSP     = 0x04,
    AX_ID_EFUSE    = 0x05,
    AX_ID_NPU      = 0x06,
    AX_ID_VENC     = 0x07,
    AX_ID_VDEC     = 0x08,
    AX_ID_JENC     = 0x09,
    AX_ID_JDEC     = 0x0a,
    AX_ID_SYS      = 0x0b,
    AX_ID_AENC     = 0x0c,
    AX_ID_IVPS     = 0x0d,
    AX_ID_MIPI     = 0x0e,
    AX_ID_ADEC     = 0x0f,
    AX_ID_DMA      = 0x10,
    AX_ID_VIN      = 0x11,
    AX_ID_USER     = 0x12,
    AX_ID_IVES     = 0x13,

    /* reserve */
    AX_ID_RESERVE  = 0x20,
    AX_ID_BUTT,
} AX_MOD_ID_E;

typedef enum {
    AX_VSCAN_FORMAT_RASTER = 0,       /* video raster scan mode */
    AX_VSCAN_FORMAT_TILE_64x16,       /* tile cell: 64pixel x 16line */
    AX_VSCAN_FORMAT_TILE_16x8,        /* tile cell: 16pixel x 8line */
    AX_VSCAN_FORMAT_RASTER_DISCRETE,  /* The data bits are aligned in bytes */
    AX_VSCAN_FORMAT_BUTT
} AX_VSCAN_FORMAT_E;

typedef enum {
    AX_COMPRESS_MODE_NONE = 0,   /* no compress */
    AX_COMPRESS_MODE_SEG,        /* compress unit is 256x1 bytes as a segment.*/
    AX_COMPRESS_MODE_TILE,       /* compress unit is a tile.*/
    AX_COMPRESS_MODE_RASTER,     /* compress unit is the whole line.  raw for VI  */
    AX_COMPRESS_MODE_FRAME,      /* compress unit is the whole frame. YUV for VI(3DNR), RGB for TDE(write)/VO(read) */

    AX_COMPRESS_MODE_BUTT
} AX_COMPRESS_MODE_E;

typedef enum {
    AX_NOTIFY_EVENT_SLEEP = 0,
    AX_NOTIFY_EVENT_WAKEUP = 1,
    AX_NOTIFY_EVENT_MAX
} AX_NOTIFY_EVENT_E;

typedef enum {
    AX_SYS_CLK_HIGH_MODE = 0,
    AX_SYS_CLK_HIGH_HOTBALANCE_MODE = 1,
    AX_SYS_CLK_MID_MODE = 2,
    AX_SYS_CLK_MID_HOTBALANCE_MODE = 3,
    AX_SYS_CLK_MAX_MODE = 4,
} AX_SYS_CLK_LEVEL_E;

typedef enum {
    AX_CPU_CLK_ID = 0,
    AX_BUS_CLK_ID = 1,
    AX_NPU_CLK_ID = 2,
    AX_ISP_CLK_ID = 3,
    AX_MM_CLK_ID = 4,
    AX_VPU_CLK_ID = 5,
    AX_SYS_CLK_MAX_ID = 6,
} AX_SYS_CLK_ID_E;


typedef struct axMOD_INFO_S {
    AX_MOD_ID_E enModId;
    AX_S32 s32GrpId;
    AX_S32 s32ChnId;
} AX_MOD_INFO_S;

typedef struct axAX_LINK_DEST_S{
    AX_U32 u32DestNum;
    AX_MOD_INFO_S astDestMod[AX_LINK_DEST_MAXNUM];
}AX_LINK_DEST_S;

typedef struct axVIDEO_FRAME_S {
    AX_U32              u32Width;
    AX_U32              u32Height;
    AX_IMG_FORMAT_E     enImgFormat;
    AX_VSCAN_FORMAT_E   enVscanFormat;
    AX_COMPRESS_MODE_E  enCompressMode;

    AX_U32              u32HeaderStride[3];
    AX_U32              u32PicStride[3];
    AX_U32              u32ExtStride[3];
    AX_U32              u32LeftPadding;

    AX_U64              u64HeaderPhyAddr[3];
    AX_U64              u64HeaderVirAddr[3];
    AX_U64              u64PhyAddr[3];          /* frame physics address*/
    AX_U64              u64VirAddr[3];          /* frame virtual address*/
    AX_U64              u64ExtPhyAddr[3];
    AX_U64              u64ExtVirAddr[3];
    AX_U32              u32BlkId[3];

    AX_S16              s16OffsetTop;           /* top offset of show area */
    AX_S16              s16OffsetBottom;        /* bottom offset of show area */
    AX_S16              s16OffsetLeft;          /* left offset of show area */
    AX_S16              s16OffsetRight;         /* right offset of show area */

    AX_U32              u32TimeRef;
    AX_U64              u64PTS;                 /* Payload TimeStamp */
    AX_U64              u64SeqNum;              /* input frame sequence number */
    AX_U64              u64UserData;            /* Reserved for user, sdk do not use */

    AX_U64              u64PrivateData;         /* SDK reserved, user do not use */
    AX_U32              u32FrameFlag;           /* FRAME_FLAG_E, can be OR operation. */

    AX_U8               u8MetaDataInternal[128];
    AX_U8               u8MetaDataUser[128];
    AX_U32               u32FrameSize;           /* FRAME Size, for isp raw and yuv. */
} AX_VIDEO_FRAME_S;

typedef struct axVIDEO_FRAME_INFO_S {
    AX_VIDEO_FRAME_S    stVFrame;
    AX_U32              u32PoolId;
    AX_MOD_ID_E         enModId;
    AX_BOOL             bEof;
} AX_VIDEO_FRAME_INFO_S;

/* OSD attribute extend */
typedef struct axOSD_BMP_ATTR_S {
    AX_BOOL bEnable;
    AX_U16 u16Alpha;
    AX_U32 u32Zindex; /* Range:[0,31], high value on top */
    AX_IMG_FORMAT_E enRgbFormat;
    AX_U32 u32ColorKey; /* rgb value of transparent color */
    AX_U32 u32BgColorLo; /* min value of background color */
    AX_U32 u32BgColorHi; /* max value of background color */

    AX_U8 *pBitmap; /* pointer to OSD template */
    AX_U64 u64PhyAddr; /* physical address of OSD template */
    AX_U32 u32BmpWidth; /* template width */
    AX_U32 u32BmpHeight; /* template height */

    AX_U32 u32DstXoffset; /* where to overlay, x0 */
    AX_U32 u32DstYoffset; /* where to overlay, y0 */
} AX_OSD_BMP_ATTR_S;

typedef enum {
    AX_ERR_INVALID_MODID        = 0x01, /* invalid module id */
    AX_ERR_INVALID_DEVID        = 0x02, /* invalid device id */
    AX_ERR_INVALID_GRPID        = 0x03, /* invalid group id */
    AX_ERR_INVALID_CHNID        = 0x04, /* invalid channel id */
    AX_ERR_INVALID_PIPEID       = 0x05, /* invalid pipe id */
    AX_ERR_INVALID_STITCHGRPID  = 0x06, /* invalid stitch group id */
    /*reserved*/
    AX_ERR_ILLEGAL_PARAM        = 0x0A, /* at lease one input value is out of range */
    AX_ERR_NULL_PTR             = 0x0B, /* at lease one input pointer is null */
    AX_ERR_BAD_ADDR             = 0x0C, /* at lease one input address is invalid */
    /*reserved*/
    AX_ERR_SYS_NOTREADY         = 0x10, /* a driver is required but not loaded */
    AX_ERR_BUSY                 = 0x11, /* a resource is busy, probably locked by other users */
    AX_ERR_NOT_INIT             = 0x12, /* module is not initialized */
    AX_ERR_NOT_CONFIG           = 0x13, /* module is not configured */
    AX_ERR_NOT_SUPPORT          = 0x14, /* requested function is not supported on this platform */
    AX_ERR_NOT_PERM             = 0x15, /* requested operation is not permitted in this state */
    AX_ERR_EXIST                = 0x16, /* target object already exists */
    AX_ERR_UNEXIST              = 0x17, /* target object does not exist */
    AX_ERR_NOMEM                = 0x18, /* failed to allocate memory from heap */
    AX_ERR_NOBUF                = 0x19, /* failed to borrow buffer from pool */
    AX_ERR_NOT_MATCH            = 0x1A, /* inconsistent parameter configuration between interfaces */
    /*reserved*/
    AX_ERR_BUF_EMPTY            = 0x20, /* buffer contains no data */
    AX_ERR_BUF_FULL             = 0x21, /* buffer contains fresh data */
    AX_ERR_QUEUE_EMPTY          = 0x22, /* failed to read as queue is empty */
    AX_ERR_QUEUE_FULL           = 0x23, /* failed to write as queue is full */
    /*reserved*/
    AX_ERR_TIMED_OUT            = 0x27, /* operation timeout */
    AX_ERR_FLOW_END             = 0x28, /* END signal detected in data stream, processing terminated */
    AX_ERR_UNKNOWN              = 0x29, /* unexpected failure, please contact manufacturer support */
    /*reserved*/

    AX_ERR_BUTT                 = 0x7F, /* maxium code, private error code of all modules
                                        ** must be greater than it */
} AX_ERR_CODE_E;

/******************************************************************************
|----------------------------------------------------------------|
||   FIXED   |   MOD_ID    | SUB_MODULE_ID |   ERR_ID            |
|----------------------------------------------------------------|
|<--8bits----><----8bits---><-----8bits---><------8bits------->|
******************************************************************************/
#define AX_DEF_ERR( module, sub_module, errid) \
    ((AX_S32)( (0x80000000L) | ((module) << 16 ) | ((sub_module)<<8) | (errid) ))

#endif //_GLOBAL_TYPE_H_
