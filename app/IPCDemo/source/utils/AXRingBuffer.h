/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _RING_BUFFER_3246FC57_D0F1_4176_A1C3_3A16FC46D1CF_H_
#define _RING_BUFFER_3246FC57_D0F1_4176_A1C3_3A16FC46D1CF_H_

#include "global.h"
#include <tuple>
#include <mutex>

class CAXRingBuffer;

class CAXRingElement {
public:
    CAXRingElement() {
        this->pBuf = nullptr;
        this->nSize = 0;
        this->nChannel = 0;
        this->bIFrame = AX_FALSE;
        this->nPts = 0;
        this->nRefCount = 0;
        this->pParent = nullptr;
        this->pHeadBuf = nullptr;
        this->nHeadSize = 0;
    }

    CAXRingElement(AX_U8* pBuf, AX_U32 nSize, AX_U32 nChn, AX_U64 u64PTS = 0, AX_BOOL isIFrame = AX_FALSE,
                   AX_U8* pHeadBuf = nullptr, AX_U32 nHeadSize = 0) {
        this->nIndex = -1;
        this->pBuf = pBuf;
        this->nSize = nSize;
        this->nChannel = nChn;
        this->nPts = u64PTS;
        this->bIFrame = isIFrame;
        this->nRefCount = 0;
        this->pParent = nullptr;
        this->pHeadBuf = pHeadBuf;
        this->nHeadSize = nHeadSize;
    }

    AX_VOID CopyFrom(CAXRingElement &element) {
        if (this->pBuf) {
            if (element.pHeadBuf && element.nHeadSize > 0) {
                memcpy(this->pBuf, element.pHeadBuf, element.nHeadSize);
                memcpy(this->pBuf + element.nHeadSize, element.pBuf, element.nSize);
            }
            else {
                memcpy(this->pBuf, element.pBuf, element.nSize);
            }
        }
        this->nSize = element.nHeadSize + element.nSize;
        this->nChannel = element.nChannel;
        this->nPts = element.nPts;
        this->bIFrame = element.bIFrame;

        this->nRefCount = 0;
        this->pParent = nullptr;
    }

    AX_S32 IncreaseRefCount() {
        return ++nRefCount;
    }

    AX_S32 DecreaseRefCount(AX_BOOL bForceClear) {
        if (bForceClear) {
            nRefCount = 0;
        }
        else {
            --nRefCount;
            if (nRefCount < 0) {
               nRefCount = 0;
            }
        }
        return nRefCount;
    }

    AX_S32 GetRefCount() {
        return nRefCount;
    }

    AX_S32 GetIndex() {
        return nIndex;
    }

    AX_S32 SetIndex(AX_S32 nIndex) {
        return this->nIndex = nIndex;
    }


private:
    AX_VOID Clear() {
        this->nSize = 0;
        this->nChannel = 0;
        this->bIFrame = AX_FALSE;
        this->nPts = 0;
        this->nRefCount = 0;
    }

public:
    AX_U8*   pBuf;
    AX_U32   nSize;
    AX_U32   nChannel;
    AX_U64   nPts;
    AX_BOOL  bIFrame;

    AX_U8* pHeadBuf;
    AX_U32 nHeadSize;

    CAXRingBuffer * pParent;

private:
    AX_S32   nRefCount;
    AX_S32   nIndex;

};

#define AXRING  "RING"

class CAXRingBuffer
{
public:
    CAXRingBuffer(AX_U32 nElementBuffSize, AX_U32 nElementCount, const char * pszName=nullptr ) {
        m_nElementCount = nElementCount;
        m_nElementBuffSize = nElementBuffSize;
        m_pRing = new CAXRingElement[m_nElementCount];
        for (AX_U32 i = 0; i < m_nElementCount; i++) {
            m_pRing[i].pBuf = new AX_U8[m_nElementBuffSize];
            memset((AX_VOID *)m_pRing[i].pBuf, 0x0, m_nElementBuffSize);
            m_pRing[i].nSize = 0;
            m_pRing[i].pParent = this;
            m_pRing[i].SetIndex(i);
        }

        m_nHeader = 0;
        m_nTail = 0;
        m_bHasLost = AX_FALSE;
        m_szName[0] = 0;
        if (pszName && strlen(pszName)) {
            strncpy(m_szName, pszName, sizeof(m_szName));
        }
    }

    ~CAXRingBuffer(AX_VOID) {
        m_nHeader = 0;
        m_nTail = 0;

        if (nullptr == m_pRing) {
            return;
        }

        m_mutex.lock();
        for (AX_U32 i = 0; i < m_nElementCount; i++) {
            if (nullptr != m_pRing[i].pBuf) {
                delete[] m_pRing[i].pBuf;
            }
        }
        delete[] m_pRing;
        m_mutex.unlock();
    }

    AX_BOOL IsFull() {
        std::lock_guard<std::mutex> lck (m_mutex);
        return (m_nTail - m_nHeader) == m_nElementCount ? AX_TRUE : AX_FALSE;
    }

    AX_BOOL IsEmpty() {
        std::lock_guard<std::mutex> lck (m_mutex);
        return (m_nTail == m_nHeader) ? AX_TRUE : AX_FALSE;
    }

