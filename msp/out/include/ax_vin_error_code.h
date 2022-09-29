/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_VIN_ERROR_CODE_H_
#define _AX_VIN_ERROR_CODE_H_

#include "ax_global_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
|----------------------------------------------------------------|
||   FIXED   |   MOD_ID    | SUB_MODULE_ID |   ERR_ID            |
|----------------------------------------------------------------|
|<--8bits----><----8bits---><-----8bits---><------8bits------->|
******************************************************************************/
/* VIN Error Code Base: 0x8011xxxx */

typedef enum {
    AX_ID_VIN_NULL      = 0x01,
    AX_ID_VIN_BUTT,
} AX_VIN_SUB_ID_E;

typedef enum {
    AX_ERR_CODE_VIN_FAILED                   = 0x80,//ISP Failed
    AX_ERR_CODE_VIN_PIPE_CREATE_ALREADY,
    AX_ERR_CODE_VIN_DRIVER_NOT_LOAD,
    AX_ERR_CODE_VIN_SNS_UNREGISTER,
    AX_ERR_CODE_VIN_NOT_BIND,
} AX_VIN_ERR_CODE_E;

/* Common Err Code */
#define AX_ERR_VIN_INVALID_MODID        AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_MODID)        /* 0x80110101 */
#define AX_ERR_VIN_INVALID_DEVID        AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_DEVID)        /* 0x80110102 */
#define AX_ERR_VIN_INVALID_CHNID        AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_CHNID)        /* 0x80110104 */
#define AX_ERR_VIN_INVALID_PIPEID       AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_INVALID_PIPEID)       /* 0x80110105 */
#define AX_ERR_VIN_ILLEGAL_PARAM        AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_ILLEGAL_PARAM)        /* 0x8011010A */
#define AX_ERR_VIN_NULL_PTR             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NULL_PTR)             /* 0x8011010B */
#define AX_ERR_VIN_INVALID_ADDR         AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_BAD_ADDR)             /* 0x8011010C */
#define AX_ERR_VIN_SYS_NOTREADY         AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_SYS_NOTREADY)         /* 0x80110110 */
#define AX_ERR_VIN_BUSY                 AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_BUSY)                 /* 0x80110111 */
#define AX_ERR_VIN_NOT_INIT             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_INIT)             /* 0x80110112 */
#define AX_ERR_VIN_ATTR_NOT_CFG         AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_CONFIG)           /* 0x80110113 */
#define AX_ERR_VIN_NOT_SUPPORT          AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_SUPPORT)          /* 0x80110114 */
#define AX_ERR_VIN_NOT_PERM             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_PERM)             /* 0x80110115 */
#define AX_ERR_VIN_OBJ_EXISTS           AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_UNEXIST)              /* 0x80110117 */
#define AX_ERR_VIN_NOMEM                AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOMEM)                /* 0x80110118 */
#define AX_ERR_VIN_NOBUF                AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOBUF)                /* 0x80110119 */
#define AX_ERR_VIN_NOT_MATCH            AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_NOT_MATCH)            /* 0x8011011A */
#define AX_ERR_VIN_BUF_FULL             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_BUF_FULL)             /* 0x80110121 */
#define AX_ERR_VIN_RES_EMPTY            AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_QUEUE_EMPTY)          /* 0x80110122 */
#define AX_ERR_VIN_QUEUE_FULL           AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_QUEUE_FULL)           /* 0x80110123 */
#define AX_ERR_VIN_TIMEOUT              AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_TIMED_OUT)            /* 0x80110127 */

#define AX_ERR_VIN_FAILED               AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_FAILED)               /* 0x80110180 */
#define AX_ERR_VIN_PIPE_CREATE_ALREADY  AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_PIPE_CREATE_ALREADY)  /* 0x80110181 */
#define AX_ERR_VIN_DRIVER_NOT_LOAD      AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_DRIVER_NOT_LOAD)      /* 0x80110182 */
#define AX_ERR_VIN_SNS_UNREGISTER       AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_SNS_UNREGISTER)       /* 0x80110183 */
#define AX_ERR_VIN_NOT_BIND             AX_DEF_ERR(AX_ID_VIN, AX_ID_VIN_NULL, AX_ERR_CODE_VIN_NOT_BIND)             /* 0x80110184 */


#ifdef __cplusplus
}
#endif

#endif //_AX_VIN_ERROR_CODE_H_
