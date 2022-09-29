/***************************************************************************
 *
 * Copyright (c) 2019-2020 Aichip Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Aichip Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Aichip Technology Co., Ltd.
 *
 ****************************************************************************/

#ifndef __AX_AX_S32ERPRETER_API_EXTERNAL_H
#define __AX_AX_S32ERPRETER_API_EXTERNAL_H

#include <stdint.h>
#include "ax_base_type.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define AX_NPU_DEV_STATUS_SUCCESS          (0)
#define AX_NPU_DEV_STATUS_HANDLE_INVALID   (0x80060081)
#define AX_NPU_DEV_STATUS_TASK_INVALID     (0x80060082)
#define AX_NPU_DEV_STATUS_NO_INIT          (0x80060083)
#define AX_NPU_DEV_STATUS_PARAM_INVALID    (0x80060084)
#define AX_NPU_DEV_STATUS_NO_RESOURCES     (0x80060085)
#define AX_NPU_DEV_STATUS_MEM_ERROR        (0x80060086)
#define AX_NPU_DEV_STATUS_HARD_ERROR       (0x80060087)
#define AX_NPU_DEV_STATUS_NOT_SUPPORT      (0x80060088)
#define AX_NPU_DEV_STATUS_TASK_BUSY        (0x80060089)

#define AX_NPU_TASK_STATUS_SUCCESS         (0)
#define AX_NPU_TASK_STATUS_FAILED          (0x8006008a)


typedef AX_VOID* AX_NPU_SDK_EX_HANDLE_T;
typedef AX_U32 AX_NPU_SDK_EX_TASK_ID_T;
typedef AX_VOID(*AX_NPU_SDK_EX_FINISH_FUNC)(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_TASK_ID_T taskId, AX_U32 error_code, AX_VOID *userData);

typedef AX_VOID* AX_NPU_SDK_EX_RUNTIME_VAR_T;
typedef AX_VOID* AX_NPU_SDK_EX_PREPROCESS_MICRO_CODE_T;

typedef struct {
    AX_ADDR phyAddr;
    AX_VOID *pVirAddr;
    AX_U32 nSize;

    AX_ADDR innerPhyAddr;
    AX_VOID *pInnerVirAddr;
    AX_U32 nInnerSize;
} AX_NPU_SDK_EX_BUF_T;

typedef struct {
    AX_NPU_SDK_EX_BUF_T *pInputs;
    AX_U32 nInputSize;
    AX_NPU_SDK_EX_BUF_T *pOutputs;
    AX_U32 nOutputSize;
} AX_NPU_SDK_EX_IO_T;

typedef struct {
    AX_NPU_SDK_EX_IO_T tIo;
    AX_NPU_SDK_EX_FINISH_FUNC fnFinishFunc;
    AX_VOID *pUserData;
} AX_NPU_SDK_EX_RESOURCE_T;

typedef enum {
    AX_NPU_MT_INVALID = 0,
    AX_NPU_MT_PHYSICAL = 1,
    AX_NPU_MT_VIRTUAL = 2
} AX_NPU_SDK_EX_MEMORY_TYPE_T;

typedef struct {
    AX_S8 *pName;
    AX_U32 *pShape;
    AX_U8 nShapeNDim;
    AX_U32 nBit;
    AX_U32 nInnerBit;
    AX_U32 nSize;
    AX_U32 nInnerSize;
    AX_NPU_SDK_EX_MEMORY_TYPE_T eMemoryType;
    AX_S32 nPreallocOCMStartAddr;
    AX_S32 nPreallocOCMEndAddr;
} AX_NPU_SDK_EX_TENSOR_META_T;

typedef struct {
    AX_NPU_SDK_EX_TENSOR_META_T *pInputs;
    AX_U32 nInputSize;
    AX_NPU_SDK_EX_TENSOR_META_T *pOutputs;
    AX_U32 nOutputSize;
} AX_NPU_SDK_EX_IO_INFO_T;

