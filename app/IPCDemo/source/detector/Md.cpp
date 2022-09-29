/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include "Md.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "AppLog.h"
#include "TimeUtil.h"
#include "inifile.h"

#define MD ("MD")
#define IS_MDCHN_CREATE(ch) ((ch) >= 0)

static inline AX_U32 GetConfidenceMBCount(AX_U8 nConfidence, const AX_IVES_RECT_S &stArea, const AX_IVES_MB_SIZE_S &stMB) {
    return (nConfidence * 1.0 / 100) * (stArea.u32W / stMB.u32W) * (stArea.u32H / stMB.u32H);
}

static inline AX_U32 align_down(AX_U32 x, AX_U32 a) {
    if (a > 0) {
        return ((x / a) * a);
    } else {
        return x;
    }
}

AX_BOOL CMD::Startup(AX_U32 nWidth, AX_U32 nHeight) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(MD, "+++");

    if (m_bInited) {
        LOG_M_W(MD, "MD is already inited");
        LOG_M_D(MD, "---");
        return AX_TRUE;
    }

    AX_S32 ret = AX_IVES_MD_Init();
    if (0 != ret) {
        LOG_M_E(MD, "MD init fail, ret = 0x%x", ret);
        return AX_FALSE;
    }

    LoadConfig();

    m_mdImgW = nWidth;
    m_mdImgH = nHeight;

    m_vecAreas.reserve(8);
    m_vecRslts.reserve(8);
    m_bInited = AX_TRUE;

    LOG_M_D(MD, "---");
    return AX_TRUE;
}

AX_VOID CMD::Cleanup(AX_VOID) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(MD, "+++");

    if (m_bInited) {
        DestoryAreas();

        AX_S32 ret = AX_IVES_MD_DeInit();
        if (0 != ret) {
            /* ignore error */
            LOG_M_E(MD, "MD de-init fail, ret = 0x%x", ret);
        }

        m_bInited = AX_FALSE;
    }

    LOG_M_D(MD, "---");
}

AX_VOID CMD::DestoryAreas(AX_VOID) {
    for (auto &m : m_vecAreas) {
        if (m) {
            if (IS_MDCHN_CREATE(m->stAttr.mdChn)) {
                AX_S32 ret = AX_IVES_MD_DestoryChn(m->stAttr.mdChn);
                if (0 != ret) {
                    /* ignore error */
                    LOG_M_E(MD, "destory MD channel %d fail, ret = 0x%x", m->stAttr.mdChn, ret);
                }
            }

            free(m);
            m = nullptr;
        }
    }

    m_vecAreas.clear();
    m_vecRslts.clear();
}

