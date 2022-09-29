/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef __AX_NPU_IMGPROC_H__
#define __AX_NPU_IMGPROC_H__

#include "./npu_common.h"
#include "ax_interpreter_external_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * 函数功能描述:
 * <br/>获取 NPU-CV KIT 版本号。
 */
const AX_S8* AX_NPU_CV_Version();

/*!
 * 函数功能描述:
 * <br/>实现图像的 Crop 操作。为了高效灵活的使用，支持同时从src图像中处理多个子图，支持选择虚拟NPU模式类别
 * \warning 支持处理：NV12/NV21/BGR/RGB/GRAY；支持4K分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: src图像指针
 * \param nDestSize: 子图数量
 * \param pDestImages: dst图像指针数组，可以同时输出多个结果
 * \param pBoxes: 各子图bounding box指针数组，NV12/NV21需要fW、fH为偶数。如果`pBoxes[i]`或`pBoxes`为空则表示不做crop操作。
 */
int AX_NPU_CV_CropImage(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Image* pSrc,
        const AX_U32 nDestSize,
        AX_NPU_CV_Image** pDestImages,
        AX_NPU_CV_Box** pBoxes);

typedef enum _AX_NPU_CV_ImageResizeAlignParam {
    AX_NPU_CV_IMAGE_FORCE_RESIZE = 0, // without keep aspect ratio, others keep aspect ratio
    AX_NPU_CV_IMAGE_HORIZONTAL_LEFT = 1, // border on the right of the image
    AX_NPU_CV_IMAGE_HORIZONTAL_CENTER = 2, // border on both sides of the image
    AX_NPU_CV_IMAGE_HORIZONTAL_RIGHT = 3, // border on the left of the image
    AX_NPU_CV_IMAGE_VERTICAL_TOP = AX_NPU_CV_IMAGE_HORIZONTAL_LEFT, // border on the bottom of the image
    AX_NPU_CV_IMAGE_VERTICAL_CENTER = AX_NPU_CV_IMAGE_HORIZONTAL_CENTER, // border on both sides of the image
    AX_NPU_CV_IMAGE_VERTICAL_BOTTOM = AX_NPU_CV_IMAGE_HORIZONTAL_RIGHT, // border on the top of the image
} AX_NPU_CV_ImageResizeAlignParam;

/*!
 * 函数功能描述:
 * <br/>实现crop然后在保持纵横比的前提下，从给定的图像resize为相同的形状。
 * <br/>支持选择虚拟NPU模式类别。
 * \warning 支持处理：NV12/NV21/BGR/RGB/GRAY；支持4K分辨率；水平或垂直方向缩小比例要小于32。
 * \warning 在较大输入/输出分辨率场景，可能会触发硬件限制条件，报错时会打印详细参数信息。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: src图像指针，支持配置宽度方向stride，若为0则默认stride为宽度大小
 * \param nDestSize: 子图数量
 * \param pDestImages: dst图像指针数组,可以同时输出多个结果，但图像shape相同；支持配置宽度方向stride，若为0则默认stride为宽度大小
 * \param pBoxes: 各子图bounding_box指针数组，NV12/NV21/RGB/BGR需要fW、fH为偶数。如果`pBoxes[i]`或`pBoxes`为空则表示不做crop操作
 * \param eImageResizeAlignParamHorizontal: 输出图像水平对齐方式
 * \param eImageResizeAlignParamVertical: 输出图像纵向对齐方式
 * \param tColor: 用于按指定颜色填充border
 */
int AX_NPU_CV_CropResizeImage(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Image* pSrc,
        const AX_U32 nDestSize,
        AX_NPU_CV_Image** pDestImages,
        AX_NPU_CV_Box** pBoxes,
        const AX_NPU_CV_ImageResizeAlignParam eImageResizeAlignParamHorizontal,
        const AX_NPU_CV_ImageResizeAlignParam eImageResizeAlignParamVertical,
        const AX_NPU_CV_Color tColor);

