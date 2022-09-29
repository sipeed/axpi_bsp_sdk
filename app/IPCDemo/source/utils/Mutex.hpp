/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <pthread.h>
#include "ax_base_type.h"

namespace ax {

class CMutex final {
public:
    CMutex(AX_VOID) {
        pthread_mutex_init(&m_mtx, NULL);
    }

    ~CMutex(AX_VOID) {
        pthread_mutex_destroy(&m_mtx);
    }

    AX_VOID lock(AX_VOID) {
        pthread_mutex_lock(&m_mtx);
    }

    AX_VOID unlock(AX_VOID) {
        pthread_mutex_unlock(&m_mtx);
    }

    pthread_mutex_t *operator()(AX_VOID) {
        return &m_mtx;
    }

    pthread_mutex_t *get(AX_VOID) {
        return &m_mtx;
    }

private:
    CMutex(const CMutex &) = delete;
    CMutex &operator=(const CMutex &) = delete;

private:
    pthread_mutex_t m_mtx;
};

class CLockGuard final {
public:
    CLockGuard(CMutex &mtx) : m_mtx(mtx) {
        m_mtx.lock();
    }
    ~CLockGuard(AX_VOID) {
        m_mtx.unlock();
    }

private:
    CLockGuard(const CLockGuard &) = delete;
    CLockGuard &operator=(const CLockGuard &) = delete;

private:
    CMutex &m_mtx;
};

} // end namespace ax
