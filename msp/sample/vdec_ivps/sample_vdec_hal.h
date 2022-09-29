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


AX_S32 LoadFileToMem(const AX_CHAR *ps8File, AX_U8 **ppu8Mem, AX_S32 *ps32Len);
void *H264DecFrameFunc(void *arg);
void *VdecThread(void *arg);
AX_S32 H264DecExitFunc(AX_VDEC_GRP VdGrp);
AX_S32 JpegDecExitFunc(AX_VDEC_GRP VdGrp);
AX_S32 SampleVdecInit(AX_PAYLOAD_TYPE_E enType);
AX_S32 SampleVdecExit();




#endif /* _SAMPLE_IVPS_HAL_H_ */
