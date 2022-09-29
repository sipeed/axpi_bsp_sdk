/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <fstream>
#include <iostream>
#include <mutex>
#include <dlfcn.h>

#include "Search.h"
#include "OptionHelper.h"
#include "PrintHelper.h"
#include "FileUtils.h"
#include "CommonUtils.h"
#include "picojson.h"

#define SEARCH           "SEARCH"

#define PICO_OBJECT get<picojson::object>()
#define PICO_OBJECT_SIZE PICO_OBJECT.size()
#define PICO_ARRAY get<picojson::array>()
#define PICO_ARRAY_SIZE PICO_ARRAY.size()
#define PICO_VALUE get<double>()
#define PICO_BOOL get<bool>()
#define PICO_STRING get<std::string>()
#define PICO_ROOT obj.PICO_OBJECT

#define FEATURE_INFO_SIZE 32
//#define FEATURE_DATA_SIZE 280

#define SEARCH_DEFAULT_ALGO_TYPE "facehuman_image_algo"
#define SEARCH_DEFAULT_ALGO_CONFIG_PATH "./config/search_config.json"

#define SEARCH_WAITING_TIMEOUT 5000

typedef struct {
    AX_U8 Info[FEATURE_INFO_SIZE];
    AX_U32 FeatureSize;
} SEARCH_FEATURE_HEADER_ST;

typedef struct {
    SEARCH_FEATURE_HEADER_ST header;
    AX_U8 Feature[0];
} SEARCH_FEATURE_DB_ST;

