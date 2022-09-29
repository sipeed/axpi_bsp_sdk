/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_IVPS_API_H_
#define _AX_IVPS_API_H_
#include "ax_base_type.h"
#include "ax_global_type.h"
#include "ax_sys_api.h"
#include "ax_ivps_type.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef AX_S32 IVPS_GRP;
typedef AX_S32 IVPS_CHN;
typedef AX_S32 IVPS_FILTER;

typedef AX_S32 IVPS_RGN_GRP;
typedef AX_U32 IVPS_RGB;

#define AX_IVPS_MAX_GRP_NUM   20
#define AX_IVPS_MAX_OUTCHN_NUM 5
#define AX_IVPS_MAX_FILTER_NUM_PER_OUTCHN 2
#define AX_IVPS_INVALID_FRMRATE (-1)

typedef enum
{
    AX_IVPS_CHN_FLIP_NONE = 0,
    AX_IVPS_CHN_FLIP,
    AX_IVPS_CHN_MIRROR,
    AX_IVPS_CHN_FLIP_AND_MIRROR,
    AX_IVPS_CHN_FLIP_BUTT
} AX_IVPS_CHN_FLIP_MODE_E;

typedef struct axIVPS_RECT_S
{
    AX_U16 nX;
    AX_U16 nY;
    AX_U16 nW;
    AX_U16 nH;
} AX_IVPS_RECT_S;

typedef struct axIVPS_POINT_S
{
    AX_S16 nX;
    AX_S16 nY;
} AX_IVPS_POINT_S;

typedef struct axIVPS_SIZE_S
{
    AX_U16 nW;
    AX_U16 nH;
} AX_IVPS_SIZE_S;

typedef struct axIVPS_BOX_S
{
    AX_S16 nX;
    AX_S16 nY;
    AX_U16 nW;
    AX_U16 nH;
} AX_IVPS_BOX_S;

typedef struct axIVPS_ASPECT_RATIO_S
{
    AX_IVPS_ASPECT_RATIO_E eMode;
    /* Typical RGB color:
 1. WHITE : 0xFFFFFF
 2. BLACK : 0x000000
 3. RED   : 0x0000FF
 4. BLUE  : 0x00FF00
 5. YELLOW: 0xFF00FF
 31       23      15      7       0
 |--------|---R---|---G---|---B---|
 */
    AX_U32 nBgColor;
    AX_IVPS_ASPECT_RATIO_ALIGN_E eAligns[2]; /* IVPS_ASPECT_RATIO_ALIGN: [0]: HORIZONTAL [1]: VERTICAL */
    AX_IVPS_RECT_S tRect;                    /* valid in ASPECT_RATIO_MANUAL mode */
} AX_IVPS_ASPECT_RATIO_S;

typedef struct axIVPS_FRAME_RATE_CTRL_S
{
    /* src frame rate (<= 0: no FRC control) */
    AX_S32 nSrcFrameRate;
    /* dst frame rate (<= 0, or nSrcFrameRate must be set) */
    AX_S32 nDstFrameRate;
} AX_IVPS_FRAME_RATE_CTRL_S;

typedef struct axIVPS_FB_INFO_S
{
    AX_POOL PoolId;
    AX_BOOL bInPlace;
    AX_BLK BlkId;
    AX_BLK BlkIdUV;
    AX_U64 nPhyAddr;
    AX_U64 nPhyAddrUV;
    AX_U32 nUvOffset;
} AX_IVPS_FB_INFO_S;

typedef enum
{
    AX_IVPS_ENGINE_CPU = 0,
    AX_IVPS_ENGINE_TDP,
    AX_IVPS_ENGINE_GDC,
    // AX_IVPS_ENGINE_VPP, //for future extension
    // AX_IVPS_ENGINE_VGP, //for future extension
    // AX_IVPS_ENGINE_IVE, //for future extension
    AX_IVPS_ENGINE_VO,
    AX_IVPS_ENGINE_NPU,
    AX_IVPS_ENGINE_DSP,
    // AX_IVPS_ENGINE_DPU,  //depth processor unit
    AX_IVPS_ENGINE_BUTT
} AX_IVPS_ENGINE_E;

