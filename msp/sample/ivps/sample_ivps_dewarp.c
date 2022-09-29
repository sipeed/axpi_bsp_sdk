/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "sample_ivps_region.h"
#include "sample_ivps_object.h"
#include "sample_ivps_util.h"
#include "sample_ivps_help.h"
#include "sample_ivps_dewarp.h"

//#define __GDC_INPUT_3840x2160__
#define __GDC_INPUT_2688x1520__
//#define __GDC_INPUT_1920x1080__
//#define __GDC_INPUT_832x480__
extern SAMPLE_IVPS_GRP_S gSampleIvpsSingle;

SAMPLE_IVPS_DEWARP_S IvpsDewarpGrp[] = {
#ifdef __GDC_INPUT_832x480__
	{
		.pMeshFile = "/opt/data/ivps/mesh/480p_r0.bin",
		.nSrcWidth = 832,
		.nSrcHeight = 480,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 832,
			.nDstHeight = 480,
			.nDstStride = 832,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 32,
				.nInterStartY = 16,
				.nMeshStartX = 7,
				.nMeshStartY = 8,
				.nMeshWidth = 47,
				.nMeshHeight = 31,
			},
		},
	},
#endif
#ifdef __GDC_INPUT_3840x2160__
	{
		.pMeshFile = "/opt/data/ivps/mesh/4k_r0.bin",
		.nSrcWidth = 3840,
		.nSrcHeight = 2160,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 3840,
			.nDstHeight = 2160,
			.nDstStride = 3840,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 32,
				.nInterStartY = 16,
				.nMeshStartX = 7,
				.nMeshStartY = 8,
				.nMeshWidth = 47,
				.nMeshHeight = 31,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/4k_1080p_r0.bin",
		.nSrcWidth = 3840,
		.nSrcHeight = 2160,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 1920,
			.nDstHeight = 1080,
			.nDstStride = 1920,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 36,
				.nMeshStartX = 4,
				.nMeshStartY = 7,
				.nMeshWidth = 79,
				.nMeshHeight = 63,
			},
		},
	},
#endif
#ifdef __GDC_INPUT_1920x1080__
	{
		.pMeshFile = "/opt/data/ivps/mesh/1080p_720p_r0.bin",
		.nSrcWidth = 1920,
		.nSrcHeight = 1088,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 1280,
			.nDstHeight = 720,
			.nDstStride = 1280,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 63,
				.nMeshHeight = 47,
			},
		},
	},
