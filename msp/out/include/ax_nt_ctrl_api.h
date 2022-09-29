#ifndef _DEF_NT_CTRL_API_H_
#define _DEF_NT_CTRL_API_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    AX_NT_CTRL_MODE_NORMAL     = 0,  //standalone board, not cascaded. default
    AX_NT_CTRL_MODE_CAS_MASTER = 1,  //cascaded master
    AX_NT_CTRL_MODE_CAS_SLAVE  = 2,  //cascaded slave
}AX_NT_CTRL_MODE_E;

/* ******************************************
Call AX_NT_CtrlInitMode before AX_NT_CtrlInit
Will be AX_NT_CTRL_MODE_NORMAL, if not called.
********************************************/
AX_S32 AX_NT_CtrlInitMode(AX_NT_CTRL_MODE_E mode);

AX_S32 AX_NT_CtrlInit(AX_U32 nPort);
AX_S32 AX_NT_CtrlDeInit(void);

#ifdef __cplusplus
}
#endif

#endif //_DEF_NT_CTRL_API_H_