/*!
 * 函数功能描述:
 * <br/>实现Y和UV分离的扣图和缩放操作，从给定的图像resize为相同的形状。
 * <br/>输入/输出图像尺寸、stride以及box坐标需要设置为偶数。
 * <br/>支持选择虚拟NPU模式类别。
 * \warning 支持处理Y和UV分离的NV12/NV21图像；支持4K分辨率；水平或垂直方向缩小比例要小于32。
 * \warning 在较大输入/输出分辨率场景，可能会触发硬件限制条件，报错时会打印详细参数信息。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrcY: 输入Y图像指针，支持配置宽度方向stride，若为0则默认stride为宽度大小
 * \param pSrcUV: 输入UV图像指针，图像宽度、高度和stride与输入Y图像保持一致
 * \param nDestSize: 子图数量
 * \param pDestImagesY: 输出Y图像指针数组,可以同时输出多个结果，但图像shape相同；支持配置宽度方向stride，若为0则默认stride为宽度大小
 * \param pDestImagesUV: 输出UV图像指针数组,可以同时输出多个结果，但图像shape相同；输出UV图像宽度、高度和stride与输出Y图像保持一致
 * \param pBoxes: 各子图bounding_box指针数组，要求fW、fH为偶数。如果`pBoxes[i]`为空则表示不做crop操作
 * \param eImageResizeAlignParamHorizontal: 输出图像水平对齐方式
 * \param eImageResizeAlignParamVertical: 输出图像纵向对齐方式
 * \param tColor: 用于按指定颜色填充border
 */
int AX_NPU_CV_CropResizeImageForSplitYUV(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Image* pSrcY,
        const AX_NPU_CV_Image* pSrcUV,
        const AX_U32 nDestSize,
        AX_NPU_CV_Image** pDestImagesY,
        AX_NPU_CV_Image** pDestImagesUV,
        AX_NPU_CV_Box** pBoxes,
        const AX_NPU_CV_ImageResizeAlignParam eImageResizeAlignParamHorizontal,
        const AX_NPU_CV_ImageResizeAlignParam eImageResizeAlignParamVertical,
        const AX_NPU_CV_Color tColor);

/*!
 * 定义 AX_NPU_CV_AlphaBlendingWithMask 算子上下文
*/
typedef AX_VOID* AX_NPU_CV_AlphaBlendingContext;

/*!
 * 函数功能描述:
 * <br/>实现前景和背景为NV12/NV21/BGR/RGB的图像按Gray Mask做blending操作。
 * <br/>背景图为整图，前景图可以只和背景图部分区域相对应，此时需要指定前景图相对背景图区域的偏移坐标。
 * <br/>blending功能完成后将结果overlay到背景图相应区域。
 * \warning 背景图做blending区域不能超出图像实际范围。
 * \warning 程序退出前，需要调用 AX_NPU_CV_DestroyAlphaBlendingContext 释放算子上下文。
 * \param pContext: 算子上下文指针。如果 `*pContext` 为空指针, 算子内部会自动分配内存。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pBackground: 背景图像指针，支持NV12/NV21/BGR/RGB
 * \param pForeground: 前景图像指针，支持NV12/NV21/BGR/RGB,色彩空间与背景图一致
 * \param pMask: Gray图像指针，Mask图像宽、高与前景图宽、高不一致时，算子内部会resize到一致。alpha值范围为[0,128]来表示小数[0,1]。
 * \param nOffsetW: 指定背景图blending区域起始位置宽度
 * \param nOffsetH: 指定背景图blending区域起始位置高度
 */
int AX_NPU_CV_AlphaBlendingWithMask(AX_NPU_CV_AlphaBlendingContext* pContext,
        const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        AX_NPU_CV_Image* pBackground, const AX_NPU_CV_Image* pForeground,
        const AX_NPU_CV_Image* pMask, AX_U32 nOffsetW, AX_U32 nOffsetH);