#define SEARCH_API_LOAD(_API_NAME_, _API_RET_, _API_PARAM_) \
                            SEARCH_API(_API_NAME_) = (_API_RET_ (*)_API_PARAM_)dlsym(m_pSearchLib, #_API_NAME_); \
                            if (NULL == SEARCH_API(_API_NAME_)) { \
                                LOG_M_E(SEARCH, "Load API of %s Failed", #_API_NAME_); \
                                return AX_FALSE; \
                            }

#define SEARCH_API_RUN_START(_API_NAME_) m_apiElapsed.reset()
#define SEARCH_API_RUN_END(_API_NAME_) LOG_M_I(SEARCH, "Run API(%s) elapsed: %d(ms)", #_API_NAME_, m_apiElapsed.ms());

extern COptionHelper gOptions;
extern CPrintHelper gPrintHelper;

using namespace std;

// Object Calculate
#define ObjectCalculate(Obj) \
    do { \
        if (strcasecmp(object_category, #Obj) == 0) { \
            Obj##_size ++; \
        } \
    } while(0)

static std::vector<std::string> split(const std::string &s, const std::string &delim)
{
    std::vector<std::string> ret;
    size_t now = 0;
    auto pos = s.find(delim);
    while (std::string::npos != pos) {
        ret.push_back(s.substr(now, pos - now));
        now = pos + delim.size();
        pos = s.find(delim, now);
    }
    if (now == 0) {
        ret.push_back(s);
    } else {
        ret.push_back(s.substr(now));
    }
    return ret;
}

static void transform_algotype_string_to_streamtype(std::string &attr_string)
{
    if (attr_string == "facehuman_video_algo") {
        attr_string = "facehuman_video_all";
    } else if (attr_string == "hvcfp_video_algo") {
        attr_string = "hvcfp_video_all";
    } else if (attr_string == "facehuman_image_algo") {
        attr_string = "facehuman_image_all";
    } else if (attr_string == "hvcfp_image_algo") {
        attr_string = "hvcfp_image_all";
    } else if (attr_string == "search_video_algo") {
        attr_string = "facehuman_video_all";
    } else if (attr_string == "facehuman_video_detect_algo") {
        attr_string = "facehuman_video_detect";
    }

    return;
}

static void transform_analyze_string_to_enum(std::string attr_string,
                                             MCV_ANALYZE_ATTR_E &analyze_attribute)
{
    if (attr_string == "ANALYZE_ATTR_FACE_LIVENESS") {
        analyze_attribute = MCV_ANALYZE_ATTR_FACE_LIVENESS;
    } else if (attr_string == "ANALYZE_ATTR_FACE_FEATURE") {
        analyze_attribute = MCV_ANALYZE_ATTR_FACE_FEATURE;
    } else if (attr_string == "ANALYZE_ATTR_FACE_EMOTION") {
        analyze_attribute = MCV_ANALYZE_ATTR_FACE_EMOTION;
    } else if (attr_string == "ANALYZE_ATTR_FACE_ATTRIBUTE") {
        analyze_attribute = MCV_ANALYZE_ATTR_FACE_ATTRIBUTE;
    } else if (attr_string == "ANALYZE_ATTR_HUMAN_ACTION") {
        analyze_attribute = MCV_ANALYZE_ATTR_HUMAN_ACTION;
    } else if (attr_string == "ANALYZE_ATTR_HUMAN_ATTRIBUTE") {
        analyze_attribute = MCV_ANALYZE_ATTR_HUMAN_ATTRIBUTE;
    } else if (attr_string == "ANALYZE_ATTR_HUMAN_FEATURE") {
        analyze_attribute = MCV_ANALYZE_ATTR_HUMAN_FEATURE;
    } else if (attr_string == "ANALYZE_ATTR_MOTOR_VEHICLE_FEATURE") {
        analyze_attribute = MCV_ANALYZE_ATTR_MOTOR_VEHICLE_FEATURE;
    } else if (attr_string == "ANALYZE_ATTR_MOTOR_VEHICLE_ATTRIBUTE") {
        analyze_attribute = MCV_ANALYZE_ATTR_MOTOR_VEHICLE_ATTRIBUTE;
    } else if (attr_string == "ANALYZE_ATTR_NON_MOTOR_VEHICLE_FEATURE") {
        analyze_attribute = MCV_ANALYZE_ATTR_NON_MOTOR_VEHICLE_FEATURE;
    } else if (attr_string == "ANALYZE_ATTR_NON_MOTOR_VEHICLE_ATTRIBUTE") {
        analyze_attribute = MCV_ANALYZE_ATTR_NON_MOTOR_VEHICLE_ATTRIBUTE;
    } else if (attr_string == "ANALYZE_ATTR_PLATE_ATTRIBUTE") {
        analyze_attribute = MCV_ANALYZE_ATTR_PLATE_ATTRIBUTE;
    }

    return;
}

static void transform_push_mode_to_enum(std::string mode_string, MCV_PUSH_MODE_E &push_mode)
{
    if (mode_string == "PUSH_MODE_FAST") {
        push_mode = MCV_PUSH_MODE_FAST;
    } else if (mode_string == "PUSH_MODE_INTERVAL") {
        push_mode = MCV_PUSH_MODE_INTERVAL;
    } else if (mode_string == "PUSH_MODE_BEST") {
        push_mode = MCV_PUSH_MODE_BEST;
    }
}

static AX_BOOL get_facehuamn_hvcfp_handle_param(
    MCV_STREAM_HANDLE_PARAM_S &handle_param, SEARCH_PARAM_ST stSearchParam,
    std::vector<std::string> &object_category, std::vector<MCV_ITEM_SETTING_S> &item_setting,
    std::vector<MCV_ANALYZE_ATTR_E> &analyze_attribute,
    std::vector<std::string> &quality_filter_object_category,
    std::vector<MCV_PUSH_QUALITY_FILTER_S> &push_quality_filter)
{
    std::ifstream ifs(stSearchParam.config_path);

    if (ifs.fail()) {
        LOG_M_E(SEARCH, "%s not exist", stSearchParam.config_path.c_str());
        return AX_FALSE;
    }

    picojson::value obj;
    ifs >> obj;

    string err = picojson::get_last_error();
    if (!err.empty() || !obj.is<picojson::object>()) {
        LOG_M_E(SEARCH, "Failed to load json config file: %s", stSearchParam.config_path.c_str());
        return AX_FALSE;
    }

    if (strstr(stSearchParam.feature_algo_type.c_str(), "facehuman_video") ||
        strstr(stSearchParam.feature_algo_type.c_str(), "facehuman_image")) {
        handle_param.pipeline_type = MCV_PIPELINE_TYPE_FACEHUMAN;
    } else if (strstr(stSearchParam.feature_algo_type.c_str(), "hvcfp_video") ||
               strstr(stSearchParam.feature_algo_type.c_str(), "hvcfp_image")) {
        handle_param.pipeline_type = MCV_PIPELINE_TYPE_HVCFP;
    }

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    AX_U32 nWidth = pStageOption->GetAiAttr().tConfig.nWidth;
    AX_U32 nHeight = pStageOption->GetAiAttr().tConfig.nHeight;

    auto &stream_config = handle_param.facehuman_stream_config;
    stream_config.panorama_enabled = PICO_ROOT["panorama_enabled"].PICO_STRING == "true" ? true : false;
    stream_config.panorama_quality = PICO_ROOT["panorama_quality"].PICO_VALUE;
    stream_config.roi_enabled = PICO_ROOT["roi"].PICO_OBJECT["enable"].PICO_STRING == "true" ? true : false;
    stream_config.roi_rect.left = (int)(PICO_ROOT["roi"].PICO_OBJECT["points"].PICO_ARRAY[0].PICO_OBJECT["x"].PICO_VALUE * nWidth);
    stream_config.roi_rect.top = (int)(PICO_ROOT["roi"].PICO_OBJECT["points"].PICO_ARRAY[0].PICO_OBJECT["y"].PICO_VALUE * nHeight);
    stream_config.roi_rect.right = (int)(PICO_ROOT["roi"].PICO_OBJECT["points"].PICO_ARRAY[1].PICO_OBJECT["x"].PICO_VALUE * nWidth);
    stream_config.roi_rect.bottom = (int)(PICO_ROOT["roi"].PICO_OBJECT["points"].PICO_ARRAY[1].PICO_OBJECT["y"].PICO_VALUE * nHeight);
    stream_config.item_setting_size = PICO_ROOT["item_setting_size"].PICO_VALUE;
    item_setting.resize(stream_config.item_setting_size);
    object_category.resize(stream_config.item_setting_size);
    for (size_t i = 0; i < stream_config.item_setting_size; i++) {
        object_category[i] = PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["object_category"].PICO_STRING;
        item_setting[i].object_category = (char *)object_category[i].c_str();
        item_setting[i].width = PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["width"].PICO_VALUE;
        item_setting[i].height = PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["height"].PICO_VALUE;

        if (PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT.end() !=
                PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT.find("pose_blur")) {
            item_setting[i].pose_blur.pitch =
                PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["pitch"].PICO_VALUE;
            item_setting[i].pose_blur.yaw =
                PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["yaw"].PICO_VALUE;
            item_setting[i].pose_blur.roll =
                PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["roll"].PICO_VALUE;
            item_setting[i].pose_blur.blur =
                PICO_ROOT["item_setting"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["blur"].PICO_VALUE;
        }
    }
    stream_config.item_setting = item_setting.data();

    if (PICO_ROOT.end() != PICO_ROOT.find("push_quality_filter_size")) {
        stream_config.push_quality_filter_size = PICO_ROOT["push_quality_filter_size"].PICO_VALUE;
        quality_filter_object_category.resize(stream_config.push_quality_filter_size);
        push_quality_filter.resize(stream_config.push_quality_filter_size);
        for (size_t i = 0; i < stream_config.push_quality_filter_size; i++) {
            quality_filter_object_category[i] =
                PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["object_category"].PICO_STRING;
            push_quality_filter[i].object_category =
                (char *)quality_filter_object_category[i].c_str();
            if (quality_filter_object_category[i] == "face") {
                push_quality_filter[i].face_quality.width =
                    PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["width"].PICO_VALUE;
                push_quality_filter[i].face_quality.height =
                    PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["height"].PICO_VALUE;

                if (PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT.end() !=
                    PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT.find("pose_blur")) {
                    push_quality_filter[i].face_quality.pose_blur.pitch =
                        PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["pitch"].PICO_VALUE;
                    push_quality_filter[i].face_quality.pose_blur.yaw =
                        PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["yaw"].PICO_VALUE;
                    push_quality_filter[i].face_quality.pose_blur.roll =
                        PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["roll"].PICO_VALUE;
                    push_quality_filter[i].face_quality.pose_blur.blur =
                        PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["pose_blur"].PICO_OBJECT["blur"].PICO_VALUE;
                }
            } else {
                push_quality_filter[i].common_quality.quality =
                    PICO_ROOT["push_quality_filter"].PICO_ARRAY[i].PICO_OBJECT["quality"].PICO_VALUE;
            }
        }
        stream_config.push_quality_filter = push_quality_filter.data();
    }

    transform_push_mode_to_enum(PICO_ROOT["push_strategy"].PICO_OBJECT["push_mode"].PICO_STRING,
                                stream_config.push_strategy.push_mode);

    stream_config.push_strategy.interval_times =
        PICO_ROOT["push_strategy"].PICO_OBJECT["interval_times"].PICO_VALUE;
    stream_config.push_strategy.push_counts =
        PICO_ROOT["push_strategy"].PICO_OBJECT["push_counts"].PICO_VALUE;
    stream_config.push_strategy.push_same_frame =
        PICO_ROOT["push_strategy"].PICO_OBJECT["push_same_frame"].PICO_STRING == "true" ? true : false;

    stream_config.analyze_attribute_size = PICO_ROOT["analyze_attribute_size"].PICO_VALUE;
    analyze_attribute.resize(stream_config.analyze_attribute_size);
    for (size_t i = 0; i < stream_config.analyze_attribute_size; i++) {
        transform_analyze_string_to_enum(PICO_ROOT["analyze_attribute"].PICO_ARRAY[i].PICO_STRING,
                                        analyze_attribute[i]);
    }
    stream_config.analyze_attribute = analyze_attribute.data();
    stream_config.push_bind_enabled = PICO_ROOT["push_bind_enabled"].PICO_STRING == "true" ? true : false;
    handle_param.detect_result_interface_enabled =
        PICO_ROOT["detect_result_interface_enabled"].PICO_STRING == "true" ? true : false;

    return AX_TRUE;
}

static AX_BOOL get_search_param(SEARCH_PARAM_ST &stSearchParam, std::string config_path)
{
    std::ifstream ifs(config_path);

    if (ifs.fail()) {
        LOG_M_E(SEARCH, "%s not exist", config_path.c_str());
        return AX_FALSE;
    }

    picojson::value obj;
    ifs >> obj;

    string err = picojson::get_last_error();
    if (!err.empty() || !obj.is<picojson::object>()) {
        LOG_M_E(SEARCH, "Failed to load json config file: %s", stSearchParam.config_path.c_str());
        return AX_FALSE;
    }

    stSearchParam.object_types.resize(PICO_ROOT["search_info"].PICO_OBJECT["object_types"].PICO_ARRAY_SIZE);
    for (size_t i = 0; i < PICO_ROOT["search_info"].PICO_OBJECT["object_types"].PICO_ARRAY_SIZE; i++) {
        stSearchParam.object_types[i] =
            PICO_ROOT["search_info"].PICO_OBJECT["object_types"].PICO_ARRAY[i].PICO_STRING;
    }
    stSearchParam.compare_score_threshold =
        PICO_ROOT["search_info"].PICO_OBJECT["compare_score_threshold"].PICO_VALUE;
    stSearchParam.capability =
        PICO_ROOT["search_info"].PICO_OBJECT["capability"].PICO_VALUE;
    stSearchParam.base_img_path =
        PICO_ROOT["search_info"].PICO_OBJECT["base_img_path"].PICO_STRING;
    stSearchParam.database_name =
        PICO_ROOT["search_info"].PICO_OBJECT["database_name"].PICO_STRING;
    stSearchParam.feature_algo_type =
        PICO_ROOT["search_info"].PICO_OBJECT["feature_algo_type"].PICO_STRING;

    //default
    if (stSearchParam.compare_score_threshold <= 0) {
        stSearchParam.compare_score_threshold = AI_SERACH_FACE_COMPARE_SCORE_DEFAULT;
    }

    //default
    if (stSearchParam.capability <= 0) {
        stSearchParam.capability = AI_SERACH_FACE_BASE_IMG_CAP_DEFAULT;
    }

    return AX_TRUE;
}

//////////////////////////////////////////////////////////////////
CSearch::CSearch(AX_VOID)
{
    m_map_object_group_id = {{"face", 1}, {"body", 2}, {"vehicle", 3}, {"cycle", 4}};
    m_map_group_object_id = {{1, 1}, {2, 1}, {3, 1}, {4, 1}};

    m_total_object_num = 0;

    m_bFinished = AX_TRUE;
    m_bForedExit = AX_FALSE;
    m_bFeatureDataBaseLoaded = AX_FALSE;

    //Feature
    memset(&m_feature_result, 0x00, sizeof(m_feature_result));
    m_GetFeatureInfoStatus = 0;
    m_feature_handle = nullptr;
    m_bGetFeatureResultThreadRunning = AX_FALSE;
    m_pGetFeatureResultThread = nullptr;
    m_bGetFeatureInfo = AX_FALSE;

    //Load
    m_bLoadFeatureDataBaseThreadRunning = AX_FALSE;
    m_pLoadFeatureDataBaseThread = nullptr;

    m_stSearchParam.compare_score_threshold = 71.0;
    m_stSearchParam.capability = 10000;
    m_stSearchParam.feature_algo_type = SEARCH_DEFAULT_ALGO_TYPE;
    m_stSearchParam.config_path = SEARCH_DEFAULT_ALGO_CONFIG_PATH;

    // init mutex
    if (0 != pthread_mutex_init(&m_condMutx, NULL)) {
        LOG_M_W(SEARCH, "pthread_mutex_init() fail");
    }

    if (0 != pthread_condattr_init(&m_condattr)) {
        LOG_M_W(SEARCH, "pthread_condattr_init() fail");
    }

    if (0 != pthread_condattr_setclock(&m_condattr, CLOCK_MONOTONIC)) {
        LOG_M_W(SEARCH, "pthread_condattr_setclock() fail");
     }

     if (0 != pthread_cond_init(&m_cond, &m_condattr)) {
         LOG_M_W(SEARCH, "pthread_cond_init() fail");
     }
}

CSearch::~CSearch(AX_VOID)
{

}

AX_BOOL CSearch::Startup(AX_VOID)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    LOG_M_W(SEARCH, "+++");

    if (!gOptions.IsActivedSearch()) {
        return AX_FALSE;
    }

    //Load Api
    if (!LoadApi()) {
        LOG_M_E(SEARCH, "Load Api failed");

        return AX_FALSE;
    }

    ret = SEARCH_API(mcv_search_init)();

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(SEARCH, "mcv_search_init error");

        return AX_FALSE;
    }

    //Stream Param
    m_stSearchParam.config_path = gOptions.GetDetectionConfigPath();

    //get search parameter
    get_search_param(m_stSearchParam, m_stSearchParam.config_path);

    transform_algotype_string_to_streamtype(m_stSearchParam.feature_algo_type);

    //Check search param
    if (m_stSearchParam.base_img_path.size() == 0) {
        LOG_M_E(SEARCH, "base_img path empty");

        return AX_FALSE;
    }

    if (m_stSearchParam.database_name.size() == 0) {
        LOG_M_E(SEARCH, "database name empty");

        return AX_FALSE;
    }

    if (!InitFeatureHandle()) {
        LOG_M_E(SEARCH, "InitFeatureHandle error");

        return AX_FALSE;
    }

    for (size_t i = 0; i < m_stSearchParam.object_types.size(); i ++) {
        AX_U64 groupid = m_map_object_group_id[m_stSearchParam.object_types[i]];

        if (!CreateGroup(groupid)) {
            return AX_FALSE;
        }

        LOG_M(SEARCH, "CreateGroup(%lld) for feature: %s", groupid, m_stSearchParam.object_types[0].c_str());
    }

    //Load database
    LoadFeatureDataBase();

    m_bFinished = AX_TRUE;
    m_bForedExit = AX_FALSE;

    LOG_M_W(SEARCH, "---");

    return AX_TRUE;
}

AX_VOID CSearch::Cleanup(AX_VOID)
{
    LOG_M_W(SEARCH, "+++");

    m_bForedExit = AX_TRUE;

    if (m_bGetFeatureResultThreadRunning) {
        m_bGetFeatureResultThreadRunning = AX_FALSE;

        if (m_pGetFeatureResultThread && m_pGetFeatureResultThread->joinable()) {
            m_pGetFeatureResultThread->join();

            delete m_pGetFeatureResultThread;
            m_pGetFeatureResultThread = nullptr;
        }
    }

    for (size_t i = 0; i < m_stSearchParam.object_types.size(); i++) {
        AX_U64 groupid = m_map_object_group_id[m_stSearchParam.object_types[i]];

        DestoryGroup(groupid);
    }

    if (m_bApiLoaded) {
        WaitForFinish();

        if (m_feature_handle) {
            SEARCH_API_RUN_START(mcv_release_stream_handle);
            SEARCH_API(mcv_release_stream_handle)(m_feature_handle);
            SEARCH_API_RUN_END(mcv_release_stream_handle);
        }

        m_featureMutex.lock();
        if (m_feature_result.m_bUsed) {
            if (m_feature_result.m_sVideoFrame.u32BlkId[0] > 0) {
                AX_POOL_DecreaseRefCnt(m_feature_result.m_sVideoFrame.u32BlkId[0], AX_ID_USER);
            }
            if (m_feature_result.m_sVideoFrame.u32BlkId[1] > 0) {
                AX_POOL_DecreaseRefCnt(m_feature_result.m_sVideoFrame.u32BlkId[1], AX_ID_USER);
            }

            memset(&m_feature_result, 0x00, sizeof(m_feature_result));
        }
        m_featureMutex.unlock();

        m_mapMutex.lock();
        for (auto &groupObjectInfo : m_mapGroupObjectInfo) {
            for (auto &objectInfo : groupObjectInfo.second) {
                delete objectInfo.second;
            }
            groupObjectInfo.second.clear();
        }

        m_mapGroupObjectInfo.clear();
        m_mapMutex.unlock();

        m_setMutex.lock();
        for (auto &groupId : m_setGroupId) {
            SEARCH_API(mcv_search_destroy)(groupId.second);
        }
        m_setGroupId.clear();
        m_setMutex.unlock();

        SEARCH_API(mcv_search_deinit)();
    }

    if (m_pSearchLib) {
        dlclose(m_pSearchLib);
        m_pSearchLib = nullptr;
    }

    m_bApiLoaded = AX_FALSE;

    LOG_M_W(SEARCH, "---");
}

AX_BOOL CSearch::LoadApi(AX_VOID)
{
    if (!m_bApiLoaded) {
        m_pSearchLib = dlopen("libmcvsdk_video.so", RTLD_LAZY);

        if (!m_pSearchLib) {
            LOG_M_E(SEARCH, "Load search lib failed");
            return AX_FALSE;
        }

        SEARCH_API_LOAD(mcv_init_sdk, MCV_RET_CODE_E, (const MCV_INIT_PARAM_S *param));
        SEARCH_API_LOAD(mcv_deinit_sdk, MCV_RET_CODE_E, (void));
        SEARCH_API_LOAD(mcv_get_version, MCV_RET_CODE_E, (const MCV_VERSION_INFO_S **version));
        SEARCH_API_LOAD(mcv_set_log_config, MCV_RET_CODE_E, (const MCV_LOG_PARAM_S *log_param));
        SEARCH_API_LOAD(mcv_get_capability, MCV_RET_CODE_E, (const MCV_CAPABILITY_S **capability));
        SEARCH_API_LOAD(mcv_query_model_package, MCV_RET_CODE_E, (const MCV_MODEL_PACKAGE_LIST_S **model_package_list));
        SEARCH_API_LOAD(mcv_create_stream_handle, MCV_RET_CODE_E, (const MCV_STREAM_HANDLE_PARAM_S *handle_param,
                                                                        MCV_STREAM_HANDLE_T *handle));
        SEARCH_API_LOAD(mcv_release_stream_handle, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
        SEARCH_API_LOAD(mcv_set_stream_config, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                     const MCV_STREAM_CONFIG_S *config));
        SEARCH_API_LOAD(mcv_reset, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
        SEARCH_API_LOAD(mcv_send_stream_frame, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, MCV_FRAME_S *frame));
        SEARCH_API_LOAD(mcv_get_detect_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                 MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                                 int timedwait_millisecond));
        SEARCH_API_LOAD(mcv_get_algorithm_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                                int timedwait_millisecond));
        SEARCH_API_LOAD(mcv_register_detect_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                               ALGORITHM_RESULT_CALLBACK_FUNC callback,
                                                               void *user_data));
        SEARCH_API_LOAD(mcv_register_algorithm_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, ALGORITHM_RESULT_CALLBACK_FUNC callback, void *user_data));
        SEARCH_API_LOAD(mcv_delete, MCV_RET_CODE_E, (void *p));
        SEARCH_API_LOAD(mcv_search_init, MCV_RET_CODE_E, (void));
        SEARCH_API_LOAD(mcv_search_deinit, MCV_RET_CODE_E, (void));
        SEARCH_API_LOAD(mcv_search_create, MCV_RET_CODE_E, (uint64_t groupid,
                                             MCV_SEARCH_CREATE_GROUP_OPTPARAM_S* optparam));
        SEARCH_API_LOAD(mcv_search_destroy, MCV_RET_CODE_E, (uint64_t groupid));
        SEARCH_API_LOAD(mcv_search_feature_insert, MCV_RET_CODE_E, (uint64_t groupid,
                                                     MCV_SEARCH_INSERT_FEATURE_OPTPARAM_S* optparam));
        SEARCH_API_LOAD(mcv_search_feature_delete, MCV_RET_CODE_E, (uint64_t groupid, uint64_t object_id, void** info));
        SEARCH_API_LOAD(mcv_search, MCV_RET_CODE_E, (uint64_t groupid, MCV_SEARCH_RUN_OPTPARAM_S* optparam,
                                      MCV_GROUP_SEARCH_RESULT_S** result));

        m_bApiLoaded = AX_TRUE;
    }

    LOG_M_W(SEARCH, "Load search API Success");

    return m_bApiLoaded;
}

