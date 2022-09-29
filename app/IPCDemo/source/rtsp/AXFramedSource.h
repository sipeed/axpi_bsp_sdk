/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __AXFRAMEDSOURCE_H__
#define __AXFRAMEDSOURCE_H__

#include "FramedSource.hh"
#include "global.h"
#include "AXRingBuffer.h"
#include <queue>

typedef struct _FrameInfo_T
{
    AX_U8 * pBuf;
    AX_U32  nSize;
} FrameInfo_T;

class AXFramedSource: public FramedSource {
public:
    static AXFramedSource* createNew(UsageEnvironment& env);

public:
    static EventTriggerId eventTriggerId;
    // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
    // encapsulate a *single* device - not a set of devices.
    // You can, however, redefine this to be a non-static member variable.
    void AddFrameBuff(AX_U8 nChn, const AX_U8* pBuf, AX_U32 nLen, AX_U64 nPts=0, AX_BOOL bIFrame=AX_FALSE);
    virtual unsigned maxFrameSize() const;

protected:
    AXFramedSource(UsageEnvironment& env);
    // called only by createNew(), or by subclass constructors
    virtual ~AXFramedSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    //virtual void doStopGettingFrames(); // optional

private:
    static void deliverFrame(void* clientData);
    void _deliverFrame();

private:
    static unsigned referenceCount; // used to count how many instances of this class currently exist
    std::queue<FrameInfo_T> m_qFrame;
    CAXRingBuffer* m_pRingBuf;

    u_int32_t m_nTriggerID;
};


#endif /*__AXFRAMEDSOURCE_H__*/