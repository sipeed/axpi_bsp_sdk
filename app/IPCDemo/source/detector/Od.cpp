/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include "Od.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "ax_isp_3a_api.h"
#include "AppLog.h"
#include "TimeUtil.h"
#include "inifile.h"

#define OD ("OD")
#define IS_ODCHN_CREATE(ch) ((ch) >= 0)
#define CHK_LUX(x) ((x) >= 1024)

static inline AX_U32 align_down(AX_U32 x, AX_U32 a) {
    if (a > 0) {
        return ((x / a) * a);
    } else {
        return x;
    }
}

AX_BOOL COD::Startup(AX_U32 nFrameRate, AX_U32 nWidth, AX_U32 nHeight) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++");

    if (m_bInited) {
        LOG_M_W(OD, "OD is already inited");
        LOG_M_D(OD, "---");
        return AX_TRUE;
    }

    AX_S32 ret = AX_IVES_OD_Init();
    if (0 != ret) {
        LOG_M_E(OD, "OD init fail, ret = 0x%x", ret);
        return AX_FALSE;
    }

    LoadConfig();

    m_nFrameRate = nFrameRate;
    m_odImgW = nWidth;
    m_odImgH = nHeight;

    m_vecAreas.reserve(8);
    m_vecRslts.reserve(8);
    m_bInited = AX_TRUE;

    LOG_M_D(OD, "---");
    return AX_TRUE;
}

AX_VOID COD::Cleanup(AX_VOID) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++");

    if (m_bInited) {
        DestoryAreas();

        AX_S32 ret = AX_IVES_OD_DeInit();
        if (0 != ret) {
            /* ignore error */
            LOG_M_E(OD, "OD de-init fail, ret = 0x%x", ret);
        }

        m_bInited = AX_FALSE;
    }

    LOG_M_D(OD, "---");
}

AX_VOID COD::DestoryAreas(AX_VOID) {
    for (auto &m : m_vecAreas) {
        if (m) {
            if (IS_ODCHN_CREATE(m->stAttr.odChn)) {
                AX_S32 ret = AX_IVES_OD_DestoryChn(m->stAttr.odChn);
                if (0 != ret) {
                    /* ignore error */
                    LOG_M_E(OD, "destory OD channel %d fail, ret = 0x%x", m->stAttr.odChn, ret);
                }
            }

            free(m);
            m = nullptr;
        }
    }

    m_vecAreas.clear();
    m_vecRslts.clear();
}

