/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _DETECTOR_BB66548A_29D0_40AD_866A_1BF131615DD3_H_
#define _DETECTOR_BB66548A_29D0_40AD_866A_1BF131615DD3_H_

#include "global.h"
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <semaphore.h>
#include "MediaFrame.h"
#include "mcv_common_struct.h"
#include "mcv_interface.h"
#include "TrackCropStage.h"
#include "Search.h"
#include "StageOptionHelper.h"
#include "Singleton.h"

/**
 * FHVP detection
 */
typedef struct _DETECTOR_STREAM_PARAM_T {
    std::string algo_type;
    std::string config_path;
} DETECTOR_STREAM_PARAM_T;

struct FRAME_ALGORITHM_RESULT_ST {
    AX_VIDEO_FRAME_S m_sVideoFrame;
    std::chrono::steady_clock::time_point m_tpStart;
};

typedef struct _DETECTOR_RUNNING_STATUS_T {
    AX_U64 nFrameSent;
    AX_U64 nFrameRecv;

    _DETECTOR_RUNNING_STATUS_T() {
        nFrameSent = 0;
        nFrameRecv = 0;
    }
} DETECTOR_RUNNING_STATUS_T;

typedef struct _DETECTOR_PUSH_STRATEGY_T {
    AX_U8 nPushMode;
    AX_U32 nInterval; // only for MCV_PUSH_MODE_INTERVAL or MCV_PUSH_MODE_FAST
    AX_U32 nPushCounts; // only for MCV_PUSH_MODE_INTERVAL or MCV_PUSH_MODE_FAST
    AX_BOOL bPushSameFrame; // AX_FALSE: push cross frame AX_TRUE: push same frame
} DETECTOR_PUSH_STRATEGY_T;

typedef struct _DETECTOR_TRACK_SIZE_T {
    AX_U8 nTrackHumanSize;
    AX_U8 nTrackVehicleSize;
    AX_U8 nTrackCycleSize;

    _DETECTOR_TRACK_SIZE_T() {
        nTrackHumanSize = AI_MAX_TRACK_HUMAN_SIZE;
        nTrackVehicleSize = AI_MAX_TRACK_VEHICLE_SIZE;
        nTrackCycleSize = AI_MAX_TRACK_CYCLE_SIZE;
    }
} DETECTOR_TRACK_SIZE_T;

typedef struct _DETECTOR_ROI_CONFIG_T {
    AX_BOOL bEnable;
    AI_Detection_Box_t tBox;
} DETECTOR_ROI_CONFIG_T;

typedef struct _DETECTOR_OBJECT_FILTER_CONFIG_T {
    AX_U32 nWidth;
    AX_U32 nHeight;
    AX_F32 fConfidence;
} DETECTOR_OBJECT_FILTER_CONFIG_T;

/* Donot use memset/memcpy */
typedef struct _DETECTOR_CONFIG_PARAM_T {
    AX_U32 nTrackType;
    AX_U32 nDrawRectType;
    DETECTOR_PUSH_STRATEGY_T tPushStrategy;
    DETECTOR_STREAM_PARAM_T tStreamParam;
    AX_F32 fCropEncoderQpLevel;
    DETECTOR_TRACK_SIZE_T tTrackSize;
    DETECTOR_ROI_CONFIG_T tRoi;
    std::map<std::string, DETECTOR_OBJECT_FILTER_CONFIG_T> tObjectFliter;
    AX_BOOL bPlateIdentify;

    _DETECTOR_CONFIG_PARAM_T() {
        nTrackType = 0;
        nDrawRectType = 0;
        bPlateIdentify = AX_TRUE;
        fCropEncoderQpLevel = AI_TRACK_QPLEVEL_DEFAULT;
        memset(&tPushStrategy, 0x00, sizeof(tPushStrategy));
        memset(&tRoi, 0x00, sizeof(tRoi));
    }
} DETECTOR_CONFIG_PARAM_T;

typedef struct _PERF_TEST_INFO_T
{
    AX_BOOL m_bAiPerfTest;
    AX_U64 nPhyAddr;
    AX_VOID* pVirAddr;
    AX_U32 nSize;
    AX_U32 nWidth;
    AX_U32 nHeight;
} PERF_TEST_INFO_T;

#define DETECTOR_API(_API_NAME_) pApi_##_API_NAME_
#define DETECTOR_API_DEF(_API_NAME_, _API_RET_, _API_PARAM_) \
                            _API_RET_ (* DETECTOR_API(_API_NAME_))_API_PARAM_ = nullptr

class CDetector : public CSingleton<CDetector>
{
    friend class CSingleton<CDetector>;

public:
    CDetector(AX_VOID);
    virtual ~CDetector(AX_VOID);

    virtual AX_BOOL Startup(AX_VOID);
    virtual AX_VOID Cleanup(AX_VOID);

