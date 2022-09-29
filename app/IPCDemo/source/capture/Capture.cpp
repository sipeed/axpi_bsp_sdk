/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include "Capture.h"

#define CAPTURE "CAPTURE"

CCapture::CCapture(AX_VOID) {

}

CCapture::~CCapture(AX_VOID) {

}

AX_BOOL CCapture::ProcessFrame(AX_U8 nChn, CMediaFrame* pFrame) {
    if (nChn >= MAX_VENC_CHANNEL_NUM) {
        LOG_M_E(CAPTURE, "Invalid chn:%d", nChn);
        return AX_FALSE;
    }

    if (m_nCaptureChn != nChn) {
        LOG_M_E(CAPTURE, "Capture channel(%d, %d) not matched", m_nCaptureChn, nChn);
        return AX_FALSE;
    }

    // update status
    {
        CLockGuard lck(m_mutexStat);
        m_bCapture = AX_FALSE;
        m_ptCaptureFrame = pFrame;
    }

    // send signal
    CLockGuard lck(m_mutexCapture);
    m_cvCapture.notify_one();

    return AX_TRUE;
}

AX_BOOL CCapture::CaptureFrame(AX_U8 nChn, AX_VIDEO_FRAME_S &Frame) {
    if (nChn >= MAX_VENC_CHANNEL_NUM) {
        LOG_M_E(CAPTURE, "Invalid chn:%d", nChn);
        return AX_FALSE;
    }

    if (m_bCapture) {
        LOG_M_E(CAPTURE, "[%d] Capture is running, please wait, and try again", nChn);
        return AX_FALSE;
    }

    if (m_ptCaptureFrame) {
        LOG_M_E(CAPTURE, "[%d] Capture frame should be free", m_nCaptureChn);
        return AX_FALSE;
    }

    // update status
    {
        CLockGuard lck(m_mutexStat);
        m_bCapture = AX_TRUE;
        m_nCaptureChn = nChn;
        m_ptCaptureFrame = NULL;
    }

    // wait signal
    CLockGuard lck(m_mutexCapture);
    AX_BOOL bRet = m_cvCapture.wait_for(m_mutexCapture, 2000, NULL);

    // update status
    {
        CLockGuard lck(m_mutexStat);
        m_bCapture = AX_FALSE;
    }

    if (!bRet) {
        LOG_M_E(CAPTURE, "wait_for error: %s", strerror(errno));

        return AX_FALSE;
    }

    if (!m_ptCaptureFrame) {
        LOG_M_E(CAPTURE, "capture error: %s", strerror(errno));

        return AX_FALSE;
    }

    // get frame
    Frame = m_ptCaptureFrame->tVideoFrame;

    return AX_TRUE;
}

AX_BOOL CCapture::FreeCaptureFrame(AX_U8 nChn, AX_VIDEO_FRAME_S &Frame) {
    if (nChn >= MAX_VENC_CHANNEL_NUM) {
        LOG_M_E(CAPTURE, "Invalid chn:%d", nChn);
        return AX_FALSE;
    }

    CLockGuard lck(m_mutexStat);

    if (m_ptCaptureFrame) {
        m_ptCaptureFrame->FreeMem();

        m_ptCaptureFrame = nullptr;
    }

    return AX_TRUE;
}