/*!
 * 函数功能描述:
 * <br/>实现前景和背景为NV12/NV21/BGR/RGB的图像按Gray Mask做blending操作。
 * <br/>背景图为整图，前景图可以只和背景图部分区域相对应，此时需要指定前景图相对背景图区域的偏移坐标。
 * <br/>blending功能完成后将结果输出到目标地址相应区域。
 * \warning 背景图做blending区域不能超出图像实际范围。
 * \warning 程序退出前，需要调用 AX_NPU_CV_DestroyAlphaBlendingContext 释放算子上下文。
 * \param pContext: 算子上下文指针。如果 `*pContext` 为空指针, 算子内部会自动分配内存。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pBackground: 背景图像指针，支持NV12/NV21/BGR/RGB
 * \param pForeground: 前景图像指针，支持NV12/NV21/BGR/RGB,色彩空间与背景图一致
 * \param pMask: Gray图像指针，Mask图像宽、高与前景图宽、高不一致时，算子内部会resize到一致。alpha值范围为[0,128]来表示小数[0,1]。
 * \param pDst: 输出图像指针，支持NV12/NV21/BGR/RGB
 * \param nOffsetW: 指定背景图blending区域起始位置宽度
 * \param nOffsetH: 指定背景图blending区域起始位置高度
 */
int AX_NPU_CV_AlphaBlendingWithMaskV2(AX_NPU_CV_AlphaBlendingContext* pContext,
        const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Image* pBackground, const AX_NPU_CV_Image* pForeground,
        const AX_NPU_CV_Image* pMask, AX_NPU_CV_Image* pDst, AX_U32 nOffsetW, AX_U32 nOffsetH);

/*!
* 函数功能描述:
   <br/>销毁 AX_NPU_CV_AlphaBlendingWithMask 算子上下文
 * \param tContext: AX_NPU_CV_AlphaBlendingWithMask 算子上下文
 */
void AX_NPU_CV_DestroyAlphaBlendingContext(AX_NPU_CV_AlphaBlendingContext tContext);

/*!
 * 函数功能描述:
  <br/>实现不同色彩空间转换
  <br/>|++ from +++|++ to +++|
  <br/>NV12/NV21==>>BGR/YUV444/LAB
  <br/>YUV444==>>GRAY
  <br/>RAW10==>>RAW16
  <br/>RAW12==>RAW16
  <br/>RGB==>> BGR/LAB
  <br/>BGR==>>NV12/YUV444/RGB
  <br/>YUYV/UYVY==>>NV12/NV21
  <br/>YUV420_LEGACY==>>NV12
 * \warning 请确保输入图像和输出图像尺寸相同；支持4K分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入图像指针
 * \param pDst: 输出图像指针
 */
int AX_NPU_CV_CSC(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Image* pSrc, AX_NPU_CV_Image* pDst);

/*!
* 函数功能描述:
   <br/>实现矩阵乘：C = alphaAB^T，支持选择虚拟 NPU 模式类别
   <br/>矩阵 A 和 B 数据类型一致，当输入矩阵 A 数据类型为 int8 时，输出矩阵 C 数据类型为 int16；当输入矩阵 A 数据类型为 float 时，输出矩阵 C 数据类型为 float；
 * \warning N 和 K 数值不宜过大，否则硬件不支持；
 * \param eVirtualNpuMode: 支持虚拟NPU模式类别：disable 和 111
 * \param fAlpha: 矩阵乘的系数
 * \param pA: 矩阵 A(MxK)，支持 int8, float
 * \param pB: 矩阵 B(NxK)，支持 int8, float
 * \param pC: 矩阵 C(MxN), 矩阵乘的结果，支持 int16，float；需要用户分配内存
 */
