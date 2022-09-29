/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_IVPS_DEWARP_H_
#define _SAMPLE_IVPS_DEWARP_H_

typedef struct
{
	char *pMeshFile;
	AX_U16 nSrcWidth;
	AX_U16 nSrcHeight;
	AX_U32 nRotate;
	AX_IVPS_DEWARP_ATTR_S tAttr;
} SAMPLE_IVPS_DEWARP_S;

AX_S32 DewarpSingleSample(const AX_VIDEO_FRAME_INFO_S *ptFrame, char *pStorePath);
AX_S32 IVPS_DewarpSingleThread(const AX_VIDEO_FRAME_INFO_S *ptFrame);

#endif /* _SAMPLE_IVPS_DEWARP_H_ */
