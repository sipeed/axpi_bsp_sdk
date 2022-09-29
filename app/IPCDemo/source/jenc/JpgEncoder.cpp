/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "JpgEncoder.h"
#include "OptionHelper.h"
#include "PrintHelper.h"
#include "JsonCfgParser.h"
#include "CommonUtils.h"
#include "IVPSStage.h"
#include <sys/time.h>
#include <atomic>
#include <thread>
#include <unistd.h>


using namespace std;

#define JENC "JENC"

extern COptionHelper gOptions;
extern CPrintHelper  gPrintHelper;
extern END_POINT_OPTIONS g_tEPOptions[MAX_VENC_CHANNEL_NUM];
extern IVPS_GROUP_CFG_T g_tIvpsGroupConfig[IVPS_GROUP_NUM];


CJpgEncoder::CJpgEncoder(AX_U8 nChannel /*= 0*/, AX_U8 nInnerIndex /*= 0*/)
: CStage(JENC)
, m_pWebServer(nullptr)
, m_pGetThread(nullptr)
, m_bGetThreadRunning(AX_FALSE)
, m_nChannel(nChannel)
, m_nCfgIndex(nInnerIndex)
{
    memset(&m_tJencConfig, 0, sizeof(m_tJencConfig));
}

CJpgEncoder::~CJpgEncoder()
{

}

static AX_VOID *JpgGetThreadFunc(AX_VOID *__this)
{
    CJpgEncoder *pThis = (CJpgEncoder *)__this;
    LOG_M(JENC, "[%d] +++", pThis->m_nChannel);

    prctl(PR_SET_NAME, "IPC_JENC_Get");

    AX_VENC_STREAM_S stVencStream = {0};
    memset(&stVencStream, 0, sizeof(AX_VENC_STREAM_S));

    int ret;
    while (pThis->m_bGetThreadRunning) {
        ret = AX_VENC_GetStream(pThis->m_nChannel, &stVencStream, -1);
        if (AX_SUCCESS != ret) {
            if (AX_ERR_VENC_FLOW_END == ret) {
                pThis->m_bGetThreadRunning = AX_FALSE;
                break;
            }

            if (AX_ERR_VENC_QUEUE_EMPTY == ret) {
                CTimeUtils::msSleep(1);
                continue;
            }
            LOG_M_E(JENC, "AX_VENC_GetStream failed, ret=0x%x!", ret);
            continue;
        }

        gPrintHelper.Add(E_PH_MOD_VENC, 0, pThis->m_nChannel);

        if (pThis->m_pWebServer && stVencStream.stPack.pu8Addr && stVencStream.stPack.u32Len > 0) {
            pThis->m_pWebServer->SendCaptureData(pThis->m_nChannel, (AX_U8 *)stVencStream.stPack.pu8Addr, stVencStream.stPack.u32Len);
        }

        ret = AX_VENC_ReleaseStream(pThis->m_nChannel, &stVencStream);
        if (AX_SUCCESS != ret) {
            LOG_M_E(JENC, "[%d] AX_VENC_ReleaseStream failed, ret=0x%x!", pThis->m_nChannel, ret);
            continue;
        }
    }

    LOG_M(JENC, "[%d] ---", pThis->m_nChannel);

    return nullptr;
}