/*
 * IVPS_PIPELINE_X is the mainstream pipeline we currently support. The mode is as follows:
 *                            => Pipe1Filter0 Pipe1Filter1 (chn 0)
 * Pipe0Filter0 Pipe0Filter1  => Pipe2Filter0 Pipe2Filter1 (chn 1)
 *                            => Pipe3Filter0 Pipe3Filter1 (chn 2)
 * Note: Filter[n]:
 * n: channel filter type(0: RFC/CROP/SCALE/ROTATE; 1:REGION)
 */
typedef enum
{
    AX_IVPS_PIPELINE_DEFAULT = 0,
    AX_IVPS_PIPELINE_BUTT
} AX_IVPS_PIPELINE_E;

/*
 * AX_IVPS_TDP_CFG_S
 * This configuration is specific to TDP engine.
 * This engine can support many functions,
 * such as mirror,flip,rotation,scale,mosaic,osd and so on.
 * For AX620, if in eCompressMode, stride and width should be 64 pixels aligned.
 * If NOT in eCompressMode, stride and width should be 32 pixels aligned.
 * 16 alignment is also supported, but the bandwidth will decrease.
 */
typedef struct axIVPS_TDP_CFG_S
{
    AX_BOOL bCrop;
    AX_IVPS_RECT_S tCropRect;
    AX_BOOL bMirror;
    AX_BOOL bFlip;
    AX_IVPS_ROTATION_E eRotation;
    AX_IVPS_ASPECT_RATIO_S tAspectRatio;
    AX_BOOL bDither;
    AX_COMPRESS_MODE_E eCompressMode;
    AX_BOOL bInplace;
} AX_IVPS_TDP_CFG_S;

/*
 * AX_IVPS_DEWARP_TYPE_E
 * AX_IVPS_DEWARP_MAP_USER: user-defined map.
 * AX_IVPS_DEWARP_DISTORTION: distortion correction.
 * AX_IVPS_DEWARP_PERSPECTIVE: affine or perspective transformation.
 */
typedef enum
{
    AX_IVPS_DEWARP_MAP_USER  = 0,
    AX_IVPS_DEWARP_DISTORTION,
    AX_IVPS_DEWARP_PERSPECTIVE,
    AX_IVPS_DEWARP_LDC,
    AX_IVPS_DEWARP_BUTT
} AX_IVPS_DEWARP_TYPE_E;

/*
 * AX_IVPS_DEWARP_DISTORTION_FACTOR_S
 * Camera Matrix(internal parameter matrix) =
 *   [fx,          0,        -cx],
 *   [0,          fy,        -cy],
 *   [0,           0,         1 ],
 * Distortion Coefficients =
 *  (k1, k2, p1, p2, k3, k4, k5, k6)
 * Note:
 * The matrix element has 6 decimal numbers.
 * i.e. If the element is 954301, 954301/1000000 = 0.954301,
 * the real value is 0.954301.
 */
typedef struct
{
    AX_S64 nCameraMatrix[9];
    AX_S64 nDistortionCoeff[8];
} AX_IVPS_DEWARP_DISTORTION_FACTOR_S;

/*
 * AX_IVPS_DEWARP_PERSPECTIVE_FACTOR_S
 * Perspective Matrix =
 *   [m(0,0),   m(0,1),   m(0,2)],
 *   [m(1,0),   m(1,1),   m(1,2)],
 *   [m(2,0),   m(2,1),   m(2,2)],
 * If [m(2,0), m(2,1), m(2,2)] = [0, 0, 1], the transformation is affine.
 * Note:
 * 1. m(2,2) is not 0.
 * 2. The matrix element has 6 decimal numbers.
 * i.e. If the element is 954301, 954301/1000000 = 0.954301,
 * the real value is 0.954301.
 */
typedef struct
{
    AX_S64 nPerspectiveMatrix[9];
} AX_IVPS_DEWARP_PERSPECTIVE_FACTOR_S;

