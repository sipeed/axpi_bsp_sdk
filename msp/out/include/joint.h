#ifndef _AX_JOINT_H_
#define _AX_JOINT_H_

#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_interpreter_external_api.h"

#ifdef __cplusplus
extern "C" {
#endif

const AX_CHAR* AX_JOINT_GetVersion();

typedef AX_VOID* AX_JOINT_HANDLE;
typedef AX_VOID* AX_JOINT_EXECUTION_CONTEXT;

#define AX_ID_NPU_SMOD_JOINT 0x10
#define AX_ERR_NPU_JOINT_SUCCESS 0x00
#if defined(CHIP_AX620) || defined(CHIP_AX170)
#define AX_ERR_NPU_JOINT_UNKNOWN_FAILURE \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x81)
#define AX_ERR_NPU_JOINT_INVALID_PARAM \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x82)
#define AX_ERR_NPU_JOINT_INIT_FAILED \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x83)
#define AX_ERR_NPU_JOINT_MALFORMED_TOPOLOGY \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x84)
#define AX_ERR_NPU_JOINT_CREATE_CONTEXT_FAILED \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x85)
#define AX_ERR_NPU_JOINT_RUN_FAILED \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x86)
#elif defined(CHIP_AX630A)
#define AX_ERR_NPU_JOINT_UNKNOWN_FAILURE \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x01)
#define AX_ERR_NPU_JOINT_INVALID_PARAM \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x02)
#define AX_ERR_NPU_JOINT_INIT_FAILED \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x03)
#define AX_ERR_NPU_JOINT_MALFORMED_TOPOLOGY \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x04)
#define AX_ERR_NPU_JOINT_CREATE_CONTEXT_FAILED \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x05)
#define AX_ERR_NPU_JOINT_RUN_FAILED \
  AX_DEF_ERR(AX_ID_NPU, AX_ID_NPU_SMOD_JOINT, 0x06)
#endif

typedef enum _AX_JOINT_TENSOR_LAYOUT_E {
  JOINT_TENSOR_LAYOUT_UNKNOWN = 0,
  JOINT_TENSOR_LAYOUT_NHWC = 1,
  JOINT_TENSOR_LAYOUT_NCHW = 2
} AX_JOINT_TENSOR_LAYOUT_T;

typedef enum {
  AX_JOINT_MT_INVALID = 0,
  /*!
   * AX_JOINT_MT_PHYSICAL类型内存必须使用AX_SYS_Mem接口分配/释放内存空间
   * AX_JOINT_MT_VIRTUAL类型内存使用malloc(new)/free(delete)等接口分配/释放内存空间
   */
  AX_JOINT_MT_PHYSICAL = 1,
  AX_JOINT_MT_VIRTUAL = 2,
} AX_JOINT_MEMORY_TYPE_T;

typedef enum {
  AX_JOINT_DT_UNKNOWN = 0,
  AX_JOINT_DT_UINT8 = 1,
  AX_JOINT_DT_UINT16 = 2,
  AX_JOINT_DT_FLOAT32 = 3,
  AX_JOINT_DT_SINT16 = 4,
  AX_JOINT_DT_SINT8 = 5,
  AX_JOINT_DT_SINT32 = 6,
  AX_JOINT_DT_UINT32 = 7,
  AX_JOINT_DT_FLOAT64 = 8,
  // data bits stored in compact form
  AX_JOINT_DT_UINT10_PACKED = 100,
  AX_JOINT_DT_UINT12_PACKED = 101,
  AX_JOINT_DT_UINT14_PACKED = 102,
  AX_JOINT_DT_UINT16_PACKED = 103,
} AX_JOINT_DATA_TYPE_T;

typedef enum {
  AX_JOINT_CS_FEATUREMAP = 0,
  AX_JOINT_CS_RAW8 = 12,
  AX_JOINT_CS_RAW10 = 1,
  AX_JOINT_CS_RAW12 = 2,
  AX_JOINT_CS_RAW14 = 11,
  AX_JOINT_CS_RAW16 = 3,
  AX_JOINT_CS_NV12 = 4,
  AX_JOINT_CS_NV21 = 5,
  AX_JOINT_CS_RGB = 6,
  AX_JOINT_CS_BGR = 7,
  AX_JOINT_CS_RGBA = 8,
  AX_JOINT_CS_GRAY = 9,
  AX_JOINT_CS_YUV444 = 10
} AX_JOINT_COLOR_SPACE_T;

typedef enum {
  // RuntimeVar 属于 NPU 硬件特有机制，请详询技术支持
  AX_JOINT_NOT_RTV = 0,
  AX_JOINT_RTV_AFFINE = 1,
  AX_JOINT_RTV_DMA_CCM_AX630A = 2,  // deprecated
  AX_JOINT_RTV_WB_CLIP = 3,
  AX_JOINT_RTV_WB_MASK_RATIO = 4,
  AX_JOINT_RTV_DRC_PARAM = 5,  // deprecated
  AX_JOINT_RTV_WB_SRC_RATIO = 6,
  AX_JOINT_RTV_WARP_CCM = 7,
  AX_JOINT_RTV_WARP_MAT33 = 8,
  AX_JOINT_RTV_YDRC_CONTEXT = 9,
  AX_JOINT_RTV_YDRC_OFFSET = 10,
  AX_JOINT_RTV_YDRC_DITHER = 11,
  AX_JOINT_RTV_YDRC_LUT = 12,
  AX_JOINT_RTV_WAIT_IFE_DONE = 13,
  AX_JOINT_RTV_HAAR_SHRINKAGE_COEFF = 14,
  AX_JOINT_RTV_HAAR_BLEND_COEFF = 15,
  AX_JOINT_RTV_HAAR_FTZ = 16,
  AX_JOINT_RTV_HAAR_BLEND_WEIGHT = 17,
  AX_JOINT_RTV_HAAR_SHRINKAGE_WEIGHT = 18,
  AX_JOINT_RTV_HAAR_OFFSET = 19,
  AX_JOINT_RTV_WB_DRC_BASE = 20,
  AX_JOINT_RTV_WB_DRC_LUT = 21,
  AX_JOINT_RTV_HSK_OCM_WAIT_ITP_EUS = 22
} AX_JOINT_RTV_TYPE_T;

typedef struct _AX_JOINT_IOMETA_EX_T {
  AX_JOINT_COLOR_SPACE_T eColorSpace;
  AX_JOINT_RTV_TYPE_T eRtvType;
  AX_U64 u64Reserved[7];
} AX_JOINT_IOMETA_EX_T;

typedef struct _AX_JOINT_SDK_ATTR_T {
  AX_NPU_SDK_EX_HARD_MODE_T eNpuMode;
  AX_U64 u64Reserved[7];
} AX_JOINT_SDK_ATTR_T;

typedef struct _AX_JOINT_IOMETA_T {
  AX_CHAR* pName;
  AX_S32* pShape;    // YUV will be treated as 1-ch data
  AX_U8 nShapeSize;  // dimension of shape
  AX_JOINT_TENSOR_LAYOUT_T eLayout;
  AX_JOINT_MEMORY_TYPE_T eMemoryType;
  AX_JOINT_DATA_TYPE_T eDataType;
  AX_JOINT_IOMETA_EX_T* pExtraMeta;
  AX_U32 nSize;
  /*!
   * `nQuantizationValue` is the total amount of possible values
   * in quantization part of a data represented in `Q` method
   * eg. for U4Q12 data, nQuantizationValue = 2**12 = 4096
   */
  AX_U32 nQuantizationValue;
  /*!
   * when pStride is NULL, there is not stride limit
   *
   * when pStride is not NULL, it holds the number of elements in stride
   * of each dimension, the size of `pStride` always equal to `nShapeSize`
   * eg. index[0:3] shape[n,c,h,w] stride[chw,hw,w,1]
   * eg. index[0:3] shape[n,h,w,c] stride[hwc,wc,c,1]
   */
  AX_S32* pStride;
#if defined(__aarch64__)
  AX_U64 u64Reserved[9];
#elif defined(__arm__)
  AX_U64 u64Reserved[11];
#endif
} AX_JOINT_IOMETA_T;

typedef enum {
  AX_JOINT_QT_BIT_UNKNOWN = 0,
  AX_JOINT_QT_BIT_8 = 1,
  AX_JOINT_QT_BIT_16 = 2,
  AX_JOINT_QT_BIT_32 = 3,
  AX_JOINT_QT_BIT_MIX = 4
} AX_JOINT_QUANTIZATION_TYPE_T;

typedef struct _AX_JOINT_IO_INFO_T {
  AX_JOINT_IOMETA_T* pInputs;
  AX_U32 nInputSize;
  AX_JOINT_IOMETA_T* pOutputs;
  AX_U32 nOutputSize;
  AX_U32 nMaxBatchSize;       // 0 for unlimited
  AX_BOOL bDynamicBatchSize;  // if true, any batch size <= nMaxBatchSize is
                              // supported
  AX_JOINT_QUANTIZATION_TYPE_T
  eQuantizationType;  // model quantization type: BIT_8, BIT_16, BIT_32,
                      // BIT_MIX
  // FilterMode
  // 是一种软硬联合的独特设计，用于加速检测类模型的推理，使用方式请详询技术支持
  AX_BOOL bFilterMode;
#if defined(__aarch64__)
  AX_U64 u64Reserved[10];
#elif defined(__arm__)
  AX_U64 u64Reserved[12];
#endif
} AX_JOINT_IO_INFO_T;

typedef struct _AX_JOINT_IO_BUFFER_T {
  AX_ADDR phyAddr;
  AX_VOID* pVirAddr;
  AX_U32 nSize;  // total size of memory
  /*!
   * pStride holds the number of elements in stride of each dimension
   * set pStride to NULL to disable stride function
   *
   * `nStrideSize` should be equal to `nShapeSize`
   * eg. index[0:3] shape[n,c,h,w] stride[chw,hw,w,1]
   * eg. index[0:3] shape[n,h,w,c] stride[hwc,wc,c,1]
   */
  AX_S32* pStride;
  AX_U8 nStrideSize;
#if defined(__aarch64__)
  AX_U64 u64Reserved[11];
#elif defined(__arm__)
  AX_U64 u64Reserved[13];
#endif
} AX_JOINT_IO_BUFFER_T;

typedef struct _AX_JOINT_IO_SETTING_T {
  AX_U32 nWbtIndex;
  AX_U64 u64Reserved[7];
} AX_JOINT_IO_SETTING_T;

typedef struct _AX_JOINT_IO_T {
  AX_JOINT_IO_BUFFER_T* pInputs;
  AX_U32 nInputSize;
  AX_JOINT_IO_BUFFER_T* pOutputs;
  AX_U32 nOutputSize;
  AX_U32 nBatchSize;  // 0 for auto detection
  AX_JOINT_IO_SETTING_T* pIoSetting;
#if defined(__aarch64__)
  AX_U64 u64Reserved[11];
#elif defined(__arm__)
  AX_U64 u64Reserved[13];
#endif
} AX_JOINT_IO_T;

typedef struct _AX_JOINT_EXECUTION_CONTEXT_SETTING_T {
  AX_U32 nBatchSize;    // 0 for self-adaption
  AX_BOOL bNoCacheMem;  // set true to disable cached mem
  AX_U64 u64Reserved[7];
} AX_JOINT_EXECUTION_CONTEXT_SETTING_T;

typedef struct AX_JOINT_MODEL_CMM_INFO {
  AX_U32 nCMMSize;
} AX_JOINT_MODEL_CMM_INFO;

typedef struct AX_JOINT_HANDLE_SETTING_T {
  AX_CHAR* name;  // if nullptr or empty, handle name will fallback to "null",
                  // only work on AX620
  AX_U64 u64Reserved[7];
} AX_JOINT_HANDLE_SETTING_T;

/*
 * 使用 Adv_Init 接口时，请务必调用配套的 Adv_Deinit 接口
 * AX_JOINT_Adv_Init/Deinit 接口会对应调用 ax_interpreter_external_api 的
 * Init/Deinit 接口，用户无需再重复调用
 */
AX_S32 AX_JOINT_Adv_Init(AX_JOINT_SDK_ATTR_T* pAttr);
AX_S32 AX_JOINT_Adv_Deinit(AX_VOID);

AX_S32 AX_JOINT_CreateHandle(AX_JOINT_HANDLE* pHandle, const AX_VOID* pData,
                             AX_U32 nDataSize);
AX_S32 AX_JOINT_CreateHandleV2(AX_JOINT_HANDLE* pHandle, const AX_VOID* pData,
                               AX_U32 nDataSize,
                               AX_JOINT_HANDLE_SETTING_T* pSetting);
AX_S32 AX_JOINT_DestroyHandle(AX_JOINT_HANDLE handle);
const AX_JOINT_IO_INFO_T* AX_JOINT_GetIOInfo(AX_JOINT_HANDLE handle);

// CreateExecutionContext will self-adapt batch size, but may consume more RAM
AX_S32 AX_JOINT_CreateExecutionContext(AX_JOINT_HANDLE handle,
                                       AX_JOINT_EXECUTION_CONTEXT* pContext);
// use CreateExecutionContextV2 for precise memory control
AX_S32 AX_JOINT_CreateExecutionContextV2(
    AX_JOINT_HANDLE handle, AX_JOINT_EXECUTION_CONTEXT* pContext,
    AX_JOINT_EXECUTION_CONTEXT_SETTING_T* pSetting);
AX_S32 AX_JOINT_DestroyExecutionContext(AX_JOINT_EXECUTION_CONTEXT context);

AX_S32 AX_JOINT_RunSync(AX_JOINT_HANDLE handle,
                        AX_JOINT_EXECUTION_CONTEXT context, AX_JOINT_IO_T* pIO);

// 以下两个接口用于封装底层细节。可以直接调用 SYS 相关接口。
typedef enum {
  AX_JOINT_ABST_DEFAULT = 0,
  AX_JOINT_ABST_CACHED = 1,
} AX_JOINT_ALLOC_BUFFER_STRATEGY_T;
AX_S32 AX_JOINT_AllocBuffer(const AX_JOINT_IOMETA_T* pMeta,
                            AX_JOINT_IO_BUFFER_T* pBuf,
                            AX_JOINT_ALLOC_BUFFER_STRATEGY_T eStrategy);
AX_S32 AX_JOINT_FreeBuffer(AX_JOINT_IO_BUFFER_T* pBuf);

/*
 * Get model type from joint file
 * This interface can be called at any time, even if AX_JOINT_Init is not
 * called. \param - [IN]  pJoint: joint file data \param - [IN]  nJointSize:
 * joint file size \param - [OUT]  pModelType: model type
 */
AX_S32 AX_JOINT_GetJointModelType(const AX_CHAR* pJoint, AX_U32 nJointSize,
                                  AX_NPU_SDK_EX_MODEL_TYPE_T* pModelType);
AX_S32 AX_JOINT_GetVNPUMode(AX_JOINT_HANDLE handle,
                            AX_NPU_SDK_EX_MODEL_TYPE_T* pModelType);
AX_S32 AX_JOINT_GetVNPUHardMode(AX_NPU_SDK_EX_HARD_MODE_T* pHardMode);
const AX_CHAR* AX_JOINT_GetModelToolsVersion(AX_JOINT_HANDLE handle);

/*
 * Get CMM usage info (in Byte) from handle
 */
AX_S32 AX_JOINT_GetCMMUsage(AX_JOINT_HANDLE handle,
                            AX_JOINT_MODEL_CMM_INFO* cmm_info);

#ifdef __cplusplus
}
#endif

#endif