AX_VOID CSearch::AsyncLoadFeatureThread(AX_VOID)
{
    FILE* pFile = NULL;
    AX_U32 db_file_size = 0;
    AX_U32 total_len_read = 0;

    LOG_M(SEARCH, "Start load feature database");

    std::chrono::steady_clock::time_point beginTime = std::chrono::steady_clock::now();

    //open database
    pFile = fopen(m_FeatureDataBaseName.c_str(), "rb");

    if (!pFile) {
        goto EXIT;
    }

    fseek(pFile, 0, SEEK_END);
    db_file_size = ftell(pFile);
    rewind(pFile);

    total_len_read = 0;

    while(!m_bForedExit && total_len_read < db_file_size) {
        AX_U64 groupid = m_map_object_group_id["face"];
        AX_U64 object_id = m_map_group_object_id[groupid];

        if (m_map_group_object_id[groupid] > m_stSearchParam.capability) {
            LOG_M_E(SEARCH, "Exceed the search capability (%d)", m_stSearchParam.capability);
            break;
        }

        //SEARCH_FEATURE_DB_ST
        SEARCH_FEATURE_HEADER_ST feature_item_header = {0};

        //read header
        AX_U32 head_size = fread((AX_U8 *)&feature_item_header, 1, sizeof(feature_item_header), pFile);

        //check header
        if (head_size != sizeof(feature_item_header)) {
            break;
        }

        //check feature size
        if (feature_item_header.FeatureSize == 0) {
            break;
        }

        total_len_read += head_size;

        AX_U8 *feature = (AX_U8 *)malloc(feature_item_header.FeatureSize);

        if (!feature) {
            LOG_M_E(SEARCH, "Memory alloc (%d) fail", feature_item_header.FeatureSize);
            break;
        }

        //read feature data
        AX_U32 feature_size = fread(feature, 1, feature_item_header.FeatureSize, pFile);

        //check feature data
        if (feature_size != feature_item_header.FeatureSize) {
            free(feature);
            break;
        }

        total_len_read += feature_size;

        SEARCH_OBJECT_INFO_ST object_info;
        m_mapMutex.lock();
        object_info.object_id = m_map_group_object_id[groupid];
        m_mapMutex.unlock();
        object_info.frame_id = 0;
        object_info.track_id = 0;
        object_info.feature_info = (AX_CHAR *)feature_item_header.Info;

        if (!AddObjectToGroup(groupid, object_id, feature, feature_size, object_info, AX_FALSE)) {
            free(feature);
            continue;
        }

        free(feature);

        m_mapMutex.lock();
        m_map_group_object_id[groupid] ++;
        m_total_object_num ++;
        m_mapMutex.unlock();
    }

EXIT:
    m_bFeatureDataBaseLoaded = AX_TRUE;

    if (pFile) {
        fclose(pFile);
    }

    auto endTime = std::chrono::steady_clock::now();
    AX_U32 nElapsed = (AX_U32)(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count());

    LOG_M(SEARCH, "Finish load feature database, loaded: %lld, elapsed: %d(ms)", m_total_object_num, nElapsed);

    return;
}

