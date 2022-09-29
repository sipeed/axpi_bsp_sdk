/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _PP_BUFFER_H__
#define _PP_BUFFER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void* PPBufferCreate(int bufferSize);
void PPBufferUpdate(void* ppbuffer,
                    uint8_t* input_buf,
                    int input_size,
                    uint8_t** p_buffer,
                    int* p_buffer_size);
void PPBufferDestory(void* ppbuffer);

#ifdef __cplusplus
}
#endif

#endif /* _PP_BUFFER_H__ */