AX_BOOL CJpgEncoder::ProcessFrame(CMediaFrame* pFrame)
{
    AX_S32 ret = AX_SUCCESS;

    if (pFrame->bIvpsFrame) {
        LOG_M_I(JENC, "[%d] Seq: %d, w: %d, h: %d, stride: %d, size: %d", m_nChannel, pFrame->nFrameID, pFrame->tVideoFrame.u32Width, pFrame->tVideoFrame.u32Height, pFrame->tVideoFrame.u32PicStride[0], pFrame->tVideoFrame.u32FrameSize);
        AX_VIDEO_FRAME_INFO_S tFrame = {0};
        tFrame.stVFrame = pFrame->tVideoFrame;
        tFrame.stVFrame.u32PicStride[1] = tFrame.stVFrame.u32PicStride[0];
        tFrame.stVFrame.u32PicStride[2] = 0;
        tFrame.stVFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
        ret = AX_VENC_SendFrame(m_nChannel, &tFrame, -1);
    } else {
        LOG_M_I(JENC, "[%d] Seq: %d, w: %d, h: %d", m_nChannel, pFrame->tFrame.tFrameInfo.stVFrame.u64SeqNum, pFrame->tFrame.tFrameInfo.stVFrame.u32Width, pFrame->tFrame.tFrameInfo.stVFrame.u32Height);
        AX_VIDEO_FRAME_INFO_S tFrame = {0};
        tFrame = pFrame->tFrame.tFrameInfo;
        tFrame.stVFrame.u32PicStride[1] = tFrame.stVFrame.u32PicStride[0];
        tFrame.stVFrame.u32PicStride[2] = 0;
        tFrame.stVFrame.enImgFormat = AX_YUV420_SEMIPLANAR;
        ret = AX_VENC_SendFrame(m_nChannel, &tFrame, -1);
    }

    if (AX_SUCCESS != ret) {
        LOG_M_E(JENC, "[%d] AX_VENC_SendFrame failed, ret=0x%x!", m_nChannel, ret);
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_BOOL CJpgEncoder::Start(AX_BOOL bReload /*= AX_TRUE*/)
{
    LOG_M(JENC, "[%d] +++", m_nChannel);

    if (m_bGetThreadRunning) {
        LOG_M_E(JENC, "JENC %d already started.");
        return AX_TRUE;
    }

    if (bReload) {
        if (!LoadConfig()) {
            return AX_FALSE;
        }
    } else {
        AX_BOOL bRet = InitParams(m_tJencConfig);
        if (!bRet) {
            LOG_M_E(JENC, "Failed to start JENC.");
        }
    }

    m_bGetThreadRunning = AX_TRUE;
    m_pGetThread = new thread(JpgGetThreadFunc, this);

    if (m_pGetThread) {
        AX_VENC_RECV_PIC_PARAM_S  tRecvParam;
        AX_VENC_StartRecvFrame(m_nChannel, &tRecvParam);
    }

    AX_BOOL bLinkMode = AX_FALSE;
    AX_U8 nISPChn = CIVPSStage::GetIspChnIndex(m_nChannel);
    AX_U8 nIvpsInnerIndex = CIVPSStage::GetIvpsChnIndex(m_nChannel);

    bLinkMode = (gOptions.IsLinkMode() && 1 == g_tIvpsGroupConfig[nISPChn].arrLinkModeFlag[nIvpsInnerIndex]) ? AX_TRUE : AX_FALSE;

    LOG_M(JENC, "[%d] ---", m_nChannel);
    return CStage::Start(bLinkMode ? AX_FALSE : AX_TRUE);
}

AX_VOID CJpgEncoder::Stop(void)
{
    LOG_M(JENC, "[%d] +++", m_nChannel);

    CStage::Stop();

    if (m_bGetThreadRunning) {
        AX_VENC_StopRecvFrame(m_nChannel);
        m_bGetThreadRunning = AX_FALSE;

        CTimeUtils::msSleep(50);
        AX_VENC_DestroyChn(m_nChannel);

        if (m_pGetThread && m_pGetThread->joinable()) {
            m_pGetThread->join();

            delete m_pGetThread;
            m_pGetThread = nullptr;
        }
    }

    LOG_M(JENC, "[%d] ---", m_nChannel);
}

AX_BOOL CJpgEncoder::LoadConfig()
{
    JPEG_CONFIG_T tJpegConfig;
    if (!CConfigParser().GetInstance()->GetJpegCfg("jenc", m_nCfgIndex, tJpegConfig)) {
        LOG_M_W(JENC, "Load jenc configuration failed, use default of OS04a10.");
        if (!tJpegConfig.FillDefault(E_SNS_TYPE_OS04A10)) {
            return AX_FALSE;
        }
    }

    m_tJencConfig = tJpegConfig;

    AX_U8 nISPChn = CIVPSStage::GetIspChnIndex(m_nChannel);
    AX_U8 nIvpsInnerIndex = CIVPSStage::GetIvpsChnIndex(m_nChannel);
    if (E_END_POINT_JENC == g_tEPOptions[m_nChannel].eEPType) {
        if (-1 != g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][0]
            && -1 != g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][1]
            && -1 != g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][2]) {
            m_tJencConfig.nWidth     = g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][0];
            m_tJencConfig.nHeight    = g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][1];
        }
    }

    m_tJencConfig.nWidth           = m_tChnAttr.tChnAttr[nISPChn].nWidth;
    m_tJencConfig.nHeight          = m_tChnAttr.tChnAttr[nISPChn].nHeight;
    m_tJencConfig.nStride          = ALIGN_UP(m_tJencConfig.nWidth, g_tIvpsGroupConfig[nISPChn].arrOutResolution[nIvpsInnerIndex][2]);

    LOG_M(JENC, "[%d] ---", m_nChannel);

    return InitParams(m_tJencConfig);
}

