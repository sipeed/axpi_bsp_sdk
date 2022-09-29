/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_IVPS_UTIL_07C3D107_75E3_491C_AF1F_6E36DEDC43F8_H_
#define _SAMPLE_IVPS_UTIL_07C3D107_75E3_491C_AF1F_6E36DEDC43F8_H_

#include <signal.h>
#include "ax_sys_api.h"
#include "ax_ivps_api.h"

#define SAMPLE_PHY_MEM_ALIGN_SIZE (16)

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])
#define ALIGN_UP(x, align) (((x) + ((align)-1)) & ~((align)-1))
#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

#define IVPS_BUF_POOL_MEM_ALLOC
//#define IVPS_SAMPLE_LOG_EN

typedef struct
{
    AX_U32 nSize;
    AX_U32 nCnt;
} BLK_INFO_S;

AX_U64 GetTickCount(AX_VOID);
AX_BOOL Split(char *pSrc, const char *pDelim, char **ppDst, AX_S32 nDstCnt, AX_S32 *pNum);
void Swap(int *a, int *b);

AX_U32 CalcImgSize(AX_U32 nStride, AX_U32 nW, AX_U32 nH, AX_IMG_FORMAT_E eType, AX_U32 nAlign);
AX_BOOL LoadImage(const char *pszImge, AX_U32 pImgSize, AX_U64 *pPhyAddr, AX_VOID **ppVirAddr);
AX_BOOL LoadImageExt(AX_POOL PoolId, const char *pszImge, AX_U32 pImgSize, AX_BLK *BlkId, AX_BOOL isYUVSplit, AX_U64 *nPhyAddr, AX_VOID **pVirAddr);
AX_BOOL LoadPureHex(AX_POOL PoolId, const char *pszImge, AX_U32 pImgSize, AX_BLK *BlkId, AX_U64 *nPhyAddr, AX_VOID **pVirAddr);

AX_BOOL SaveImage(const char *pszImge, AX_U32 pBufSize, AX_U64 pPhyAddr, AX_VOID *ppVirAddr);
AX_VOID SaveFile(AX_VIDEO_FRAME_INFO_S *tDstFrame, AX_S32 nGrpIdx, AX_S32 nChnIdx, char *pFilePath);
AX_VOID SaveFileExt(AX_VIDEO_FRAME_S *tDstFrame, AX_S32 nGrpIdx, AX_S32 nChnIdx, char *pFilePath, char *pFileName);

AX_S32 IVPS_PoolFloorInit(BLK_INFO_S *pBlkInfo, AX_U32 nNum);
AX_S32 BufCreate(AX_POOL *PoolId, AX_U32 nBlkSize, AX_U32 nBlkCnt);
AX_S32 BufDestroy(AX_POOL PoolId);
AX_S32 BufDestroyAll(AX_VOID);
AX_S32 BufPoolBlockAddrGet(AX_POOL PoolId, AX_U32 BlkSize, AX_U64 *nPhyAddr, AX_VOID **pVirAddr, AX_BLK *BlkId);

typedef AX_VOID (*pTimeOutHandler)(AX_S32 nSigNo, siginfo_t *pInfo, AX_VOID *pArgs);
AX_BOOL StartFpsTimer(AX_U32 microseconds, pTimeOutHandler pfn);
AX_VOID KillFpsTimer(AX_VOID);
AX_VOID usDelay(AX_U32 nUs);
char *ExtractFilePath(char *pFile);

#ifdef IVPS_SAMPLE_LOG_EN
#define ALOGD(fmt, ...) printf("\033[1;30;37mDEBUG  :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // white
#define ALOGI(fmt, ...) printf("\033[1;30;32mINFO   :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // green
#else
#define ALOGD(fmt, ...) \
    do                  \
    {                   \
    } while (0)
#define ALOGI(fmt, ...) \
    do                  \
    {                   \
    } while (0)
#endif
#define ALOGW(fmt, ...) printf("\033[1;30;33mWARN   :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // yellow
#define ALOGE(fmt, ...) printf("\033[1;30;31mERROR  :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // red
#define ALOGN(fmt, ...) printf("\033[1;30;37mINFO   :[%s:%d] " fmt "\033[0m\n", __func__, __LINE__, ##__VA_ARGS__) // white

#endif /* _SAMPLE_IVPS_UTIL_07C3D107_75E3_491C_AF1F_6E36DEDC43F8_H_ */