int AX_NPU_CV_MatMul(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_F32 fAlpha, const AX_NPU_CV_Matrix2D* pA, const AX_NPU_CV_Matrix2D* pB, AX_NPU_CV_Matrix2D* pC);


/*!
 * 定义 AX_NPU_CV_Laplacian 算子上下文
*/
typedef AX_VOID* AX_NPU_CV_LaplacianContext;

typedef struct _AX_NPU_CV_LaplacianParams {
    AX_S8 nKernelValue[9];
} AX_NPU_CV_LaplacianParams;

/*!
 * 函数功能描述:
 * <br/>实现对输入矩阵进行拉普拉斯变换操作。
 * <br/>输入矩阵行和列数要求2对齐；最小支持 2x32；最大支持 4K 分辨率。
 * \warning 程序退出前，需要调用 AX_NPU_CV_DestroyLaplacianContext 释放算子上下文。
 * \param pContext: 算子上下文指针。如果 `*pContext` 为空指针, 算子内部会自动分配内存。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入矩阵，支持 uint8
 * \param pDst: 输出矩阵，支持 uint16/int16；需要用户分配内存
 * \param pLaplacianParams: 拉普拉斯变换模版系数；如果系数不需要更新,`pLaplacianParams`可给空指针，算子内部会使用`*Context`中的参数来计算；
 */
int AX_NPU_CV_Laplacian(AX_NPU_CV_LaplacianContext* pContext, const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst, AX_NPU_CV_LaplacianParams* pLaplacianParams);

/*!
* 函数功能描述:
   <br/>销毁 AX_NPU_CV_Laplacian 算子上下文
 * \param tContext: AX_NPU_CV_Laplacian 算子上下文
 */
void AX_NPU_CV_DestroyLaplacianContext(AX_NPU_CV_LaplacianContext tContext);

/*!
 * 函数功能描述:
 * <br/>实现对输入矩阵的像素值乘以比例系数。
 * <br/>输入矩阵行和列数要求2对齐；最小支持 2x32；最大支持 4K 分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入矩阵，支持 uint16/int16；
 * \param pDst: 输出矩阵，支持 uint16/int16；需要用户分配内存
 * \param fScaleRatio: 比例系数, 支持范围[-8, 8)
 */
int AX_NPU_CV_MultRatio(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst, AX_F64 fScaleRatio);

/*!
 * 函数功能描述:
 * <br/>实现两个输入矩阵中对应位置元素相乘
 * <br/>输入矩阵行和列数要求2对齐；最小支持 2x32；最大支持 4K 分辨率。
 * \warning 由于虚拟NPU模式下，NPU内部计算单元近似取整方式不同，1_1_1模式下为ROUND_TO_EVEN, 1_1_2和disable模式下为四舍五入，故相同输入在不同虚拟NPU模式下结果会略有差异。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc0: 输入矩阵，支持 uint16/int16；
 * \param pSrc1: 输入矩阵，支持 uint8,实际含义为u1.7,即1bit整数，7bit小数；
 * \param pDst: 输出矩阵，支持 uint16/int16；需要用户分配内存
 */
int AX_NPU_CV_HadamardProduct(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Matrix2D* pSrc0, const AX_NPU_CV_Matrix2D* pSrc1, AX_NPU_CV_Matrix2D* pDst);