typedef struct
{
    AX_BOOL bAspect; /* Whether aspect ration is keep */
    AX_S16 nXRatio; /* Range: [0, 100], field angle ration of horizontal, valid when bAspect=0. */
    AX_S16 nYRatio; /* Range: [0, 100], field angle ration of vertical, valid when bAspect=0. */
    AX_S16 nXYRatio; /* Range: [0, 100], field angle ration of all,valid when bAspect=1. */
    AX_S16 nCenterXOffset; /* Range: [-511, 511], horizontal offset of the image distortion center relative to image center. */
    AX_S16 nCenterYOffset; /* Range: [-511, 511], vertical offset of the image distortion center relative to image center. */
    AX_S16 nDistortionRatio; /* Range: [0, 1023], LDC distortion ratio. 512 is best undistortion */
} AX_IVPS_DEWARP_LDC_FACTOR_S;

/*
 * AX_IVPS_GDC_CFG_S
 * This configuration is specific to GDC engine.
 * If bDewarpEnable is AX_TRUE, dewarp function
 * (such as user-defined map, distortion correction, affine or perspective transformation) is enabled.
 * Otherwise, GDC only performs functions such as crop, rotation, mirror, flip and scaling.
 * For user-defined map type, nMeshTableSize is fixed 1188, which means 1188 lines.
 * Each line occupies 8 bytes. So you need to malloc 1188x8 bytes of physical memory to store the mesh table.
 * If eCompressMode is TILE, stride and width should be 64 pixels aligned.
 * If eCompressMode is NONE, only stride should be 64 pixels aligned, while
 * input frame width should be 2 pixels aligned, output frame width should be 16 pixels aligned.

 */
typedef struct axIVPS_GDC_CFG_S
{
    AX_BOOL bDewarpEnable;
    AX_BOOL bCrop;
    AX_IVPS_RECT_S tCropRect;
    AX_IVPS_ROTATION_E eRotation;
    AX_BOOL bMirror;
    AX_BOOL bFlip;

    AX_COMPRESS_MODE_E eCompressMode;
    AX_BOOL bEnhanceMode;

    AX_IVPS_DEWARP_TYPE_E bDewarpType;
    union
    {
        /* AX_IVPS_DEWARP_DISTORTION */
        AX_IVPS_DEWARP_DISTORTION_FACTOR_S tDistortionFactor;
        /* AX_IVPS_DEWARP_PERSPECTIVE */
        AX_IVPS_DEWARP_PERSPECTIVE_FACTOR_S tPerspectiveFactor;
        /* AX_IVPS_DEWARP_LDC */
        AX_IVPS_DEWARP_LDC_FACTOR_S tLdcFactor;
    };
    /* AX_IVPS_DEWARP_MAP_USER */
    AX_U16 nInterStartX;
    AX_U16 nInterStartY;
    AX_U16 nMeshStartX;
    AX_U16 nMeshStartY;
    AX_U16 nMeshWidth;
    AX_U16 nMeshHeight;
    AX_U64 nPhyAddr;
    AX_U64 nMeshTableVirAddr;
    AX_U32 nMeshTableSize;
} AX_IVPS_GDC_CFG_S;

/*
 * AX_IVPS_VO_CFG_S
 * This configuration is specific to VO engine.
 * This engine can support ovly one function,
 * alpha blending.
 */
typedef struct axIVPS_VO_CFG_S
{
    AX_BOOL bCrop;
    AX_IVPS_RECT_S tCropRect;
} AX_IVPS_VO_CFG_S;

/*
 * AX_IVPS_FILTER_S
 * A pipeline consists of a filter or several.
 * Each filter can complete specific functions.
 * You can choose the engine to do this.
 */
typedef struct axIVPS_FILTER_S
{
    AX_BOOL bEnable;
    AX_IVPS_FRAME_RATE_CTRL_S tFRC;
    AX_U32 nDstPicOffsetX0;
    AX_U32 nDstPicOffsetY0;
    AX_U32 nDstPicWidth;
    AX_U32 nDstPicHeight;
    AX_U32 nDstPicStride;
    AX_U32 nDstFrameWidth;
    AX_U32 nDstFrameHeight;
    /* nW & nH & nStride 16 bytes alignment */
    AX_IMG_FORMAT_E eDstPicFormat;

    AX_IVPS_ENGINE_E eEngine;
    union /* engine specific config data */
    {
        AX_IVPS_TDP_CFG_S tTdpCfg;
        AX_IVPS_GDC_CFG_S tGdcCfg;
        AX_IVPS_VO_CFG_S tVoCfg;
    };
} AX_IVPS_FILTER_S;

