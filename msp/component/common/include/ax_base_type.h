/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_BASE_TYPE_H_
#define _AX_BASE_TYPE_H_

// #include <stdbool.h>

/* types of variables typedef */
typedef unsigned long long int  AX_U64;
typedef unsigned int            AX_U32;
typedef unsigned short          AX_U16;
typedef unsigned char           AX_U8;
typedef long long int           AX_S64;
typedef int                     AX_S32;
typedef short                   AX_S16;
typedef signed char             AX_S8;
typedef char                    AX_CHAR;
typedef long                    AX_LONG;
typedef unsigned long           AX_ULONG;
typedef unsigned long           AX_ADDR;
typedef float                   AX_F32;
typedef double                  AX_F64;
typedef void                    AX_VOID;
typedef unsigned int            AX_SIZE_T;

typedef enum {
    AX_FALSE = 0,
    AX_TRUE  = 1,
} AX_BOOL;

#ifndef AX_NULL
    #define AX_NULL 0
#endif

#endif //_AX_BASE_TYPE_H_