/*!
 * AX_NPU_CV_Thresh 算子模式
*/
typedef enum {
    AX_NPU_CV_THRESH_MODE_BINARY = 0, /*!<src <= iLowThresh, dst = iMinValue; src > iLowThresh, dst = iMaxValue*/
    AX_NPU_CV_THRESH_MODE_CLIP_MAX = 1, /*!<src <= iLowThresh, dst = src; src > iLowThresh, dst = iMaxValue*/
    AX_NPU_CV_THRESH_MODE_CLIP_MIN = 2, /*!<src <= iLowThresh, dst = iMinValue; src > iLowThresh, dst = src*/
    AX_NPU_CV_THRESH_MODE_MIN_MID_MAX = 3, /*!<src <= iLowThresh, dst = iMinValue; iLowThresh < src <= iHighThresh, dst = iMidValue; src > iHighThresh, dst = iMaxValue*/
    AX_NPU_CV_THRESH_MODE_ORI_MID_MAX = 4, /*!<src <= iLowThresh, dst = src; iLowThresh < src <= iHighThresh, dst = iMidValue; src > iHighThresh, dst = iMaxValue*/
    AX_NPU_CV_THRESH_MODE_MIN_MID_ORI = 5, /*!<src <= iLowThresh, dst = iMinValue; iLowThresh < src <= iHighThresh, dst = iMidValue; src > iHighThresh, dst = src*/
    AX_NPU_CV_THRESH_MODE_MIN_ORI_MAX = 6, /*!<src <= iLowThresh, dst = iMinValue; iLowThresh < src <= iHighThresh, dst = src; src > iHighThresh, dst = iMaxValue*/
    AX_NPU_CV_THRESH_MODE_ORI_MID_ORI = 7, /*!<src <= iLowThresh, dst = src; iLowThresh < src <= iHighThresh, dst = iMidValue; src > iHighThresh, dst = src*/
} AX_NPU_CV_THRESH_MODE_E;

/*!
 * AX_NPU_CV_Thresh 算子参数
*/
typedef struct _AX_NPU_CV_ThreshParams {
    AX_S32 iLowThresh; /*!<低阈值 */
    AX_S32 iHighThresh; /*!<高阈值 */
    AX_S32 iMinValue; /*!<最小值 */
    AX_S32 iMidValue; /*!<中间值 */
    AX_S32 iMaxValue; /*!<最大值 */
    AX_NPU_CV_THRESH_MODE_E eMode; /*!<阈值化模式 */
} AX_NPU_CV_ThreshParams;

/*!
 * 定义 AX_NPU_CV_Thresh 算子上下文
*/
typedef AX_VOID* AX_NPU_CV_ThreshContext;
/*!
 * 函数功能描述:
 * <br/>实现输入矩阵阈值化
 * <br/>输入矩阵行和列数要求2对齐；最小支持 2x32；最大支持 4K 分辨率。
 * \warning 只有在`BINARY`和`MIN_MID_MAX`模式下，允许输入输出数据类型不一致，其它模式下输入输出数据类型必须一致
 * \warning `iLowThresh` 和 `iHighThresh` 不能超出输入数据类型值域范围，且iLowThresh <= iHighThresh
 * \warning `iMinValue`、`iMidValue`和`iMaxValue`不能超出输出数据类型值域范围
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入矩阵，支持 uint16/int16/uint8；
 * \param pThreshParams: 阈值化算子参数
 * \param pDst: 输出矩阵，支持 uint16/int16/uint8；需要用户分配内存
 */
int AX_NPU_CV_Thresh(AX_NPU_CV_ThreshContext* pContext, const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Matrix2D* pSrc, const AX_NPU_CV_ThreshParams* pThreshParams, AX_NPU_CV_Matrix2D* pDst);
/*!
* 函数功能描述:
   <br/>销毁 AX_NPU_CV_Thresh 算子上下文
 * \param tContext: AX_NPU_CV_Thresh 算子上下文
 */
void AX_NPU_CV_DestroyThreshContext(AX_NPU_CV_LaplacianContext tContext);

/*!
 * 函数功能描述:
 * <br/>实现图像仿射变换
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入图像，支持BGR/RGB/RGBA/RGGB-4ch 8bit/16bit, 支持YUV420(NV12/NV21），最大支持 4K 分辨率
 * \param pDst: 输出图像，格式和channel需要和输入一致
 * \param pMat33: 3x3变换矩阵
 * \param interp: 支持BILINEAR、NEAREST两种插值方式
 * \param const_val: 越界点设为该值
 */