AX_BOOL CSearch::LoadFeatureDataBase(AX_VOID)
{
    //Init database file
    if (m_stSearchParam.base_img_path.size() > 0 && m_stSearchParam.database_name.size() > 0) {
        AX_CHAR sz_dat[256] = {0};
        size_t sz_dat_len = 0;

        sz_dat_len = snprintf(sz_dat, sizeof(sz_dat) - 1, "%s", m_stSearchParam.base_img_path.c_str());

        if (sz_dat[sz_dat_len - 1] == '/') {
            sz_dat[sz_dat_len - 1] = '\0';
        }

        snprintf(sz_dat + strlen(sz_dat), sizeof(sz_dat) - 1, "/%s", m_stSearchParam.database_name.c_str());

        m_FeatureDataBaseName = sz_dat;

         //try making the dir if not exist
        if (access(m_stSearchParam.base_img_path.c_str(), 0) != 0) {
            if (mkdir(m_stSearchParam.base_img_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
                LOG_M_E(SEARCH, "Could not create base_img path: %s", m_stSearchParam.base_img_path.c_str());
            }
        }
    }
    else {
        m_bFeatureDataBaseLoaded = AX_TRUE;

        return AX_FALSE;
    }

    std::thread t(&CSearch::AsyncLoadFeatureThread, this);

    t.detach();

    return AX_TRUE;
}

AX_BOOL CSearch::CreateGroup(AX_U64 groupid)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    if (IsGroupExist(groupid)) {
        LOG_M_E(SEARCH, "groupid(%lld) already exist");

        return AX_FALSE;
    }
    else {
        MCV_SEARCH_CREATE_GROUP_OPTPARAM_S group_opt_param;
        memset(&group_opt_param, 0, sizeof(group_opt_param));
        group_opt_param.search_type = MCV_SEARCH_TYPE_NPU;

        ret = SEARCH_API(mcv_search_create)(groupid, &group_opt_param);

        if (MCV_RET_SUCCESS == ret) {
            m_setMutex.lock();
            m_setGroupId[groupid] = groupid;
            m_setMutex.unlock();
        }
        else {
            LOG_M_E(SEARCH, "mcv_search_create groupid(%lld) failed", groupid);

            return AX_FALSE;
        }
    }

    return AX_TRUE;
}

