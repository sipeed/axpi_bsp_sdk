/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

#include "global.h"
#include "MediaFrame.h"
#include "Stage.h"
#include "BaseSensor.h"
#include "WebServer.h"
#include "Mpeg4Encoder.h"
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
#include <queue>

#define OSD_BUFF_SIZE (512)


enum class RcType
{
    VENC_RC_NONE = -1,
    VENC_RC_CBR,
    VENC_RC_VBR,
    VENC_RC_AVBR,
    VENC_RC_QPMAP,
    VENC_RC_FIXQP,
    VENC_RC_MAX
};

typedef struct _stRCInfo
{
    RcType eRCType;
    AX_U32 nMinQp;
    AX_U32 nMaxQp;
    AX_U32 nMinIQp;
    AX_U32 nMaxIQp;
    AX_U32 nMaxIProp;
    AX_U32 nMinIProp;
    AX_S32 nIntraQpDelta;
} RC_INFO_T;

typedef struct _stVideoConfig
{
    AX_PAYLOAD_TYPE_E ePayloadType;
    AX_U32 nGOP;
    AX_U32 nFrameRate;
    AX_U32 nStride;
    AX_S32 nInWidth;
    AX_S32 nInHeight;
    AX_S32 nOutWidth;
    AX_S32 nOutHeight;
    AX_S32 nOffsetCropX;
    AX_S32 nOffsetCropY;
    AX_S32 nOffsetCropW;
    AX_S32 nOffsetCropH;
    AX_IMG_FORMAT_E eImgFormat;
    RC_INFO_T stRCInfo;
    AX_S32 nBitrate;

    _stVideoConfig() {
        memset(this, 0, sizeof(_stVideoConfig));
    }

} VIDEO_CONFIG_T, *VIDEO_CONFIG_PTR;

typedef struct _VIDEO_ATTR_T
{
    AX_BOOL enable;
    AX_S32 encode_type;
    AX_S32 frame_rate;
    AX_S32 bit_rate;
    AX_S32 width;
    AX_S32 height;
    AX_S32 x;
    AX_S32 y;
    AX_S32 w;
    AX_S32 h;

    _VIDEO_ATTR_T() {
        memset(this, 0, sizeof(_VIDEO_ATTR_T));
    }

    _VIDEO_ATTR_T(VIDEO_CONFIG_T& tCopy) {
        this->enable         = AX_TRUE;
        this->encode_type    = tCopy.ePayloadType;
        this->frame_rate     = tCopy.nFrameRate;
        this->bit_rate       = tCopy.nBitrate;
        this->width          = tCopy.nOutWidth;
        this->height         = tCopy.nOutHeight;
        this->x              = tCopy.nOffsetCropX;
        this->y              = tCopy.nOffsetCropY;
        this->w              = tCopy.nOffsetCropW;
        this->h              = tCopy.nOffsetCropH;
    }

    AX_BOOL operator==(const _VIDEO_ATTR_T tAnother) const {
        return (this->enable            == tAnother.enable
                && this->encode_type    == tAnother.encode_type
                && this->frame_rate     == tAnother.frame_rate
                && this->bit_rate       == tAnother.bit_rate
                && this->width          == tAnother.width
                && this->height         == tAnother.height
                && this->x              == tAnother.x
                && this->y              == tAnother.y
                && this->w              == tAnother.w
                && this->h              == tAnother.h) ? AX_TRUE : AX_FALSE;
    }

    AX_BOOL IsAttrChanged(const _VIDEO_ATTR_T tAnother) {
        return (this->encode_type    == tAnother.encode_type
                && this->frame_rate  == tAnother.frame_rate
                && this->bit_rate    == tAnother.bit_rate
                && this->width       == tAnother.width
                && this->height      == tAnother.height
                && this->x           == tAnother.x
                && this->y           == tAnother.y
                && this->w           == tAnother.w
                && this->h           == tAnother.h) ? AX_FALSE : AX_TRUE;
    }

    AX_BOOL IsSwitcherChanged(const _VIDEO_ATTR_T tAnother) {
        return (this->enable == tAnother.enable) ? AX_FALSE : AX_TRUE;
    }

    AX_U32 compare(const _VIDEO_ATTR_T tAnother) {
        if (*this == tAnother) {
            return 0;
        } else if (!IsAttrChanged(tAnother) && !IsSwitcherChanged(tAnother)) {
            return 1;
        } else if (!IsAttrChanged(tAnother)) {
            return 2;
        } else if (!IsSwitcherChanged(tAnother)) {
            return 3;
        }

        return -1;
    }

} VIDEO_ATTR_T, *VIDEO_ATTR_PTR;

class AXRtspServer;
class CVideoEncoder : public CStage
{
public:
    CVideoEncoder(AX_U8 nChannel, AX_U8 nInnerIndex);
    virtual ~CVideoEncoder();

    virtual AX_BOOL Start(AX_BOOL bReload = AX_TRUE);
    virtual AX_VOID Stop(void);
    virtual AX_BOOL ProcessFrame(CMediaFrame* pFrame);

    AX_VOID SetRTSPServer(AXRtspServer* rtspServer);
    AX_VOID SetWebServer(CWebServer* webServer);
    AX_VOID SetChnAttr(AX_VIN_CHN_ATTR_T tAttr);
    AX_VOID SetMp4ENC(CMPEG4Encoder* pMpeg4Encoder);

    AX_BOOL ResetByResolution(const VIDEO_ATTR_T& attr, AX_BOOL bStop = AX_TRUE);
    AX_BOOL IsParamChange(const VIDEO_ATTR_T& attr);
    AX_BOOL IsResolutionChanged(const VIDEO_ATTR_T& attr);
    AX_BOOL UpdateFramerate(AX_F32 fSrcFrameRate, AX_F32 fDstFrameRate = -1);

protected:
    AX_BOOL LoadConfig();
    AX_BOOL InitParams(VIDEO_CONFIG_T& config);
    AX_VOID ProcOSD(CMediaFrame* pFrame);

    AX_BOOL LoadFont();

public:
    AX_U8   m_nInnerIndex;
    AX_U8   m_nSnsID;
    AX_U8   m_nChannel;
    AX_BOOL m_bH265;
    AX_BOOL m_bGetThreadRunning;

    AXRtspServer*   m_pRtspServer;
    CWebServer*     m_pWebServer;
    CMPEG4Encoder*  m_pMpeg4Encoder;

private:
    // CJpgEncoder*    m_pJpegEncoder;
    thread*         m_pGetThread;
    VIDEO_CONFIG_T  m_tVideoConfig;
    AX_VIN_CHN_ATTR_T m_tChnAttr;

    static CBmpOSD  m_sfont;
};

#endif
