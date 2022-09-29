/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_VDEC_API_H_
#define _AX_VDEC_API_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_comm_codec.h"
#include "ax_comm_vdec.h"

#ifdef __cplusplus
extern "C" {
#endif

AX_S32 AX_VDEC_Init();
AX_S32 AX_VDEC_DeInit();

AX_S32 AX_VDEC_CreateGrp(AX_VDEC_GRP VdGrp, const AX_VDEC_GRP_ATTR_S *pstAttr);
AX_S32 AX_VDEC_DestroyGrp(AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_GetGrpAttr(AX_VDEC_GRP VdGrp, AX_VDEC_GRP_ATTR_S *pstAttr);
AX_S32 AX_VDEC_SetGrpAttr(AX_VDEC_GRP VdGrp, const AX_VDEC_GRP_ATTR_S *pstAttr);

AX_S32 AX_VDEC_StartRecvStream(AX_VDEC_GRP VdGrp);
AX_S32 AX_VDEC_StopRecvStream(AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_QueryStatus(AX_VDEC_GRP VdGrp,AX_VDEC_GRP_STATUS_S *pstStatus);

AX_S32 AX_VDEC_ResetGrp(AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_SetGrpParam(AX_VDEC_GRP VdGrp, const AX_VDEC_GRP_PARAM_S* pstParam);
AX_S32 AX_VDEC_GetGrpParam(AX_VDEC_GRP VdGrp, AX_VDEC_GRP_PARAM_S* pstParam);

AX_S32 AX_VDEC_SetProtocolParam(AX_VDEC_GRP VdGrp, const AX_VDEC_PRTCL_PARAM_S *pstParam);
AX_S32 AX_VDEC_GetProtocolParam(AX_VDEC_GRP VdGrp,AX_VDEC_PRTCL_PARAM_S *pstParam);

/* s32MilliSec: -1 is block,0 is no block,other positive number is timeout */
AX_S32 AX_VDEC_SendStream(AX_VDEC_GRP VdGrp, const AX_VDEC_STREAM_S *pstStream, AX_S32 s32MilliSec);

AX_S32 AX_VDEC_GetFrame(AX_VDEC_GRP VdGrp, AX_VIDEO_FRAME_INFO_S *pstFrameInfo, AX_S32 s32MilliSec);
AX_S32 AX_VDEC_ReleaseFrame(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_S *pstFrameInfo);

AX_S32 AX_VDEC_GetUserData(AX_VDEC_GRP VdGrp, AX_VDEC_USERDATA_S *pstUserData, AX_S32 s32MilliSec);
AX_S32 AX_VDEC_ReleaseUserData(AX_VDEC_GRP VdGrp, const AX_VDEC_USERDATA_S *pstUserData);

AX_S32 AX_VDEC_SetUserPic(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_S *pstUsrPic);
AX_S32 AX_VDEC_EnableUserPic(AX_VDEC_GRP VdGrp, AX_BOOL bInstant);
AX_S32 AX_VDEC_DisableUserPic(AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_SetDisplayMode(AX_VDEC_GRP VdGrp, AX_VDEC_DISPLAY_MODE_E enDisplayMode);
AX_S32 AX_VDEC_GetDisplayMode(AX_VDEC_GRP VdGrp, AX_VDEC_DISPLAY_MODE_E *penDisplayMode);

AX_S32 AX_VDEC_AttachPool(AX_VDEC_GRP VdGrp, AX_POOL Pool);
AX_S32 AX_VDEC_DetachPool(AX_VDEC_GRP VdGrp);

AX_S32 AX_VDEC_Debug_Init();
AX_S32 AX_VDEC_Debug_DeInit();
AX_S32 AX_VDEC_SetDebugFifoDepth(AX_VDEC_GRP VdGrp, AX_U32 u32FifoDepth);
AX_S32 AX_VDEC_GetDebugFifoFrame(AX_VDEC_GRP VdGrp, AX_VIDEO_FRAME_INFO_S *pstFrameInfo);
AX_S32 AX_VDEC_ReleaseDebugFifoFrame(AX_VDEC_GRP VdGrp, const AX_VIDEO_FRAME_INFO_S *pstFrameInfo);


#ifdef __cplusplus
}
#endif

#endif
