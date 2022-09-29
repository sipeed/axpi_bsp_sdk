/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "pp_buffer.h"

typedef struct {
    uint8_t* buffer[2];
    int bufferSize;
    int bufferIndex;
    int dataIndex;
} PPBuffer;

void* PPBufferCreate(int bufferSize)
{
    PPBuffer* ppbuf = malloc(sizeof(PPBuffer));
    assert(ppbuf);
    ppbuf->buffer[0] = malloc(bufferSize);
    ppbuf->buffer[1] = malloc(bufferSize);
    assert(ppbuf->buffer[0] && ppbuf->buffer[1]);
    ppbuf->bufferSize = bufferSize;
    ppbuf->bufferIndex = 0;
    ppbuf->dataIndex = 0;

    return ppbuf;
}

void PPBufferUpdate(void* ppbuffer,
                    uint8_t* input_buf,
                    int input_size,
                    uint8_t** p_buffer,
                    int* p_buffer_size)
{
    PPBuffer* ppbuf = (PPBuffer*)ppbuffer;
    assert(input_size <= ppbuf->bufferSize);

    int leftSize = ppbuf->bufferSize - ppbuf->dataIndex;
    if (leftSize >= input_size) {
        memcpy(&ppbuf->buffer[ppbuf->bufferIndex][ppbuf->dataIndex], input_buf, input_size);
        ppbuf->dataIndex += input_size;
        if (ppbuf->dataIndex >= ppbuf->bufferSize) {
            int doneIndex = ppbuf->bufferIndex;
            ppbuf->bufferIndex = (ppbuf->bufferIndex == 0 ? 1 : 0);
            ppbuf->dataIndex = 0;
            //printf("leftSize = input_size, copyed: %d, doneIndex: %d\n", input_size, doneIndex);
            *p_buffer = ppbuf->buffer[doneIndex];
            *p_buffer_size = ppbuf->bufferSize;
        } else {
            //printf("leftSize > input_size, copyed %d size\n", input_size);
            *p_buffer = NULL;
            *p_buffer_size = 0;
        }
    } else {
        memcpy(&ppbuf->buffer[ppbuf->bufferIndex][ppbuf->dataIndex], input_buf, leftSize);
        int doneIndex = ppbuf->bufferIndex;
        ppbuf->bufferIndex = (ppbuf->bufferIndex == 0 ? 1 : 0);
        memcpy(&ppbuf->buffer[ppbuf->bufferIndex][0], input_buf + leftSize, input_size - leftSize);
        ppbuf->dataIndex = input_size - leftSize;
        //printf("leftSize < input_size, copyed: %d, doneIndex: %d\n", input_size, doneIndex);
        *p_buffer = ppbuf->buffer[doneIndex];
        *p_buffer_size = ppbuf->bufferSize;
    }
}

void PPBufferDestory(void* ppbuffer)
{
    PPBuffer* ppbuf = (PPBuffer*)ppbuffer;
    free(ppbuf->buffer[0]);
    free(ppbuf->buffer[1]);
    free(ppbuf);
}