AX_BOOL CSearch::DestoryGroup(AX_U64 groupid)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    if (IsGroupExist(groupid)) {
        ret = SEARCH_API(mcv_search_destroy)(groupid);

        if (MCV_RET_SUCCESS == ret) {
            m_setMutex.lock();
            m_setGroupId.erase(groupid);
            m_setMutex.unlock();
        }
        else {
            LOG_M_E(SEARCH, "mcv_search_destroy groupid(%lld) failed", groupid);
        }
    }
    else {
        LOG_M_E(SEARCH, "groupid(%lld) not exist", groupid);
    }

    return AX_TRUE;
}

AX_BOOL CSearch::AddFeatureToGroup(std::string infoStr, MCV_ALGORITHM_RESULT_S *algorithm_result)
{
    auto check_object = [&](char *object_category) -> AX_BOOL {
        auto iterator = std::find_if(
            m_stSearchParam.object_types.begin(), m_stSearchParam.object_types.end(),
            [&](std::string &group_object) { return group_object == object_category; });
        if (iterator == m_stSearchParam.object_types.end()) {
            return AX_FALSE;
        } else {
            return AX_TRUE;
        }
    };

    if (!algorithm_result || algorithm_result->object_size == 0) {
        return AX_FALSE;
    }

    // Object Definition
    int Face_size = 0;

    for (size_t i = 0; i < algorithm_result->object_size; i++) {
        const char *object_category = (const char *)algorithm_result->object_items[i].object_category;
        if (object_category) {
            // Object Calculate
            ObjectCalculate(Face);
        }
    }

    //TODO
    if (Face_size != 1) {
        LOG_M_E(SEARCH, "Face_size: %d, exceed one face", Face_size);
        return AX_FALSE;
    }

    AX_BOOL bRet = AX_FALSE;

    for (size_t i = 0; i < algorithm_result->object_size; i ++) {
        if (check_object(algorithm_result->object_items[i].object_category) &&
            algorithm_result->object_items[i].feature_list_size > 0) {
            char *object_category = algorithm_result->object_items[i].object_category;
            AX_U8 *feature = (AX_U8 *)algorithm_result->object_items[i].feature_list[0].data_ptr;
            AX_U32 feature_size = algorithm_result->object_items[i].feature_list[0].data_size;

            AX_U64 groupid = m_map_object_group_id[object_category];
            AX_U64 object_id = m_map_group_object_id[groupid];
            SEARCH_OBJECT_INFO_ST object_info;
            object_info.object_id = object_id;
            object_info.frame_id = algorithm_result->object_items[i].frame_id;
            object_info.track_id = algorithm_result->object_items[i].track_id;
            object_info.feature_info = infoStr;

            if (m_map_group_object_id[groupid] > m_stSearchParam.capability) {
                LOG_M_E(SEARCH, "Exceed the search capability (%d)", m_stSearchParam.capability);
                return AX_FALSE;
            }

            if (!AddObjectToGroup(groupid, object_id, feature, feature_size, object_info)) {
                return AX_FALSE;
            }

            m_mapMutex.lock();
            m_map_group_object_id[groupid] ++;
            m_total_object_num ++;
            m_mapMutex.unlock();

            bRet = AX_TRUE;
        }
    }

    return bRet;
}

AX_BOOL CSearch::AddObjectToGroup(AX_U64 groupid, AX_U64 object_id, const AX_U8 *feature, AX_U32 feature_size,
                                   const SEARCH_OBJECT_INFO_ST &obj_info, AX_BOOL bSaveFeature /* = AX_TRUE*/) {
    AX_BOOL bRet = AX_FALSE;
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    if (IsGroupExist(groupid)) {
        auto iter = m_mapGroupObjectInfo[groupid].find(obj_info.feature_info.c_str());
        if (iter != m_mapGroupObjectInfo[groupid].end()) {
            LOG_M(SEARCH, "groupid(%lld) %s already exist", groupid, obj_info.feature_info.c_str());
            return bRet;
        }

        SEARCH_OBJECT_INFO_ST *pobject_info = new SEARCH_OBJECT_INFO_ST(obj_info);
        MCV_SEARCH_INSERT_FEATURE_OPTPARAM_S optparam = {0};
        AX_U64 faceids[1];
        void *faceinfos[1];
        faceids[0] = object_id;
        faceinfos[0] = (void *)pobject_info;

        optparam.batch_size = 1;
        optparam.object_ids = faceids;
        optparam.features = (AX_U8 *)feature;
        optparam.object_infos = faceinfos;

        ret = SEARCH_API(mcv_search_feature_insert)(groupid, &optparam);

        if (MCV_RET_SUCCESS == ret) {
            if (bSaveFeature) {
                bRet = SaveFeature(obj_info.feature_info, feature, feature_size);

                if (!bRet) {
                    SEARCH_API(mcv_search_feature_delete)(groupid, object_id, (AX_VOID **)&pobject_info);
                }
                else {
                    m_mapMutex.lock();
                    m_mapGroupObjectInfo[groupid][obj_info.feature_info] = pobject_info;
                    m_mapMutex.unlock();
                }
            }
            else {
                m_mapMutex.lock();
                m_mapGroupObjectInfo[groupid][obj_info.feature_info] = pobject_info;
                m_mapMutex.unlock();

                bRet = AX_TRUE;
            }
        }
        else {
            delete pobject_info;

            LOG_M_E(SEARCH, "mcv_search_feature_insert groupid(%lld) failed: %d", groupid, ret);
        }
    }
    else {
        LOG_M_E(SEARCH, "groupid(%lld) not exist", groupid);
    }

    return bRet;
}

AX_BOOL CSearch::DeleteObjectFromGroup(AX_U64 groupid, std::string infoStr)
{
    AX_BOOL bRet = AX_FALSE;
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;
    SEARCH_OBJECT_INFO_ST *obj_info = NULL;

    if (IsGroupExist(groupid)) {
        auto iter = m_mapGroupObjectInfo[groupid].find(infoStr);
        if (iter != m_mapGroupObjectInfo[groupid].end()) {
            AX_U64 object_id = iter->second->object_id;

            ret = SEARCH_API(mcv_search_feature_delete)(groupid, object_id, (void **)&obj_info);

            if (MCV_RET_SUCCESS == ret) {
                m_mapMutex.lock();
                delete m_mapGroupObjectInfo[groupid][infoStr];
                m_mapGroupObjectInfo[groupid].erase(infoStr);
                m_mapMutex.unlock();

                bRet = DeleteFeature(infoStr);

                LOG_M(SEARCH, "feature groupid(%lld)(%lld) %s deleted", groupid, object_id, infoStr.c_str());
            } else {
                LOG_M_E(SEARCH, "mcv_search_feature_delete groupid(%lld)(%lld) %s failed", groupid, object_id, infoStr.c_str());
            }
        }
        else {
            LOG_M_E(SEARCH, "groupid(%lld) %s not find", groupid, infoStr.c_str());
        }
    }
    else {
        LOG_M_E(SEARCH, "groupid(%lld) not exist", groupid);
    }

    return bRet;
}

