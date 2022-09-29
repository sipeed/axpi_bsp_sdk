/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_IVPS_VENC_H_
#define _SAMPLE_IVPS_VENC_H_

#include "ax_ivps_api.h"

typedef struct
{
	AX_U32 nSrcWidth;
	AX_U32 nSrcHeight;
	AX_U32 nSrcStride;
	AX_U32 nDstWidth;
	AX_U32 nDstHeight;
} SAMPLE_IVPS_VENC_CHN_S;

AX_S32 IVPS_VencInit(AX_U8 pipe);
AX_S32 IVPS_VencDeInit(AX_U8 pipe);
AX_S32 IVPS_VencThreadStart(AX_VOID *p);

#endif /* _SAMPLE_IVPS_VENC_H_ */
