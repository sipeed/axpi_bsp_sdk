/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef __AX_ISP_DEBUG_H__
#define __AX_ISP_DEBUG_H__

#include "ax_base_type.h"

#define AX_ISP_HIST_FLAG_LIST_MAX_NUM 221

typedef struct {
    AX_U8 nHistFlagList[AX_ISP_HIST_FLAG_LIST_MAX_NUM]; /* Hist flag list. Accuracy:U8 Range: [0, 15] */
} AX_ISP_IQ_DEBUG_PARAM_T;

AX_S32 AX_ISP_IQ_GetDebugParam(AX_U8 pipe, AX_ISP_IQ_DEBUG_PARAM_T *pIQDebugParam);

#endif // __AX_ISP_DEBUG_H__