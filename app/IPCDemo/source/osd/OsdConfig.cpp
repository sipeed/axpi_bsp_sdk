#include "OsdConfig.h"

#define OSD "OSD"


CAXOsdCfg::CAXOsdCfg(AX_VOID)
{

}

CAXOsdCfg::~CAXOsdCfg(AX_VOID)
{

}

AX_BOOL CAXOsdCfg::Init()
{
    return AX_TRUE;
}

AX_S32 CAXOsdCfg::SetOsd(AX_U32 nChn, const OSD_CFG_T *pOsdArr, const AX_S32 nOsdSize)
{
    CHECK_CHANNEL(nChn);
    CHECK_POINTER(pOsdArr);
    CHECK_OSD_NUMBER(nOsdSize);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    memcpy(&m_tOsdCfgWrite[nChn][0], pOsdArr, sizeof(OSD_CFG_T) * nOsdSize);
    m_nOsdCfgNum[nChn] = nOsdSize;

    return AX_SUCCESS;
}

AX_S32 CAXOsdCfg::GetOsd(AX_U32 nChn, const OSD_CFG_T **pOsdArr, AX_S32 *pOsdSize)
{
    CHECK_CHANNEL(nChn);
    CHECK_POINTER(pOsdArr);
    CHECK_POINTER(pOsdSize);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    *pOsdSize = m_nOsdCfgNum[nChn];
    memcpy(&m_tOsdCfgRead[nChn][0], &m_tOsdCfgWrite[nChn][0], sizeof(OSD_CFG_T) * (*pOsdSize));
    *pOsdArr = &m_tOsdCfgRead[nChn][0];

    if (*pOsdSize > 0) {
        for (AX_S32 i = 0; i < *pOsdSize; i++) {
            LOG_M_I(OSD, "nChn %d, OSD %d: ===============================", nChn, i);
            LOG_M_I(OSD, "enable: %d", m_tOsdCfgRead[nChn][i].bEnable);
            LOG_M_I(OSD, "Type: %d", m_tOsdCfgRead[nChn][i].eType);
            LOG_M_I(OSD, "matchWidth: %d", m_tOsdCfgRead[nChn][i].nMatchWidth);
            LOG_M_I(OSD, "matchHeight: %d", m_tOsdCfgRead[nChn][i].nMatchHeight);
            LOG_M_I(OSD, "xBoundary: %d", m_tOsdCfgRead[nChn][i].nBoundaryX);
            LOG_M_I(OSD, "yBoundary: %d", m_tOsdCfgRead[nChn][i].nBoundaryY);
            LOG_M_I(OSD, "align: %d", m_tOsdCfgRead[nChn][i].eAlign);
            LOG_M_I(OSD, "ARGB: 0x%08X", m_tOsdCfgRead[nChn][i].nARGB);
            LOG_M_I(OSD, "AllMatch: %d", m_tOsdCfgRead[nChn][i].bAllMatch);

            if (OSD_TYPE_PICTURE == m_tOsdCfgRead[nChn][i].eType) {
                LOG_M_I(OSD, "Bitmap: %s", m_tOsdCfgRead[nChn][i].tPicAttr.szFileName);
            } else if (OSD_TYPE_STRING == m_tOsdCfgRead[nChn][i].eType) {
                LOG_M_I(OSD, "String: %s, %d", m_tOsdCfgRead[nChn][i].tStrAttr.szStr, strlen(m_tOsdCfgRead[nChn][i].tStrAttr.szStr));
            } else if (OSD_TYPE_TIME == m_tOsdCfgRead[nChn][i].eType) {
                LOG_M_I(OSD, "TimeFormat: %d", m_tOsdCfgRead[nChn][i].tTimeAttr.eFormat);
            }
        }
    }

    return OSD_SUCCESS;
}

AX_S32 CAXOsdCfg::AddOsd(AX_U32 nChn, const OSD_CFG_T *pOsd, AX_S32 *pOsdHandle)
{
    CHECK_CHANNEL(nChn);
    CHECK_POINTER(pOsd);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    CHECK_POINTER(pOsdHandle);

    AX_U32 nSize = m_nOsdCfgNum[nChn];
    CHECK_OSD_NUMBER(nSize + 1);

    memcpy(&m_tOsdCfgWrite[nChn][nSize], pOsd, sizeof(OSD_CFG_T));
    m_nOsdCfgNum[nChn]++;

    *pOsdHandle = m_nOsdCfgNum[nChn] - 1;

    return OSD_SUCCESS;
}

AX_S32 CAXOsdCfg::UpdateOsd(AX_U32 nChn, AX_S32 nOsdHandle, const OSD_CFG_T *pOsd)
{
    CHECK_CHANNEL(nChn);
    CHECK_POINTER(pOsd);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    CHECK_OSD_HANDLE(nChn, nOsdHandle);

    memcpy(&m_tOsdCfgWrite[nChn][nOsdHandle], pOsd, sizeof(OSD_CFG_T));

    return OSD_SUCCESS;
}

AX_S32 CAXOsdCfg::RemoveOsd(AX_U32 nChn, AX_S32 nOsdHandle)
{
    CHECK_CHANNEL(nChn);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    CHECK_OSD_HANDLE(nChn, nOsdHandle);

    AX_U32 nSize = m_nOsdCfgNum[nChn];
    AX_U32 nMoveSize = nSize - nOsdHandle - 1;

    if (nMoveSize > 0) {
        memcpy(&m_tOsdCfgWrite[nChn][nOsdHandle], &m_tOsdCfgWrite[nChn][nOsdHandle + 1], sizeof(OSD_CFG_T) * nMoveSize);
    }

    m_nOsdCfgNum[nChn]--;

    return OSD_SUCCESS;
}

AX_S32 CAXOsdCfg::TypeOn(AX_U32 nChn, AX_U32 nFlag)
{
    CHECK_CHANNEL(nChn);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    OSD_CFG_T *pOsdCfg = m_tOsdCfgWrite[nChn];
    AX_S32 nOsdSize = m_nOsdCfgNum[nChn];
    for (AX_S32 i = 0; i < nOsdSize; ++i) {
        if ((1 << pOsdCfg[i].eType) & nFlag) {
            pOsdCfg[i].bEnable = AX_TRUE;
        }
    }

    return OSD_SUCCESS;
}

AX_S32 CAXOsdCfg::TypeOff(AX_U32 nChn, AX_U32 nFlag)
{
    CHECK_CHANNEL(nChn);

    std::unique_lock<std::mutex> lck(m_mtxOsd);

    OSD_CFG_T *pOsdCfg = m_tOsdCfgWrite[nChn];
    AX_S32 nOsdSize = m_nOsdCfgNum[nChn];
    for (AX_S32 i = 0; i < nOsdSize; ++i) {
        if ((1 << pOsdCfg[i].eType) & nFlag) {
            pOsdCfg[i].bEnable = AX_FALSE;
        }
    }

    return OSD_SUCCESS;
}
