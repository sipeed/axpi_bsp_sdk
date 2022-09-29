/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_ISP_ERROR_CODE_H_
#define _AX_ISP_ERROR_CODE_H_

#include "ax_base_type.h"
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
/* ISP Error Code Base: 0x8001xxxx */

typedef enum {
    AX_ID_ISP_NULL      = 0x01,
    AX_ID_ISP_BUTT,
} AX_ISP_SUB_ID_E;

typedef enum {
    AX_ERR_CODE_ISP_FAILED                   = 0x80,//ISP Failed
    AX_ERR_CODE_ISP_PIPE_CREATE_ALREADY,
    AX_ERR_CODE_ISP_DRIVER_NOT_LOAD,
    AX_ERR_CODE_ISP_SNS_UNREGISTER,
    AX_ERR_CODE_ISP_STATE_ERROR,
    AX_ERR_CODE_ISP_STATE_PROCESS_ERROR,
    AX_ERR_CODE_ISP_NOT_EVEN,
    AX_ERR_CODE_ISP_NOT_ODD,
    AX_ERR_CODE_ISP_IRQ_FAIL,
    AX_ERR_CODE_ISP_3A_STAT_GET_FAIL
} AX_ISP_ERR_CODE_E;

/* Common Err Code */
#define AX_ERR_ISP_NULL_PTR             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NULL_PTR)           /* 0x8001010B */
#define AX_ERR_ISP_INVALID_DEVID        AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_INVALID_DEVID)      /* 0x80010102 */
#define AX_ERR_ISP_INVALID_PIPEID       AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_INVALID_PIPEID)     /* 0x80010105 */
#define AX_ERR_ISP_INVALID_CHNID        AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_INVALID_CHNID)      /* 0x80010104 */
#define AX_ERR_ISP_ILLEGAL_PARAM        AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_ILLEGAL_PARAM)      /* 0x8001010A */
#define AX_ERR_ISP_NOT_SUPPORT          AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_SUPPORT)        /* 0x80010114 */
#define AX_ERR_ISP_NOMEM                AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOMEM)              /* 0x80010118 */
#define AX_ERR_ISP_TIMEOUT              AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_TIMED_OUT)          /* 0x80010127 */
#define AX_ERR_ISP_RES_EMPTY            AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_QUEUE_EMPTY)        /* 0x80010122 */
#define AX_ERR_ISP_NOT_INIT             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_INIT)           /* 0x80010112 */
#define AX_ERR_ISP_ATTR_NOT_CFG         AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_NOT_CONFIG)         /* 0x80010113 */
#define AX_ERR_ISP_INVALID_ADDR         AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_BAD_ADDR)           /* 0x8001010C */
#define AX_ERR_ISP_OBJ_EXISTS           AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_UNEXIST)            /* 0x80010117 */

#define AX_ERR_ISP_FAILED               AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_FAILED)               /* 0x80010180 */
#define AX_ERR_ISP_PIPE_CREATE_ALREADY  AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_PIPE_CREATE_ALREADY)  /* 0x80010181 */
#define AX_ERR_ISP_DRIVER_NOT_LOAD      AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_DRIVER_NOT_LOAD)      /* 0x80010182 */
#define AX_ERR_ISP_SNS_UNREGISTER       AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_SNS_UNREGISTER)       /* 0x80010183 */
#define AX_ERR_ISP_STATE_ERROR          AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_STATE_ERROR)          /* 0x80010184 */
#define AX_ERR_ISP_STATE_PROCESS_ERROR  AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_STATE_PROCESS_ERROR)  /* 0x80010185 */
#define AX_ERR_ISP_NOT_EVEN             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_NOT_EVEN)             /* 0x80010186 */
#define AX_ERR_ISP_NOT_ODD              AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_NOT_ODD)              /* 0x80010187 */
#define AX_ERR_ISP_IRQ_FAIL             AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_IRQ_FAIL)             /* 0x80010188 */
#define AX_ERR_ISP_3A_STAT_GET_FAIL     AX_DEF_ERR(AX_ID_ISP, AX_ID_ISP_NULL, AX_ERR_CODE_ISP_3A_STAT_GET_FAIL)     /* 0x80010189 */

#ifdef __cplusplus
}
#endif

#endif //_AX_ISP_ERROR_CODE_H_
