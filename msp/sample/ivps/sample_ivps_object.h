/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_IVPS_OBJECT_86D33EF4_3665_42A0_A2F0_DE2D28E37865_H_
#define _SAMPLE_IVPS_OBJECT_86D33EF4_3665_42A0_A2F0_DE2D28E37865_H_

#include "ax_ivps_api.h"

typedef AX_S32 IVPS_GRP;

#define SAMPLE_IVPS_SENDFRAME_TIMEOUT (-1) //-1: block
#define SAMPLE_IVPS_GETFRAME_TIMEOUT (-1)  //(-1)  //-1: block

#define SAMPLE_MAX_IVPS_GRP_NUM (2)
#define SAMPLE_MAX_IVPS_CHN_NUM (128)

#define DEBUG_SEND_FRAME
#define DEBUG_LINK_FRAME

#define CHECK_RESULT(_statement)                                      \
	{                                                                 \
		AX_S32 _ret = (_statement);                                   \
		if (0 != _ret)                                                \
		{                                                             \
			ALOGE("ERROR [func: %s line: %d]\n", __func__, __LINE__); \
			return _ret;                                              \
		}                                                             \
	}

/* link */
#define SAMPLE_IVPS_GRP0 (IVPS_GRP)0
/* user */
#define SAMPLE_IVPS_GRP1 (IVPS_GRP)1

#define SAMPLE_IVPS_CHN0 (IVPS_CHN)0
#define SAMPLE_IVPS_CHN1 (IVPS_CHN)1
#define SAMPLE_IVPS_CHN2 (IVPS_CHN)2
#define SAMPLE_IVPS_CHN3 (IVPS_CHN)3
#define SAMPLE_IVPS_CHN4 (IVPS_CHN)4

#define SAMPLE_IVPS_GRP0_CHN0_WIDTH 800
#define SAMPLE_IVPS_GRP0_CHN0_HEIGHT 480
#define SAMPLE_IVPS_GRP0_CHN0_STRIDE 800
#define SAMPLE_IVPS_GRP0_CHN1_WIDTH (1920)
#define SAMPLE_IVPS_GRP0_CHN1_HEIGHT (1080)
#define SAMPLE_IVPS_GRP0_CHN1_STRIDE (1920)
#define SAMPLE_IVPS_GRP0_CHN2_WIDTH (1280)
#define SAMPLE_IVPS_GRP0_CHN2_HEIGHT (768)
#define SAMPLE_IVPS_GRP0_CHN2_STRIDE (1280)

#define SAMPLE_IVPS_CHN0_WIDTH_DEFAULT (1920)
#define SAMPLE_IVPS_CHN0_HEIGHT_DEFAULT (1080)
#define SAMPLE_IVPS_CHN0_STRIDE_DEFAULT (1920)
#define SAMPLE_IVPS_CHN1_WIDTH_DEFAULT (1920)
#define SAMPLE_IVPS_CHN1_HEIGHT_DEFAULT (1080)
#define SAMPLE_IVPS_CHN1_STRIDE_DEFAULT (1920)
#define SAMPLE_IVPS_CHN2_WIDTH_DEFAULT (1280)
#define SAMPLE_IVPS_CHN2_HEIGHT_DEFAULT (768)
#define SAMPLE_IVPS_CHN2_STRIDE_DEFAULT (1280)
#define SAMPLE_IVPS_CHN3_WIDTH_DEFAULT (1920)
#define SAMPLE_IVPS_CHN3_HEIGHT_DEFAULT (1080)
#define SAMPLE_IVPS_CHN3_STRIDE_DEFAULT (1920)

#define SAMPLE_UP_USER_PIPE0 (0)
#define SAMPLE_UP_USER_CHN0 (0)
#define SAMPLE_UP_USER_CHN1 (1)

#define SAMPLE_DOWN_USER_PIPE0 (0)
#define SAMPLE_DOWN_USER_CHN0 (0)
#define SAMPLE_DOWN_USER_CHN1 (1)

#define SAMPLE_USER_CHN0_WIDTH (800)
#define SAMPLE_USER_CHN0_HEIGHT (480)
#define SAMPLE_USER_CHN0_STRIDE (800)
#define SAMPLE_USER_CHN1_WIDTH (1920)
#define SAMPLE_USER_CHN1_HEIGHT (1080)
#define SAMPLE_USER_CHN1_STRIDE (1920)

#define SAMPLE_MAX_DYNAMIC_OUTPUT (3)

