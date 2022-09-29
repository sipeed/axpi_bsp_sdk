/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#ifndef _AX_SYS_API_
#define _AX_SYS_API_
#include "ax_global_type.h"
#include "ax_base_type.h"
#include "ax_pool_type.h"

/* flags */
#define AX_MEM_CACHED (1 << 1)    /* alloc mem is cached */
#define AX_MEM_NONCACHED (1 << 2) /* alloc mem is not cached */

/*Error Code*/
#define AX_ERR_CMM_ILLEGAL_PARAM     0x800B000A
#define AX_ERR_CMM_NULL_PTR          0x800B000B
#define AX_ERR_CMM_NOTREADY          0x800B0010
#define AX_ERR_CMM_NOMEM             0x800B0018
#define AX_ERR_CMM_MMAP_FAIL         0x800B0080
#define AX_ERR_CMM_MUNMAP_FAIL       0x800B0081
#define AX_ERR_CMM_FREE_FAIL         0x800B0082
#define AX_ERR_CMM_UNKNOWN           0x800B0083

#define AX_ERR_PTS_ILLEGAL_PARAM     0x800B020A
#define AX_ERR_PTS_NULL_PTR          0x800B020B
#define AX_ERR_PTS_NOTREADY          0x800B0210
#define AX_ERR_PTS_NOT_PERM          0x800B0215

#define AX_ERR_LINK_ILLEGAL_PARAM    0x800B030A
#define AX_ERR_LINK_NULL_PTR         0x800B030B
#define AX_ERR_LINK_NOTREADY         0x800B0310
#define AX_ERR_LINK_NOT_SUPPORT      0x800B0314
#define AX_ERR_LINK_NOT_PERM         0x800B0315
#define AX_ERR_LINK_UNEXIST          0x800B0317
#define AX_ERR_LINK_TABLE_FULL       0x800B0380
#define AX_ERR_LINK_TABLE_EMPTY      0x800B0381
#define AX_ERR_LINK_UNKNOWN          0x800B0382

#define AX_ERR_POWER_ILLEGAL_PARAM   0x800B040A
#define AX_ERR_POWER_NULL_PTR        0x800B040B
#define AX_ERR_POWER_NOTREADY        0x800B0410
#define AX_ERR_POWER_NOT_SUPPORT     0x800B0414
#define AX_ERR_POWER_NOT_PERM        0x800B0415

#define AX_ERR_CLK_ILLEGAL_PARAM     0x800B050A
#define AX_ERR_CLK_NOTREADY          0x800B0510

/* PowerManager Notify Callback */
typedef AX_S32 (*NotifyEventCallback)(const AX_NOTIFY_EVENT_E event,AX_VOID * pdata);

