/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _CMEDIA_FRAME_H
#define _CMEDIA_FRAME_H

#include "global.h"

class CMediaFrame;
class IFrameRelease
{
public:
    virtual AX_VOID MediaFrameRelease(CMediaFrame *pMediaFrame)=0;
};

class CMediaFrame
{
public:
    AX_IMG_INFO_T         tFrame;
    AX_VIDEO_FRAME_S      tVideoFrame;
    AX_BOOL               bIvpsFrame;
    AX_U32                nPipeID;
    AX_U32                nStride;
    AX_U32                nFrameID;
    AX_U32                nFramePTS;
    AX_U32                nPoolID;
    AX_U8                 nChannel;
    AX_U8                 nIvpsReleaseGrp;
    AX_U8                 nReleaseChannel;
    IFrameRelease*        pFrameRelease;

    CMediaFrame() {
        memset(&tFrame, 0, sizeof(AX_IMG_INFO_T));
        memset(&tVideoFrame, 0, sizeof(AX_VIDEO_FRAME_S));
        bIvpsFrame = AX_FALSE;
        nPipeID = 0;
        nStride = 0;
        nFrameID = 0;
        nFramePTS = 0;
        nPoolID = 0;
        nChannel = 0;
        nIvpsReleaseGrp = 0;
        nReleaseChannel = 0;
        pFrameRelease = NULL;
    }

    virtual ~CMediaFrame() {}

    AX_VOID FreeMem(void) {
        if (pFrameRelease) {
            pFrameRelease->MediaFrameRelease(this);
        }
    }

    AX_BOOL SaveYuv(string strPath) {
        AX_BOOL bRet = AX_FALSE;
        FILE* pFile = fopen(strPath.c_str(), "wb");
        if (bIvpsFrame) {
            if (tVideoFrame.u32FrameSize == fwrite((void *)tVideoFrame.u64VirAddr[0], 1, tVideoFrame.u32FrameSize, pFile)) {
                bRet = AX_TRUE;
            }
        } else {
            if (tFrame.tFrameInfo.stVFrame.u32FrameSize == fwrite((void *)tFrame.tFrameInfo.stVFrame.u64VirAddr[0], 1, tFrame.tFrameInfo.stVFrame.u32FrameSize, pFile)) {
                bRet = AX_TRUE;
            }
        }

        fclose(pFile);

        return bRet;
    }
};

#endif // _CMEDIA_FRAME_H