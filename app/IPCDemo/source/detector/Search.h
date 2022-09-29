/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SEACRH_BB66548A_29D0_40AD_866A_1BF131615DD3_H_
#define _SEACRH_BB66548A_29D0_40AD_866A_1BF131615DD3_H_

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
#include "mcv_search.h"
#include "AiSearch.h"
#include "Singleton.h"

/**
 * Search
 */
typedef struct _SEARCH_PARAM_ST {
    //Stream
    std::string config_path;

    //Search
    std::vector<std::string> object_types;
    float compare_score_threshold;
    AX_U32 capability;
    std::string base_img_path;
    std::string database_name;
    std::string feature_algo_type;
} SEARCH_PARAM_ST;

struct SEARCH_FEATURE_RESULT_ST {
    AX_BOOL m_bUsed;
    AX_VIDEO_FRAME_S m_sVideoFrame;
    std::chrono::steady_clock::time_point m_tpStart;
};

typedef struct {
    AX_U64 object_id;
    AX_U64 frame_id;
    AX_U32 track_id;
    std::string feature_info;
} SEARCH_OBJECT_INFO_ST;

#define SEARCH_API(_API_NAME_) pApi_##_API_NAME_
#define SEACRH_API_DEF(_API_NAME_, _API_RET_, _API_PARAM_) \
                            _API_RET_ (* SEARCH_API(_API_NAME_))_API_PARAM_ = nullptr

class CSearch : public CSingleton<CSearch>
{
    friend class CSingleton<CSearch>;

public:
    CSearch(AX_VOID);
    virtual ~CSearch(AX_VOID);

    virtual AX_BOOL Startup(AX_VOID);
    virtual AX_VOID Cleanup(AX_VOID);

    AX_BOOL AddFeatureToGroup(std::string infoStr, MCV_ALGORITHM_RESULT_S *algorithm_result);

    AX_BOOL CreateGroup(AX_U64 groupid);
    AX_BOOL DestoryGroup(AX_U64 groupid);
    AX_BOOL AddObjectToGroup(AX_U64 groupid, AX_U64 object_id, const AX_U8 *feature, AX_U32 feature_size,
                                       const SEARCH_OBJECT_INFO_ST &obj_info, AX_BOOL bSaveFeature = AX_TRUE);
    AX_BOOL DeleteObjectFromGroup(AX_U64 groupid, std::string infoStr);
    AX_BOOL IsGroupExist(AX_U64 groupid);

    //Feature
    AX_BOOL ProcessFeature(CMediaFrame* pFrame);
    AX_BOOL AsyncRecvFeatureResult(AX_VOID);
    AX_S32 GetFeatureInfo(const AX_CHAR *str);
    AX_S32 DeleteFeatureInfo(const AX_CHAR *str);

    //Search
    AX_BOOL SyncSearch(MCV_OBJECT_ITEM_S *object_item);

private:
    /* virtual function of CSingleton */
    AX_BOOL Init(AX_VOID) override {
        return AX_TRUE;
    };

    AX_BOOL LoadApi(AX_VOID);

    AX_BOOL WaitForFinish(AX_VOID);

    //Feature
    AX_BOOL SaveFeature(std::string featureInfo, const AX_U8 *feature, AX_U32 feature_size);
    AX_BOOL DeleteFeature(std::string featureInfo);
    AX_BOOL GenerateFeatureName(const AX_CHAR *str);
    AX_BOOL SetStreamConfig(MCV_STREAM_HANDLE_T ptStreamHandle);
    AX_BOOL InitFeatureHandle(AX_VOID);
    AX_BOOL FeatureResultHandler(MCV_ALGORITHM_RESULT_S *algorithm_result);
    AX_S32 Waitfor(AX_S32 msTimeOut = -1);

    //Load
    AX_BOOL LoadFeatureDataBase(AX_VOID);
    AX_VOID AsyncLoadFeatureThread(AX_VOID);

    //Search
    AX_BOOL Search(AX_U64 groupid, AX_U8 *features, int feature_cnt, int top_k,
                         MCV_GROUP_SEARCH_RESULT_S **result);

public:
    //Feature
    AX_BOOL m_bGetFeatureResultThreadRunning;
    AX_S32 m_GetFeatureInfoStatus;

    //Load
    AX_BOOL m_bLoadFeatureDataBaseThreadRunning;

protected:
    AX_BOOL m_bFinished;
    AX_BOOL m_bForedExit;
    AX_BOOL m_bFeatureDataBaseLoaded;

private:
    SEARCH_PARAM_ST m_stSearchParam;
    std::string m_FeatureDataBaseName;
    std::map<std::string, AX_U64> m_map_object_group_id;
    std::map<AX_U64, AX_U64> m_map_group_object_id;
    AX_U64 m_total_object_num;

