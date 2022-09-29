/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <time.h>
#include <functional>
#include "Mutex.hpp"

/*
 * because the version of used stdc++ library is too old, wait_for of std::condition_variable uses real time not
 * monotionic. so replace condition by pthread_xxx of posix. When stdc++ library is upgrade, below classes can be
 * replaced by c++11 condition_variable and mutex.
 */
namespace ax {

/* condition by posix */
using CondPredicate = std::function<bool()>;

class CCondition final {
public:
    CCondition(AX_VOID) {
        pthread_condattr_init(&m_attr);
        pthread_condattr_setclock(&m_attr, CLOCK_MONOTONIC);
        pthread_cond_init(&m_cond, &m_attr);
    }

    ~CCondition(AX_VOID) {
        pthread_cond_destroy(&m_cond);
        pthread_condattr_destroy(&m_attr);
    }

    AX_VOID wait(CMutex &mtx, CondPredicate p) {
        if (p) {
            while (!p()) {
                pthread_cond_wait(&m_cond, mtx());
            }
        } else {
            /* not recommend because of spurious wakeup */
            pthread_cond_wait(&m_cond, mtx());
        }
    }

    AX_BOOL wait_for(CMutex &mtx, AX_U32 ms, CondPredicate p) {
        struct timespec tv;
        clock_gettime(CLOCK_MONOTONIC, &tv);
        tv.tv_sec += ms / 1000;
        tv.tv_nsec += (ms % 1000) * 1000000;
        if (tv.tv_nsec >= 1000000000) {
            tv.tv_nsec -= 1000000000;
            tv.tv_sec += 1;
        }

        AX_S32 ret = 0;
        if (p) {
            while (!p() && (0 == ret)) {
                ret = pthread_cond_timedwait(&m_cond, mtx(), &tv);
            }
        } else {
            /* not recommend because of spurious wakeup */
            ret = pthread_cond_timedwait(&m_cond, mtx(), &tv);
        }

        return (0 == ret) ? AX_TRUE : AX_FALSE;
    }

    AX_VOID notify_one(AX_VOID) {
        pthread_cond_signal(&m_cond);
    }

    AX_VOID notify_all(AX_VOID) {
        pthread_cond_broadcast(&m_cond);
    }

    pthread_cond_t *operator()(AX_VOID) {
        return &m_cond;
    }

    pthread_cond_t *get(AX_VOID) {
        return &m_cond;
    }

private:
    CCondition(const CCondition &) = delete;
    CCondition &operator=(const CCondition &) = delete;

private:
    pthread_cond_t m_cond;
    pthread_condattr_t m_attr;
};

} // end namespace ax