typedef enum {
    AX_NPU_ABST_DEFAULT = 0,
    AX_NPU_ABST_CACHED = 1,
} AX_NPU_SDK_EX_ALLOC_BUFFER_STRATEGY_T;

typedef enum {
    AX_NPU_VIRTUAL_DISABLE = 0,
    AX_NPU_VIRTUAL_3_1 = 1,
    AX_NPU_VIRTUAL_2_2 = 2,
    AX_NPU_VIRTUAL_1_1 = 3
} AX_NPU_SDK_EX_HARD_MODE_T;

typedef enum {
    AX_NPU_EX_DDR_BW_LIMIT_DISABLE = 0,
    AX_NPU_EX_DDR_BW_LIMIT_2P1GB = 1,
    AX_NPU_EX_DDR_BW_LIMIT_2P8GB = 2,
    AX_NPU_EX_DDR_BW_LIMIT_3P5GB = 3,
    AX_NPU_EX_DDR_BW_LIMIT_4P1GB = 4,
    AX_NPU_EX_DDR_BW_LIMIT_4P8GB = 5,
    AX_NPU_EX_DDR_BW_LIMIT_5P5GB = 6,
    AX_NPU_EX_DDR_BW_LIMIT_6P2GB = 7,
    AX_NPU_EX_DDR_BW_LIMIT_6P9GB = 8,
    AX_NPU_EX_DDR_BW_LIMIT_7P6GB = 9,
    AX_NPU_EX_DDR_BW_LIMIT_8P3GB = 10,
    AX_NPU_EX_DDR_BW_LIMIT_9P0GB = 11,
    AX_NPU_EX_DDR_BW_LIMIT_9P9GB = 12,
    AX_NPU_EX_DDR_BW_LIMIT_MAX
} AX_NPU_SDK_EX_DDR_LIM_T;

typedef struct {
    AX_NPU_SDK_EX_HARD_MODE_T eHardMode;
} AX_NPU_SDK_EX_ATTR_T;

typedef struct {
    AX_NPU_SDK_EX_HARD_MODE_T eHardMode;
    AX_NPU_SDK_EX_DDR_LIM_T eDdrBwLimit;
#if defined(__aarch64__)
    AX_U64 u64Reserved[8];
#elif defined(__arm__)
    AX_U64 u64Reserved[8];
#endif
} AX_NPU_SDK_EX_ATTR_V1_T;

typedef enum {
    AX_NPU_MODEL_TYPE_DEFUALT = 0,
    AX_NPU_MODEL_TYPE_3_1_1 = 1,
    AX_NPU_MODEL_TYPE_3_1_2 = 2,
    AX_NPU_MODEL_TYPE_2_2_1 = 3,
    AX_NPU_MODEL_TYPE_2_2_2 = 4,
    AX_NPU_MODEL_TYPE_1_1_1 = 5,
    AX_NPU_MODEL_TYPE_1_1_2 = 6
} AX_NPU_SDK_EX_MODEL_TYPE_T;

typedef struct {
    uint64_t total_cycle;
    uint32_t eu_cycle[9];
    uint32_t valid_eu_num;
} AX_NPU_SDK_EX_WORK_CYCLE_T;

typedef enum {
    AX_NPU_RTV_AFFINE = 1,
    AX_NPU_RTV_DMA_CCM_AX630A = 2,
    AX_NPU_RTV_WB_CLIP = 3,
    AX_NPU_RTV_MASK_RATIO = 4,
    AX_NPU_RTV_DRC_PARAM = 5,
    AX_NPU_RTV_SRC_RATIO = 6,
    AX_NPU_RTV_WARP_CCM = 7,
    AX_NPU_RTV_WARP_MAT33 = 8,
    AX_NPU_RTV_YDRC_CONTEXT = 9,
    AX_NPU_RTV_YDRC_OFFSET = 10,
    AX_NPU_RTV_YDRC_DITHER = 11,
    AX_NPU_RTV_YDRC_LUT = 12,
    AX_NPU_RTV_WAIT_IFE_DONE = 13,
    AX_NPU_RTV_HAAR_SHRINKAGE_COEFF = 14,
    AX_NPU_RTV_HAAR_BLEND_COEFF = 15,
    AX_NPU_RTV_HAAR_FTZ = 16,
    AX_NPU_RTV_HAAR_BLEND_WEIGHT = 17,
    AX_NPU_RTV_HAAR_SHRINKAGE_WEIGHT = 18,
    AX_NPU_RTV_HAAR_OFFSET = 19,
    AX_NPU_RTV_HSK_OCM_WAIT_ITP_EUS = 20
} AX_NPU_SDK_EX_RTV_TYPE_T;

