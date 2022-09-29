/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_VO_H_
#define _AX_VO_H_

#include "ax_global_type.h"
#include "ax_sys_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AX_VO_WIDTH_MAX        4096
#define AX_VO_HEIGHT_MAX       4096
#define AX_VO_DEV_MAX          4
#define AX_VO_LAYER_MAX        31
#define AX_VO_BIND_LAYER_MAX   2
#define AX_VO_CHN_MAX          16

#define AX_GRAPHIC_LAYER_MAX   2

#define AX_ID_VO_PUB_SMOD      0x8A
#define AX_ERR_VO_INVALID_DEVID               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_INVALID_DEVID)
#define AX_ERR_VO_INVALID_CHNID               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_INVALID_CHNID)
#define AX_ERR_VO_ILLEGAL_PARAM               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_ILLEGAL_PARAM)
#define AX_ERR_VO_NULL_PTR                    AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_NULL_PTR)
#define AX_ERR_VO_NOT_PERM                    AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_NOT_PERM)
#define AX_ERR_VO_NODEV                       AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_UNEXIST)
#define AX_ERR_VO_NOMEM                       AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, AX_ERR_NOMEM)

#define AX_ERR_VO_DEV_OP_FAIL                 AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x80)
#define AX_ERR_VO_LAYER_OP_FAIL               AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x81)
#define AX_ERR_VO_CHN_OP_FAIL                 AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x82)
#define AX_ERR_VO_INVALID_LAYERID             AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x83)
#define AX_ERR_VO_OPEN_FILE_FAIL              AX_DEF_ERR(AX_ID_VO, AX_ID_VO_PUB_SMOD, 0x84)

typedef AX_U32 VO_DEV;
typedef AX_U32 VO_LAYER;
typedef AX_U32 GRAPHIC_LAYER;
typedef AX_U32 VO_CHN;
typedef AX_U32 VO_WBC;

typedef enum {
    VO_INTF_VIRT = 0,
    VO_INTF_DPI,
    VO_INTF_DSI0,
    VO_INTF_DSI1,
    VO_INTF_2DSI,
    VO_INTF_BT656,
    VO_INTF_BT1120,
    VO_INTF_BUTT
} VO_INTF_TYPE_E;

typedef enum {
    VO_OUTPUT_USER = 1,
    VO_OUTPUT_480P60,
    VO_OUTPUT_576P50,
    VO_OUTPUT_720P24,
    VO_OUTPUT_720P25,
    VO_OUTPUT_720P30,
    VO_OUTPUT_720P50,
    VO_OUTPUT_720P60,
    VO_OUTPUT_800P60,
    VO_OUTPUT_1080P24,
    VO_OUTPUT_1080P25,
    VO_OUTPUT_1080P30,
    VO_OUTPUT_1080P50,
    VO_OUTPUT_1080P60,
    VO_OUTPUT_3840x2160_24,
    VO_OUTPUT_3840x2160_25,
    VO_OUTPUT_3840x2160_30,
    VO_OUTPUT_3840x2160_50,
    VO_OUTPUT_3840x2160_60,
    VO_OUTPUT_4096x2160_24,
    VO_OUTPUT_4096x2160_25,
    VO_OUTPUT_4096x2160_30,
    VO_OUTPUT_4096x2160_50,
    VO_OUTPUT_4096x2160_60,
    VO_OUTPUT_BUTT,
} VO_INTF_SYNC_E;

typedef struct axVO_RESO_S {
    AX_U32 u32Width;
    AX_U32 u32Height;
    AX_U32 u32RefreshRate;
} VO_RESO_S;

typedef struct axVO_SIZE_S {
    AX_U32 u32Width;
    AX_U32 u32Height;
} VO_SIZE_S;

typedef struct axVO_RECT_S {
    AX_U32 u32X;
    AX_U32 u32Y;
    AX_U32 u32Width;
    AX_U32 u32Height;
} VO_RECT_S;

