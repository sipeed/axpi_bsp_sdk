/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_COMM_AIO_H_
#define _AX_COMM_AIO_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum axAUDIO_BIT_WIDTH_E {
    AX_AUDIO_BIT_WIDTH_8   = 0,             /* 8bit width */
    AX_AUDIO_BIT_WIDTH_16  = 1,             /* 16bit width*/
    AX_AUDIO_BIT_WIDTH_24  = 2,             /* 24bit width*/
    AX_AUDIO_BIT_WIDTH_BUTT,
} AX_AUDIO_BIT_WIDTH_E;

typedef enum axAUDIO_SOUND_MODE_E {
    AX_AUDIO_SOUND_MODE_MONO   = 0,         /*mono*/
    AX_AUDIO_SOUND_MODE_STEREO = 1,         /*stereo*/
    AX_AUDIO_SOUND_MODE_BUTT
} AX_AUDIO_SOUND_MODE_E;

typedef struct axAUDIO_FRAME_S {
    AX_AUDIO_BIT_WIDTH_E   enBitwidth;      /*audio frame bitwidth*/
    AX_AUDIO_SOUND_MODE_E  enSoundmode;     /*audio frame momo or stereo mode*/
    AX_U8  *u64VirAddr;
    AX_U64  u64PhyAddr;
    AX_U64  u64TimeStamp;                   /*audio frame timestamp*/
    AX_U32  u32Seq;                         /*audio frame seq*/
    AX_U32  u32Len;                         /*data lenth in frame*/
    AX_U32  u32PoolId[2];
    AX_BOOL bEof;
} AX_AUDIO_FRAME_S;

typedef struct axAUDIO_STREAM_S {
    AX_U8 *pStream;         /* the virtual address of stream */
    AX_U64 u64PhyAddr;      /* the physics address of stream */
    AX_U32 u32Len;          /* stream lenth, by bytes */
    AX_U64 u64TimeStamp;    /* frame time stamp*/
    AX_U32 u32Seq;          /* frame seq,if stream is not a valid frame,u32Seq is 0*/
    AX_BOOL bEof;
} AX_AUDIO_STREAM_S;

#ifdef __cplusplus
}
#endif

#endif