int AX_NPU_CV_Warp(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Image* pSrc, const AX_NPU_CV_Image* pDst, const float *pMat33, const AX_NPU_CV_Interp interp, const int const_val);
/*
 * <br/>实现前景RGBA和背景RGBA/NV12的图像做blending操作。
 * <br/>背景图为整图，前景图可以只和背景图部分区域相对应，此时需要指定前景图相对背景图区域的偏移坐标。
 * <br/>blending功能完成后将结果输出到目标地址相应区域。
 * \warning 背景图做blending区域不能超出图像实际范围。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pBackground: 背景图像指针，支持RGBA/NV12
 * \param pForeground: 前景图像指针，支持RGBA
 * \param pDst: 输出图像指针，支持RGBA
 * \param nOffsetW: 指定背景图blending区域起始位置宽度
 * \param nOffsetH: 指定背景图blending区域起始位置高度
 */
int AX_NPU_CV_AlphaBlending(
        const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        AX_NPU_CV_Image* pBackground, const AX_NPU_CV_Image* pForeground,
        AX_NPU_CV_Image* pDst, AX_U32 nOffsetW, AX_U32 nOffsetH);

typedef AX_VOID* AX_NPU_CV_L2NormalizeContext;
/*!
 * 函数功能描述:
 * <br/>实现对输入矩阵进行 L2 normalize 计算
 * <br/>输入矩阵行和列数要求2对齐；最小支持 2x32；最大支持 4K 分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入矩阵，支持 float；
 * \param pDst: 输出矩阵，支持 float；需要用户分配内存
 */
int AX_NPU_CV_L2Normalize(AX_NPU_CV_L2NormalizeContext*Context, const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode, const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst);

/*!
 * 函数功能描述:
 * <br/>Destroy L2Normalize算子的上下文
* \param tContext: AX_NPU_CV_L2Normalize 算子上下文
 */
void AX_NPU_CV_DestroyL2NormalizeContext(AX_NPU_CV_L2NormalizeContext Context);

/*!
 * <br/>Saxpy：C = alpha*A + B
 * <br/>最大支持 2000x4000x1(HWC) 分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param fAlpha: 系数 alpha
 * \param pA: 输入矩阵 A，支持 float/int8
 * \param pB: 输入矩阵 B，支持 float/int8
 * \param pC: 输出矩阵 C，支持 float/int8/int16
 */
int AX_NPU_CV_Saxpy(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_F32 fAlpha, const AX_NPU_CV_Matrix2D* pA, const AX_NPU_CV_Matrix2D* pB, AX_NPU_CV_Matrix2D* pC);


/*!
 * 定义 AX_NPU_CV_Sgemm 算子上下文
*/
typedef AX_VOID* AX_NPU_CV_SgemmContext;

/*!
 * <br/>Sgemm：D = alpha*op( A )*op( B ) + beta*C，暂时不支持输入 C 矩阵
 * <br/>最大支持 2K 分辨率。
 * \param pContext: 算子上下文指针。如果 `*pContext` 为空指针, 算子内部会自动分配内存。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param bTransA: 是否对 A 矩阵转置，False：op( A ) = A， True：op( A ) = A^T,
 * \param bTransB: 是否对 B 矩阵转置，False：op( B ) = B， True：op( B ) = B^T,
 * \param fAlpha: 系数 alpha
 * \param pA: 输入矩阵 A，支持 float
 * \param pB: 输入矩阵 B，支持 float
 * \param fBeta: 系数 beta
 * \param pC: 输入矩阵 C，支持 float
 * \param pD: 输出矩阵 D，支持 float
 */
int AX_NPU_CV_Sgemm(AX_NPU_CV_SgemmContext* pContext,
        const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_BOOL bTransA, const AX_BOOL bTransB,
        const AX_F32 fAlpha,
        const AX_NPU_CV_Matrix2D* pA, const AX_NPU_CV_Matrix2D* pB,
        const AX_F32 fBeta, const AX_NPU_CV_Matrix2D* pC,
        AX_NPU_CV_Matrix2D* pD);