typedef struct {
    AX_NPU_SDK_EX_RTV_TYPE_T eType;
    AX_S32 nIndex;
    AX_S8 *pName;
    AX_U64 u64Reserved[8];
} AX_NPU_SDK_EX_RTV_META_T;

typedef struct {
    AX_NPU_SDK_EX_RTV_META_T *pMeta;
    AX_U32 nMetaSize;
    AX_U64 u64Reserved[8];
} AX_NPU_SDK_EX_RTV_INFO_T;


typedef struct {
    AX_NPU_SDK_EX_PREPROCESS_MICRO_CODE_T tPreprocessMicroCode;
    AX_NPU_SDK_EX_RUNTIME_VAR_T tRuntimeVar;
    AX_U32 nWbtIndex;
    AX_U32 nRuntimeVarSize;
#if defined(__aarch64__)
    AX_U64 u64Reserved[8];
#elif defined(__arm__)
    AX_U64 u64Reserved[9];
#endif
} AX_NPU_SDK_EX_ExtraParameters;

typedef struct {
    AX_BOOL bHasPreprocessMicroCode;
    AX_BOOL bHasRuntimeVar;
    AX_U32 nWbtNum;
#if defined(__aarch64__)
    AX_U64 u64Reserved[8];
#elif defined(__arm__)
    AX_U64 u64Reserved[8];
#endif
} AX_NPU_SDK_EX_ExtraParametersInfo;

typedef struct {
    AX_U32 nModelMcodeSize;
    AX_U32 nCvPreProcessMcodeSize;
    AX_U32 nWbtSize;
    AX_U32 nRingbufferSize;
#if defined(__aarch64__)
    AX_U64 u64Reserved[8];
#elif defined(__arm__)
    AX_U64 u64Reserved[8];
#endif
} AX_NPU_SDK_EX_MODEL_CMM_INFO_T;

const AX_CHAR* AX_NPU_SDK_EX_GetVersion();
AX_S32 AX_NPU_SDK_EX_Init(AX_VOID);
AX_VOID AX_NPU_SDK_EX_Deinit(AX_VOID);
AX_S32 AX_NPU_SDK_EX_Create_handle(AX_NPU_SDK_EX_HANDLE_T *handle, const AX_VOID *dotNeuAddr, AX_S32 dotNeuLen);
AX_S32 AX_NPU_SDK_EX_Create_handle_prealloc_wbt(
    AX_NPU_SDK_EX_HANDLE_T *handle, const AX_VOID *dotNeuAddr, AX_S32 dotNeuLen,
    AX_U64 wbt_phy, const AX_VOID* wbt_vir, AX_S32 wbt_size
);
AX_S32 AX_NPU_SDK_EX_Create_handle_prealloc_wbt_v2(
    AX_NPU_SDK_EX_HANDLE_T *handle, const AX_VOID *dotNeuAddr, AX_S32 dotNeuLen,
    AX_U64 wbt_phy, const AX_VOID* wbt_vir, AX_S32 wbt_size,
    const AX_S8 *name);
AX_S32 AX_NPU_SDK_EX_Run_task_async(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_RESOURCE_T *resource, AX_NPU_SDK_EX_TASK_ID_T *taskId);
AX_S32 AX_NPU_SDK_EX_Run_task_sync(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_IO_T *io);

