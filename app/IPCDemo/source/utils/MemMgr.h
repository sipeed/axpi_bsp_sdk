/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _MEM_MGR_3246FC57_D0F1_4176_A1C3_3A16FC46D1CF_H_
#define _MEM_MGR_3246FC57_D0F1_4176_A1C3_3A16FC46D1CF_H_

#include "global.h"
#include <vector>

/**
 * CMA and cache memory allocate and free
 */
class CMemMgr
{
public:
    CMemMgr(void);
    ~CMemMgr(void);

    static AX_BOOL Alloc(AX_U64 *pPhyAddr, void **ppVirAddr, AX_U32 u32Size, const void *pData = NULL);
    static AX_BOOL Free(AX_U64 phyAddr, void *pVirAddr);
    static AX_BOOL CacheAlloc(AX_U64 *pPhyAddr, void **ppVirAddr, AX_U32 u32Size, const void *pData = NULL);
    static AX_BOOL CacheFlush(AX_U64 phyAddr, void *pVirAddr, AX_U32 u32Size);

    AX_BOOL AllocEx(AX_U64 *pPhyAddr, void **ppVirAddr, AX_U32 u32Size, const void *pData = NULL, AX_BOOL bDeleteLater = AX_FALSE);

    AX_U32 gc(void);

private:
    std::vector<std::pair<AX_U64, void *>> m_arrMemAllocated;
};


#endif /* _MEM_MGR_3246FC57_D0F1_4176_A1C3_3A16FC46D1CF_H_ */