AX_BOOL COD::LoadConfig(AX_VOID) {
    inifile::IniFile f;
    std::string strIniFile{"./config/ive.conf"};
    AX_S32 ret = f.Load(strIniFile);
    if (0 != ret) {
        LOG_M_E(OD, "load %s fail", strIniFile.c_str());
        return AX_FALSE;
    }

    std::string strSect = "OD";
    if (f.HasSection(strSect)) {
        f.GetIntValue(strSect, "threshold Y", &m_nThrdY);
        f.GetIntValue(strSect, "confidence Y", &m_nConfidenceY);
        f.GetIntValue(strSect, "lux threshold", &m_nLuxThrd);
        f.GetIntValue(strSect, "lux diff", &m_nLuxDiff);
    } else {
        LOG_M_E(OD, "invalid %s file: no section: %s", strIniFile.c_str(), strSect.c_str());
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_S32 COD::AddArea(AX_U32 x, AX_U32 y, AX_U32 w, AX_U32 h, AX_U32 nWidth, AX_U32 nHeight) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++");

    AX_F32 fx = ( nWidth > 0) ? (m_odImgW * 1.0 /  nWidth) : 1.0;
    AX_F32 fy = (nHeight > 0) ? (m_odImgH * 1.0 / nHeight) : 1.0;

    AX_IVES_RECT_S stArea;
    stArea.u32X = align_down(fx * x, 2);
    stArea.u32Y = align_down(fy * y, 2);
    stArea.u32W = align_down(fx * w, 2);
    stArea.u32H = align_down(fy * h, 2);

    if (0 == stArea.u32W || 0 == stArea.u32H) {
        LOG_M_E(OD, "invalid area [%d, %d, %d, %d] in %dx%d!", x, y, w, h, nWidth, nHeight);
        return -1;
    }

    AX_S32 nAreaId = -1;
    AX_U32 nCount = m_vecAreas.size();
    if (nCount > 0) {
        /* find the same area or freed area id */
        for (AX_U32 i = 0; i < nCount; ++i) {
            if (m_vecAreas[i]) {
                if (IsEqualArea(m_vecAreas[i]->stAttr.stArea, stArea)) {
                    LOG_M_W(OD, "area [%d, %d, %d, %d] already exists, id: %d", stArea.u32X, stArea.u32Y, stArea.u32W, stArea.u32H, i);
                    LOG_M_D(OD, "---");
                    return (AX_S32)i;
                }
            } else {
                if (nAreaId < 0) {
                    /* found a freed channel */
                    nAreaId = (AX_S32)i;
                }
            }
        }
    }

    OD_AREA_ATTR_T *pArea = (OD_AREA_ATTR_T *)malloc(sizeof(OD_AREA_ATTR_T));
    if (!pArea) {
        LOG_M_E(OD, "malloc od area memory fail, %s", strerror(errno));
        return -1;
    }

    memset(pArea, 0, sizeof(*pArea));
    pArea->stAttr.odChn = -1; /* indicated area channel is not created */
    pArea->stAttr.stArea = stArea;
    pArea->stAttr.u32FrameRate = m_nFrameRate;
    pArea->stAttr.u8ThrdY = (AX_U8)m_nThrdY;
    pArea->stAttr.u8ConfidenceY = (AX_U8)m_nConfidenceY;
    pArea->stAttr.u32LuxThrd = (AX_U32)m_nLuxThrd;
    pArea->stAttr.u32LuxDiff = (AX_U32)m_nLuxDiff;
    pArea->nFrameProcessed = 0;

    if (m_bInited) {
        if (nAreaId < 0) {
            pArea->stAttr.odChn = (AX_S32)nCount;
        } else {
            pArea->stAttr.odChn = nAreaId;
        }
        AX_S32 ret = AX_IVES_OD_CreateChn(pArea->stAttr.odChn, &pArea->stAttr);
        if (0 != ret) {
            LOG_M_E(OD, "Create OD channel %d fail, ret = 0x%x", pArea->stAttr.odChn, ret);
            free(pArea);
            return -1;
        }
    }

    if (nAreaId < 0) {
        m_vecAreas.push_back(pArea);
        m_vecRslts.push_back(0);
        nAreaId = (AX_S32)nCount;
    } else {
        m_vecAreas[nAreaId] = pArea;
        m_vecRslts[nAreaId] = 0;
    }

    LOG_M_I(OD, "OD area [%d, %d, %d, %d] is added, id: %d", stArea.u32X, stArea.u32Y, stArea.u32W, stArea.u32H, nAreaId);
    LOG_M_D(OD, "---");
    return nAreaId;
}

AX_BOOL COD::RemoveArea(AX_S32 nAreaId) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++");

    if (nAreaId < 0) {
        LOG_M_E(OD, "invalid area id %d", nAreaId);
        return AX_FALSE;
    }

    const AX_U32 nAreaCount = m_vecAreas.size();
    for (AX_U32 i = 0; i < nAreaCount; ++i) {
        if (!m_vecAreas[i]) {
            continue;
        }

        if (nAreaId == (AX_S32)i) {
            if (IS_ODCHN_CREATE(m_vecAreas[i]->stAttr.odChn)) {
                AX_S32 ret = AX_IVES_OD_DestoryChn(m_vecAreas[i]->stAttr.odChn);
                if (0 != ret) {
                    LOG_M_E(OD, "destory OD channel %d fail, ret = 0x%x", m_vecAreas[i]->stAttr.odChn, ret);
                    return AX_FALSE;
                }
            }

            free(m_vecAreas[i]);
            m_vecAreas[i] = nullptr;
            m_vecRslts[i] = 0;

            LOG_M_D(OD, "---");
            return AX_TRUE;
        }
    }

    LOG_M_E(OD, "Area %d not exist!", nAreaId);
    return AX_FALSE;
}

