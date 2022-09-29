/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _AX_VIN_EXT_API_H_
#define _AX_VIN_EXT_API_H_

#include "ax_base_type.h"
#include "ax_vin_api.h"
#include "ax_isp_common.h"

AX_S32 AX_VIN_EXT_GetRawFrame(AX_U8 devId, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *ptImgInfo,
                                    AX_S32 timeOutMs);
AX_S32 AX_VIN_EXT_ReleaseRawFrame(AX_U8 devId, AX_SNS_HDR_FRAME_E eSnsFrame, AX_IMG_INFO_T *ptImgInfo);
AX_S32 AX_VIN_EXT_GetAttr(AX_U8 pipe, AX_DEV_ATTR_T *ptDevAttr);
AX_S32 AX_VIN_EXT_FifoDepthChange(AX_U8 pipe, AX_S32 nFifoDepth);
AX_S32 AX_VIN_EXT_Init(void);
AX_S32 AX_VIN_EXT_Deinit(void);
#endif
