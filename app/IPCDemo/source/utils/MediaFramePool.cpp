/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "MediaFramePool.h"

#define MF_POOL "MF_POOL"

AX_BOOL CMediaFramePool::InitImgPool(AX_S32 nImageSize, AX_U32 nPoolCapacity/* = DEFAULT_POOL_IMG_NUM*/)
{
    if (nImageSize <= 0) {
        LOG_M_E(MF_POOL, "Initial image pool failed with wrong size(%d)!", nImageSize);
        return AX_FALSE;
    }

    if (0 == nPoolCapacity || nPoolCapacity > MAX_POOL_IMG_NUM) {
        LOG_M_E(MF_POOL, "Initial image pool failed with wrong capacity(%d)!", nPoolCapacity);
        return AX_FALSE;
    }

    m_vecPoolImg.reserve(nPoolCapacity);

    for (AX_U32 i = 0; i < nPoolCapacity; i++) {
        CMediaFrame *pMediaFrame = new (std::nothrow) CMediaFrame();
        AX_S32 ret = AX_SYS_MemAlloc(&pMediaFrame->tFrame.tFrameInfo.stVFrame.u64PhyAddr[0], (AX_VOID **)&pMediaFrame->tFrame.tFrameInfo.stVFrame.u64VirAddr[0], nImageSize, 0x100, NULL);
        if (0 != ret) {
            LOG_M_E(MF_POOL, "AX_SYS_MemAlloc fail, ret=0x%x", ret);
            return AX_FALSE;
        }

        m_vecPoolImg.emplace_back(pMediaFrame);
    }

    return AX_TRUE;
}

AX_VOID CMediaFramePool::DeInitImgPool()
{
    for (AX_U32 i = 0; i < m_vecPoolImg.size(); i++) {
        if (0 != m_vecPoolImg[i]->tFrame.tFrameInfo.stVFrame.u64PhyAddr[0] || 0 != m_vecPoolImg[i]->tFrame.tFrameInfo.stVFrame.u64VirAddr[0]) {
            AX_SYS_MemFree(m_vecPoolImg[i]->tFrame.tFrameInfo.stVFrame.u64PhyAddr[0], (AX_VOID *)m_vecPoolImg[i]->tFrame.tFrameInfo.stVFrame.u64VirAddr[0]);
        }
        SAFE_DELETE_PTR(m_vecPoolImg[i]);
    }
}

CMediaFrame* CMediaFramePool::BorrowImg()
{
    std::lock_guard<std::mutex> lck(m_poolMutex);

    for (AX_U32 i = 0;i < m_vecPoolImg.size(); i++) {
        if (0 == m_vecPoolImg[i]->tFrame.tFrameInfo.stVFrame.u32Width && 0 == m_vecPoolImg[i]->tFrame.tFrameInfo.stVFrame.u32Height) {
            return m_vecPoolImg[i];
        }
    }

    return nullptr;
}

AX_VOID CMediaFramePool::GiveBackImg(CMediaFrame* pFrame)
{
    std::lock_guard<std::mutex> lck(m_poolMutex);

    if (pFrame) {
        pFrame->tFrame.tFrameInfo.stVFrame.u32Width  = 0;
        pFrame->tFrame.tFrameInfo.stVFrame.u32Height = 0;
    }
}