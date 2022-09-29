/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _LOG_TRACE_H_
#define _LOG_TRACE_H_
#include <stdio.h>
#include "ax_global_type.h"
#include "ax_sys_log.h"

#if 0
#define MACRO_BLACK         "\033[1;30;30m"
#define MACRO_RED           "\033[1;30;31m"
#define MACRO_GREEN         "\033[1;30;32m"
#define MACRO_YELLOW        "\033[1;30;33m"
#define MACRO_BLUE          "\033[1;30;34m"
#define MACRO_PURPLE        "\033[1;30;35m"
#define MACRO_WHITE         "\033[1;30;37m"
#define MACRO_END           "\033[0m"
#else
#define MACRO_BLACK
#define MACRO_RED
#define MACRO_GREEN
#define MACRO_YELLOW
#define MACRO_BLUE
#define MACRO_PURPLE
#define MACRO_WHITE
#define MACRO_END
#endif

#define AX_MSYS_LOG_TAG  "MSYS"

#define AX_LOG_ERR(fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_ERROR,  MACRO_RED"[E][%32s][%4d]: "fmt MACRO_END, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_WARN(fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_WARN, MACRO_BLUE"[W][%32s][%4d]: "fmt MACRO_END, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_INFO(fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_INFO, MACRO_GREEN"[I][%32s][%4d]: "fmt MACRO_END, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_DBG(fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_DEBUG, MACRO_WHITE"[D][%32s][%4d]: "fmt MACRO_END, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_NOTICE(fmt,...) \
    AX_SYS_LogPrint(SYS_LOG_NOTICE, MACRO_PURPLE"[N][%32s][%4d]: "fmt MACRO_END, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define AX_LOG_ERR_EX(tag,id,fmt,...) \
    AX_SYS_LogPrint_Ex(SYS_LOG_ERROR, tag, id, MACRO_RED"[E][%32s][%4d]: "fmt MACRO_END , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_WARN_EX(tag,id,fmt,...) \
    AX_SYS_LogPrint_Ex(SYS_LOG_WARN, tag, id, MACRO_BLUE"[W][%32s][%4d]: "fmt MACRO_END , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_INFO_EX(tag,id,fmt,...) \
    AX_SYS_LogPrint_Ex(SYS_LOG_INFO, tag, id, MACRO_GREEN"[I][%32s][%4d]: "fmt MACRO_END , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_DBG_EX(tag,id,fmt,...) \
    AX_SYS_LogPrint_Ex(SYS_LOG_DEBUG, tag, id, MACRO_WHITE"[D][%32s][%4d]: "fmt MACRO_END , __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define AX_LOG_NOTICE_EX(tag,id,fmt,...) \
    AX_SYS_LogPrint_Ex(SYS_LOG_NOTICE, tag, id, MACRO_PURPLE"[N][%32s][%4d]: "fmt MACRO_END , __FUNCTION__, __LINE__, ##__VA_ARGS__);

#endif //_LOG_TRACE_H_