AX_BOOL CSearch::Search(AX_U64 groupid, AX_U8 *features, int feature_cnt, int top_k,
                         MCV_GROUP_SEARCH_RESULT_S **result)
{
    AX_BOOL bRet = AX_FALSE;
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    if (IsGroupExist(groupid)) {
        MCV_SEARCH_RUN_OPTPARAM_S optparam;
        optparam.batch_size = 1;
        optparam.top_k = top_k;
        optparam.features = features;
        optparam.verdict_userdatas = nullptr;
        optparam.verdict_func = nullptr;

        ret = SEARCH_API(mcv_search)(groupid, &optparam, result);

        if (MCV_RET_SUCCESS == ret) {
            bRet = AX_TRUE;
        }
        else {
            LOG_M_E(SEARCH, "mcv_search groupid(%lld) failed", groupid);
        }
    }
    else {
        LOG_M_E(SEARCH, "groupid(%lld) not exist", groupid);
    }

    return bRet;
}

AX_BOOL CSearch::SyncSearch(MCV_OBJECT_ITEM_S *object_item)
{
    AX_BOOL bRet = AX_FALSE;

    if (!gOptions.IsActivedSearchFromWeb()) {
        return AX_TRUE;
    }

    if (!object_item) {
        return AX_FALSE;
    }

    if (m_total_object_num == 0) {
        LOG_M_I(SEARCH, "no feature data list, ignore");
        return AX_FALSE;
    }

    auto check_object = [&](char *object_category) -> AX_BOOL {
        auto iterator = std::find_if(
            m_stSearchParam.object_types.begin(), m_stSearchParam.object_types.end(),
            [&](std::string &group_object) { return group_object == object_category; });
        if (iterator == m_stSearchParam.object_types.end()) {
            return AX_FALSE;
        } else {
            return AX_TRUE;
        }
    };

    char *object_category = object_item->object_category;
    if (check_object(object_category) && object_item->feature_list_size > 0) {
        AX_U64 groupid = m_map_object_group_id[object_category];
        AX_U8 *feature = (AX_U8 *)object_item->feature_list[0].data_ptr;
        MCV_GROUP_SEARCH_RESULT_S *search_result = nullptr;

        bRet = Search(groupid, feature, 1, 1, &search_result);

        if (!bRet) {
            return AX_FALSE;
        }

        bRet = AX_FALSE;

        for (size_t i = 0; i < search_result->batch_size; i++) {
            for (size_t j = 0; j < search_result->top_k; j++) {
                if (search_result->scores[i * search_result->top_k + j] < m_stSearchParam.compare_score_threshold) {
                    continue;
                }

                int object_group_index = search_result->object_id[i * search_result->top_k + j];
                SEARCH_OBJECT_INFO_ST *object_info =
                                        (SEARCH_OBJECT_INFO_ST *)search_result->info[i * search_result->top_k + j];

                LOG_M(SEARCH, "Found(feature: %s, track_id: %lld, score: %.2f, name: %s, index: %d)",
                                object_category,
                                object_item->track_id,
                                search_result->scores[i * search_result->top_k + j],
                                object_info->feature_info.c_str(),
                                object_group_index);

                bRet = AX_TRUE;
            }
        }

        SEARCH_API(mcv_delete)(search_result);
    }

    return bRet;
}

AX_BOOL CSearch::IsGroupExist(AX_U64 groupid)
{
    AX_BOOL bRet = AX_FALSE;

    m_setMutex.lock();
    auto iter = m_setGroupId.find(groupid);
    if (iter != m_setGroupId.end()) {
        bRet = AX_TRUE;
    } else {
        bRet = AX_FALSE;
    }
    m_setMutex.unlock();

    return bRet;
}

AX_BOOL CSearch::ProcessFeature(CMediaFrame* pFrame)
{
    AX_BOOL bRet = AX_FALSE;

    if (m_bForedExit) {
        return bRet;
    }

    //Get feature info
    if (!gOptions.IsActivedSearchFromWeb() || !m_bGetFeatureInfo) {
        return bRet;
    }

    m_FeatureMutex.lock();
    m_bGetFeatureInfo = AX_FALSE;
    m_FeatureMutex.unlock();

    if (!m_bApiLoaded) {
        LOG_M_E(SEARCH, "Detect api not loaded");
        goto PROC_EXIT;
    }

    if (!m_feature_handle) {
        LOG_M_E(SEARCH, "Stream handle not init");
        goto PROC_EXIT;
    }

    if (m_feature_result.m_bUsed) {
        LOG_M_E(SEARCH, "Feature is running");
        goto PROC_EXIT;
    }

    LOG_M_I(SEARCH, "+++");

    if (!m_bForedExit) {
        AX_VIDEO_FRAME_INFO_S tFrame = {0};
        if (pFrame->bIvpsFrame) {
            tFrame.stVFrame = pFrame->tVideoFrame;
        } else {
            tFrame = pFrame->tFrame.tFrameInfo;
        }

        m_bFinished = AX_FALSE;

        MCV_FRAME_S frame;
        memset(&frame, 0, sizeof(MCV_FRAME_S));
        frame.image_type = MCV_IMAGE_TYPE_NV12;
        frame.image_data.addr_type = MCV_IMAGE_SOURCE_ADDR;
        frame.header.extra_type = MCV_FRAME_EXTRA_TYPE_VIDEO;
        frame.image_data.addr.stride[0] = tFrame.stVFrame.u32PicStride[0];
        frame.image_data.addr.stride[1] = tFrame.stVFrame.u32PicStride[1];
        frame.image_data.addr.stride[2] = tFrame.stVFrame.u32PicStride[2];

        frame.image_data.addr.phy_addr[0] = (uint64_t)tFrame.stVFrame.u64PhyAddr[0];
        frame.image_data.addr.phy_addr[1] = (uint64_t)tFrame.stVFrame.u64PhyAddr[1];
        frame.image_data.addr.phy_addr[2] = (uint64_t)tFrame.stVFrame.u64PhyAddr[2];
        frame.image_data.addr.vir_addr[0] = (void *)tFrame.stVFrame.u64VirAddr[0];
        frame.image_data.addr.vir_addr[1] = (void *)tFrame.stVFrame.u64VirAddr[1];
        frame.image_data.addr.vir_addr[2] = (void *)tFrame.stVFrame.u64VirAddr[2];

        frame.header.frame_id = (AX_U64)pFrame->nFrameID;
        frame.header.width = tFrame.stVFrame.u32Width;
        frame.header.height = tFrame.stVFrame.u32Height;

        MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

        ret = SEARCH_API(mcv_send_stream_frame)(m_feature_handle, &frame);

        if (MCV_RET_SUCCESS == ret) {
            m_featureMutex.lock();
            m_feature_result.m_bUsed = AX_TRUE;
            m_feature_result.m_sVideoFrame = tFrame.stVFrame;
            m_feature_result.m_tpStart = std::chrono::steady_clock::now();

            if (tFrame.stVFrame.u32BlkId[0] > 0) {
                AX_POOL_IncreaseRefCnt(tFrame.stVFrame.u32BlkId[0], AX_ID_USER);
            }
            if (tFrame.stVFrame.u32BlkId[1] > 0) {
                AX_POOL_IncreaseRefCnt(tFrame.stVFrame.u32BlkId[1], AX_ID_USER);
            }
            m_featureMutex.unlock();

            bRet = AX_TRUE;
        }

        m_bFinished = AX_TRUE;
    }

    LOG_M_I(SEARCH, "---");

PROC_EXIT:
    if (!bRet) {
        m_GetFeatureInfoStatus = 1;

        pthread_mutex_lock(&m_condMutx);
        pthread_cond_signal(&m_cond);    // send signal
        pthread_mutex_unlock(&m_condMutx);
    }

    return bRet;
}