typedef struct axIVPS_PIPELINE_ATTR_S
{
    AX_IVPS_FB_INFO_S tFbInfo;
    AX_U8 nOutChnNum;
    AX_IVPS_FILTER_S tFilter[AX_IVPS_MAX_OUTCHN_NUM + 1][AX_IVPS_MAX_FILTER_NUM_PER_OUTCHN];
    AX_U8 nOutFifoDepth[AX_IVPS_MAX_OUTCHN_NUM];
    AX_U16 nInDebugFifoDepth;
} AX_IVPS_PIPELINE_ATTR_S;

typedef struct
{
    AX_U8 nInFifoDepth;
    AX_IVPS_PIPELINE_E ePipeline;
} AX_IVPS_GRP_ATTR_S;

/* ---------------------------- Pipeline APIs ---------------------------- */

AX_S32 AX_IVPS_Init(AX_VOID);
AX_S32 AX_IVPS_Deinit(AX_VOID);
AX_S32 AX_IVPS_CreateGrp(IVPS_GRP IvpsGrp, const AX_IVPS_GRP_ATTR_S *ptGrpAttr);
AX_S32 AX_IVPS_DestoryGrp(IVPS_GRP IvpsGrp);
AX_S32 AX_IVPS_SetPipelineAttr(IVPS_GRP IvpsGrp, const AX_IVPS_PIPELINE_ATTR_S *ptPipelineAttr);
AX_S32 AX_IVPS_GetPipelineAttr(IVPS_GRP IvpsGrp, AX_IVPS_PIPELINE_ATTR_S *ptPipelineAttr);
AX_S32 AX_IVPS_StartGrp(IVPS_GRP IvpsGrp);
AX_S32 AX_IVPS_StopGrp(IVPS_GRP IvpsGrp);
AX_S32 AX_IVPS_EnableChn(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn);
AX_S32 AX_IVPS_DisableChn(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn);
AX_S32 AX_IVPS_SendFrame(IVPS_GRP IvpsGrp, const AX_VIDEO_FRAME_S *ptFrame, AX_S32 nMilliSec);
AX_S32 AX_IVPS_GetChnFrame(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn, AX_VIDEO_FRAME_S *ptFrame, AX_S32 nMilliSec);
AX_S32 AX_IVPS_ReleaseChnFrame(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn, AX_VIDEO_FRAME_S *ptFrame);
AX_S32 AX_IVPS_GetChnFd(IVPS_GRP IvpsGrp, IVPS_CHN IvpsChn);
AX_S32 AX_IVPS_GetDebugFifoFrame(IVPS_GRP IvpsGrp, AX_VIDEO_FRAME_S *ptFrame);
AX_S32 AX_IVPS_ReleaseDebugFifoFrame(IVPS_GRP IvpsGrp, AX_VIDEO_FRAME_S *ptFrame);
AX_S32 AX_IVPS_CloseAllFd(AX_VOID);

/***************************************************************************************************************/
/*                                               REGION                                                        */
/***************************************************************************************************************/
typedef AX_S32 IVPS_RGN_HANDLE;
#define AX_IVPS_MAX_RGN_HANDLE_NUM   (32)
#define AX_IVPS_INVALID_REGION_HANDLE (IVPS_RGN_HANDLE)(-1)
#define AX_IVPS_REGION_MAX_DISP_NUM (32)

typedef enum
{
    AX_IVPS_RGN_TYPE_LINE = 0,
    AX_IVPS_RGN_TYPE_RECT = 1,
    AX_IVPS_RGN_TYPE_POLYGON = 2, /* convex quadrilateral */
    AX_IVPS_RGN_TYPE_MOSAIC = 3,
    AX_IVPS_RGN_TYPE_OSD = 4,
    AX_IVPS_RGN_TYPE_BUTT
} AX_IVPS_RGN_TYPE_E;

