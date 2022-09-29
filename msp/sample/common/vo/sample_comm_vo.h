/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "ax_sys_api.h"
#include "ax_base_type.h"
#include "ax_vo_api.h"

#ifndef SAMPLE_PRT
#define SAMPLE_PRT(fmt...)   \
do {\
    printf("[%s-%d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif


#define ALIGN_UP(x, a)           ( ( ((x) + ((a) - 1) ) / a ) * a )
#define ALIGN_DOWN(x, a)         ( ( (x) / (a)) * (a) )

#define SAMPLE_VLAYER_MAX        2

typedef enum axSAMPLE_VO_MODE_E {
    VO_MODE_1MUX,
    VO_MODE_2MUX,
    VO_MODE_4MUX,
    VO_MODE_8MUX,
    VO_MODE_9MUX,
    VO_MODE_16MUX,
    VO_MODE_25MUX,
    VO_MODE_36MUX,
    VO_MODE_49MUX,
    VO_MODE_64MUX,
    VO_MODE_2X4,
    VO_MODE_BUTT
} SAMPLE_VO_MODE_E;

typedef struct axSAMPLE_COMM_VO_LAYER_CONFIG_S {
    /* for layer */
    VO_LAYER                VoLayer;
    VO_VIDEO_LAYER_ATTR_S   stVoLayerAttr;

    /* for chnnel */
    SAMPLE_VO_MODE_E        enVoMode;
    AX_IMG_FORMAT_E         enChnPixFmt;
    AX_U32                  u32FifoDepth;

    /* for sys link */
    AX_MOD_INFO_S           stSrcMod;
    AX_MOD_INFO_S           stDstMod;
} SAMPLE_VO_LAYER_CONFIG_S;

typedef struct axSAMPLE_VO_CONFIG_S {
    /* for device */
    VO_DEV                  VoDev;
    VO_INTF_TYPE_E          enVoIntfType;
    VO_INTF_SYNC_E          enIntfSync;
    VO_RESO_S               stReso;

    AX_U32                  u32FifoDepth;
    AX_U32                  u32LayerNr;

    SAMPLE_VO_LAYER_CONFIG_S stVoLayer[SAMPLE_VLAYER_MAX];

    /* for graphic layer */
    AX_S32                  s32EnableGLayer;
    GRAPHIC_LAYER           GraphicLayer;

    /* for common pool */
    AX_U32 u32CommonPoolCnt;
    AX_POOL_FLOORPLAN_T stPoolFloorPlan;
} SAMPLE_VO_CONFIG_S;

AX_S32 SAMPLE_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);
AX_S32 SAMPLE_COMM_VO_StopDev(VO_DEV VoDev);

AX_S32 SAMPLE_COMM_VO_StartLayer(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);
AX_S32 SAMPLE_COMM_VO_StopLayer(VO_LAYER VoLayer);

AX_S32 SAMPLE_COMM_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode, AX_U32 u32FifoDepth);
AX_S32 SAMPLE_COMM_VO_StopChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode);

AX_S32 SAMPLE_COMM_VO_StartVO(SAMPLE_VO_CONFIG_S *pstVoConfig);
AX_S32 SAMPLE_COMM_VO_StopVO(SAMPLE_VO_CONFIG_S *pstVoConfig);

