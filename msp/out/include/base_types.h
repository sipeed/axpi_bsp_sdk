/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _BASE_TYPES_H_
#define _BASE_TYPES_H_

/* types of variables typedef */
typedef unsigned long long int  u64;
typedef unsigned int            u32;
typedef unsigned short          u16;
typedef unsigned char           u8;
typedef long long int           s64;
typedef int                     s32;
typedef short                   s16;
typedef char                    s8;
typedef unsigned long           ulong;
typedef unsigned long           addr_t;
typedef float                   f32;
typedef double                  f64;

typedef enum {
    ax_false = 0,
    ax_true  = 1,
} ax_bool;

#endif //_BASE_TYPES_H_