typedef enum
{
    AX_IVPS_RGN_LAYER_OVERLAY = 0,
    AX_IVPS_RGN_LAYER_OVERLAY_EXT0,
    AX_IVPS_RGN_LAYER_OVERLAY_EXT1,
    AX_IVPS_RGN_LAYER_OVERLAY_EXT2,
    AX_IVPS_RGN_LAYER_OVERLAY_EXT3,
    AX_IVPS_RGN_LAYER_COVER, /*only support mosaic/rectangle(solid or not)*/
    AX_IVPS_RGN_LAYER_BUTT
} AX_IVPS_RGN_LAYER_E;

typedef struct axIVPS_COLORKEY_S
{
    AX_BOOL bEnable;
    AX_BOOL bColorKeyInv; /* 0: winin threshold, 1: outside threshold */
    IVPS_RGB nBgColorLo;  /* min value of background color */
    IVPS_RGB nBgColorHi;  /* max value of background color */
} AX_IVPS_COLORKEY_S;

typedef struct axIVPS_BITCOLOR_S
{
    AX_BOOL bEnable;
    IVPS_RGB nColor;
    IVPS_RGB nColorInv;
    IVPS_RGB nColorInvThr;
} AX_IVPS_BITCOLOR_S;

typedef struct axIVPS_RGN_CHN_ATTR_S
{
    AX_S32 nZindex;
    AX_BOOL bSingleCanvas; /* AX_TURE: single canvas; AX_FALSE: double canvas */
    AX_U16 nAlpha; /* 0 - 1024, 0: transparent, 1024: opaque*/
    AX_IMG_FORMAT_E eFormat;
    AX_IVPS_BITCOLOR_S nBitColor; /*only for bitmap*/
    AX_IVPS_COLORKEY_S nColorKey; /* only for Overlay 0 */
} AX_IVPS_RGN_CHN_ATTR_S;

typedef struct
{
    AX_IVPS_POINT_S tPTs[10];
    AX_U8 nPointNum;
    AX_U32 nLineWidth; /* [1 - 16]  */
    IVPS_RGB nColor;   /* RGB Color: 0xRRGGBB, eg: red: 0xFF0000 */
    AX_U8 nAlpha;      /* 0 - 255, 0: transparent, 255: opaque*/
} AX_IVPS_RGN_LINE_S;

typedef struct
{
    union
    {
        AX_IVPS_RECT_S tRect;     /* AX_IVPS_OVERLAY_RECT     */
        AX_IVPS_POINT_S tPTs[10]; /* AX_IVPS_OVERLAY_POLYGON, up to 10*/
    };
    AX_U8 nPointNum;
    AX_U32 nLineWidth; /* [1 - 16]  */
    IVPS_RGB nColor;   /* RGB Color: 0xRRGGBB */
    AX_U8 nAlpha;      /* 0 - 255, 0: transparent, 255: opaque*/
    AX_BOOL bSolid;    /* if AX_TRUE, fill the rect with the nColor */
    /*
      The style of rectangle is like below
      [            ]

      [            ]
      if bCornerRect is AX_TRUE, then bSolid is always AX_FALSE
  */
    AX_BOOL bCornerRect;
} AX_IVPS_RGN_POLYGON_S;

typedef struct
{
    AX_IVPS_RECT_S tRect;
    AX_IVPS_MOSAIC_BLK_SIZE_E eBklSize;
} AX_IVPS_RGN_MOSAIC_S;

typedef AX_OSD_BMP_ATTR_S AX_IVPS_RGN_OSD_S;

typedef union
{
    AX_IVPS_RGN_LINE_S tLine;
    AX_IVPS_RGN_POLYGON_S tPolygon;
    AX_IVPS_RGN_MOSAIC_S tMosaic;
    AX_IVPS_RGN_OSD_S tOSD;
} AX_IVPS_RGN_DISP_U;

typedef struct
{
    AX_BOOL bShow;
    AX_IVPS_RGN_TYPE_E eType;
    AX_IVPS_RGN_DISP_U uDisp;
} AX_IVPS_RGN_DISP_S;