AX_BOOL CMD::LoadConfig(AX_VOID) {
    inifile::IniFile f;
    std::string strIniFile{"./config/ive.conf"};
    AX_S32 ret = f.Load(strIniFile);
    if (0 != ret) {
        LOG_M_E(MD, "load %s fail", strIniFile.c_str());
        return AX_FALSE;
    }

    std::string strSect = "MD";
    if (f.HasSection(strSect)) {
        std::string strVal;
        if (0 == f.GetStringValue(strSect, "mb", &strVal)) {
            std::string::size_type pos = strVal.find('x');
            if (std::string::npos != pos) {
                AX_S32 nW = atoi(strVal.substr(0, pos).c_str());
                AX_S32 nH = atoi(strVal.substr(pos + 1, -1).c_str());
                if (nW > 0 && nH > 0) {
                    m_stMbSize.u32W = nW;
                    m_stMbSize.u32H = nH;
                }
            }
        }
        f.GetIntValue(strSect, "threshold Y", &m_nThrdY);
        f.GetIntValue(strSect, "confidence", &m_nConfidenceY);
    } else {
        LOG_M_E(MD, "invalid %s file: no section: %s", strIniFile.c_str(), strSect.c_str());
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_S32 CMD::AddArea(AX_U32 x, AX_U32 y, AX_U32 w, AX_U32 h, AX_U32 nWidth, AX_U32 nHeight) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(MD, "+++");

    AX_F32 fx = ( nWidth > 0) ? (m_mdImgW * 1.0 /  nWidth) : 1.0;
    AX_F32 fy = (nHeight > 0) ? (m_mdImgH * 1.0 / nHeight) : 1.0;

    /* Area should be divided by MB */
    AX_IVES_RECT_S stArea;
    stArea.u32X = align_down(fx * x, 2);
    stArea.u32Y = align_down(fy * y, 2);
    stArea.u32W = align_down(fx * w, m_stMbSize.u32W);
    stArea.u32H = align_down(fy * h, m_stMbSize.u32H);

    if (0 == stArea.u32W || 0 == stArea.u32H) {
        LOG_M_E(MD, "invalid area [%d, %d, %d, %d] in %dx%d, mb: %dx%d!", x, y, w, h, nWidth, nHeight, m_stMbSize.u32W, m_stMbSize.u32H);
        return -1;
    }

    AX_S32 nAreaId = -1;
    AX_U32 nCount = m_vecAreas.size();
    if (nCount > 0) {
        /* find the same area or freed area id */
        for (AX_U32 i = 0; i < nCount; ++i) {
            if (m_vecAreas[i]) {
                if (IsEqualArea(m_vecAreas[i]->stAttr.stArea, stArea)) {
                    LOG_M_W(MD, "area [%d, %d, %d, %d] already exists, id: %d", stArea.u32X, stArea.u32Y, stArea.u32W, stArea.u32H, i);
                    LOG_M_D(MD, "---");
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

    MD_AREA_ATTR_T *pArea = (MD_AREA_ATTR_T *)malloc(sizeof(MD_AREA_ATTR_T));
    if (!pArea) {
        LOG_M_E(MD, "malloc md area memory fail, %s", strerror(errno));
        return -1;
    }

    memset(pArea, 0, sizeof(*pArea));
    pArea->stAttr.mdChn = -1; /* indicated area channel is not created */
    pArea->stAttr.enAlgMode = AX_MD_MODE_REF;
    pArea->stAttr.stMbSize = m_stMbSize;
    pArea->stAttr.stArea = stArea;
    pArea->stAttr.u8ThrY = m_nThrdY;
    pArea->nConfidence = GetConfidenceMBCount(m_nConfidenceY, stArea, m_stMbSize);

    if (m_bInited) {
        if (nAreaId < 0) {
            pArea->stAttr.mdChn = (AX_S32)nCount;
        } else {
            pArea->stAttr.mdChn = nAreaId;
        }
        AX_S32 ret = AX_IVES_MD_CreateChn(pArea->stAttr.mdChn, &pArea->stAttr);
        if (0 != ret) {
            LOG_M_E(MD, "Create MD channel %d fail, ret = 0x%x", pArea->stAttr.mdChn, ret);
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

    LOG_M_I(MD, "MD area [%d, %d, %d, %d] is added, id: %d", stArea.u32X, stArea.u32Y, stArea.u32W, stArea.u32H, nAreaId);
    LOG_M_D(MD, "---");
    return nAreaId;
}

AX_BOOL CMD::RemoveArea(AX_S32 nAreaId) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(MD, "+++");

    if (nAreaId < 0) {
        LOG_M_E(MD, "invalid area id %d", nAreaId);
        return AX_FALSE;
    }

    const AX_U32 nAreaCount = m_vecAreas.size();
    for (AX_U32 i = 0; i < nAreaCount; ++i) {
        if (!m_vecAreas[i]) {
            continue;
        }

        if (nAreaId == (AX_S32)i) {
            if (IS_MDCHN_CREATE(m_vecAreas[i]->stAttr.mdChn)) {
                AX_S32 ret = AX_IVES_MD_DestoryChn(m_vecAreas[i]->stAttr.mdChn);
                if (0 != ret) {
                    LOG_M_E(MD, "destory MD channel %d fail, ret = 0x%x", m_vecAreas[i]->stAttr.mdChn, ret);
                    return AX_FALSE;
                }
            }

            free(m_vecAreas[i]);
            m_vecAreas[i] = nullptr;
            m_vecRslts[i] = 0;

            LOG_M_D(MD, "---");
            return AX_TRUE;
        }
    }

    LOG_M_E(MD, "Area %d not exist!", nAreaId);
    return AX_FALSE;
}

const std::vector<AX_U8> &CMD::ProcessFrame(const CMediaFrame *pFrame) {
    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(MD, "+++");

    AX_S32 ret;
    AX_IVES_IMAGE_S *pstImg = (AX_IVES_IMAGE_S *)&pFrame->tVideoFrame;

    const AX_U32 nAreaCount = m_vecAreas.size();
    for (AX_U32 i = 0; i < nAreaCount; ++i) {
        MD_AREA_ATTR_T *m = m_vecAreas[i];
        if (m) {
            if (!IS_MDCHN_CREATE(m->stAttr.mdChn)) {
                m->stAttr.mdChn = i;
                ret = AX_IVES_MD_CreateChn(m->stAttr.mdChn, &m_vecAreas[i]->stAttr);
                if (0 != ret) {
                    LOG_M_E(MD, "Create MD channel %d fail, ret = 0x%x", m->stAttr.mdChn, ret);
                    m->stAttr.mdChn = -1; /* restore to uncreated */
                    continue;
                }
            }

            AX_MD_MB_THR_S stThrs{0, nullptr};
            AX_U64 nTick1 = CTimeUtils::GetTickCount();
            ret = AX_IVES_MD_Process(m->stAttr.mdChn, pstImg, &stThrs);
            AX_U64 nTick2 = CTimeUtils::GetTickCount();
            if (0 != ret) {
                LOG_M_E(MD, "frame id: %lld, MD process area %d fail, ret = 0x%x", pstImg->u64SeqNum, i, ret);
                continue;
            } else {
                AX_U32 nSumThrs = 0;
                for (AX_U32 k = 0; k < stThrs.u32Count; ++k) {
                    nSumThrs += stThrs.pMbThrs[k];
                }

                AX_U8 nLastRslt = m_vecRslts[i];

                /* marked 1 if the count of '1' mb > confidence */
                m_vecRslts[i] = (nSumThrs >= m->nConfidence) ? 1 : 0;
                LOG_M_I(MD, "frame id: %lld, area %d md: %d, elapsed: %lld ms", pstImg->u64SeqNum, i, m_vecRslts[i], nTick2 - nTick1);

                if (1 == m_vecRslts[i] && nLastRslt != m_vecRslts[i]) {
                    SendWebEvent(i);
                }
            }
        }
    }

    LOG_M_D(MD, "---");
    return m_vecRslts;
}

AX_BOOL CMD::SetThresholdY(AX_S32 nAreaId, AX_U8 nThrd, AX_U8 nConfidence) {
    if (nAreaId < 0) {
        LOG_M_E(MD, "invalid area id %d", nAreaId);
        return AX_FALSE;
    }

    std::lock_guard<std::mutex> lck(m_mutx);
    LOG_M_D(MD, "+++, area: %d, Y thrd: %d, confidence: %d", nAreaId, nThrd, nConfidence);

    if (nAreaId >= (AX_S32)m_vecAreas.size() || !m_vecAreas[nAreaId]) {
        LOG_M_E(MD, "area %d not exist!", nAreaId);
        return AX_FALSE;
    } else {
        m_vecAreas[nAreaId]->stAttr.u8ThrY = nThrd;
        m_vecAreas[nAreaId]->nConfidence =
            GetConfidenceMBCount(nConfidence, m_vecAreas[nAreaId]->stAttr.stArea, m_vecAreas[nAreaId]->stAttr.stMbSize);
    }

    LOG_M_D(MD, "---, area: %d, Y thrd: %d, confidence: %d", nAreaId, nThrd, nConfidence);
    return AX_TRUE;
}

AX_VOID CMD::SetWebServer(CWebServer *pWebServer) {
    m_pWebServer = pWebServer;
}

AX_VOID CMD::GetDefaultThresholdY(AX_U8 &nThrd, AX_U8 &nConfidence) {
    nThrd = m_nThrdY;
    nConfidence = m_nConfidenceY;
}

AX_VOID CMD::SendWebEvent(AX_S32 nAreaId) {
    if (m_pWebServer) {
        AI_EVENTS_DATA_T tEvent;
        tEvent.eType = E_AI_EVENTS_TYPE_MD;
        tEvent.tMD.nAreaID = nAreaId;
        m_pWebServer->SendEventsData(&tEvent);
    }
}