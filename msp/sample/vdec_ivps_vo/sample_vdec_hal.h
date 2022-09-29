/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_VDEC_HAL_H_
#define _SAMPLE_VDEC_HAL_H_


#include "ax_sys_api.h"
#include "ax_base_type.h"

#define SIZE_ALIGN(x,align) ((((x)+(align)-1)/(align))*(align))
#define SAMPLE_VDEC_USERPIC_TEST
#ifdef SAMPLE_VDEC_USERPIC_TEST
#define USR_PIC_WIDTH 1920
#define USR_PIC_HEIGHT 1080
#endif
#ifndef ALIGN_UP
#define ALIGN_UP(x, align)      (((x) + ((align) - 1)) & ~((align) - 1))
#endif
extern AX_S32 userPicTest;
AX_S32 PoolUserPicDisplay(AX_VIDEO_FRAME_INFO_S *pstUserPic);
AX_S32 PoolUserPicDeinit(AX_VIDEO_FRAME_INFO_S *pstUserPic);
AX_S32 PoolUserPicInit(AX_VIDEO_FRAME_INFO_S *pstUserPic);
AX_S32 LoadFileToMem(const AX_CHAR *ps8File, AX_U8 **ppu8Mem, AX_S32 *ps32Len);
void *H264DecFrameFunc(void *arg);
void *VdecThread(void *arg);
AX_S32 H264DecExitFunc(AX_VDEC_GRP VdGrp);
AX_S32 JpegDecExitFunc(AX_VDEC_GRP VdGrp);



#endif /* _SAMPLE_IVPS_HAL_H_ */
