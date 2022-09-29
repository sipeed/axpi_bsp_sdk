/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#ifndef _AX_SYS_LOG_H_
#define _AX_SYS_LOG_H_
#include <axsyslog.h>
#include "ax_base_type.h"
#include "ax_global_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
    LOG_EMERG     system is unusable
    LOG_ALERT     action must be taken immediately
    LOG_CRIT      critical conditions
    LOG_ERR       error conditions
    LOG_WARNING   warning conditions
    LOG_NOTICE    normal, but significant, condition
    LOG_INFO      informational message
    LOG_DEBUG     debug-level message
*/

AX_VOID AX_SYS_LogOpen();
AX_VOID AX_SYS_LogClose();
AX_VOID AX_SYS_LogPrint(AX_S32 level, AX_CHAR const *pFormat, ...);
AX_VOID AX_SYS_LogOutput(AX_LOG_TARGET_E target, AX_LOG_LEVEL_E level, AX_CHAR const *format, va_list vlist);
AX_VOID AX_SYS_LogPrint_Ex(AX_S32 level, AX_CHAR const *tag, int id, AX_CHAR const *pFormat, ...);
AX_VOID AX_SYS_LogOutput_Ex(AX_LOG_TARGET_E target, AX_LOG_LEVEL_E level, AX_CHAR const *tag, int id, AX_CHAR const *format, va_list vlist);

#ifdef __cplusplus
}
#endif

#endif //_AX_SYS_LOG_H_
