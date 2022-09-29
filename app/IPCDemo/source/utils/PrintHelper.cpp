/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "PrintHelper.h"

#define PRINT_HELPER "PRINT"

CPrintHelper::CPrintHelper(AX_VOID)
{
    m_pPrintThread = nullptr;
    m_bPrintThreadWorking = AX_FALSE;
    m_nTickStart = 0;
    m_nTickEnd = 0;
    m_nPrintCnt = 0;
    m_bProfiling = AX_FALSE;
    m_bEnableStart = AX_FALSE;
}

CPrintHelper::~CPrintHelper(AX_VOID)
{

}

AX_VOID CPrintHelper::PrintThreadFunc()
{
    prctl(PR_SET_NAME, "IPC_PRINT");

    m_nTickStart = CTimeUtils::GetTickCount();
    m_bPrintThreadWorking = AX_TRUE;

    while (m_bPrintThreadWorking) {
        if (!m_bEnableStart) {
            m_nTickStart = CTimeUtils::GetTickCount();
            CTimeUtils::msSleep(100);
            continue;
        }

        m_nTickEnd = CTimeUtils::GetTickCount();
        if ((m_nTickEnd - m_nTickStart) >= PRINT_INTERVAL * 1000) {
            m_stCameraStatisticsInfo.Print();
            m_stIvpsStatisticsInfo.Print();
            m_stVencStatisticsInfo.Print();
            if (gOptions.IsActivedDetect() && gOptions.IsActivedDetectFromWeb()) {
                m_stDetPerfStatisticsInfo.Print();
            }
            m_stWebStatisticsInfo.Print();

#ifdef RUN_PROFILING
            if (m_nPrintCnt++ > 0) {
                DoProfiling();
            }
            else {
                m_bProfiling = AX_TRUE;
            }
#endif

            m_nTickStart = m_nTickEnd;
        }

        CTimeUtils::msSleep(PRINT_INTERVAL*1000/2);
    }
}

AX_VOID CPrintHelper::Start()
{
    LOG_M(PRINT_HELPER, "+++");
    m_pPrintThread = new thread(&CPrintHelper::PrintThreadFunc, this);
    LOG_M(PRINT_HELPER, "---");
}

AX_VOID CPrintHelper::Stop()
{
    LOG_M(PRINT_HELPER, "+++");

    m_bPrintThreadWorking = AX_FALSE;
    m_pPrintThread->join();
    delete m_pPrintThread;
    m_pPrintThread = nullptr;

    LOG_M(PRINT_HELPER, "---");
}

AX_VOID CPrintHelper::Add(PRINT_HELPER_MOD_E eModType, AX_U32 nGrpID, AX_U32 nChn)
{
    m_bEnableStart = AX_TRUE;

    switch (eModType) {
        case E_PH_MOD_CAMERA:
            m_stCameraStatisticsInfo.Add(nChn);
            break;
        case E_PH_MOD_VENC:
            m_stVencStatisticsInfo.Add(nChn);
            break;
        case E_PH_MOD_IVPS:
            m_stIvpsStatisticsInfo.Add(nGrpID, nChn);
            break;
        case E_PH_MOD_WEB_CONN:
            m_stWebStatisticsInfo.AddConn();
            break;
        case E_PH_MOD_DET:
            m_stVencStatisticsInfo.Add(nChn);
            break;
        default:
            break;
    }
}

AX_VOID CPrintHelper::Add(PRINT_HELPER_MOD_E eModType, AX_VOID *pCtx)
{
    m_bEnableStart = AX_TRUE;

    switch (eModType) {
        case E_PH_MOD_DET_PERF:
            m_stDetPerfStatisticsInfo.Add(pCtx);
            break;
        default:
            break;
    }
}

AX_VOID CPrintHelper::Remove(PRINT_HELPER_MOD_E eModType, AX_U32 nChn)
{
    switch (eModType) {
        case E_PH_MOD_CAMERA:
            m_stCameraStatisticsInfo.Remove(nChn);
            break;
        case E_PH_MOD_VENC:
            break;
        case E_PH_MOD_IVPS:
            break;
        case E_PH_MOD_WEB_CONN:
            m_stWebStatisticsInfo.ClearConn();
            break;
        default:
            break;
    }
}

