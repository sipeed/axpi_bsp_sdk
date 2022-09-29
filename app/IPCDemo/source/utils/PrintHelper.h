/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _PRINT_HELPER_H_
#define _PRINT_HELPER_H_

#include "global.h"
#include "BaseSensor.h"
#include "OptionHelper.h"
#include "IVPSStage.h"
#include <thread>
#include <map>
#include <list>

#define PRINT_INTERVAL (10)

extern COptionHelper gOptions;
extern END_POINT_OPTIONS g_tEPOptions[MAX_VENC_CHANNEL_NUM];
extern IVPS_GROUP_CFG_T g_tIvpsGroupConfig[IVPS_GROUP_NUM];
extern AX_BOOL IsIVPSFromISPLinkable(AX_U8 nGrp);

typedef enum _PRINT_HELPER_MOD_E
{
    E_PH_MOD_CAMERA = 0,
    E_PH_MOD_IVPS,
    E_PH_MOD_VENC,
    E_PH_MOD_WEB,
    E_PH_MOD_WEB_CONN,
    E_PH_MOD_DET,
    E_PH_MOD_DET_PERF,
    E_PH_MOD_MAX,
} PRINT_HELPER_MOD_E;

typedef enum _PRINT_HELPER_PIPE_POINT_E
{
    E_PH_PIPE_PT_BASE = 0,
    E_PH_PIPE_PT_CAMERA,
    E_PH_PIPE_PT_CAMERA_VENC,
    E_PH_PIPE_PT_MAX
} PRINT_HELPER_PIPE_POINT_E;

typedef struct _PIPE_POINT_TIMESPAN_INFO_T
{
    AX_U32 nCameraTotalTS[MAX_ISP_CHANNEL_NUM];
    AX_U32 nCameraNum[MAX_ISP_CHANNEL_NUM];
    AX_U32 nCameraMaxTS[MAX_ISP_CHANNEL_NUM];
    AX_U32 nCameraMinTS[MAX_ISP_CHANNEL_NUM];

    AX_U32 nVencTotalTS[MAX_VENC_CHANNEL_NUM];
    AX_U32 nVencNum[MAX_VENC_CHANNEL_NUM];
    AX_U32 nVencMaxTS[MAX_VENC_CHANNEL_NUM];
    AX_U32 nVencMinTS[MAX_VENC_CHANNEL_NUM];

    _PIPE_POINT_TIMESPAN_INFO_T() {
        Clear();
    }

    AX_VOID Clear() {
        memset(this, 0, sizeof(_PIPE_POINT_TIMESPAN_INFO_T));
    }

    double GetCameraAvgSpan(AX_U32 nChannel) {
        return (double)((nCameraTotalTS[nChannel]) / (double)(nCameraNum[nChannel]) / 1000);
    }

    double GetVencAvgSpan(AX_U32 nChannel) {
        return (double)((nVencTotalTS[nChannel]) / (double)(nVencNum[nChannel]) / 1000);
    }

} PIPE_POINT_TIMESPAN_INFO_T;

typedef struct _CAMERA_PRINT_INFO_T
{
    AX_U32 arrReceivedFrames[MAX_ISP_CHANNEL_NUM];
    AX_U32 arrReleasedFrames[MAX_ISP_CHANNEL_NUM];
    AX_U32 arrQueueRemains[MAX_ISP_CHANNEL_NUM];
    AX_U32 arrPeriodFrames[MAX_ISP_CHANNEL_NUM];

    _CAMERA_PRINT_INFO_T() {
        memset(this, 0, sizeof(_CAMERA_PRINT_INFO_T));
    }

    AX_VOID Add(AX_U32 nChn) {
        if (nChn < 0 || nChn >= MAX_ISP_CHANNEL_NUM) {
            return;
        }

        arrReceivedFrames[nChn]++;
        arrPeriodFrames[nChn]++;
    }

    AX_VOID Remove(AX_U32 nChn) {
        if (nChn < 0 || nChn >= MAX_ISP_CHANNEL_NUM) {
            return;
        }

        arrReleasedFrames[nChn]++;
    }

    AX_VOID UpdateQueueRemains(AX_U32 nChn, AX_U32 nRemains) {
        if (nChn < 0 || nChn >= MAX_ISP_CHANNEL_NUM) {
            return;
        }

        arrQueueRemains[nChn] = nRemains;
    }

    AX_VOID Print() {
        AX_CHAR szHMS[10] = {0};
        for (AX_U32 i = 0; i < MAX_ISP_CHANNEL_NUM; i++) {
            if (IsIVPSFromISPLinkable(i)) {
                continue;
            }
            LOG_M("Sensor", "[%d] [%d] [%s] fps %5.2f, recv %d, free %d, left %d",
                0,
                i,
                CTimeUtils::GetCurrTimeStr(&szHMS[0]),
                arrPeriodFrames[i] * 1.0 / PRINT_INTERVAL,
                arrReceivedFrames[i],
                arrReleasedFrames[i],
                arrQueueRemains[i]);

            arrPeriodFrames[i] = 0;
        }
    }

} CAMERA_PRINT_INFO_T;