/*!
* 函数功能描述:
   <br/>销毁 AX_NPU_CV_Sgemm 算子上下文
 * \param tContext: AX_NPU_CV_Sgemm 算子上下文
 */
void AX_NPU_CV_DestroySgemmContext(AX_NPU_CV_SgemmContext tContext);

typedef AX_VOID* AX_NPU_CV_ConvArithContext;
/*!
 * <br/>AX_NPU_CV_MatAdd:  实现矩阵 C = A + B
 * <br/>最小支持 1x16；最大支持 4K 分辨率。
 * \param pContext: 算子上下文指针。如果 `*pContext` 为空指针, 算子内部会自动分配内存。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pAdd1: 输入A矩阵，支持uint8
 * \param pAdd2: 输入B矩阵，支持uint8
 * \param pDst: 输出C矩阵，支持uint8, 结果clip到(0,255)
 */
int AX_NPU_CV_MatAdd(AX_NPU_CV_ConvArithContext *pContext,
        const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        AX_NPU_CV_Matrix2D *pAdd1,
        AX_NPU_CV_Matrix2D *pAdd2,
        AX_NPU_CV_Matrix2D * pDst);

AX_VOID AX_NPU_CV_DestroyConvArithContext(AX_NPU_CV_ConvArithContext context);
/*!
 * <br/>AX_NPU_CV_MatAdd:  实现矩阵 C = A - B
 * <br/>最小支持 1x16；最大支持 4K 分辨率。
 * \param pContext: 算子上下文指针。如果 `*pContext` 为空指针, 算子内部会自动分配内存。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pAdd1: 输入A矩阵，支持uint8
 * \param pAdd2: 输入B矩阵，支持uint8
 * \param pDst: 输出C矩阵，支持uint8, 结果clip到(0,255)
 */
int AX_NPU_CV_MatSub(AX_NPU_CV_ConvArithContext *pContext,
        const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        AX_NPU_CV_Matrix2D *pAdd1,
        AX_NPU_CV_Matrix2D *pAdd2,
        AX_NPU_CV_Matrix2D * pDst);
/*!
 * <br/>Sigmoid：sigmoid算子， y = 1/(1+exp(-x))
 * <br/>最小支持 1x16；最大支持 2000x4000x1(HWC)分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入矩阵 pSrc，支持 float, 范围[-20,20]。
 * \param pDst: 输出矩阵 pDst，支持 float
 */
int AX_NPU_CV_Sigmoid(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst);

/*!
 * <br/>Softmax：按行计算矩阵的Softmax
 * <br/>最大支持 1000x2000x1(HWC)分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入矩阵 pSrc，支持 float
 * \param pDst: 输出矩阵 pDst，支持 float
 */
int AX_NPU_CV_Softmax(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst);
/*
 * 统计方法
*/
typedef enum _AX_NPU_CV_StatsMethod {
    AX_NPU_CV_SM_SUM = 0, // 统计和
    AX_NPU_CV_SM_MAX = 1, // 统计最大值
    AX_NPU_CV_SM_ARGMAX = 2, // 统计最大值位置
    AX_NPU_CV_SM_MIN = 3, // 统计最小值
    AX_NPU_CV_SM_ARGMIN = 4, // 统计最小值位置
} AX_NPU_CV_StatsMethod;

/*!
 * <br/>函数功能描述: 按行统计矩阵元素，功能包括：求和(sum)，最大值(max)，最大值位置(argmax)，最小值(min)，最小值位置(argmin)
 * <br/>数据类型支持(float, int8, uint8, int16, uint16), 统计方法AX_NPU_CV_SM_SUM 仅支持float矩阵
 * <br/>最大支持 3000x4000x1(HWC)分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param eStatsMethod: 按行统计方法
 * \param pSrc: 输入矩阵 pSrc
 * \param pDst: 输出矩阵 pDst
 */
