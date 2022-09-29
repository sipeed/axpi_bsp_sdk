/***************************************************************************
 *
 * Copyright (c) 2019-2020 Aichip Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Aichip Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Aichip Technology Co., Ltd.
 *
 ****************************************************************************/

#ifndef AX_INTERPRETER_EXTERNAL2_API_H
#define AX_INTERPRETER_EXTERNAL2_API_H

#include "ax_interpreter_external_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef AX_VOID* AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T;

typedef enum _AX_NPU_SDK_EX2_BATCH_TYPE {
    AX_NPU_BATCH_TYPE_INVALID = 0,
    AX_NPU_BATCH_TYPE_BLOCK = 1,
    AX_NPU_BATCH_TYPE_GATHER = 2
} AX_NPU_SDK_EX2_BATCH_TYPE_E;

typedef struct _AX_NPU_SDK_EX2_BATCH_INFO {
    AX_NPU_SDK_EX2_BATCH_TYPE_E eBatchType;
    AX_U32 nBatchSize;
    AX_U64 u64Reserved[8];
} AX_NPU_SDK_EX2_BATCH_INFO_T;

AX_S32 AX_NPU_SDK_EX2_GetMaxBatchSize(AX_NPU_SDK_EX_HANDLE_T hHandle, AX_U32 *pMaxBatchSize);
AX_BOOL AX_NPU_SDK_EX2_IsSupportDynamicBatch(AX_NPU_SDK_EX_HANDLE_T hHandle);

AX_S32 AX_NPU_SDK_EX2_CreateExecutionContext(AX_NPU_SDK_EX_HANDLE_T hHandle, AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T *pContext, const AX_NPU_SDK_EX2_BATCH_INFO_T *pBatchInfo);
AX_S32 AX_NPU_SDK_EX2_DestroyExecutionContext(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext);

AX_S32 AX_NPU_SDK_EX2_SetBatchInfo(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext, const AX_NPU_SDK_EX2_BATCH_INFO_T *pBatchInfo);
AX_S32 AX_NPU_SDK_EX2_GetBatchInfo(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext, AX_NPU_SDK_EX2_BATCH_INFO_T *pBatchInfo);
const AX_NPU_SDK_EX_IO_INFO_T* AX_NPU_SDK_EX2_GetIOInfo(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext);

AX_S32 AX_NPU_SDK_EX2_RunTaskAsync(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext,
    AX_NPU_SDK_EX_RESOURCE_T *pResource,
    AX_NPU_SDK_EX_TASK_ID_T *pTaskId,
    AX_NPU_SDK_EX_ExtraParameters *pExtraParameters);
AX_S32 AX_NPU_SDK_EX2_RunTaskSync(AX_NPU_SDK_EX2_EXECUTION_CONTEXT_T hContext, AX_NPU_SDK_EX_IO_T *pIO, AX_NPU_SDK_EX_ExtraParameters *pExtraParameters);

#ifdef __cplusplus
}
#endif

#endif