typedef struct _IVPS_PRINT_INFO_T
{
    AX_U32 arrReceivedFrames[IVPS_GROUP_NUM][8];
    AX_U32 arrPeriodFrames[IVPS_GROUP_NUM][8];

    _IVPS_PRINT_INFO_T() {
        memset(this, 0, sizeof(_IVPS_PRINT_INFO_T));
    }

    AX_VOID Add(AX_U32 nGroup, AX_U32 nChn) {
        if (nChn < 0) {
            return;
        }

        arrReceivedFrames[nGroup][nChn]++;
        arrPeriodFrames[nGroup][nChn]++;
    }

    AX_VOID Print() {
        AX_CHAR szHMS[10] = {0};
        for (AX_U32 i = 0; i < IVPS_GROUP_NUM; i++) {
            for (AX_U32 j = 0; j < g_tIvpsGroupConfig[i].nGrpChnNum; j++) {
                if (0 == g_tIvpsGroupConfig[i].arrLinkModeFlag[j]) {
                    LOG_M("IVPS", "[%d] [%d] [%s] fps %5.2f, recv %d",
                        i,
                        j,
                        CTimeUtils::GetCurrTimeStr(&szHMS[0]),
                        arrPeriodFrames[i][j] * 1.0 / PRINT_INTERVAL,
                        arrReceivedFrames[i][j]);

                    arrPeriodFrames[i][j] = 0;
                }
            }
        }
    }

} IVPS_PRINT_INFO_T;

typedef struct _VENC_PRINT_INFO_T
{
    AX_U32 nVencReceivedFrames[MAX_VENC_CHANNEL_NUM];
    AX_U32 nVencPeriodFrames[MAX_VENC_CHANNEL_NUM];

    _VENC_PRINT_INFO_T() {
        memset(this, 0, sizeof(_VENC_PRINT_INFO_T));
    }

    AX_VOID Add(AX_U32 nChn) {
        if (nChn < 0 || nChn >= MAX_VENC_CHANNEL_NUM) {
            return;
        }

        if (nVencReceivedFrames[nChn] > 0) {
            /* Ignore the header frame */
            nVencPeriodFrames[nChn]++;
        }
        nVencReceivedFrames[nChn]++;
    }

    AX_VOID Print() {
        AX_CHAR szHMS[10] = {0};
        for (AX_U32 i = 0; i < MAX_VENC_CHANNEL_NUM; i++) {
            if (E_END_POINT_DET == g_tEPOptions[i].eEPType) {
                if (gOptions.IsActivedDetect() && gOptions.IsActivedDetectFromWeb()) {
                    LOG_M("DETECTION", "[%d] [%d] [%s] fps %5.2f, recv %d",
                        0,
                        i,
                        CTimeUtils::GetCurrTimeStr(&szHMS[0]),
                        nVencPeriodFrames[i] * 1.0 / PRINT_INTERVAL,
                        nVencReceivedFrames[i] > 0 ? nVencReceivedFrames[i] - 1 : 0); /* Ignore the header frame */
                }
            } else if (E_END_POINT_JENC == g_tEPOptions[i].eEPType) {
                LOG_M("JENC", "[%d] [%d] [%s] fps %5.2f, recv %d",
                    0,
                    i,
                    CTimeUtils::GetCurrTimeStr(&szHMS[0]),
                    nVencPeriodFrames[i] * 1.0 / PRINT_INTERVAL,
                    nVencReceivedFrames[i] > 0 ? nVencReceivedFrames[i] - 1 : 0); /* Ignore the header frame */
            } else if (E_END_POINT_VENC == g_tEPOptions[i].eEPType) {
                LOG_M("VENC", "[%d] [%d] [%s] fps %5.2f, recv %d",
                    0,
                    i,
                    CTimeUtils::GetCurrTimeStr(&szHMS[0]),
                    nVencPeriodFrames[i] * 1.0 / PRINT_INTERVAL,
                    nVencReceivedFrames[i] > 0 ? nVencReceivedFrames[i] - 1 : 0); /* Ignore the header frame */
            }

            nVencPeriodFrames[i] = 0;
        }
    }

} VENC_PRINT_INFO_T;