AX_BOOL CSearch::WaitForFinish(AX_VOID)
{
    while(!m_bFinished) {
        CTimeUtils::msSleep(10);
    }

    //Wait handle finish
    auto waitStart = std::chrono::steady_clock::now();
    do {
        m_featureMutex.lock();
        AX_BOOL bFeatureRunning = m_feature_result.m_bUsed;
        m_featureMutex.unlock();

        if (!bFeatureRunning) {
            break;
        }

        auto waitEnd = std::chrono::steady_clock::now();
        AX_U32 nElapsed = (AX_U32)(std::chrono::duration_cast<std::chrono::milliseconds>(waitEnd - waitStart).count());

        if (nElapsed >= SEARCH_WAITING_TIMEOUT) {
            break;
        }

        CTimeUtils::msSleep(10);
    } while(1);

    return AX_TRUE;
}

static AX_VOID AsyncRecvAlgorithmFeatureResultThread(AX_VOID *__this)
{
    CSearch *pThis = (CSearch *)__this;

    while (pThis->m_bGetFeatureResultThreadRunning) {
        if (!pThis->AsyncRecvFeatureResult()) {
            CTimeUtils::msSleep(1);
        }
    }
}

AX_BOOL CSearch::AsyncRecvFeatureResult(AX_VOID)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;
    MCV_ALGORITHM_RESULT_S *palgorithm_result = nullptr;

    ret = SEARCH_API(mcv_get_algorithm_result)(m_feature_handle, &palgorithm_result, 0);

    if (MCV_RET_SUCCESS != ret) {
        if (MCV_RET_EMPTY_RESULT != ret) {
            LOG_M_E(SEARCH, "mcv_get_algorithm_result error: %d", ret);
        }

        return AX_FALSE;
    }

    return FeatureResultHandler(palgorithm_result);
}

AX_BOOL CSearch::FeatureResultHandler(MCV_ALGORITHM_RESULT_S *algorithm_result)
{
    AX_BOOL bRet = AX_FALSE;
    auto endTime = std::chrono::steady_clock::now();

    if (!algorithm_result) {
        return AX_FALSE;
    }

    bRet = AddFeatureToGroup(m_FeatureInfoStr, algorithm_result);

    m_featureMutex.lock();
    if (m_feature_result.m_bUsed) {
        AX_U32 nElapsed = (AX_U32)(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_feature_result.m_tpStart).count());

        LOG_M_I(SEARCH, "Get feature elapsed %d(ms)", nElapsed);

        if (m_feature_result.m_sVideoFrame.u32BlkId[0] > 0) {
            AX_POOL_DecreaseRefCnt(m_feature_result.m_sVideoFrame.u32BlkId[0], AX_ID_USER);
        }
        if (m_feature_result.m_sVideoFrame.u32BlkId[1] > 0) {
            AX_POOL_DecreaseRefCnt(m_feature_result.m_sVideoFrame.u32BlkId[1], AX_ID_USER);
        }

        memset(&m_feature_result, 0x00, sizeof(m_feature_result));
    }
    m_featureMutex.unlock();

    SEARCH_API(mcv_delete)(algorithm_result);

    if (bRet) {
        m_GetFeatureInfoStatus = 0;
    }
    else {
        m_GetFeatureInfoStatus = 1;
    }

    pthread_mutex_lock(&m_condMutx);
    pthread_cond_signal(&m_cond);    // send signal
    pthread_mutex_unlock(&m_condMutx);

    return bRet;
}

AX_BOOL CSearch::InitFeatureHandle(AX_VOID)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    //capability query
    const MCV_CAPABILITY_S *capability = nullptr;
    ret = SEARCH_API(mcv_get_capability)(&capability);
    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(SEARCH, "mcv_get_capability error");
        return AX_FALSE;
    }

    //stream type
    std::string algo_type = m_stSearchParam.feature_algo_type;

    LOG_M(SEARCH, "Use algo: %s", algo_type.c_str());

    AX_BOOL find_algo_type = AX_FALSE;
    for (size_t i = 0; i < capability->pipline_config_size; i++) {
        LOG_M_I(SEARCH, "pipline_config_key: %s", capability->pipline_config[i].pipline_config_key);
        auto type = split(capability->pipline_config[i].pipline_config_key, ":");
        if (type[2] == algo_type) {
            algo_type = capability->pipline_config[i].pipline_config_key;
            find_algo_type = AX_TRUE;
        }
    }

    SEARCH_API(mcv_delete)((void *)capability);

    if (!find_algo_type) {
        LOG_M_E(SEARCH, "not find algo_type for %s", algo_type.c_str());
        return AX_FALSE;
    }

    MCV_STREAM_HANDLE_PARAM_S handle_param{0};
    std::vector<std::string> json_data_config;
    std::vector<MCV_META_INFO_S> meta_info;
    std::vector<std::string> object_category;
    std::vector<MCV_ITEM_SETTING_S> item_setting;
    std::vector<MCV_ANALYZE_ATTR_E> analyze_attribute;
    std::vector<std::string> quality_filter_object_category;
    std::vector<MCV_PUSH_QUALITY_FILTER_S> push_quality_filter;

    AX_BOOL bRet = AX_TRUE;
    handle_param.pipline_config_key = (char *)algo_type.c_str();
    if (m_stSearchParam.config_path.size() > 0) {
        if (strstr(algo_type.c_str(), "facehuman_video") ||
            strstr(algo_type.c_str(), "facehuman_image") ||
            strstr(algo_type.c_str(), "hvcfp_video") ||
            strstr(algo_type.c_str(), "hvcfp_image")) {
            bRet = get_facehuamn_hvcfp_handle_param(
                handle_param, m_stSearchParam, object_category, item_setting, analyze_attribute,
                quality_filter_object_category, push_quality_filter);
        } else {
            LOG_M_E(SEARCH, "algo_type:%s not support", m_stSearchParam.feature_algo_type.c_str());
            return AX_FALSE;
        }
    }
    else {
        LOG_M_E(SEARCH, "config_path invalid");
        return AX_FALSE;
    }

    if (!bRet) {
        LOG_M_E(SEARCH, "get handle param error");
        return AX_FALSE;
    }

    handle_param.skip_frame_by_user = true;
    handle_param.buffer_depth = (AX_AI_DETECT_FRAME_DEPTH ? AX_AI_DETECT_FRAME_DEPTH : 1);
    if (algo_type.find("image") != std::string::npos) {
        handle_param.stream_type = MCV_STREAM_TYPE_IMAGE;
    } else {
        handle_param.stream_type = MCV_STREAM_TYPE_VIDEO;
    }
    handle_param.smooth_result_enabled = false;

    SEARCH_API_RUN_START(mcv_create_stream_handle);
    ret = DETECTOR_API(mcv_create_stream_handle)(&handle_param, &m_feature_handle);
    SEARCH_API_RUN_END(mcv_create_stream_handle);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(SEARCH, "Init handle error");
        return AX_FALSE;
    } else {
        LOG_M_I(SEARCH, "Init handle success");
    }

    if (strstr(algo_type.c_str(), "facehuman_video") ||
        strstr(algo_type.c_str(), "facehuman_image") ||
        strstr(algo_type.c_str(), "hvcfp_video") ||
        strstr(algo_type.c_str(), "hvcfp_image")) {
        bRet = SetStreamConfig(m_feature_handle);
    }

    if (!bRet) {
        LOG_M_E(SEARCH, "SetStreamConfig error");
        return AX_FALSE;
    }

    m_bGetFeatureResultThreadRunning = AX_TRUE;
    m_pGetFeatureResultThread = new thread(AsyncRecvAlgorithmFeatureResultThread, this);

    return AX_TRUE;
}