enum
{
	COLOR_GREEN = 0x000000,
	COLOR_RED = 0x0000FF,
	COLOR_BLUE = 0x00FF00,
	COLOR_PURPLE = 0x00FFFF,
	COLOR_DARK_GREEN = 0xFF0000,
	COLOR_YELLOW = 0xFF00FF,
	COLOR_DARK_BLACK = 0x008080,
	COLOR_GRAY = 0x808080,
	COLOR_WHITE = 0xFF8080,
	COLOR_NUM = 9
};

typedef struct axIVPS_IMAGE_S
{
	AX_U16 nW;
	AX_U16 nH;
	AX_U32 nStride;
	AX_IMG_FORMAT_E eFormat;
	AX_U32 nPhyAddr;
	AX_VOID *pVirAddr;
	AX_POOL PoolId;
	AX_BLK BlkId;
} AX_IVPS_IMAGE_S;


typedef struct
{
	IVPS_GRP IvpsGrp;
	IVPS_CHN IvpsChn;
	AX_MOD_INFO_S tSrcMod;
	AX_MOD_INFO_S tDstMod;
	AX_S32 nIvpsRepeatNum;
	AX_U32 nPhyAddr;
	AX_U8 *pVirAddr;
} SAMPLE_IVPS_CHN_S;

typedef struct axSAMPLE_IVPS_INPUT_INFO_S
{
	AX_U16 nX;
	AX_U16 nY;
	AX_U16 nW;
	AX_U16 nH;
	AX_U32 nStride;
	AX_IMG_FORMAT_E eFormat;
	IVPS_GRP IvpsGrp;
} SAMPLE_IVPS_INPUT_INFO_S;

typedef struct
{
	AX_S32 nEfd;
	IVPS_GRP nIvpsGrp;
	SAMPLE_IVPS_INPUT_INFO_S tInputInfo;
	AX_IVPS_GRP_ATTR_S tGrpAttr;
	AX_S32 nIvpsRepeatNum;
	AX_S32 nIvpsAttrRepeatNum;
	AX_U32 nPhyAddr[2];
	AX_U8 *pVirAddr[2];
	AX_BLK BlkId0;
	AX_BLK BlkId1;
	char *pFilePath;
	AX_IVPS_PIPELINE_ATTR_S tPipelineAttr;

	struct
	{
		AX_S32 nFd;
		AX_BOOL bEmpty;
		IVPS_CHN nIvpsChn;
	} arrOutChns[SAMPLE_MAX_IVPS_CHN_NUM];
} SAMPLE_IVPS_GRP_S;

typedef struct
{
	char *pMeshFile;
	AX_IVPS_GDC_CFG_S tGdcCfg;
} SAMPLE_IVPS_DEWARP_CFG_S;

typedef struct
{
	char *pImgFile;
	AX_IVPS_IMAGE_S tImage;
	AX_IVPS_RECT_S tRect;

	AX_U32 nAlpha;
	AX_U32 nChn;
	AX_U32 nColor; /*for rectangle*/
	AX_U32 nLineW; /*for rectangle*/
	AX_IVPS_MOSAIC_BLK_SIZE_E eBlkSize;
} SAMPLE_IVPS_IMAGE_S;


AX_BOOL ThreadLoopStateGet(AX_VOID);
AX_VOID ThreadLoopStateSet(AX_BOOL bValue);

AX_S32 IVPS_StartGrp(SAMPLE_IVPS_GRP_S *p);
AX_S32 IVPS_StopGrp(const SAMPLE_IVPS_GRP_S *p);
AX_S32 IVPS_ThreadStart(AX_VOID *p);
AX_S32 IVPS_ThreadStartV2(AX_VOID *src, AX_VOID *dst);
AX_S32 IVPS_AttrThreadStart(SAMPLE_IVPS_GRP_S *pGrp);

AX_S32 IVPS_StartSingle(IVPS_CHN IvpsChn);
AX_S32 IVPS_StopSingle(IVPS_CHN IvpsChn);
AX_S32 IVPS_ProcessSingle(IVPS_CHN IvpsChn);

char *FrameInfo_Get(char *optArg, AX_VIDEO_FRAME_INFO_S *ptFrame);
char *OverlayInfo_Get(char *optArg, AX_VIDEO_FRAME_INFO_S *ptOverlay);

AX_S32 CoverInfo_Get(char *optArg, AX_IVPS_RGN_MOSAIC_S *ptCover);
AX_S32 ImageBuf_Get(AX_VIDEO_FRAME_INFO_S *ptImage, char *pImgFile);

#endif /* _SAMPLE_IVPS_OBJECT_86D33EF4_3665_42A0_A2F0_DE2D28E37865_H_ */