int AX_NPU_CV_Stats(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_StatsMethod eStatsMethod, const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst);

/*!
 * <br/>函数功能描述: 计算矩阵方差
 * <br/>数据类型支持(uint8)
 * <br/>最大支持 1024x1024x1(HWC)分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别,仅支持disable和1_1_1
 * \param pSrc: 输入矩阵 pSrc, 仅支持uint8输入
 * \param pDst: 输出矩阵 pDst, 仅支持float输出
 */
int AX_NPU_CV_Variance(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
    const AX_NPU_CV_Matrix2D* pSrc, AX_NPU_CV_Matrix2D* pDst);
/*
 * DMA拷贝模式：
 *      AX_NPU_CV_DMA_MODE_DIRECT_COPY：直接快速拷贝
 *      AX_NPU_CV_DMA_MODE_INTERVAL_COPY： 间隔拷贝
 */
typedef enum _AX_NPU_CV_DMA_MODE{
    AX_NPU_CV_DMA_MODE_DIRECT_COPY,
    AX_NPU_CV_DMA_MODE_INTERVAL_COPY,
} AX_NPU_CV_DMA_MODE;

/**
 * DMA拷贝控制参数
 *     DIRECT_COPY模式下，box参数有效
 *     INTERVAL_COPY模式下，u8VerSegRows/u8HorSegSize/u8ElemSize参数有效, 参数有效范围(1,255)
 *     INTERVAL_COPY模式下，需要输入矩阵行/列数，分别是u8VerSegRows/u8HorSegSize的整数倍
 */
typedef struct _AX_NPU_CV_DMA_CTRL{
    AX_NPU_CV_DMA_MODE mode;
    union{
        AX_NPU_CV_Box box;
        struct{
            AX_U8 u8VerSegRows;
            AX_U8 u8HorSegSize;
            AX_U8 u8ElemSize;
        };
    };
}AX_NPU_CV_DMA_CTRL;

/*!
 * <br/>函数功能描述: 提供DMA操作方法
 * <br/>数据类型支持(GRAY，RGB)
 * <br/>最大支持 4K分辨率，最小2x32
 * \param eVirtualNpuMode: 虚拟NPU模式类别
 * \param pSrc: 输入图像 pSrc
 * \param pDst: 输出图像 pDst
 */
int HI_MPI_IVE_DMA(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
    AX_NPU_CV_Image * pSrc, AX_NPU_CV_Image *pDst, AX_NPU_CV_DMA_CTRL *pstCtrl);
/*
 * 定义NCC的输出内存信息
*/
typedef struct _AX_NPU_CV_NCC_MEM_S{
    AX_U64 u64Numerator;//sum(A*B)，A和B矩阵的内积
    AX_U64 u64QuadSum1;//sum(A*A)
    AX_U64 u64QuadSum2;//sum(B*B)
}AX_NPU_CV_NCC_MEM;

/*!
 * <br/>函数功能描述:计算两相同分辨率灰度图像的归一化相互关系系数
 * <br/>数据类型支持GRAY
 * <br/>最大支持 4K分辨率。
 * \param eVirtualNpuMode: 虚拟NPU模式类别,仅支持disable和1_1_1两种模式
 * \param pSrc0: 输入灰度图像pSrc0
 * \param pSrc1: 输入灰度图像pSrc1
 * \param pDst: 输出计算结果 pDst
 */
int AX_NPU_CV_NCC(const AX_NPU_SDK_EX_MODEL_TYPE_T eVirtualNpuMode,
        const AX_NPU_CV_Image* pSrc0, const AX_NPU_CV_Image* pSrc1, AX_NPU_CV_NCC_MEM * pDst);

#ifdef __cplusplus
}
#endif

#endif
