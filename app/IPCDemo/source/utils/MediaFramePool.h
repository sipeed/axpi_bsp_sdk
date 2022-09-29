/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _MEDIA_FRAME_POOL_H_
#define _MEDIA_FRAME_POOL_H_

#include "global.h"
#include "MediaFrame.h"

#include <mutex>

#define MAX_POOL_IMG_NUM (20)
#define DEFAULT_POOL_IMG_NUM (8)

class CMediaFramePool
{
public:
    CMediaFramePool(void);
    virtual ~CMediaFramePool(void);

public:
    AX_BOOL InitImgPool(AX_S32 nImageSize, AX_U32 nPoolCapacity = DEFAULT_POOL_IMG_NUM);
    AX_VOID DeInitImgPool();
    CMediaFrame* BorrowImg();
    AX_VOID GiveBackImg(CMediaFrame* pImageFrame);

private:
    // CMediaFrame m_arrPoolImg[MAX_POOL_IMG_NUM];
    vector<CMediaFrame*> m_vecPoolImg;
    std::mutex m_poolMutex;

};

#endif // _MEDIA_FRAME_POOL_H_