AX_VOID CPrintHelper::UpdateQueueRemains(PRINT_HELPER_MOD_E eModType, AX_U32 nChn, AX_U32 nRemains)
{
    switch (eModType) {
        case E_PH_MOD_CAMERA:
            m_stCameraStatisticsInfo.UpdateQueueRemains(nChn, nRemains);
            break;
        case E_PH_MOD_VENC:
            break;
        default:
            break;
    }
}

AX_VOID CPrintHelper::AddTimeSpan(PRINT_HELPER_PIPE_POINT_E ePtType, AX_U32 nChn, AX_U64 nSeqNum)
{
#ifdef RUN_PROFILING
    if (!m_bProfiling)
        return;

    if (nSeqNum == 0)
        return;

    tuple<AX_U32, AX_U32> pairChnSeq = std::make_tuple(nChn, nSeqNum);
    std::chrono::high_resolution_clock::time_point u64Tick  = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point *pLastTS = nullptr;

    AX_U32 *pTotalTS = nullptr, *pNum   = nullptr,
           *pMaxTS   = nullptr, *pMinTS = nullptr;

    switch (ePtType)
    {
    case E_PH_PIPE_PT_BASE:
        {
            m_mapFrameTS[pairChnSeq] = u64Tick;
        }
        break;
    case E_PH_PIPE_PT_CAMERA:
        {
            pTotalTS = &m_stPipePtTsInfo.nCameraTotalTS[nSnsID][nChn];
            pNum     = &m_stPipePtTsInfo.nCameraNum[nSnsID][nChn];
            pMaxTS   = &m_stPipePtTsInfo.nCameraMaxTS[nSnsID][nChn];
            pMinTS   = &m_stPipePtTsInfo.nCameraMinTS[nSnsID][nChn];
            if (m_mapFrameTS.find(pairChnSeq) == m_mapFrameTS.end()) {
                m_mapFrameTS[pairChnSeq] = u64Tick;
            } else {
                pLastTS  = &m_mapFrameTS[pairChnSeq];
            }
        }
        break;

    case E_PH_PIPE_PT_IVPS_VENC:
        {
            pTotalTS = &m_stPipePtTsInfo.nVencTotalTS[nChn];
            pNum     = &m_stPipePtTsInfo.nVencNum[nChn];
            pMaxTS   = &m_stPipePtTsInfo.nVencMaxTS[nChn];
            pMinTS   = &m_stPipePtTsInfo.nVencMinTS[nChn];
            if (m_mapFrameTS.find(pairChnSeq) == m_mapFrameTS.end()) {
                m_mapFrameTS[pairChnSeq] = u64Tick;
            } else {
                pLastTS  = &m_mapFrameTS[pairChnSeq];
            }
        }
        break;
    default:
        break;
    }

    if (pLastTS) {
        AX_S32 nElapse = (AX_S32)std::chrono::duration_cast<std::chrono::microseconds>(u64Tick - *pLastTS).count();
        if (nElapse < 0) {
            return;
        }

        *pTotalTS += nElapse;
        *pNum     += 1;

        if (*pNum == 1) {
            *pMaxTS = nElapse;
            *pMinTS = nElapse;
        } else {
            *pMaxTS = AX_MAX((*pMaxTS), (AX_U32)nElapse);
            *pMinTS = AX_MIN((*pMinTS), (AX_U32)nElapse);
        }

        *pLastTS = u64Tick;
    }
#endif
}

AX_VOID CPrintHelper::DoProfiling()
{
    m_bProfiling = AX_FALSE;

    /* ISP(out)->Venc(out) */
    for (AX_U32 i = 0; i < MAX_VENC_CHANNEL_NUM; i++) {
        LOG_M_W("PROFILING", "[%d][ISP(out)->Venc(out)]: avg: %6.2f, max: %3d, min: %3d",
            i, m_stPipePtTsInfo.GetVencAvgSpan(i), m_stPipePtTsInfo.nVencMaxTS[i] / 1000, m_stPipePtTsInfo.nVencMinTS[i] / 1000);
    }

    /* reset data */
    m_stPipePtTsInfo.Clear();

    m_bProfiling = AX_TRUE;
}
