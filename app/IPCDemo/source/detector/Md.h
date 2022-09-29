/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#pragma once
#include <mutex>
#include <vector>
#include "ax_ives_api.h"
#include "MediaFrame.h"
#include "WebServer.h"
#include "Singleton.h"

typedef struct {
    AX_MD_CHN_ATTR_S stAttr;
    AX_U32 nConfidence;
} MD_AREA_ATTR_T;

class CMD : public CSingleton<CMD> {
    friend class CSingleton<CMD>;

public:
    AX_VOID SetWebServer(CWebServer *pWebServer);

    AX_BOOL Startup(AX_U32 nWidth, AX_U32 nHeight);
    AX_VOID Cleanup(AX_VOID);

    AX_VOID GetDefaultThresholdY(AX_U8 &nThrd, AX_U8 &nConfidence);

    /* add an area and return area id (equal to MD chn id), if returns -1 means fail */
    AX_S32  AddArea(AX_U32 x, AX_U32 y, AX_U32 w, AX_U32 h, AX_U32 nWidth, AX_U32 nHeight);
    AX_BOOL RemoveArea(AX_S32 nAreaId);

    AX_BOOL SetThresholdY(AX_S32 nAreaId, AX_U8 nThrd, AX_U8 nConfidence);

    /* process and return md results */
    const std::vector<AX_U8> &ProcessFrame(const CMediaFrame *pFrame);

protected:
    AX_BOOL IsEqualArea(const AX_IVES_RECT_S &a, const AX_IVES_RECT_S &b) const {
        return ((a.u32X == b.u32X) && (a.u32Y == b.u32Y) && (a.u32W == b.u32W) && (a.u32H == b.u32H)) ? AX_TRUE : AX_FALSE;
    }

    AX_BOOL LoadConfig(AX_VOID);
    AX_VOID DestoryAreas(AX_VOID);
    AX_VOID SendWebEvent(AX_S32 nAreaId);

private:
    CMD(AX_VOID) noexcept = default;
    virtual ~CMD(AX_VOID) = default;

    /* virtual function of CSingleton */
    AX_BOOL Init(AX_VOID) override {
        LoadConfig();
        return AX_TRUE;
    };

private:
    std::mutex m_mutx;
    std::vector<MD_AREA_ATTR_T *> m_vecAreas;
    std::vector<AX_U8> m_vecRslts;
    AX_BOOL m_bInited{AX_FALSE};
    AX_S32 m_nConfidenceY{20};
    AX_S32 m_nThrdY{50};
    AX_IVES_MB_SIZE_S m_stMbSize{32,32};
    AX_U32 m_mdImgW{0};
    AX_U32 m_mdImgH{0};

    CWebServer *m_pWebServer{nullptr};
};