    AX_BOOL Put(CAXRingElement &element) {
        std::lock_guard<std::mutex> lck (m_mutex);
        //LOG_M(AXRING, "[%s] +++, tail=%llu, head=%llu, iFrame=%d", m_szName, m_nTail, m_nHeader,element.bIFrame);

        if (!m_pRing || (element.nSize + element.nHeadSize) > m_nElementBuffSize) {
            // must not go to here
            if (element.bIFrame) {
                m_bHasLost = AX_TRUE;
            }
            LOG_M(AXRING, "[%s] ---, drop one frame for size: %d is bigger than buffer size(%d)",
                                                    m_szName, element.nSize, m_nElementBuffSize);
            return AX_FALSE;
        }

        if ((m_nTail - m_nHeader) == m_nElementCount) {
            // is full
            LOG_M(AXRING, "[%s] is full, tail=%llu, head=%llu", m_szName, m_nTail, m_nHeader);
            if (element.bIFrame) {
                // replace the tail with new I Frame
                AX_U64 nIndex = (m_nTail - 1) % m_nElementCount;
                AX_S32 nRefCount= m_pRing[nIndex].GetRefCount();
                if (nRefCount == 0) {
                    m_nTail--;
                    m_bHasLost = AX_FALSE;
                    LOG_M(AXRING, "[%s] ---, replace last one with i frame for is full", m_szName);
                } else {
                    m_bHasLost = AX_TRUE;
                    LOG_M(AXRING, "[%s] ---, drop one i frame for is full", m_szName);
                    return AX_FALSE;
                }

            }
            else {
                m_bHasLost = AX_TRUE; // mark to lost all behind P Frame
                LOG_M(AXRING, "[%s] ---, drop one p frame for is full", m_szName);
                return AX_FALSE;
            }

        } else {
            if (element.bIFrame)  {
                m_bHasLost = AX_FALSE; // add new I frame
            }
            else {
                if (m_bHasLost)  {
                    // drop this P Frame
                    LOG_M(AXRING, "[%s] ---, drop one p frame for lost yet", m_szName);
                    return AX_FALSE;
                }
            }
        }

        AX_U64 nIndex = m_nTail % m_nElementCount;
        AX_S32 nRefCount= m_pRing[nIndex].GetRefCount();
        if (nRefCount != 0) {
            m_bHasLost = AX_TRUE;
            LOG_M(AXRING, "[%s] ---, drop one frame for refcount is none zero, nIndex=%llu, nRefCount=%d", m_szName, nIndex, nRefCount);
            return AX_FALSE;
        }

        m_pRing[nIndex].CopyFrom(element);
        m_pRing[nIndex].IncreaseRefCount();
        m_pRing[nIndex].pParent = this;
        m_nTail++;

        //LOG_M(AXRING, "[%s] ---, tail=%llu, head=%llu, pBuf=%p, size=%u", m_szName, m_nTail, m_nHeader, m_pRing[nIndex].pBuf, m_pRing[nIndex].nSize);
        return AX_TRUE;
    }

    CAXRingElement* Get() {
        CAXRingElement* element = nullptr;
        std::lock_guard<std::mutex> lck (m_mutex);
        //LOG_M(AXRING, "[%s] +++, tail=%llu, head=%llu", m_szName, m_nTail, m_nHeader);
        if (m_nHeader == m_nTail) {
            // is emtpy
            //LOG_M(AXRING, "[%s] ---, empty, tail=%llu, head=%llu", m_szName, m_nTail, m_nHeader);
            return element;
        }
        AX_U64 nIndex = m_nHeader % m_nElementCount;
        m_pRing[nIndex].IncreaseRefCount();
        // AX_S32 nRefCount = m_pRing[nIndex].IncreaseRefCount();
        element = &m_pRing[nIndex];
        // LOG_M(AXRING, "[%s] ---, tail=%llu, head=%llu, pBuf=%p, size=%u, nRef=%d", m_szName, m_nTail, m_nHeader, element->pBuf, element->nSize, nRefCount);
        return element;
    }

    AX_BOOL Pop(AX_BOOL bForce=AX_TRUE) {
        std::lock_guard<std::mutex> lck (m_mutex);
        // LOG_M(AXRING, "[%s] +++, tail=%llu, head=%llu", m_szName, m_nTail, m_nHeader);
        if (m_nHeader == m_nTail) {
            // is emtpy
            //LOG_M(AXRING, "[%s] ---, no element to be pop, tail=%llu, head=%llu", m_szName, m_nTail, m_nHeader);
            return AX_FALSE;
        }

        AX_U64 nIndex = m_nHeader % m_nElementCount;
        // AX_S32 nRefCount = m_pRing[nIndex].DecreaseRefCount(bForce);
        m_pRing[nIndex].DecreaseRefCount(bForce);
        m_nHeader++;
        // LOG_M(AXRING, "[%s] ---, tail=%llu, head=%llu, nRef=%d", m_szName, m_nTail, m_nHeader, nRefCount);

        return AX_TRUE;
    }

    AX_VOID Free(CAXRingElement* ele, AX_BOOL bForce=AX_FALSE) {
        if(!ele) {
            return;
        }
        std::lock_guard<std::mutex> lck (m_mutex);
        AX_S32 nIndex = ele->GetIndex();
        if (nIndex >= 0 && nIndex < (AX_S32)m_nElementCount) {
            m_pRing[nIndex].DecreaseRefCount(bForce);
            // AX_S32 nRefCount= m_pRing[nIndex].DecreaseRefCount(bForce);
            // LOG_M(AXRING, "[%s] nIndex=%d, nRefCount=%d", m_szName, nIndex, nRefCount);
        }
    }

    AX_U32 Size() {
        if (m_nTail < m_nHeader) {
            return 0;
        }
        return m_nTail-m_nHeader;
    }

private:
    CAXRingElement* m_pRing;
    AX_U32 m_nElementCount;
    AX_U32 m_nElementBuffSize;
    AX_U64 m_nHeader;
    AX_U64 m_nTail;
    AX_BOOL m_bHasLost;
    std::mutex m_mutex;
    char  m_szName[64];
};

#endif // _RING_BUFFER_3246FC57_D0F1_4176_A1C3_3A16FC46D1CF_H_