AX_BOOL CJpgEncoder::InitParams(JPEG_CONFIG_T& config)
{
    LOG_M(JENC, "[%d] +++", m_nChannel);
    AX_VENC_CHN_ATTR_S stVencChnAttr;
    memset(&stVencChnAttr, 0, sizeof(AX_VENC_CHN_ATTR_S));

    AX_U8 nISPChn = CIVPSStage::GetIspChnIndex(m_nChannel);
    AX_U8 nIvpsInnerIndex = CIVPSStage::GetIvpsChnIndex(m_nChannel);

    stVencChnAttr.stVencAttr.u32MaxPicWidth  = MAX_JENC_PIC_WIDTH;
    stVencChnAttr.stVencAttr.u32MaxPicHeight = MAX_JENC_PIC_HEIGHT;

    stVencChnAttr.stVencAttr.u32PicWidthSrc  = config.nWidth;
    stVencChnAttr.stVencAttr.u32PicHeightSrc = config.nHeight;

    stVencChnAttr.stVencAttr.u32CropOffsetX  = 0;
    stVencChnAttr.stVencAttr.u32CropOffsetY  = 0;
    stVencChnAttr.stVencAttr.u32CropWidth    = config.nWidth;
    stVencChnAttr.stVencAttr.u32CropHeight   = config.nHeight;

    stVencChnAttr.stVencAttr.u32BufSize      = config.nStride * config.nHeight * 2; /*stream buffer size*/

    if (config.eImgFormat == AX_YUV420_PLANAR ||
        config.eImgFormat == AX_YUV420_SEMIPLANAR ||
        config.eImgFormat == AX_YUV420_SEMIPLANAR_VU) {
    } else {
        LOG_M_E(JENC, "Invalid image format (%d)!", config.eImgFormat);
        return AX_FALSE;
    }

    if (gOptions.IsLinkMode() && 1 == g_tIvpsGroupConfig[nISPChn].arrLinkModeFlag[nIvpsInnerIndex]) {
        stVencChnAttr.stVencAttr.enLinkMode = AX_LINK_MODE;
    } else {
        stVencChnAttr.stVencAttr.enLinkMode = AX_NONLINK_MODE;
    }

    stVencChnAttr.stVencAttr.enType = PT_JPEG;

    LOG_M(JENC, "[%d] w:%d, h:%d", m_nChannel, stVencChnAttr.stVencAttr.u32PicWidthSrc, stVencChnAttr.stVencAttr.u32PicHeightSrc);

    AX_S32 ret = AX_VENC_CreateChn(m_nChannel, &stVencChnAttr);
    if (AX_SUCCESS != ret) {
        LOG_M_E(JENC, "[%d] AX_VENC_CreateChn failed, ret=0x%x!", m_nChannel, ret);
        return AX_FALSE;
    }

    AX_VENC_JPEG_PARAM_S stJpegParam;
    memset(&stJpegParam, 0, sizeof(AX_VENC_JPEG_PARAM_S));
    ret = AX_VENC_GetJpegParam(m_nChannel, &stJpegParam);
    if (AX_SUCCESS != ret) {
        LOG_M(JENC, "[%d] AX_VENC_GetJpegParam failed!", m_nChannel, ret);
        return AX_FALSE;
    }

    stJpegParam.u32Qfactor = config.nQpLevel;

    ret = AX_VENC_SetJpegParam(m_nChannel, &stJpegParam);
    if (AX_SUCCESS != ret) {
        LOG_M(JENC, "[%d] AX_VENC_SetJpegParam failed!", m_nChannel, ret);
        return AX_FALSE;
    }

    LOG_M(JENC, "[%d] ---", m_nChannel);

    return AX_TRUE;
}

AX_VOID CJpgEncoder::SetWebServer(CWebServer* webServer)
{
    m_pWebServer = webServer;
}

AX_VOID CJpgEncoder::SetChnAttr(AX_VIN_CHN_ATTR_T tAttr)
{
    m_tChnAttr = tAttr;
}

AX_BOOL CJpgEncoder::ResetByResolution(const JPEG_CONFIG_T& attr, AX_BOOL bStop /*= AX_TRUE*/)
{
    LOG_M(JENC, "[%d] +++", m_nChannel);
    if (bStop) {
        Stop();
    }

    m_tJencConfig.nWidth       = attr.nWidth;
    m_tJencConfig.nHeight      = attr.nHeight;
    m_tJencConfig.nStride      = attr.nStride;

    Start(AX_FALSE);

    LOG_M(JENC, "[%d] ---", m_nChannel);
    return AX_TRUE;
}
