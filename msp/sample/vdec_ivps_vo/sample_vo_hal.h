/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_VO_HAL_H_
#define _SAMPLE_VO_HAL_H_


#include "sample_comm_vo.h"


typedef struct _SAMPLE_INPUT_PARAM {
    AX_U32 u32PicWidth;
    AX_U32 u32PicHeight;
} SAMPLE_PARAM_S;

AX_S32 VoInit(AX_U32 u32CamChn, SAMPLE_PARAM_S *pstInParam, SAMPLE_VO_CONFIG_S *pstVoConf);
AX_VOID VoDeInit(SAMPLE_VO_CONFIG_S *pstVoConf);
void *VoGetFrameThread(void *arg);

AX_S32 ParseVoPubAttr(AX_CHAR *pStr, SAMPLE_VO_CONFIG_S *pstVoConf);


#endif /* _SAMPLE_VO_HAL_H_ */