typedef struct axVO_PUB_ATTR_S {
    VO_INTF_TYPE_E enIntfType;
    VO_INTF_SYNC_E enIntfSync;
    VO_RESO_S stReso;
} VO_PUB_ATTR_S;

typedef enum {
    VO_LAYER_SYNC_NORMAL,
    VO_LAYER_SYNC_SHUTTLE,
    VO_LAYER_SYNC_GROUPING,
    VO_LAYER_SYNC_PRIMARY,
    VO_LAYER_SYNC_BUTT,
} VO_LAYER_SYNC_E;

typedef enum {
    VO_LAYER_OUT_BUF_POOL,
    VO_LAYER_OUT_BUF_INPLACE,
    VO_LAYER_OUT_BUF_BUTT,
} VO_LAYER_BUF_E;

typedef enum {
    VO_LAYER_OUT_TO_FIFO = 1,
    VO_LAYER_OUT_TO_LINK = 2,
} VO_LAYER_DISPATCH_MODE_E;

typedef struct axVO_VIDEO_LAYER_ATTR_S {
    VO_RECT_S stDispRect;              /* RW; Display resolution */
    VO_SIZE_S stImageSize;             /* RW; Canvas size of the video layer */
    AX_IMG_FORMAT_E enPixFmt;
    VO_LAYER_SYNC_E enLayerSync;
    AX_U32 u32PrimaryChnId;
    AX_U32 u32FrameRate;
    AX_U32 u32FifoDepth;
    AX_U32 u32ChnNr;
    AX_U32 u32BkClr;
    VO_LAYER_BUF_E enLayerBuf;
    AX_U32 u32InplaceChnId;
    AX_POOL u32PoolId;
    VO_LAYER_DISPATCH_MODE_E enDispatchMode;
    AX_U64 u64KeepChnPrevFrameBitmap;
} VO_VIDEO_LAYER_ATTR_S;

typedef struct axVO_CHN_ATTR_S {
    VO_RECT_S stRect;
    AX_U32 u32FifoDepth;
} VO_CHN_ATTR_S;

/* Device Relative Settings */

AX_S32 AX_VO_Init(AX_VOID);
AX_S32 AX_VO_Deinit(AX_VOID);

AX_S32 AX_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr);
AX_S32 AX_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);

AX_S32 AX_VO_Enable(VO_DEV VoDev);
AX_S32 AX_VO_Disable(VO_DEV VoDev);

/* Video Relative Settings */

AX_S32 AX_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
AX_S32 AX_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

AX_S32 AX_VO_EnableVideoLayer(VO_LAYER VoLayer);
AX_S32 AX_VO_DisableVideoLayer(VO_LAYER VoLayer);

AX_S32 AX_VO_BindVideoLayer(VO_LAYER VoLayer, VO_DEV VoDev);
AX_S32 AX_VO_UnBindVideoLayer(VO_LAYER VoLayer, VO_DEV VoDev);

AX_S32 AX_VO_BindGraphicLayer(GRAPHIC_LAYER GraphicLayer, VO_DEV VoDev);
AX_S32 AX_VO_UnBindGraphicLayer(GRAPHIC_LAYER GraphicLayer, VO_DEV VoDev);

/* This interface obtains the FD corresponding to the layer for the select operation. Closing operation is not allowed */
AX_S32 AX_VO_GetLayerFd(VO_LAYER VoLayer, AX_S32 *s32Fd);
AX_S32 AX_VO_GetLayerFrame(VO_LAYER VoLayer, AX_VIDEO_FRAME_S *pstVoFrame, AX_S32 s32MS);
AX_S32 AX_VO_ReleaseLayerFrame(VO_LAYER VoLayer, const AX_VIDEO_FRAME_S *pstVoFrame);

/* Channel Relative Operations */

AX_S32 AX_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn);
AX_S32 AX_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn);

AX_S32 AX_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr);
AX_S32 AX_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr);

AX_S32 AX_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, AX_VIDEO_FRAME_S *pstVoFrame, AX_S32 s32MS);


#ifdef __cplusplus
}
#endif

#endif /* _AX_VO_H_ */