// run task v2 deprecated, please use v3
AX_S32 AX_NPU_SDK_EX_Run_task_async_v2(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_PREPROCESS_MICRO_CODE_T preprocessMicroCode, AX_NPU_SDK_EX_RESOURCE_T *resource, AX_NPU_SDK_EX_TASK_ID_T *taskId);
AX_S32 AX_NPU_SDK_EX_Run_task_sync_v2(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_PREPROCESS_MICRO_CODE_T preprocessMicroCode, AX_NPU_SDK_EX_IO_T *io);

AX_S32 AX_NPU_SDK_EX_Run_task_async_v3(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_RESOURCE_T *resource, AX_NPU_SDK_EX_TASK_ID_T *taskId, AX_NPU_SDK_EX_ExtraParameters *pExtraParamters);
AX_S32 AX_NPU_SDK_EX_Run_task_sync_v3(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_IO_T *io, AX_NPU_SDK_EX_ExtraParameters *pExtraParamters);

AX_S32 AX_NPU_SDK_EX_Cancel_task(AX_NPU_SDK_EX_TASK_ID_T taskId);
AX_S32 AX_NPU_SDK_EX_Destroy_handle(AX_NPU_SDK_EX_HANDLE_T handle);
AX_VOID AX_NPU_SDK_EX_Hard_reset(AX_VOID);
AX_S32 AX_NPU_SDK_EX_Init_with_attr(AX_NPU_SDK_EX_ATTR_T *pNpuAttr);
AX_S32 AX_NPU_SDK_EX_Init_with_attr_v1(AX_NPU_SDK_EX_ATTR_V1_T *pNpuAttr_v1);
AX_S32 AX_NPU_SDK_EX_Get_Attr(AX_NPU_SDK_EX_ATTR_T *pNpuAttr);

AX_S32 AX_NPU_SDK_EX_Alloc_buffer(AX_NPU_SDK_EX_TENSOR_META_T* meta, AX_NPU_SDK_EX_BUF_T *buf, AX_NPU_SDK_EX_ALLOC_BUFFER_STRATEGY_T strategy);
AX_S32 AX_NPU_SDK_EX_Free_buffer(AX_NPU_SDK_EX_BUF_T *buf);
const AX_NPU_SDK_EX_IO_INFO_T* AX_NPU_SDK_EX_Get_io_info(AX_NPU_SDK_EX_HANDLE_T handle);
AX_S32 AX_NPU_SDK_EX_Get_Model_type(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_MODEL_TYPE_T *pModelType);
AX_S32 AX_NPU_SDK_EX_Get_Dot_neu_type(const AX_VOID *dotNeuAddr, AX_S32 dotNeuLen, AX_NPU_SDK_EX_MODEL_TYPE_T *pModelType);
AX_S32 AX_NPU_SDK_EX_Get_Work_Cycle(AX_NPU_SDK_EX_WORK_CYCLE_T *pNpuCycle);

const AX_NPU_SDK_EX_RTV_INFO_T* AX_NPU_SDK_EX_Get_RTV_info(AX_NPU_SDK_EX_HANDLE_T handle);

AX_S32 AX_NPU_SDK_EX_Get_extra_parameters_info(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_ExtraParametersInfo *pExtraInfo);
AX_S32 AX_NPU_SDK_EX_Get_wbt_params_list(AX_NPU_SDK_EX_HANDLE_T handle, AX_CHAR*** pNames, AX_U32* pNameSize);
AX_S32 AX_NPU_SDK_EX_Get_model_cmm_info(AX_NPU_SDK_EX_HANDLE_T handle, AX_NPU_SDK_EX_MODEL_CMM_INFO_T *pCmmInfo);
AX_S32 AX_NPU_SDK_EX_Get_model_vm_consumption(AX_NPU_SDK_EX_HANDLE_T handle, AX_U32 *pPeak, AX_U32 *pPermanent);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
