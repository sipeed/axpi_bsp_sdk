/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_POOL_TYPE_H_
#define _AX_POOL_TYPE_H_
#include "ax_global_type.h"
#include "ax_base_type.h"

#define AX_INVALID_POOLID  (-1U)
#define AX_INVALID_BLOCKID (-1U)

#define AX_MAX_POOLS 16
#define AX_MAX_COMM_POOLS 16
#define AX_MAX_PRIV_POOLS 16
#define AX_MAX_BLKS_PER_POOL  256
#define AX_MAX_BLK_REFCNT 64

#define MAX_PARTITION_NAME_LEN 32
#define MAX_PARTITION_COUNT 16

#define AX_ERR_POOL_ILLEGAL_PARAM  0x800B010A
#define AX_ERR_POOL_NULL_PTR       0x800B010B
#define AX_ERR_POOL_NOTREADY       0x800B0110
#define AX_ERR_POOL_BUSY           0x800B0111
#define AX_ERR_POOL_NOT_SUPPORT    0x800B0114
#define AX_ERR_POOL_NOT_PERM       0x800B0115
#define AX_ERR_POOL_UNEXIST        0x800B0117
#define AX_ERR_POOL_NOMEM          0x800B0118
#define AX_ERR_POOL_MMAP_FAIL      0x800B0180
#define AX_ERR_POOL_MUNMAP_FAIL    0x800B0181
#define AX_ERR_POOL_BLKFREE_FAIL   0x800B0182
#define AX_ERR_POOL_UNKNOWN        0x800B0183

typedef AX_U32 AX_POOL;
typedef AX_U32 AX_BLK;

typedef enum {
    POOL_CACHE_MODE_NONCACHE = 0,
    POOL_CACHE_MODE_CACHED = 1,
    POOL_CACHE_MODE_BUTT
} AX_POOL_CACHE_MODE_E;

typedef enum {
    POOL_SOURCE_COMMON = 0,
    POOL_SOURCE_PRIVATE = 1,
    POOL_SOURCE_USER = 2,
    POOL_SOURCE_BUTT
} AX_POOL_SOURCE_E;

typedef struct {
    AX_U64 MetaSize;
    AX_U64 BlkSize;
    AX_U32 BlkCnt; /* range:(0,256] */
    AX_BOOL IsMergeMode; /* logically merged with common pool, make common pool bigger*/
    AX_POOL_CACHE_MODE_E CacheMode;
    AX_S8 PartitionName[MAX_PARTITION_NAME_LEN];
} AX_POOL_CONFIG_T;

typedef struct {
    AX_POOL_CONFIG_T CommPool[AX_MAX_COMM_POOLS];
} AX_POOL_FLOORPLAN_T;

typedef struct {
    AX_U64 PhysAddr;
    AX_U32 SizeKB;
    AX_S8  Name[MAX_PARTITION_NAME_LEN];
} AX_PARTITION_INFO_T;

typedef struct {
    AX_U32 PartitionCnt;/* range:1~MAX_PARTITION_COUNT */
    AX_PARTITION_INFO_T PartitionInfo[MAX_PARTITION_COUNT];
} AX_CMM_PARTITION_INFO_T;
#endif //_AX_POOL_TYPE_H_