#endif
#ifdef __GDC_INPUT_2688x1520__

	{
		.pMeshFile = "/opt/data/ivps/mesh/4m_1968x1104_r0.bin",
		.nSrcWidth = 2688,
		.nSrcHeight = 1520,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 1968,
			.nDstHeight = 1104,
			.nDstStride = 1984,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 79,
				.nMeshHeight = 63,
			},
		},
	},

	{
		.pMeshFile = "/opt/data/ivps/mesh/4m_1984x1104_r0.bin",
		.nSrcWidth = 2688,
		.nSrcHeight = 1520,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 1984,
			.nDstHeight = 1104,
			.nDstStride = 1984,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 79,
				.nMeshHeight = 63,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/2688x1520_r0.bin",
		.nSrcWidth = 2688,
		.nSrcHeight = 1520,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 2688,
			.nDstHeight = 1520,
			.nDstStride = 2688,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 111,
				.nMeshHeight = 63,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/2688x1520_r90.bin",
		.nSrcWidth = 2688,
		.nSrcHeight = 1520,
		.nRotate = 90,
		.tAttr = {
			.nDstWidth = 1520,
			.nDstHeight = 2688,
			.nDstStride = 1536,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.eRotation = AX_IVPS_ROTATION_90,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 63,
				.nMeshHeight = 111,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/2688x1520_r180.bin",
		.nSrcWidth = 2688,
		.nSrcHeight = 1520,
		.nRotate = 180,
		.tAttr = {
			.nDstWidth = 2688,
			.nDstHeight = 1520,
			.nDstStride = 2688,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.eRotation = AX_IVPS_ROTATION_180,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 111,
				.nMeshHeight = 63,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/2688x1520_r270.bin",
		.nSrcWidth = 2688,
		.nSrcHeight = 1520,
		.nRotate = 270,
		.tAttr = {
			.nDstWidth = 1520,
			.nDstHeight = 2688,
			.nDstStride = 1536,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.eRotation = AX_IVPS_ROTATION_270,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 63,
				.nMeshHeight = 111,
			},
		},
	},
#endif
#ifdef __GDC_INPUT_3840x2160__
	{
		.pMeshFile = "/opt/data/ivps/mesh/3840x2160_r0.bin",
		.nSrcWidth = 3840,
		.nSrcHeight = 2160,
		.nRotate = 0,
		.tAttr = {
			.nDstWidth = 3840,
			.nDstHeight = 2160,
			.nDstStride = 3840,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 143,
				.nMeshHeight = 95,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/3840x2160_r90.bin",
		.nSrcWidth = 3840,
		.nSrcHeight = 2160,
		.nRotate = 90,
		.tAttr = {
			.nDstWidth = 2160,
			.nDstHeight = 3840,
			.nDstStride = 2160,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 95,
				.nMeshHeight = 143,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/3840x2160_r180.bin",
		.nSrcWidth = 3840,
		.nSrcHeight = 2160,
		.nRotate = 180,
		.tAttr = {
			.nDstWidth = 3840,
			.nDstHeight = 2160,
			.nDstStride = 3840,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 143,
				.nMeshHeight = 95,
			},
		},
	},
	{
		.pMeshFile = "/opt/data/ivps/mesh/3840x2160_r270.bin",
		.nSrcWidth = 3840,
		.nSrcHeight = 2160,
		.nRotate = 270,
		.tAttr = {
			.nDstWidth = 2160,
			.nDstHeight = 3840,
			.nDstStride = 2160,
			.eImgFormat = AX_YUV420_SEMIPLANAR,
			.tGdcCfg = {
				.bDewarpEnable = AX_TRUE,
				.nInterStartX = 0,
				.nInterStartY = 0,
				.nMeshStartX = 0,
				.nMeshStartY = 0,
				.nMeshWidth = 95,
				.nMeshHeight = 143,
			},
		},
	},
#endif

};

static AX_S32 DewarpSingleCore(const AX_VIDEO_FRAME_INFO_S *ptFrame,
							   AX_IVPS_DEWARP_ATTR_S *ptDewarpAttr,
							   const char *pMeshPath,
							   char *pStorePath,
							   char *fileName)
{
	AX_S32 ret = 0;

	AX_VIDEO_FRAME_S tDstFrame;
	AX_BLK BlkId, MeshBlkId;
	AX_U32 nImgSize;
	AX_U64 nPhyAddr = 0;

	memset(&tDstFrame, 0, sizeof(tDstFrame));

	ptDewarpAttr->eImgFormat = AX_YUV420_SEMIPLANAR;

	nImgSize = CalcImgSize(ptDewarpAttr->nDstStride, ptDewarpAttr->nDstWidth, ptDewarpAttr->nDstHeight, ptDewarpAttr->eImgFormat, 16);
	CHECK_RESULT(BufPoolBlockAddrGet(ptFrame->u32PoolId, nImgSize, &tDstFrame.u64PhyAddr[0], (AX_VOID **)(&tDstFrame.u64VirAddr[0]), &BlkId));
	tDstFrame.u64PhyAddr[1] = tDstFrame.u64PhyAddr[0] + ptDewarpAttr->nDstStride * ptDewarpAttr->nDstHeight;

	memset((AX_VOID *)((AX_U32)tDstFrame.u64VirAddr[0]), 0x00, nImgSize);
	ptDewarpAttr->tGdcCfg.nMeshTableSize = 1188;
	LoadPureHex(-1, pMeshPath, 0, &MeshBlkId, &nPhyAddr, (AX_VOID **)&ptDewarpAttr->tGdcCfg.nMeshTableVirAddr);
	printf("Mesh nPhyAddr:%llx\n", nPhyAddr);
	AX_IVPS_Dewarp(&ptFrame->stVFrame, &tDstFrame, ptDewarpAttr);
	SaveFileExt(&tDstFrame, 0, 0, pStorePath, fileName);
	ret = AX_POOL_ReleaseBlock(BlkId);
	if (ret)
	{
		ALOGE("IVPS Release BlkId fail, ret = %x\n", ret);
	}
	ret = AX_POOL_ReleaseBlock(MeshBlkId);
	if (ret)
	{
		ALOGE("IVPS Release Mesh BlkId fail, ret = %x\n", ret);
	}

	return ret;
}

AX_S32 DewarpSingleSample(const AX_VIDEO_FRAME_INFO_S *ptFrame, char *pStorePath)
{
	AX_S32 i, ret = 0;
	char fileName[64] = {0};

	for (i = 0; i < sizeof(IvpsDewarpGrp) / sizeof(IvpsDewarpGrp[0]); i++)
	{
		if (ptFrame->stVFrame.u32Width != IvpsDewarpGrp[i].nSrcWidth || ptFrame->stVFrame.u32Height != IvpsDewarpGrp[i].nSrcHeight)
		{
			continue;
		}
		snprintf(fileName, 64, "Dewarp_%u", IvpsDewarpGrp[i].nRotate);
		ret = DewarpSingleCore(ptFrame, &IvpsDewarpGrp[i].tAttr, IvpsDewarpGrp[i].pMeshFile, pStorePath, fileName);
		if (ret)
		{
			ALOGE("IVPS Release BlkId fail, ret = %x\n", ret);
			return ret;
		}
	}
	return ret;
}

static AX_S32 DewarpSingleSample1(const AX_VIDEO_FRAME_INFO_S *ptFrame)
{
	AX_S32 ret = 0;
	char fileName[64] = {0};

	snprintf(fileName, 64, "Dewarp_%u", IvpsDewarpGrp[0].nRotate);
	printf("Dewarp1 stride:%d width:%d height:%d \n", IvpsDewarpGrp[0].tAttr.nDstStride, IvpsDewarpGrp[0].tAttr.nDstWidth, IvpsDewarpGrp[0].tAttr.nDstHeight);
	ret = DewarpSingleCore(ptFrame, &IvpsDewarpGrp[0].tAttr, IvpsDewarpGrp[0].pMeshFile, gSampleIvpsSingle.pFilePath, fileName);
	if (ret)
	{
		ALOGE("IVPS Release BlkId fail, ret = %x\n", ret);
	}
	return ret;
}

static AX_S32 DewarpSingleSample2(const AX_VIDEO_FRAME_INFO_S *ptFrame)
{
	AX_S32 ret = 0;
	char fileName[64] = {0};

	snprintf(fileName, 64, "Dewarp_%u", IvpsDewarpGrp[1].nRotate);
	printf("Dewarp2 stride:%d width:%d height:%d \n", IvpsDewarpGrp[1].tAttr.nDstStride, IvpsDewarpGrp[1].tAttr.nDstWidth, IvpsDewarpGrp[1].tAttr.nDstHeight);

	ret = DewarpSingleCore(ptFrame, &IvpsDewarpGrp[1].tAttr, IvpsDewarpGrp[1].pMeshFile, gSampleIvpsSingle.pFilePath, fileName);
	if (ret)
	{
		ALOGE("IVPS Release BlkId fail, ret = %x\n", ret);
	}
	return ret;
}

static AX_VOID *DewarpSingleThread1(AX_VOID *pArg)
{
	while (!ThreadLoopStateGet())
	{
		DewarpSingleSample1(pArg);
	}
	return (AX_VOID *)0;
}
static AX_VOID *DewarpSingleThread2(AX_VOID *pArg)
{
	while (!ThreadLoopStateGet())
	{
		DewarpSingleSample2(pArg);
	}
	return (AX_VOID *)0;
}

/*
 * IVPS_DewarpSingleThread()
 * Multithreaded run test.
 */
AX_S32 IVPS_DewarpSingleThread(const AX_VIDEO_FRAME_INFO_S *ptFrame)
{
	pthread_t tid = 0;

	if (0 != pthread_create(&tid, NULL, DewarpSingleThread1, (AX_VOID *)ptFrame))
	{
		return -1;
	}
	pthread_detach(tid);

	if (0 != pthread_create(&tid, NULL, DewarpSingleThread2, (AX_VOID *)ptFrame))
	{
		return -1;
	}
	pthread_detach(tid);
	return 0;
}