const std::vector<AX_U8> &COD::ProcessFrame(const CMediaFrame *pFrame) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++");

    AX_S32 ret;
    AX_IVES_OD_IMAGE_S stImg;
    stImg.pstImg = (AX_IVES_IMAGE_S *)&pFrame->tVideoFrame;

    AX_ISP_IQ_AE_STATUS_T tAE;
    if (0 == AX_ISP_IQ_GetAeStatus(0, &tAE)) {
        stImg.u32Lux = tAE.tAlgStatus.nLux;
    }
    else {
        stImg.u32Lux = 0;

        LOG_M_E(OD, "AX_ISP_IQ_GetAeStatus(pipe: %d) fail", 0);
    }

    const AX_U32 nAreaCount = m_vecAreas.size();
    for (AX_U32 i = 0; i < nAreaCount; ++i) {
        OD_AREA_ATTR_T *m = m_vecAreas[i];
        if (m) {
            if (!IS_ODCHN_CREATE(m->stAttr.odChn)) {
                m->stAttr.odChn = i;
                ret = AX_IVES_OD_CreateChn(m->stAttr.odChn, &m->stAttr);
                if (0 != ret) {
                    LOG_M_E(OD, "Create OD channel %d fail, ret = 0x%x", m->stAttr.odChn, ret);
                    m->stAttr.odChn = -1; /* restore to uncreated */
                    continue;
                }
            }

            ++m->nFrameProcessed;

            AX_U8 nLastRslt = m_vecRslts[i];
            AX_U64 nTick1 = CTimeUtils::GetTickCount();
            ret = AX_IVES_OD_Process(m->stAttr.odChn, &stImg, &m_vecRslts[i]);
            AX_U64 nTick2 = CTimeUtils::GetTickCount();
            if (0 != ret) {
                LOG_M_E(OD, "OD process area %d fail, ret = 0x%x", i, ret);
                continue;
            } else {
                LOG_M_I(OD, "frame id: %lld (%lld), area %d od: %d, lux: %d, elapsed: %lld ms", stImg.pstImg->u64SeqNum,
                             m->nFrameProcessed, i, m_vecRslts[i], stImg.u32Lux, nTick2 - nTick1);

                if (1 == m_vecRslts[i] && nLastRslt != m_vecRslts[i]) {
                    SendWebEvent(i);
                }
            }
        }
    }

    LOG_M_D(OD, "---");
    return m_vecRslts;
}

AX_BOOL COD::SetThresholdY(AX_S32 nAreaId, AX_U8 nThrd, AX_U8 nConfidence) {
    if (nAreaId < 0) {
        LOG_M_E(OD, "invalid area id %d", nAreaId);
        return AX_FALSE;
    }

    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++, area: %d, Y thrd: %d, confidence: %d", nAreaId, nThrd, nConfidence);

    if (nAreaId >= (AX_S32)m_vecAreas.size() || !m_vecAreas[nAreaId]) {
        LOG_M_E(OD, "area %d not exist!", nAreaId);
        return AX_FALSE;
    } else {
        m_vecAreas[nAreaId]->stAttr.u8ThrdY = nThrd;
        m_vecAreas[nAreaId]->stAttr.u8ConfidenceY = nConfidence;
    }

    LOG_M_D(OD, "---, area: %d, Y thrd: %d, confidence: %d", nAreaId, nThrd, nConfidence);
    return AX_TRUE;
}

AX_BOOL COD::SetLuxThreshold(AX_S32 nAreaId, AX_U32 nThrd, AX_U32 nDiff) {
    if (nAreaId < 0) {
        LOG_M_E(OD, "invalid area id %d", nAreaId);
        return AX_FALSE;
    }

    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(OD, "+++, area: %d, lux thrd: %d, diff: %d", nAreaId, nThrd, nDiff);

    if (nAreaId >= (AX_S32)m_vecAreas.size() || !m_vecAreas[nAreaId]) {
        LOG_M_E(OD, "area %d not exist!", nAreaId);
        return AX_FALSE;
    } else {
        m_vecAreas[nAreaId]->stAttr.u32LuxThrd = nThrd;
        m_vecAreas[nAreaId]->stAttr.u32LuxDiff = nDiff;
    }

    LOG_M_D(OD, "---, area: %d, lux thrd: %d, diff: %d", nAreaId, nThrd, nDiff);
    return AX_TRUE;
}

AX_VOID COD::SetWebServer(CWebServer *pWebServer) {
    m_pWebServer = pWebServer;
}

AX_VOID COD::GetDefaultThresholdY(AX_U8 &nThrd, AX_U8 &nConfidence) {
    nThrd = m_nThrdY;
    nConfidence = m_nConfidenceY;
}

AX_VOID COD::SendWebEvent(AX_S32 nAreaId) {
    if (m_pWebServer) {
        AI_EVENTS_DATA_T tEvent;
        tEvent.eType = E_AI_EVENTS_TYPE_OD;
        tEvent.tMD.nAreaID = nAreaId;
        m_pWebServer->SendEventsData(&tEvent);
    }
}
