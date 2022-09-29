/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "MemMgr.h"
#include <string.h>

#define PHY_MEM_ALIGN_SIZE  (128)
#define MEM                 "MEM"

//////////////////////////////////////////////////////////////////////////
CMemMgr::CMemMgr(void)
{
    m_arrMemAllocated.clear();
}

CMemMgr::~CMemMgr(void)
{
    gc();
}

AX_U32 CMemMgr::gc(void)
{
    for (auto &m : m_arrMemAllocated) {
        AX_SYS_MemFree(m.first, m.second);
    }

    AX_U32 nCount = m_arrMemAllocated.size();
    m_arrMemAllocated.clear();
    return nCount;
}

AX_BOOL CMemMgr::Alloc(AX_U64 *pPhyAddr, void **ppVirAddr, AX_U32 u32Size, const void *pData/* = NULL */)
{
    AX_S32 ret = AX_SYS_MemAlloc(pPhyAddr, ppVirAddr, u32Size, PHY_MEM_ALIGN_SIZE, NULL);
    if (AX_SDK_PASS != ret) {
        LOG_M_E(MEM, "AX_SYS_MemAlloc() failed, size:0x%x, error: 0x%x", u32Size, ret);
        return AX_FALSE;
    }

    if (pData) {
        memcpy(*ppVirAddr, pData, u32Size);
    }

    return AX_TRUE;
}

AX_BOOL CMemMgr::Free(AX_U64 phyAddr, void *pVirAddr)
{
    AX_S32 ret = AX_SYS_MemFree(phyAddr, pVirAddr);
    if (AX_SDK_PASS != ret) {
        LOG_M_E(MEM, "AX_SYS_MemFree() failed, error: 0x%x", ret);
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_BOOL CMemMgr::AllocEx(AX_U64 *pPhyAddr, void **ppVirAddr, AX_U32 u32Size, const void *pData/* = NULL */, AX_BOOL bDeleteLater/* = AX_FALSE*/)
{
    if (!Alloc(pPhyAddr, ppVirAddr, u32Size, pData)) {
        return AX_FALSE;
    }

    if (bDeleteLater) {
        m_arrMemAllocated.emplace_back(*pPhyAddr, *ppVirAddr);
    }

    return AX_TRUE;
}

AX_BOOL CMemMgr::CacheAlloc(AX_U64 *pPhyAddr, void **ppVirAddr, AX_U32 u32Size, const void *pData/* = NULL */)
{
    AX_S32 ret = AX_SYS_MemAllocCached(pPhyAddr, ppVirAddr, u32Size, PHY_MEM_ALIGN_SIZE, NULL);
    if (AX_SDK_PASS != ret) {
        LOG_M_E(MEM, "AX_SYS_MemAllocCached() failed, size:0x%x, error: 0x%x", u32Size, ret);
        return AX_FALSE;
    }

    if (pData) {
        memcpy(*ppVirAddr, pData, u32Size);
    }

    return AX_TRUE;
}

AX_BOOL CMemMgr::CacheFlush(AX_U64 phyAddr, void *pVirAddr, AX_U32 u32Size)
{
    AX_S32 ret = AX_SYS_MemFlushCache(phyAddr, pVirAddr, u32Size);
    if (AX_SDK_PASS != ret) {
        LOG_M_E(MEM, "AX_SYS_MemFlushCache() failed, phy: 0x%llx, size:0x%x, error: 0x%x", phyAddr, u32Size, ret);
        return AX_FALSE;
    }

    return AX_TRUE;
}