#ifdef __cplusplus
extern "C"
{
#endif

AX_S32 AX_SYS_Init(AX_VOID);
AX_S32 AX_SYS_Deinit(AX_VOID);

/* CMM API */
AX_S32 AX_SYS_MemAlloc(AX_U64 *phyaddr, AX_VOID **pviraddr, AX_U32 size, AX_U32 align, const AX_S8 *token);
AX_S32 AX_SYS_MemAllocCached(AX_U64 *phyaddr, AX_VOID **pviraddr, AX_U32 size, AX_U32 align, const AX_S8 *token);
AX_S32 AX_SYS_MemFlushCache(AX_U64 phyaddr, AX_VOID *pviraddr, AX_U32 size);
AX_S32 AX_SYS_MemInvalidateCache(AX_U64 phyaddr, AX_VOID *pviraddr, AX_U32 size);
AX_S32 AX_SYS_MemFree(AX_U64 phyaddr, AX_VOID *pviraddr);
AX_VOID * AX_SYS_Mmap(AX_U64 phyaddr, AX_U32 size);
AX_VOID * AX_SYS_MmapCache(AX_U64 phyaddr, AX_U32 size);
AX_VOID * AX_SYS_MmapFast(AX_U64 phyaddr, AX_U32 size);
AX_VOID * AX_SYS_MmapCacheFast(AX_U64 phyaddr, AX_U32 size);
AX_S32 AX_SYS_Munmap(AX_VOID *pviraddr, AX_U32 size);
AX_S32 AX_SYS_MflushCache(AX_U64 phyaddr, AX_VOID *pviraddr, AX_U32 size);
AX_S32 AX_SYS_MinvalidateCache(AX_U64 phyaddr, AX_VOID *pviraddr, AX_U32 size);
AX_S32 AX_SYS_MemGetBlockInfoByPhy(AX_U64 phyaddr, AX_S32 *pmemType, AX_VOID **pviraddr, AX_U32 *pblockSize);
AX_S32 AX_SYS_MemGetBlockInfoByVirt(AX_VOID *pviraddr, AX_U64 *phyaddr, AX_S32 *pmemType);
AX_S32 AX_SYS_MemGetPartitionInfo(AX_CMM_PARTITION_INFO_T *pCmmPartitionInfo);
AX_S32 AX_SYS_MemSetConfig(const AX_MOD_INFO_S *pModInfo,const AX_S8 *pPartitionName);
AX_S32 AX_SYS_MemGetConfig(const AX_MOD_INFO_S *pModInfo,AX_S8 *pPartitionName);

/* LINK API */
AX_S32 AX_SYS_Link(const AX_MOD_INFO_S *pSrc,const AX_MOD_INFO_S *pDest);
AX_S32 AX_SYS_UnLink(const AX_MOD_INFO_S *pSrc,const AX_MOD_INFO_S *pDest);
AX_S32 AX_SYS_GetLinkByDest(const AX_MOD_INFO_S *pDest,AX_MOD_INFO_S *pSrc);
AX_S32 AX_SYS_GetLinkBySrc(const AX_MOD_INFO_S *pSrc,AX_LINK_DEST_S *pLinkDest);

/* POOL API */
AX_S32 AX_POOL_SetConfig(const AX_POOL_FLOORPLAN_T *pPoolFloorPlan);
AX_S32 AX_POOL_GetConfig (AX_POOL_FLOORPLAN_T *pPoolFloorPlan);
AX_S32 AX_POOL_Init (AX_VOID);
AX_S32 AX_POOL_Exit (AX_VOID);
AX_POOL AX_POOL_CreatePool(AX_POOL_CONFIG_T *pPoolConfig);
AX_S32 AX_POOL_MarkDestroyPool(AX_POOL PoolId);
AX_BLK AX_POOL_GetBlock(AX_POOL PoolId, AX_U64 BlkSize,const AX_S8 *pPartitionName);
AX_S32 AX_POOL_ReleaseBlock(AX_BLK BlockId);
AX_BLK AX_POOL_PhysAddr2Handle(AX_U64 PhysAddr);
AX_U64 AX_POOL_Handle2PhysAddr(AX_BLK BlockId);
AX_U64 AX_POOL_Handle2MetaPhysAddr(AX_BLK BlockId);
AX_POOL AX_POOL_Handle2PoolId(AX_BLK BlockId);
AX_S32 AX_POOL_MmapPool(AX_POOL PoolId);
AX_S32 AX_POOL_MunmapPool(AX_POOL PoolId);
AX_S32 AX_POOL_FlushCache(AX_U64 PhysAddr, AX_VOID *pVirAddr,AX_U32 Size);
AX_VOID *AX_POOL_GetBlockVirAddr(AX_BLK BlockId);
AX_VOID *AX_POOL_GetMetaVirAddr(AX_BLK BlockId);
AX_S32 AX_POOL_IncreaseRefCnt (AX_BLK BlockId,AX_MOD_ID_E ModId);
AX_S32 AX_POOL_DecreaseRefCnt (AX_BLK BlockId,AX_MOD_ID_E ModId);

/* PTS API */
AX_S32 AX_SYS_GetCurPTS(AX_U64 *pu64CurPTS);
AX_S32 AX_SYS_InitPTSBase(AX_U64 u64PTSBase);
AX_S32 AX_SYS_SyncPTS(AX_U64 u64PTSBase);

/* LOG API */
AX_S32 AX_SYS_SetLogLevel(AX_LOG_LEVEL_E target); /* local config */
AX_S32 AX_SYS_SetLogTarget(AX_LOG_TARGET_E target); /* local config */
AX_S32 AX_SYS_EnableTimestamp(AX_BOOL enable); /* global config */

/* PM API */
AX_S32 AX_SYS_Sleep(AX_VOID);
AX_S32 AX_SYS_WakeLock(const AX_MOD_ID_E ModId);
AX_S32 AX_SYS_WakeUnlock(const AX_MOD_ID_E ModId);
AX_S32 AX_SYS_RegisterEventCb(const AX_MOD_ID_E ModId,NotifyEventCallback pFunction,AX_VOID * pData);
AX_S32 AX_SYS_UnregisterEventCb(const AX_MOD_ID_E ModId);

/* CLOCK API */
AX_S32 AX_SYS_CLK_SetLevel(AX_SYS_CLK_LEVEL_E nLevel);
AX_SYS_CLK_LEVEL_E AX_SYS_CLK_GetLevel(AX_VOID);
AX_S32 AX_SYS_CLK_Single_RateSet(AX_SYS_CLK_ID_E clkId, AX_ULONG rate);

#ifdef __cplusplus
}
#endif

#endif //_AX_SYS_API_