typedef struct _WEB_PRINT_INFO_T
{
    AX_U32 nWebConnCount;
    AX_U32 nWebFrames[MAX_WEB_CHANNEL_NUM];
    AX_U32 nWebPeriodFrames[MAX_WEB_CHANNEL_NUM];
    AX_U32 nWebQueueRemains[MAX_WEB_CHANNEL_NUM];

    _WEB_PRINT_INFO_T() {
        memset(this, 0, sizeof(_WEB_PRINT_INFO_T));
    }

    AX_VOID Add(AX_U32 nChn) {
        nWebFrames[nChn]++;
        nWebPeriodFrames[nChn]++;
    }

    AX_VOID AddConn() {
        nWebConnCount++;
    }

    AX_VOID UpdateQueueRemains(AX_U32 nChn, AX_U32 nRemains) {
        if (nChn < 0 || nChn >= MAX_WEB_CHANNEL_NUM) {
            return;
        }

        nWebQueueRemains[nChn] = nRemains;
    }

    AX_VOID ClearConn() {
        nWebConnCount = 0;
    }

    AX_VOID Print() {
        AX_CHAR szHMS[10] = {0};
        LOG_M("WEB SERVER", "[X] [X] [%s] conn %d", CTimeUtils::GetCurrTimeStr(&szHMS[0]), nWebConnCount);
    }
} WEB_PRINT_INFO_T;

typedef struct _DET_PERF_INFO_DETAIL_T
{
    AX_U32 nBodys;
    AX_U32 nVehicles;
    AX_U32 nCycles;
    AX_U32 nFaces;
    AX_U32 nPlates;
} DET_PERF_INFO_DETAIL_T;

typedef struct _DET_PERF_INFO_T
{
    AX_U32 nElapsed;
    DET_PERF_INFO_DETAIL_T tTargets;
    DET_PERF_INFO_DETAIL_T tTracks;
} DET_PERF_INFO_T;

typedef struct _DET_PERF_PRINT_INFO_T
{
    AX_U32 nPeroidElapsed;
    AX_U32 nPeroidFrames;
    AX_U32 nMinElapsed;
    AX_U32 nMaxElapsed;
    AX_U64 nTotalFrames;
    DET_PERF_INFO_DETAIL_T tPeroidTargets;
    DET_PERF_INFO_DETAIL_T tPeroidTracks;
    AX_U32 nMaxTargets;
    AX_U32 nMaxTracks;

    _DET_PERF_PRINT_INFO_T() {
        memset(this, 0, sizeof(_DET_PERF_PRINT_INFO_T));

        nMinElapsed = 0xFFFFFFFF;
    }

    AX_VOID Add(AX_VOID *pCtx) {
        if (pCtx) {
            DET_PERF_INFO_T *pPerfInfo = (DET_PERF_INFO_T *)pCtx;

            nPeroidElapsed += pPerfInfo->nElapsed;
            nMinElapsed = AX_MIN(nMinElapsed, pPerfInfo->nElapsed);
            nMaxElapsed = AX_MAX(nMaxElapsed, pPerfInfo->nElapsed);

            nPeroidFrames ++;
            nTotalFrames ++;

            // target
            tPeroidTargets.nBodys += pPerfInfo->tTargets.nBodys;
            tPeroidTargets.nVehicles += pPerfInfo->tTargets.nVehicles;
            tPeroidTargets.nCycles += pPerfInfo->tTargets.nCycles;
            tPeroidTargets.nFaces += pPerfInfo->tTargets.nFaces;
            tPeroidTargets.nPlates += pPerfInfo->tTargets.nPlates;

            AX_U32 nTargets = pPerfInfo->tTargets.nBodys + pPerfInfo->tTargets.nVehicles
                                + pPerfInfo->tTargets.nCycles + pPerfInfo->tTargets.nFaces
                                + pPerfInfo->tTargets.nPlates;
            nMaxTargets = AX_MAX(nMaxTargets, nTargets);

            // tracks
            tPeroidTracks.nBodys += pPerfInfo->tTracks.nBodys;
            tPeroidTracks.nVehicles += pPerfInfo->tTracks.nVehicles;
            tPeroidTracks.nCycles += pPerfInfo->tTracks.nCycles;
            tPeroidTracks.nFaces += pPerfInfo->tTracks.nFaces;
            tPeroidTracks.nPlates += pPerfInfo->tTracks.nPlates;

            AX_U32 nTracks = pPerfInfo->tTracks.nBodys + pPerfInfo->tTracks.nVehicles
                                + pPerfInfo->tTracks.nCycles + pPerfInfo->tTracks.nFaces
                                + pPerfInfo->tTracks.nPlates;
            nMaxTracks = AX_MAX(nMaxTracks, nTracks);
        }
    }

    AX_VOID Print() {
        AX_CHAR szHMS[10] = {0};
        LOG_M("DETECTION", "[%d] [%d] [%s] AI perf: (min: %d, avg: %d, max: %d)",
            0,
            DETECTOR_IVPS_CHANNEL_NO,
            CTimeUtils::GetCurrTimeStr(&szHMS[0]),
            (nMinElapsed == 0xFFFFFFFF) ? 0 : nMinElapsed,
            (nPeroidFrames == 0) ? 0 : (nPeroidElapsed / nPeroidFrames),
            nMaxElapsed);

        LOG_M("DETECTION", "[%d] [%d] [%s] AI cap: (fps %5.2f, proc %llu)",
            0,
            DETECTOR_IVPS_CHANNEL_NO,
            CTimeUtils::GetCurrTimeStr(&szHMS[0]),
            nPeroidFrames * 1.0 / PRINT_INTERVAL,
            nTotalFrames);

        AX_U32 nTotalTargets = tPeroidTargets.nBodys + tPeroidTargets.nVehicles
                            + tPeroidTargets.nCycles + tPeroidTargets.nFaces
                            + tPeroidTargets.nPlates;

        AX_U32 nTotalTracks = tPeroidTracks.nBodys + tPeroidTracks.nVehicles
                            + tPeroidTracks.nCycles + tPeroidTracks.nFaces
                            + tPeroidTracks.nPlates;

        LOG_M("DETECTION", "[%d] [%d] [%s] AI targets(total: %u, max: %u), tracks(total: %u, max: %u)",
            0,
            DETECTOR_IVPS_CHANNEL_NO,
            CTimeUtils::GetCurrTimeStr(&szHMS[0]),
            nTotalTargets, nMaxTargets,
            nTotalTracks, nMaxTracks);

        nPeroidElapsed = 0;
        nPeroidFrames = 0;
        nMinElapsed = 0xFFFFFFFF;
        nMaxElapsed = 0;

        memset(&tPeroidTargets, 0x00, sizeof(tPeroidTargets));
        memset(&tPeroidTracks, 0x00, sizeof(tPeroidTracks));
        nMaxTargets = 0;
        nMaxTracks = 0;
    }
} DET_PERF_PRINT_INFO_T;