AX_BOOL CSearch::SetStreamConfig(MCV_STREAM_HANDLE_T ptStreamHandle)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    if (ptStreamHandle == nullptr) {
        LOG_M_E(SEARCH, "stream handle null..");

        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    //set frame cache depth
    MCV_COMMON_THRESHOLD_CONFIG_S threshold;
    threshold.value = (float)1.0;
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"frame_cache_depth";
    tStreamItem.value = (void *)&threshold;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    ret = SEARCH_API(mcv_set_stream_config)(ptStreamHandle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(SEARCH, "mcv_set_stream_config error: %d", ret);
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_BOOL CSearch::GenerateFeatureName(const AX_CHAR *str)
{
    AX_CHAR sz_dat[128] = {0};

    if (!str) {
        time_t t;
        struct tm tm;

        t = time(NULL);
        localtime_r(&t, &tm);

        snprintf(sz_dat, sizeof(sz_dat) - 1, "face_%04d%02d%02d%02d%02d%02d",
                tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    else {
        snprintf(sz_dat, sizeof(sz_dat) - 1, "face_%s", str);
    }

    m_FeatureInfoStr = sz_dat;

    return AX_TRUE;
}

AX_BOOL CSearch::SaveFeature(std::string featureInfo, const AX_U8 *feature, AX_U32 feature_size)
{
    if (m_FeatureDataBaseName.size() > 0
        && featureInfo.size() > 0
        && feature
        && feature_size > 0) {
         //try making the dir if not exist
        if (access(m_stSearchParam.base_img_path.c_str(), 0) != 0) {
            if (mkdir(m_stSearchParam.base_img_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0) {
                LOG_M_E(SEARCH, "Could not create base_img path: %s", m_stSearchParam.base_img_path.c_str());
                return AX_FALSE;
            }
        }

        //SEARCH_FEATURE_DB_ST
        AX_U32 data_size = sizeof(SEARCH_FEATURE_HEADER_ST) + feature_size;
        AX_U8 *data_ptr = (AX_U8 *)malloc(data_size);

        if (!data_ptr) {
            LOG_M_E(SEARCH, "Memory alloc (%d) fail", data_size);

            return AX_FALSE;
        }

        memset(data_ptr, 0x00, data_size);
        memcpy(data_ptr, featureInfo.c_str(), (featureInfo.size() > (FEATURE_INFO_SIZE - 1)) ? (FEATURE_INFO_SIZE - 1) : featureInfo.size());
        memcpy(data_ptr + FEATURE_INFO_SIZE, (AX_U8 *)&feature_size, sizeof(AX_U16));
        memcpy(data_ptr + sizeof(SEARCH_FEATURE_HEADER_ST), feature, feature_size);

        //open database
        FILE* pFile = fopen(m_FeatureDataBaseName.c_str(), "a+");

        if (!pFile) {
            LOG_M_E(SEARCH, "Open feature databae: %s fail", m_FeatureDataBaseName.c_str());

            free(data_ptr);

            return AX_FALSE;
        }

        AX_U32 nWriteCount = fwrite(data_ptr, 1, data_size, pFile);

        fclose(pFile);
        free(data_ptr);

        if (nWriteCount == data_size) {
            LOG_M_I(SEARCH, "Save feature to file_name: %s", m_FeatureDataBaseName.c_str());

            return AX_TRUE;
        }
        else {
            LOG_M_E(SEARCH, "Save feature to file_name: %s fail", m_FeatureDataBaseName.c_str());

            return AX_FALSE;
        }
    }

    return AX_FALSE;
}

AX_BOOL CSearch::DeleteFeature(std::string featureInfo)
{
    AX_BOOL bRet = AX_FALSE;

    if (m_FeatureDataBaseName.size() > 0
        && featureInfo.size() > 0) {
        FILE* pFile = NULL;
        AX_U32 db_file_size = 0;

        //open database
        pFile = fopen(m_FeatureDataBaseName.c_str(), "rb");

        if (!pFile) {
            LOG_M_E(SEARCH, "%s not exist", m_FeatureDataBaseName.c_str());
            return AX_FALSE;
        }

        fseek(pFile, 0, SEEK_END);
        db_file_size = ftell(pFile);
        rewind(pFile);

        AX_U8 *db_content = (AX_U8 *)malloc(db_file_size);

        if (!db_content) {
            LOG_M_E(SEARCH, "Memory alloc (%d) fail", db_file_size);

            fclose(pFile);

            return AX_FALSE;
        }

        fread(db_content, 1, db_file_size, pFile);

        fclose(pFile);

        AX_U32 total_len_read = 0;
        AX_U8 *tmp_db_ptr = (AX_U8 *)db_content;
        while(total_len_read < db_file_size) {
            //SEARCH_FEATURE_DB_ST
            SEARCH_FEATURE_HEADER_ST *feature_item_header = (SEARCH_FEATURE_HEADER_ST *)tmp_db_ptr;
            AX_U32 nItemSize = sizeof(SEARCH_FEATURE_HEADER_ST) + feature_item_header->FeatureSize;

            if (featureInfo == (AX_CHAR *)feature_item_header->Info) {
                // delete data
                if (db_file_size > total_len_read + nItemSize) {
                    memcpy(tmp_db_ptr, tmp_db_ptr + nItemSize, db_file_size - (total_len_read + nItemSize));
                }

                //update to file
                pFile = fopen(m_FeatureDataBaseName.c_str(), "wb");

                if (pFile) {
                    if (db_file_size > nItemSize) {
                        fwrite(db_content, 1, db_file_size - nItemSize, pFile);
                    }
                    fclose(pFile);

                    LOG_M(SEARCH, "db updated");

                    bRet = AX_TRUE;
                }
                break;
            }

            total_len_read += nItemSize;
            tmp_db_ptr += nItemSize;
        }

        free(db_content);
    }

    if (!bRet) {
        LOG_M_E(SEARCH, "%s not find", featureInfo.c_str());
    }

    return bRet;
}

AX_S32 CSearch::GetFeatureInfo(const AX_CHAR *str)
{
    if (!gOptions.IsActivedSearchFromWeb()) {
        //not support
        m_GetFeatureInfoStatus = 2;

        return m_GetFeatureInfoStatus;
    }

    if (!m_bFeatureDataBaseLoaded) {

        //Database loading
        m_GetFeatureInfoStatus = 3;

        return m_GetFeatureInfoStatus;
    }

    m_FeatureMutex.lock();
    GenerateFeatureName(str);
    m_FeatureMutex.unlock();

    auto iter = m_mapGroupObjectInfo[1].find(m_FeatureInfoStr);
    if (iter != m_mapGroupObjectInfo[1].end()) {
        LOG_M_E(SEARCH, "%s already exist", m_FeatureInfoStr.c_str());
        return 1;
    }

    m_FeatureMutex.lock();
    m_bGetFeatureInfo = AX_TRUE;
    m_FeatureMutex.unlock();

    m_GetFeatureInfoStatus = 1;

    Waitfor(2000);

    return m_GetFeatureInfoStatus;
}

AX_S32 CSearch::DeleteFeatureInfo(const AX_CHAR *str)
{
    if (!str) {
        return 1;
    }

    m_FeatureMutex.lock();
    GenerateFeatureName(str);
    m_FeatureMutex.unlock();

    if (!DeleteObjectFromGroup(1, m_FeatureInfoStr)) {
        return 1;
    }

    return 0;
}

AX_S32 CSearch::Waitfor(AX_S32 msTimeOut /* = -1 */)
{
    AX_S32 ret = 0;

    pthread_mutex_lock(&m_condMutx);

    // msTimeOut = 0, code exec block
    if (msTimeOut < 0) {
        pthread_cond_wait(&m_cond, &m_condMutx);
    }
    else { // wiat for time = msTimeOut
        struct timespec tv;
        clock_gettime(CLOCK_MONOTONIC, &tv);
        tv.tv_sec += msTimeOut / 1000;
        tv.tv_nsec += (msTimeOut  % 1000) * 1000000;
        if (tv.tv_nsec >= 1000000000) {
            tv.tv_nsec -= 1000000000;
            tv.tv_sec += 1;
        }

        ret = pthread_cond_timedwait(&m_cond, &m_condMutx, &tv);
        if (ret != 0) {
            LOG_M_E(SEARCH, "pthread_cond_timedwait error: %s", strerror(errno));
        }
    }

    pthread_mutex_unlock(&m_condMutx);

    return ret;
}

