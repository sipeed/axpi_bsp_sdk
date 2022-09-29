#ifndef _DEF_NT_STREAM_API_H_
#define _DEF_NT_STREAM_API_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

AX_S32 AX_NT_StreamInit(AX_U32 nStreamPort);
AX_S32 AX_NT_StreamDeInit(void);
AX_S32 AX_NT_SetStreamSource(AX_U8 pipe);

#ifdef __cplusplus
}
#endif

#endif //_DEF_NT_STREAM_API_H_
