/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_LENS_AF_STRUCT_H_
#define _AX_LENS_AF_STRUCT_H_

#include "ax_base_type.h"

#define ACTUATOR_MAX_NUM 8

typedef struct _AX_LENS_ACTUATOR_AF_FUNC_T_ {
    /* af focus actuator */
    AX_S32 (*pfn_af_focus_init)(AX_U8 nPipeId, AX_U8 nBusNum, AX_U8 nCs);
    AX_U8  (*pfn_af_focus_rstb_status)(AX_U8 nPipeId);
    AX_U8  (*pfn_af_focus_get_status)(AX_U8 nPipeId);
    AX_S32 (*pfn_af_focus_to_dest_pos)(AX_U8 nPipeId, AX_S32 nPos, AX_U32 nPps);
    AX_S32 (*pfn_af_focus_to_dest_pos_direction)(AX_U8 nPipeId, AX_S32 nPos, AX_S32 nDirection);
    AX_S32 (*pfn_af_focus_exit)(AX_U8 nPipeId);
    /* af zoom actuator */
    AX_S32 (*pfn_af_zoom_init)(AX_U8 nPipeId, AX_U8 nBusNum, AX_U8 nCs);
    AX_U8  (*pfn_af_zoom_rstb_status)(AX_U8 nPipeId);
    AX_U8  (*pfn_af_zoom1_get_status)(AX_U8 nPipeId);
    AX_U8  (*pfn_af_zoom2_get_status)(AX_U8 nPipeId);
    AX_S32 (*pfn_af_zoom1_to_dest_pos)(AX_U8 nPipeId, AX_S32 nPos, AX_U32 nPps);
    AX_S32 (*pfn_af_zoom2_to_dest_pos)(AX_U8 nPipeId, AX_S32 nPos, AX_U32 nPps);
    AX_S32 (*pfn_af_zoom1_to_dest_pos_direction)(AX_U8 nPipeId, AX_S32 nPos, AX_S32 nDirection);
    AX_S32 (*pfn_af_zoom2_to_dest_pos_direction)(AX_U8 nPipeId, AX_S32 nPos, AX_S32 nDirection);
    AX_S32 (*pfn_af_zoom_exit)(AX_U8 nPipeId);

} AX_LENS_ACTUATOR_AF_FUNC_T;

#endif