    CElapsedTimer m_apiElapsed;
    std::map<AX_U64, std::map<std::string, SEARCH_OBJECT_INFO_ST *>> m_mapGroupObjectInfo;
    std::map<AX_U64, AX_U64> m_setGroupId;
    std::mutex m_setMutex;
    std::mutex m_mapMutex;

    //Feature
    std::mutex m_featureMutex;
    SEARCH_FEATURE_RESULT_ST m_feature_result;
    MCV_STREAM_HANDLE_T m_feature_handle = nullptr;
    thread *m_pGetFeatureResultThread = nullptr;
    AX_BOOL m_bGetFeatureInfo = AX_FALSE;
    std::mutex m_FeatureMutex;
    std::string m_FeatureInfoStr;

    //Load
    thread *m_pLoadFeatureDataBaseThread = nullptr;

    pthread_mutex_t m_condMutx;
    pthread_cond_t m_cond;
    pthread_condattr_t m_condattr;

    //API for Search
    AX_BOOL m_bApiLoaded = AX_FALSE;
    AX_VOID* m_pSearchLib = nullptr;

    SEACRH_API_DEF(mcv_init_sdk, MCV_RET_CODE_E, (const MCV_INIT_PARAM_S *param));
    SEACRH_API_DEF(mcv_deinit_sdk, MCV_RET_CODE_E, (void));
    SEACRH_API_DEF(mcv_get_version, MCV_RET_CODE_E, (const MCV_VERSION_INFO_S **version));
    SEACRH_API_DEF(mcv_set_log_config, MCV_RET_CODE_E, (const MCV_LOG_PARAM_S *log_param));
    SEACRH_API_DEF(mcv_get_capability, MCV_RET_CODE_E, (const MCV_CAPABILITY_S **capability));
    SEACRH_API_DEF(mcv_query_model_package, MCV_RET_CODE_E, (const MCV_MODEL_PACKAGE_LIST_S **model_package_list));
    SEACRH_API_DEF(mcv_create_stream_handle, MCV_RET_CODE_E, (const MCV_STREAM_HANDLE_PARAM_S *handle_param,
                                                                    MCV_STREAM_HANDLE_T *handle));
    SEACRH_API_DEF(mcv_release_stream_handle, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
    SEACRH_API_DEF(mcv_set_stream_config, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                 const MCV_STREAM_CONFIG_S *config));
    SEACRH_API_DEF(mcv_reset, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
    SEACRH_API_DEF(mcv_send_stream_frame, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, MCV_FRAME_S *frame));
    SEACRH_API_DEF(mcv_get_detect_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                             MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                             int timedwait_millisecond));
    SEACRH_API_DEF(mcv_get_algorithm_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                            MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                            int timedwait_millisecond));
    SEACRH_API_DEF(mcv_register_detect_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                           ALGORITHM_RESULT_CALLBACK_FUNC callback,
                                                           void *user_data));
    SEACRH_API_DEF(mcv_register_algorithm_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, ALGORITHM_RESULT_CALLBACK_FUNC callback, void *user_data));
    SEACRH_API_DEF(mcv_delete, MCV_RET_CODE_E, (void *p));
    SEACRH_API_DEF(mcv_search_init, MCV_RET_CODE_E, (void));
    SEACRH_API_DEF(mcv_search_deinit, MCV_RET_CODE_E, (void));
    SEACRH_API_DEF(mcv_search_create, MCV_RET_CODE_E, (uint64_t groupid,
                                         MCV_SEARCH_CREATE_GROUP_OPTPARAM_S* optparam));
    SEACRH_API_DEF(mcv_search_destroy, MCV_RET_CODE_E, (uint64_t groupid));
    SEACRH_API_DEF(mcv_search_feature_insert, MCV_RET_CODE_E, (uint64_t groupid,
                                                 MCV_SEARCH_INSERT_FEATURE_OPTPARAM_S* optparam));
    SEACRH_API_DEF(mcv_search_feature_delete, MCV_RET_CODE_E, (uint64_t groupid, uint64_t object_id, void** info));
    SEACRH_API_DEF(mcv_search, MCV_RET_CODE_E, (uint64_t groupid, MCV_SEARCH_RUN_OPTPARAM_S* optparam,
                                  MCV_GROUP_SEARCH_RESULT_S** result));
};

#endif /* _SEARCH_BB66548A_29D0_40AD_866A_1BF131615DD3_H_ */