AX_BOOL CCapture::CapturePicture(AX_U32 nChn, AX_VOID **ppBuf, AX_U32 *pBufSize, AX_U32 nQpLevel) {
    if (nChn >= MAX_VENC_CHANNEL_NUM) {
        LOG_M_E(CAPTURE, "Invalid chn:%d", nChn);
        return AX_FALSE;
    }

    if (!ppBuf || !pBufSize) {
        LOG_M_E(CAPTURE, "nil pointer");
        return AX_FALSE;
    }

    if (m_stVencStream.stPack.pu8Addr) {
        LOG_M_E(CAPTURE, "[%d] Capture picture data should be free", m_nCaptureChn);
        return AX_FALSE;
    }

    AX_BOOL bRet = AX_FALSE;
    AX_BOOL bFrameGet = AX_FALSE;
    AX_BOOL bJencChnCreate = AX_FALSE;
    const AX_U8 nJencChn = CAPTURE_VENC_CHANNEL_ID;
    AX_S32 ret = AX_SUCCESS;
    AX_VIDEO_FRAME_INFO_S tFrame = {0};
    AX_U32 u32Width = 0;
    AX_U32 u32Height = 0;
    AX_VENC_RECV_PIC_PARAM_S tRecvParam = {0};
    AX_BOOL bJencStreamGet = AX_FALSE;

    if (!CaptureFrame(nChn, tFrame.stVFrame)) {
        goto JENC_EXIT;
    }

    bFrameGet = AX_TRUE;

    AX_VENC_CHN_ATTR_S stVencChnAttr;
    memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));

    u32Width = tFrame.stVFrame.u32Width;
    u32Height = tFrame.stVFrame.u32Height;

    stVencChnAttr.stVencAttr.u32MaxPicWidth = MAX_JENC_PIC_WIDTH;
    stVencChnAttr.stVencAttr.u32MaxPicHeight = MAX_JENC_PIC_HEIGHT;

    stVencChnAttr.stVencAttr.u32PicWidthSrc = u32Width;
    stVencChnAttr.stVencAttr.u32PicHeightSrc = u32Height;

    stVencChnAttr.stVencAttr.u32CropOffsetX = 0;
    stVencChnAttr.stVencAttr.u32CropOffsetY = 0;
    stVencChnAttr.stVencAttr.u32CropWidth = u32Width;
    stVencChnAttr.stVencAttr.u32CropHeight = u32Height;
    stVencChnAttr.stVencAttr.u32BufSize = u32Width * u32Height * 3 / 8; /*stream buffer size*/

    stVencChnAttr.stVencAttr.enLinkMode = AX_NONLINK_MODE;

    stVencChnAttr.stVencAttr.enType = PT_JPEG;

    ret = AX_VENC_CreateChn(nJencChn, &stVencChnAttr);
    if (AX_SUCCESS != ret) {
        LOG_M_E(CAPTURE, "[%d] AX_VENC_CreateChn(%d X %d, size=%d) fail, ret=0x%x", nJencChn, u32Width, u32Height, stVencChnAttr.stVencAttr.u32BufSize, ret);
        goto JENC_EXIT;
    }

    bJencChnCreate = AX_TRUE;

    AX_VENC_JPEG_PARAM_S stJpegParam;
    memset(&stJpegParam, 0, sizeof(AX_VENC_JPEG_PARAM_S));
    ret = AX_VENC_GetJpegParam(nJencChn, &stJpegParam);
    if (AX_SUCCESS != ret) {
        LOG_M_E(CAPTURE, "[%d] AX_VENC_GetJpegParam fail", nJencChn, ret);
        goto JENC_EXIT;
    }

    stJpegParam.u32Qfactor = nQpLevel;

    ret = AX_VENC_SetJpegParam(nJencChn, &stJpegParam);
    if (AX_SUCCESS != ret) {
        LOG_M_E(CAPTURE, "[%d] AX_VENC_SetJpegParam fail", nJencChn, ret);
        goto JENC_EXIT;
    }

    AX_VENC_StartRecvFrame(nJencChn, &tRecvParam);

    tFrame.stVFrame.u64PhyAddr[1] = tFrame.stVFrame.u64PhyAddr[0] + tFrame.stVFrame.u32PicStride[0] * tFrame.stVFrame.u32Height;
    tFrame.stVFrame.u32PicStride[1] = tFrame.stVFrame.u32PicStride[0];
    tFrame.stVFrame.u32PicStride[2] = 0;
    tFrame.stVFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
    ret = AX_VENC_SendFrame(nJencChn, &tFrame, 2000);

    if (AX_SUCCESS != ret) {
        LOG_M_E(CAPTURE, "[%d] AX_VENC_SendFrame fail, ret=0x%x", nJencChn, ret);
        goto JENC_EXIT;
    }

    memset(&m_stVencStream, 0x00, sizeof(m_stVencStream));
    ret = AX_VENC_GetStream(nJencChn, &m_stVencStream, 2000);
    if (AX_SUCCESS != ret) {
        LOG_M_E(CAPTURE, "AX_VENC_GetStream fail, ret=0x%x", ret);
        goto JENC_EXIT;
    }

    bJencStreamGet = AX_TRUE;

    *ppBuf = (AX_VOID *)m_stVencStream.stPack.pu8Addr;
    *pBufSize = m_stVencStream.stPack.u32Len;

    bRet = AX_TRUE;

JENC_EXIT:
    if (bFrameGet) {
        FreeCaptureFrame(nChn, tFrame.stVFrame);
    }

    if (!bRet) {
        if (bJencStreamGet) {
            AX_VENC_ReleaseStream(nJencChn, &m_stVencStream);
        }

        if (bJencChnCreate) {
            AX_VENC_StopRecvFrame(nJencChn);
            AX_VENC_DestroyChn(nJencChn);
        }
    }

    return bRet;
}

AX_BOOL CCapture::FreeCapturePicture(AX_U32 nChn, AX_VOID *pBuf) {
    if (nChn >= MAX_VENC_CHANNEL_NUM) {
        LOG_M_E(CAPTURE, "Invalid chn:%d", nChn);
        return AX_FALSE;
    }

    if (pBuf && ((AX_U32)pBuf == (AX_U32)m_stVencStream.stPack.pu8Addr)) {
        const AX_U8 nJencChn = CAPTURE_VENC_CHANNEL_ID;

        AX_VENC_ReleaseStream(nJencChn, &m_stVencStream);
        memset(&m_stVencStream, 0x00, sizeof(m_stVencStream));

        AX_VENC_StopRecvFrame(nJencChn);
        AX_VENC_DestroyChn(nJencChn);
    }

    return AX_TRUE;
}

AX_BOOL CCapture::GetCaptureStat(AX_U8 nChn) {
    CLockGuard lck(m_mutexStat);

    return ((m_bCapture && (m_nCaptureChn == nChn)) ? AX_TRUE : AX_FALSE);
}