    AX_BOOL ProcessDetect(CMediaFrame* pFrame, AX_S32 nGaps = -1);
    AX_BOOL AsyncRecvDetectionResult(AX_VOID);
    AX_VOID BindCropStage(CTrackCropStage* pStage);
    DETECTOR_CONFIG_PARAM_T GetConfig(AX_VOID);
    AX_BOOL SetConfig(DETECTOR_CONFIG_PARAM_T *pConfig);
    AX_BOOL UpdateConfig(const AI_ATTR_T& tAiAttr);
    AX_BOOL SetRoi(DETECTOR_ROI_CONFIG_T *ptRoi);
    AX_BOOL SetPushStrategy(DETECTOR_PUSH_STRATEGY_T *ptPushStrategy);
    AX_BOOL SetObjectFilter(std::string strObject, DETECTOR_OBJECT_FILTER_CONFIG_T *ptObjectFliter);
    AX_BOOL SetTrackSize(DETECTOR_TRACK_SIZE_T *ptTrackSize);
    AX_BOOL SetCropEncoderQpLevel(AX_F32 fCropEncoderQpLevel);

private:
    /* virtual function of CSingleton */
    AX_BOOL Init(AX_VOID) override {
        return AX_TRUE;
    };

    AX_BOOL InitConfigParam(AX_VOID);
    AX_BOOL LoadApi(AX_VOID);
    AX_BOOL SetStreamConfig(MCV_STREAM_HANDLE_T ptStreamHandle, DETECTOR_CONFIG_PARAM_T *ptConfigParam);
    AX_BOOL InitStreamHandle(AX_VOID);
    AX_BOOL DetectionResultHandler(MCV_ALGORITHM_RESULT_S *algorithm_result);
    AX_BOOL ClearAlgorithmData(MCV_ALGORITHM_RESULT_S *algorithm_result);
    AX_BOOL ClearAlgorithmData(void);
    AX_BOOL WaitForFinish(AX_BOOL bWaitBlock = AX_FALSE);
    AX_BOOL LoadPerfTestInfo(AX_VOID);
    AX_BOOL UpdateAiAttr(AX_VOID);

public:
    AX_BOOL m_bGetResultThreadRunning;

protected:
    AX_BOOL m_bFinished;
    AX_BOOL m_bForedExit;
    AX_U64 m_nFrame_id;

private:
    MCV_STREAM_HANDLE_T m_stream_handle = nullptr;
    std::mutex m_mutex;
    std::map<AX_U32, FRAME_ALGORITHM_RESULT_ST> m_mapDataResult;
    AX_BOOL m_bThreadRunning;
    thread *m_pGetResultThread = nullptr;
    CTrackCropStage *m_pTrackCropStage = nullptr;
    CElapsedTimer m_apiElapsed;
    DETECTOR_RUNNING_STATUS_T m_tRunningStatus;
    std::mutex m_stMutex;
    DETECTOR_CONFIG_PARAM_T m_tConfigParam;

    //Search
    CSearch *m_pObjectSearch = nullptr;

    //AI Perf Test
    PERF_TEST_INFO_T m_perfTestInfo;

    //API for detect
    AX_BOOL m_bApiLoaded = AX_FALSE;
    AX_VOID* m_pDetectLib = nullptr;

    DETECTOR_API_DEF(mcv_init_sdk, MCV_RET_CODE_E, (const MCV_INIT_PARAM_S *param));
    DETECTOR_API_DEF(mcv_deinit_sdk, MCV_RET_CODE_E, (void));
    DETECTOR_API_DEF(mcv_get_version, MCV_RET_CODE_E, (const MCV_VERSION_INFO_S **version));
    DETECTOR_API_DEF(mcv_set_log_config, MCV_RET_CODE_E, (const MCV_LOG_PARAM_S *log_param));
    DETECTOR_API_DEF(mcv_get_capability, MCV_RET_CODE_E, (const MCV_CAPABILITY_S **capability));
    DETECTOR_API_DEF(mcv_query_model_package, MCV_RET_CODE_E, (const MCV_MODEL_PACKAGE_LIST_S **model_package_list));
    DETECTOR_API_DEF(mcv_create_stream_handle, MCV_RET_CODE_E, (const MCV_STREAM_HANDLE_PARAM_S *handle_param,
                                                                    MCV_STREAM_HANDLE_T *handle));
    DETECTOR_API_DEF(mcv_release_stream_handle, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
    DETECTOR_API_DEF(mcv_set_stream_config, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                 const MCV_STREAM_CONFIG_S *config));
    DETECTOR_API_DEF(mcv_reset, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
    DETECTOR_API_DEF(mcv_send_stream_frame, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, MCV_FRAME_S *frame));
    DETECTOR_API_DEF(mcv_get_detect_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                             MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                             int timedwait_millisecond));
    DETECTOR_API_DEF(mcv_get_algorithm_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                            MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                            int timedwait_millisecond));
    DETECTOR_API_DEF(mcv_register_detect_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                           ALGORITHM_RESULT_CALLBACK_FUNC callback,
                                                           void *user_data));
    DETECTOR_API_DEF(mcv_register_algorithm_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, ALGORITHM_RESULT_CALLBACK_FUNC callback, void *user_data));
    DETECTOR_API_DEF(mcv_delete, MCV_RET_CODE_E, (void *p));
};

#endif /* _DETECTOR_BB66548A_29D0_40AD_866A_1BF131615DD3_H_ */
