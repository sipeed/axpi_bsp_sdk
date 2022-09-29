/***************************************************************************
 *
 * Copyright (c) 2019-2020 Aichip Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Aichip Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Aichip Technology Co., Ltd.
 *
 ****************************************************************************/

#ifndef __AX_AX_S32ERPRETER_ADVANCED_API_EXTERNAL_H
#define __AX_AX_S32ERPRETER_ADVANCED_API_EXTERNAL_H

#include "ax_interpreter_external_api.h"
#include "ax_interpreter_external2_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct {
    AX_NPU_SDK_EX_BUF_T *pBuf;
    AX_U64 u64Reserved[7];
} AX_NPU_SDK_EX_ADV_BUF_T;

typedef struct {
    AX_NPU_SDK_EX_ADV_BUF_T *pInputs;
    AX_U32 nInputSize;
    AX_NPU_SDK_EX_ADV_BUF_T *pOutputs;
    AX_U32 nOutputSize;
    AX_U32 nWbtIndex;
    AX_NPU_SDK_EX_RUNTIME_VAR_T pRuntimeVar;
    AX_U32 nRuntimeVarSize;
#if defined(__aarch64__)
    AX_U64 u64Reserved[10];
#elif defined(__arm__)
    AX_U64 u64Reserved[12];
#endif
} AX_NPU_SDK_EX_ADV_IO_T;

typedef struct {
    AX_NPU_SDK_EX_ADV_IO_T tIo;
    AX_NPU_SDK_EX_FINISH_FUNC fnFinishFunc;
    AX_VOID *pUserData;
#if defined(__aarch64__)
    AX_U64 u64Reserved[14];
#elif defined(__arm__)
    AX_U64 u64Reserved[15];
#endif
} AX_NPU_SDK_EX_ADV_RESOURCE_T;

typedef enum {
    AX_NPU_TDT_BYTE = 0,
    AX_NPU_TDT_UINT8 = 1,
    AX_NPU_TDT_UINT16 = 2,
    AX_NPU_TDT_INT16 = 3,
    AX_NPU_TDT_INT32 = 4,
    AX_NPU_TDT_INT64 = 5,
    AX_NPU_TDT_FLOAT32 = 6,
    AX_NPU_TDT_FLOAT64 = 7,
} AX_NPU_SDK_EX_ADV_TENSOR_DTYPE;

typedef struct {
    AX_NPU_SDK_EX_TENSOR_META_T *pTensorMeta;
    AX_NPU_SDK_EX_ADV_TENSOR_DTYPE eDType;
#if defined(__aarch64__)
    AX_U64 u64Reserved[6];
#elif defined(__arm__)
    AX_U64 u64Reserved[7];
#endif
} AX_NPU_SDK_EX_ADV_TENSOR_META_T;

typedef struct {
    AX_NPU_SDK_EX_ADV_TENSOR_META_T *pInputs;
    AX_U32 nInputSize;
    AX_NPU_SDK_EX_ADV_TENSOR_META_T *pOutputs;
    AX_U32 nOutputSize;
    AX_S8** pWbtNames;
    AX_U32 nWbtNum;
    AX_BOOL bHasRuntimeVar;
#if defined(__aarch64__)
    AX_U64 u64Reserved[10];
#elif defined(__arm__)
    AX_U64 u64Reserved[12];
#endif
} AX_NPU_SDK_EX_ADV_IO_INFO_T;

AX_S32 AX_NPU_SDK_EX_ADV_Run_task_async(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_ADV_RESOURCE_T *resource, AX_NPU_SDK_EX_TASK_ID_T *taskId);
AX_S32 AX_NPU_SDK_EX_ADV_Run_task_sync(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_ADV_IO_T *io);

AX_S32 AX_NPU_SDK_EX2_ADV_RunTaskSync(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext, AX_NPU_SDK_EX_ADV_IO_T *pIO);

const AX_NPU_SDK_EX_ADV_IO_INFO_T* AX_NPU_SDK_EX_ADV_Get_io_info(AX_NPU_SDK_EX_HANDLE_T handle);

AX_S32 AX_NPU_SDK_EX_ADV_Make_io_buffer(AX_ADDR phyAddr, AX_VOID *pVirAddr, AX_U32 nSize,
    AX_ADDR innerPhyAddr, AX_VOID *pInnerVirAddr, AX_U32 nInnerSize, AX_NPU_SDK_EX_BUF_T *buf);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
