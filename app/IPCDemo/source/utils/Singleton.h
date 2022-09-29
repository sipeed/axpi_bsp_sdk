/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#pragma once
#include "global.h"

/* https://stackoverflow.com/questions/34519073/inherit-singleton */
template<typename T>
class CSingleton
{
public:
    static T *GetInstance(AX_VOID) noexcept (std::is_nothrow_constructible<T>::value) {
        static T instance;
        static AX_BOOL sbInit = AX_FALSE;
        if (!sbInit) {
            if (!instance.Init()) {
                return nullptr;
            }
            else {
                sbInit = AX_TRUE;
            }
        }
        return &instance;
    };

    virtual AX_BOOL Init() = 0;

protected:
    CSingleton(AX_VOID) noexcept = default;
    virtual ~CSingleton(AX_VOID) = default;

private:
    CSingleton(const CSingleton &rhs) = delete;
    CSingleton &operator=(const CSingleton &rhs) = delete;
};
