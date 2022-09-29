/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H

#include "global.h"
#include "WebServer.h"
#include "MediaFrame.h"
#include "BaseSensor.h"
#include "Stage.h"

#define MAX_IMG_NUM (20)


typedef struct stJpegConfig
{
    /* Common values for JPEG/MJPEG */
    AX_IMG_FORMAT_E eImgFormat;
    AX_S32 nWidth;
    AX_S32 nHeight;
    AX_U32 nStride;
    /* Values for JPEG */
    AX_S32 nQpLevel;
    /* Values for MJPEG */
    // AX_S32 eRCType;
    // AX_S32 nFrameRate;
    // AX_S32 nBitrate;
    // AX_S32 nFixQP;

    stJpegConfig() {
        memset(this, 0, sizeof(stJpegConfig));
        // eRCType     = RcType::VENC_RC_NONE;
        eImgFormat  = AX_YUV420_SEMIPLANAR;
        // nFrameRate  = 25;
        nQpLevel    = 90;
    }

    AX_BOOL FillDefault(SNS_TYPE_E eSnsType) {
        // this->eRCType    = RcType::VENC_RC_CBR;
        this->eImgFormat = AX_YUV420_SEMIPLANAR;
        // this->nBitrate   = 4000;
        this->nQpLevel   = 90;
        // this->nFixQP     = 25;

        return AX_TRUE;
    }
} JPEG_CONFIG_T, *JPEG_CONFIG_PTR;

class CJpgEncoder: public CStage
{
public:
    CJpgEncoder(AX_U8 nChannel, AX_U8 nInnerIndex);
    ~CJpgEncoder();

    virtual AX_BOOL Start(AX_BOOL bReload = AX_TRUE);
    virtual AX_VOID Stop();
    virtual AX_BOOL ProcessFrame(CMediaFrame* pFrame);

    AX_VOID SetWebServer(CWebServer* webServer);
    AX_VOID SetChnAttr(AX_VIN_CHN_ATTR_T tAttr);

    AX_BOOL ResetByResolution(const JPEG_CONFIG_T& attr, AX_BOOL bStop = AX_TRUE);

protected:
    AX_BOOL LoadConfig();
    AX_BOOL InitParams(JPEG_CONFIG_T& config);

public:
    CWebServer* m_pWebServer;
    thread*     m_pGetThread;
    AX_BOOL     m_bGetThreadRunning;

    AX_U8       m_nChannel;
    AX_U8       m_nCfgIndex;
    AX_VIN_CHN_ATTR_T m_tChnAttr;

    JPEG_CONFIG_T  m_tJencConfig;
};


#endif // JPEG_ENCODER_H