typedef struct
{
    AX_U32 nNum;
    AX_IVPS_RGN_CHN_ATTR_S tChnAttr;
    AX_IVPS_RGN_DISP_S arrDisp[AX_IVPS_REGION_MAX_DISP_NUM];
} AX_IVPS_RGN_DISP_GROUP_S;

typedef struct axIVPS_GDI_ATTR_S
{
    AX_U16 nThick;
    AX_U16 nAlpha;
    AX_U32 nColor;
    AX_BOOL bSolid;  /* if AX_TRUE, fill the rect with the nColor */
    AX_BOOL bAbsCoo; /*is Absolute Coordinate*/
} AX_IVPS_GDI_ATTR_S;

typedef struct axIVPS_RGN_CANVAS_INFO_S
{
    AX_BLK BlkId;
    AX_U64 nPhyAddr;
    AX_VOID *pVirAddr;
    AX_U32 nStride;
    AX_U16 nW;
    AX_U16 nH;
    AX_IMG_FORMAT_E eFormat;
} AX_IVPS_RGN_CANVAS_INFO_S;

/*
Create region
@return : return the region handle created
*/
IVPS_RGN_HANDLE AX_IVPS_RGN_Create(AX_VOID);
/*
Destroy created region
@param - [IN]  hRegion: specifies the region handle created by AX_IVPS_RGN_Create
*/
AX_S32 AX_IVPS_RGN_Destroy(IVPS_RGN_HANDLE hRegion);
/*
Attach region to IVPS channel
@param - [IN]  hRegion: specifies the region handle created by AX_IVPS_RGN_Create
@param - [IN]  IvpsGrp:  specifies the group to attach.
@param - [IN]  IvpsFilter:  specifies the filter of the group to attach.
*/
AX_S32 AX_IVPS_RGN_AttachToFilter(IVPS_RGN_HANDLE hRegion, IVPS_GRP IvpsGrp, IVPS_FILTER IvpsFilter);
/*
Detach region from IVPS channel
@param - [IN]  hRegion: specifies the region handle created by AX_IVPS_RGN_Create
@param - [IN]  IvpsGrp:  specifies the group to detach.
@param - [IN]  IvpsFilter:  specifies the filter of the group to detach.
*/
AX_S32 AX_IVPS_RGN_DetachFromFilter(IVPS_RGN_HANDLE hRegion, IVPS_GRP IvpsGrp, IVPS_FILTER IvpsFilter);
AX_S32 AX_IVPS_RGN_Update(IVPS_RGN_HANDLE hRegion, const AX_IVPS_RGN_DISP_GROUP_S *ptDisp);

/***************************************************************************************************************/
/*                                                   OFFLINE                                                   */
/***************************************************************************************************************/

AX_S32 AX_IVPS_CmmCopy(AX_U64 nSrcPhyAddr, AX_U64 nDstPhyAddr, AX_U64 nMemSize);
AX_S32 AX_IVPS_Rotation(const AX_VIDEO_FRAME_S *ptSrc, AX_IVPS_ROTATION_E eRotation, AX_VIDEO_FRAME_S *ptDst);
AX_S32 AX_IVPS_FlipAndRotation(const AX_VIDEO_FRAME_S *ptSrc, AX_IVPS_CHN_FLIP_MODE_E eFlipMode,
                               AX_IVPS_ROTATION_E eRotation, AX_VIDEO_FRAME_S *ptDst);
AX_S32 AX_IVPS_CSC(const AX_VIDEO_FRAME_S *ptSrc, AX_VIDEO_FRAME_S *ptDst);
AX_S32 AX_IVPS_CropResize(const AX_VIDEO_FRAME_S *ptSrc, AX_VIDEO_FRAME_S *ptDst, const AX_IVPS_ASPECT_RATIO_S *ptAspectRatio);
AX_S32 AX_IVPS_AlphaBlending(const AX_VIDEO_FRAME_S *ptSrc, const AX_VIDEO_FRAME_S *ptOverlay,
                             const AX_IVPS_POINT_S tOffset, AX_VIDEO_FRAME_S *ptDst);
