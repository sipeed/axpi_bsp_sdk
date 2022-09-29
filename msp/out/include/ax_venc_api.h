/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_VENC_API_H_
#define _AX_VENC_API_H_

#include <stdbool.h>
#include "ax_comm_venc.h"


#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

AX_S32 AX_VENC_Init(const AX_VENC_MOD_ATTR_S *pstModAttr);
AX_S32 AX_VENC_Deinit();

AX_S32 AX_VENC_CreateChn(VENC_CHN VeChn, const AX_VENC_CHN_ATTR_S *pstAttr);
AX_S32 AX_VENC_DestroyChn(VENC_CHN VeChn);

AX_S32 AX_VENC_SendFrame(VENC_CHN VeChn, const AX_VIDEO_FRAME_INFO_S *pstFrame , AX_S32 s32MilliSec);
AX_S32 AX_VENC_SendFrameEx(VENC_CHN VeChn, const AX_USER_FRAME_INFO_S *pstFrame, AX_S32 s32MilliSec);

AX_S32 AX_VENC_SelectChn(AX_CHN_STREAM_STATUS_S *pstChnStrmState, AX_S32 s32MilliSec);
AX_S32 AX_VENC_GetStream(VENC_CHN VeChn, AX_VENC_STREAM_S *pstStream, AX_S32 s32MilliSec);
AX_S32 AX_VENC_ReleaseStream(VENC_CHN VeChn, const AX_VENC_STREAM_S *pstStream);
AX_S32 AX_VENC_GetStreamBufInfo(VENC_CHN VeChn, AX_VENC_STREAM_BUF_INFO_T * pstStreamBufInfo);

AX_S32 AX_VENC_StartRecvFrame(VENC_CHN VeChn, const AX_VENC_RECV_PIC_PARAM_S *pstRecvParam);
AX_S32 AX_VENC_StopRecvFrame(VENC_CHN VeChn);

AX_S32 AX_VENC_SetRoiAttr(VENC_CHN VeChn, const AX_VENC_ROI_ATTR_S *pstRoiAttr);
AX_S32 AX_VENC_GetRoiAttr(VENC_CHN VeChn, AX_U32 u32Index, AX_VENC_ROI_ATTR_S *pstRoiAttr);

AX_S32 AX_VENC_SetRcParam(VENC_CHN VeChn, const AX_VENC_RC_PARAM_S *pstRcParam);
AX_S32 AX_VENC_GetRcParam(VENC_CHN VeChn, AX_VENC_RC_PARAM_S *pstRcParam);

AX_S32 AX_VENC_SetModParam(AX_VENC_ENCODER_TYPE_E enVencType, const AX_VENC_MOD_PARAM_S *pstModParam);
AX_S32 AX_VENC_GetModParam(AX_VENC_ENCODER_TYPE_E enVencType, AX_VENC_MOD_PARAM_S *pstModParam);

AX_S32 AX_VENC_SetSpsVuiParam(VENC_CHN VeChn, const AX_VENC_SPS_VUI_PARAM_S *pstSpsVuiParam);
AX_S32 AX_VENC_GetSpsVuiParam(VENC_CHN VeChn, AX_VENC_SPS_VUI_PARAM_S *pstSpsVuiParam);

AX_S32 AX_VENC_SetChnAttr(VENC_CHN VeChn, const AX_VENC_CHN_ATTR_S *pstChnAttr);
AX_S32 AX_VENC_GetChnAttr(VENC_CHN VeChn, AX_VENC_CHN_ATTR_S *pstChnAttr);

AX_S32 AX_VENC_SetRateJamStrategy(VENC_CHN VeChn, const AX_VENC_RATE_JAM_CFG_S *pstRateJamParam);
AX_S32 AX_VENC_GetRateJamStrategy(VENC_CHN VeChn, AX_VENC_RATE_JAM_CFG_S *pstRateJamParam);

AX_S32 AX_VENC_SetSuperFrameStrategy(VENC_CHN VeChn, const AX_VENC_SUPERFRAME_CFG_S *pstSuperFrameCfg);
AX_S32 AX_VENC_GetSuperFrameStrategy(VENC_CHN VeChn, AX_VENC_SUPERFRAME_CFG_S *pstSuperFrameCfg);

AX_S32 AX_VENC_InsertUserData(VENC_CHN VeChn, const AX_U8 *pu8Data, AX_U32 u32Len);

AX_S32 AX_VENC_RequestIDR(VENC_CHN VeChn, AX_BOOL bInstant);

AX_S32 AX_VENC_QueryStatus(VENC_CHN VeChn, AX_VENC_CHN_STATUS_S *pstStatus);

AX_S32 AX_VENC_SetJpegParam(VENC_CHN VeChn, const AX_VENC_JPEG_PARAM_S *pstJpegParam);
AX_S32 AX_VENC_GetJpegParam(VENC_CHN VeChn, AX_VENC_JPEG_PARAM_S *pstJpegParam);

AX_S32 AX_VENC_SetDebugFifoDepth(VENC_CHN VeChn, AX_VENC_ENCODER_TYPE_E enType, AX_U16 u16InFifoDepth, AX_U16 u16OutFifoDepth);

AX_S32 AX_VENC_GetInDebugFifoFrame(VENC_CHN VeChn, AX_VENC_ENCODER_TYPE_E enType, AX_VIDEO_FRAME_S  *pstVFrame);

AX_S32 AX_VENC_ReleaseInDebugFifoFrame(VENC_CHN VeChn, AX_VENC_ENCODER_TYPE_E enType, const AX_VIDEO_FRAME_S *pstVFrame);



#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of _AX_VENC_API_H_ */