class CPrintHelper
{
public:
    CPrintHelper(AX_VOID);
    ~CPrintHelper(AX_VOID);

public:
    AX_VOID Start();
    AX_VOID Stop();
    AX_VOID Add(PRINT_HELPER_MOD_E eModType, AX_U32 nGrpID, AX_U32 nChn);
    AX_VOID Add(PRINT_HELPER_MOD_E eModType, AX_VOID *pCtx);
    AX_VOID Remove(PRINT_HELPER_MOD_E eModType, AX_U32 nChn);
    AX_VOID UpdateQueueRemains(PRINT_HELPER_MOD_E eModType, AX_U32 nChn, AX_U32 nRemains);
    AX_VOID AddTimeSpan(PRINT_HELPER_PIPE_POINT_E ePtType, AX_U32 nChn, AX_U64 nSeqNum);

private:
    AX_VOID PrintThreadFunc();
    AX_VOID DoProfiling();

private:
    AX_BOOL m_bEnableStart;
    AX_U64 m_nTickStart;
    AX_U64 m_nTickEnd;
    AX_U32 m_nPrintCnt;
    thread* m_pPrintThread;
    AX_BOOL m_bPrintThreadWorking;
    AX_BOOL m_bProfiling;

    CAMERA_PRINT_INFO_T  m_stCameraStatisticsInfo;
    IVPS_PRINT_INFO_T    m_stIvpsStatisticsInfo;
    VENC_PRINT_INFO_T    m_stVencStatisticsInfo;
    WEB_PRINT_INFO_T     m_stWebStatisticsInfo;
    DET_PERF_PRINT_INFO_T m_stDetPerfStatisticsInfo;

    PIPE_POINT_TIMESPAN_INFO_T m_stPipePtTsInfo;
    map<tuple<AX_U32, AX_U32>, std::chrono::high_resolution_clock::time_point> m_mapFrameTS;

    /* Temp solution because JENC(out) contains none seqNum info */
    list<AX_U64> m_listJencFrameTS, m_listJencFrameTS2;
};

#endif // _PRINT_HELPER_H_