AX_S32 AX_IVPS_DrawOsd(AX_IVPS_RGN_CANVAS_INFO_S *ptCanvas, const AX_OSD_BMP_ATTR_S arrBmp[], const AX_U32 nNum);
AX_S32 AX_IVPS_DrawLine(const AX_IVPS_RGN_CANVAS_INFO_S *ptCanvas, AX_IVPS_GDI_ATTR_S tAttr, const AX_IVPS_POINT_S tPoint[], AX_U32 nPointNum);
AX_S32 AX_IVPS_DrawPolygon(const AX_IVPS_RGN_CANVAS_INFO_S *ptCanvas, AX_IVPS_GDI_ATTR_S tAttr, const AX_IVPS_POINT_S tPoint[], AX_U32 nPointNum);
AX_S32 AX_IVPS_DrawRect(const AX_IVPS_RGN_CANVAS_INFO_S *ptCanvas, AX_IVPS_GDI_ATTR_S tAttr, AX_IVPS_RECT_S tRect);


/***************************************************************************************************************/
/*                                                   GDC                                                       */
/***************************************************************************************************************/

/*
 * AX_IVPS_DEWARP_ATTR_S
 * GDC only support AX_YUV420_SEMIPLANAR format.
 * Note: nDstStride should be 64 bytes aligned,
 * nDstWidth should be 16 bytes aligned, nDstHeight should be 2 bytes aligned,
 * eCompressMode should be set to NONE.
 */
typedef struct axIVPS_DEWARP_ATTR_S
{
    AX_U16 nDstWidth;  /* dst width, if 0, equal to width  of src image, should be 2 align */
    AX_U16 nDstHeight; /* dst height, if 0, equal to height of src image, should be 2 align */
    AX_U32 nDstStride; /* dst stride, should be align to 64 */
    AX_IMG_FORMAT_E eImgFormat;
    AX_IVPS_GDC_CFG_S tGdcCfg;
} AX_IVPS_DEWARP_ATTR_S;

AX_S32 AX_IVPS_Dewarp(const AX_VIDEO_FRAME_S *pSrc, AX_VIDEO_FRAME_S *pDst, const AX_IVPS_DEWARP_ATTR_S *ptAttr);

#ifdef __cplusplus
}
#endif


/***************************************************************************************************************/
/*                                                   NPU                                                       */
/***************************************************************************************************************/
/*!
* Function Description:
*< br / > Realize crop and scaling operation by NPU, and resize the given image to the same shape.
*< br / > The input/output image size, stripe and box coordinates need to be set to an even number.
*\ warning Support NV12/NV21/RGB888/BGR888/BITMAP, Images in input and output formats must be consistent.
*\ warning Supports processing NV12/NV21 images with y and UV separation; Support 4K resolution;
           The horizontal or vertical reduction scale shall not be rounded up more than 32.
*\ warning In scenarios with large I/O resolution, hardware restrictions may be triggered. When an error is reported,
           detailed parameter information will be printed.
*\ param ptSrc: Source image pointer. It supports configuring the width direction stripe. If it is 0, the default stride is the width size.
*\ param tBox: each subgraph bounding_ Box pointer array. nW and nH are required to be even numbers.
           'tBox[i]' shall not be empty. If tBox[i] is equal to the original image, it means that no crop operation is performed.
*\ param nCropNum: number of subgraphs, nCropNum: 1~16.
*\ param ptDst: Output image pointer array. Multiple results can be output at the same time, but the image format is the same.
           The configuration of width direction stripe is supported. If it is 0, the default stripe is the width size.
*\ param ptAspectRatio:
          eMode:      Only support AX_IVPS_ASPECT_RATIO_STRETCH and AX_IVPS_ASPECT_RATIO_AUTO.
          eAligns[0]: Output image horizontal alignment.
          eAligns[1]: Output image vertical alignment.
          tcolor:     Used to fill the border with the specified color. Configuration is not supported temporarily. The default color is black.
*/
AX_S32 AX_IVPS_CropResizeNpu(const AX_VIDEO_FRAME_S *ptSrc,const AX_IVPS_BOX_S tBox[], AX_U32 nCropNum,
                                 AX_VIDEO_FRAME_S *ptDst[], const AX_IVPS_ASPECT_RATIO_S *ptAspectRatio);


#endif /* _AX_IVPS_API_H_ */
