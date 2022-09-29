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

#include "Detector.h"
#include "OptionHelper.h"
#include "PrintHelper.h"
#include "WebServer.h"
#include "picojson.h"

#define DETECTION           "DETECTION"

#define DETECTION_VENC_CHN_ENV_STR "MEGVII_VENC_CHN"
#define DETECTION_VENC_CHN_DEFAULT "8"

#define DETECTION_LOAD_MODEL_USE_CMM_ENV_STR "MCV_LOAD_MODEL_USE_CMM"  // 0: USE os malloc, 1: USE CMM
#define DETECTION_LOAD_MODEL_USE_CMM "1"
#define DETECTION_LOAD_MODEL_USE_OS "0"

#define DETECTION_PERF_TEST_ENABLE_ENV_STR "AI_PERF_TEST_ENABLE"
#define DETECTION_PERF_TEST_YUV_PATH_ENV_STR "AI_PERF_TEST_YUV_PATH"

#define DETECTION_DEFAULT_PUSH_MODE MCV_PUSH_MODE_BEST
#define DETECTION_DEFAULT_PUSH_INTERVAL 2000
#define DETECTION_DEFAULT_PUSH_COUNTS 1
#define DETECTION_DEFAULT_PUSH_SAME_FRAME AX_TRUE

#define DETECTION_DEFAULT_ALGO_TYPE "hvcfp_video_algo"
#define DETECTION_DEFAULT_ALGO_CONFIG_PATH "./config/hvcfp_config.json"

#define PICO_OBJECT get<picojson::object>()
#define PICO_OBJECT_SIZE PICO_OBJECT.size()
#define PICO_ARRAY get<picojson::array>()
#define PICO_ARRAY_SIZE PICO_ARRAY.size()
#define PICO_VALUE get<double>()
#define PICO_BOOL get<bool>()
#define PICO_STRING get<std::string>()
#define PICO_ROOT obj.PICO_OBJECT

#define DETECTION_RETRY_LIMIT_TIMES 3
#define DETECTION_RETRY_UNLIMIT_TIMES 5
#define DETECTION_WAITING_TIMEOUT 5000

#define DETECTOR_API_LOAD(_API_NAME_, _API_RET_, _API_PARAM_) \
                            DETECTOR_API(_API_NAME_) = (_API_RET_ (*)_API_PARAM_)dlsym(m_pDetectLib, #_API_NAME_); \
                            if (NULL == DETECTOR_API(_API_NAME_)) { \
                                LOG_M_E(DETECTION, "Load API of %s Failed", #_API_NAME_); \
                                return AX_FALSE; \
                            }

#define DETECTOR_API_RUN_START(_API_NAME_) m_apiElapsed.reset()
#define DETECTOR_API_RUN_END(_API_NAME_) LOG_M_I(DETECTION, "Run API(%s) elapsed: %d(ms)", #_API_NAME_, m_apiElapsed.ms());

// Object Define
#define ObjectDefinition(Obj) \
    int Obj##_size = 0; \
    int Obj##_size_index = 0;

// Object Calculate
#define ObjectCalculate(Obj) \
    do { \
        if (strcasecmp(object_category, #Obj) == 0) { \
            Obj##_size ++; \
            tPerfInfo.tTargets.n##Obj##s ++; \
            if (algorithm_result->object_items[i].has_crop_frame \
                && algorithm_result->object_items[i].crop_frame.frame_data \
                && 0 < algorithm_result->object_items[i].crop_frame.frame_data_size) { \
                    tPerfInfo.tTracks.n##Obj##s ++; \
                } \
        } \
    } while(0)

// Object Create
#define ObjectCreate(Obj) \
    do { \
        if (Obj##_size > 0) { \
            pDetectionResult->n##Obj##Size = Obj##_size; \
            pDetectionResult->p##Obj##s = new AI_Detection_##Obj##Result_t[Obj##_size]; \
        } \
    } while(0)

// Object Destory
#define ObjectDestroy(Obj) \
    do { \
        if (pDetectionResult->p##Obj##s != nullptr) { \
            delete[] pDetectionResult->p##Obj##s; \
            pDetectionResult->p##Obj##s = nullptr; \
        } \
    } while(0)

// Object Assign
#define ObjectAssign(Obj) \
    do { \
        if (strcasecmp(object_category, #Obj) == 0) { \
            if (Obj##_size_index < Obj##_size \
                && pDetectionResult->p##Obj##s) { \
                auto* _obj = pDetectionResult->p##Obj##s + Obj##_size_index; \
                _obj->tBox = tBox; \
                _obj->fConfidence = fConfidence;                              \
                _obj->u64TrackId = u64TrackId;                                \
                Obj##AttrResult(algorithm_result->object_items[i], _obj->t##Obj##Attr, JpegInfo); \
                Obj##_size_index ++; \
            } \
        } \
    } while (0)

// Object Track
#define ObjectTrack(Obj) \
    do { \
        if (strcasecmp(object_category, #Obj) == 0) { \
            DoTracking(algorithm_result->object_items[i]); \
        } \
    } while (0)

extern COptionHelper gOptions;
extern CPrintHelper gPrintHelper;

using namespace std;

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
    } else if (attr_string == "facehuman_video_lite_algo") {
        attr_string = "facehuman_video_all";
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
    MCV_STREAM_HANDLE_PARAM_S &handle_param, DETECTOR_STREAM_PARAM_T stStreamParam,
    std::vector<std::string> &object_category, std::vector<MCV_ITEM_SETTING_S> &item_setting,
    std::vector<MCV_ANALYZE_ATTR_E> &analyze_attribute,
    std::vector<std::string> &quality_filter_object_category,
    std::vector<MCV_PUSH_QUALITY_FILTER_S> &push_quality_filter,
    DETECTOR_CONFIG_PARAM_T &stConfigParam)
{
    std::ifstream ifs(stStreamParam.config_path);

    if (ifs.fail()) {
        LOG_M_E(DETECTION, "%s not exist", stStreamParam.config_path.c_str());
        return AX_FALSE;
    }

    picojson::value obj;
    ifs >> obj;

    string err = picojson::get_last_error();
    if (!err.empty() || !obj.is<picojson::object>()) {
        LOG_M_E(DETECTION, "Failed to load json config file: %s", stStreamParam.config_path.c_str());
        return AX_FALSE;
    }

    if (strstr(stStreamParam.algo_type.c_str(), "facehuman_video") ||
        strstr(stStreamParam.algo_type.c_str(), "facehuman_image")) {
        handle_param.pipeline_type = MCV_PIPELINE_TYPE_FACEHUMAN;
    } else if (strstr(stStreamParam.algo_type.c_str(), "hvcfp_video") ||
               strstr(stStreamParam.algo_type.c_str(), "hvcfp_image")) {
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
    stConfigParam.tRoi.bEnable = (AX_BOOL)stream_config.roi_enabled;
    stConfigParam.tRoi.tBox.fX = (AX_F32)stream_config.roi_rect.left/nWidth;
    stConfigParam.tRoi.tBox.fY = (AX_F32)stream_config.roi_rect.top/nHeight;
    stConfigParam.tRoi.tBox.fW = (AX_F32)(stream_config.roi_rect.right - stream_config.roi_rect.left + 1)/nWidth;
    stConfigParam.tRoi.tBox.fH = (AX_F32)(stream_config.roi_rect.bottom - stream_config.roi_rect.top + 1)/nHeight;
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

    if (PICO_ROOT.end() != PICO_ROOT.find("crop_encoder_qplevel")) {
        stConfigParam.fCropEncoderQpLevel = PICO_ROOT["crop_encoder_qplevel"].PICO_VALUE;
    }

    if (PICO_ROOT.end() != PICO_ROOT.find("max_track_size")) {
        if (PICO_ROOT["max_track_size"].PICO_OBJECT.end() !=
            PICO_ROOT["max_track_size"].PICO_OBJECT.find("human")) {
            stConfigParam.tTrackSize.nTrackHumanSize = PICO_ROOT["max_track_size"].PICO_OBJECT["human"].PICO_VALUE;
        }
        if (PICO_ROOT["max_track_size"].PICO_OBJECT.end() !=
            PICO_ROOT["max_track_size"].PICO_OBJECT.find("vehicle")) {
            stConfigParam.tTrackSize.nTrackVehicleSize = PICO_ROOT["max_track_size"].PICO_OBJECT["vehicle"].PICO_VALUE;
        }
        if (PICO_ROOT["max_track_size"].PICO_OBJECT.end() !=
            PICO_ROOT["max_track_size"].PICO_OBJECT.find("cycle")) {
            stConfigParam.tTrackSize.nTrackCycleSize = PICO_ROOT["max_track_size"].PICO_OBJECT["cycle"].PICO_VALUE;
        }
    }

    if (PICO_ROOT.end() != PICO_ROOT.find("object_filter_size")
        && PICO_ROOT.end() != PICO_ROOT.find("object_filter")) {
        AX_U32 object_filter_size = PICO_ROOT["object_filter_size"].PICO_VALUE;

        for (size_t i = 0; i < object_filter_size; i++) {
            std::string fliter_object_category = PICO_ROOT["object_filter"].PICO_ARRAY[i].PICO_OBJECT["object_category"].PICO_STRING;
            stConfigParam.tObjectFliter[fliter_object_category].nWidth =
                PICO_ROOT["object_filter"].PICO_ARRAY[i].PICO_OBJECT["width"].PICO_VALUE;
            stConfigParam.tObjectFliter[fliter_object_category].nHeight =
                PICO_ROOT["object_filter"].PICO_ARRAY[i].PICO_OBJECT["height"].PICO_VALUE;
            stConfigParam.tObjectFliter[fliter_object_category].fConfidence =
                PICO_ROOT["object_filter"].PICO_ARRAY[i].PICO_OBJECT["confidence"].PICO_VALUE;
        }
    }

    return AX_TRUE;
}

static void BodyAttrResult(const MCV_OBJECT_ITEM_S &object_items, AI_Body_Attr_t &human_attr, JpegDataInfo &JpegInfo)
{
    picojson::value obj;

    JpegInfo.eType = JPEG_TYPE_BODY;

    human_attr.bExist = AX_FALSE;
    human_attr.strSafetyCap = "";
    human_attr.strHairLength = "";

    for (size_t i = 0; i < object_items.meta_info_size; i++) {
        if (!strcmp(object_items.meta_info[i].type, "human_safety_cap")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            human_attr.bExist = AX_TRUE;
            human_attr.strSafetyCap = PICO_ROOT["safety_cap"].PICO_OBJECT["name"].PICO_STRING;
        } else if (!strcmp(object_items.meta_info[i].type, "human_attr")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            human_attr.bExist = AX_TRUE;
            human_attr.strHairLength = PICO_ROOT["hair_length"].PICO_OBJECT["name"].PICO_STRING;
        }
    }
}

static void VehicleAttrResult(const MCV_OBJECT_ITEM_S &object_items, AI_Vehicle_Attr_t &vehicle_attr, JpegDataInfo &JpegInfo) {
    picojson::value obj;

    JpegInfo.eType = JPEG_TYPE_VEHICLE;

    vehicle_attr.bExist = AX_FALSE;
    vehicle_attr.strVehicleColor = "";
    vehicle_attr.strVehicleSubclass = "";

    for (size_t i = 0; i < object_items.meta_info_size; i++) {
        if (!strcmp(object_items.meta_info[i].type, "vehicle_cls")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            vehicle_attr.bExist = AX_TRUE;
            vehicle_attr.strVehicleSubclass = PICO_ROOT["classifications"].PICO_OBJECT["name"].PICO_STRING;
        } else if (!strcmp(object_items.meta_info[i].type, "vehicle_attr")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            vehicle_attr.bExist = AX_TRUE;
            vehicle_attr.strVehicleColor = PICO_ROOT["color"].PICO_OBJECT["name"].PICO_STRING;
        }
    }
}

static void CycleAttrResult(const MCV_OBJECT_ITEM_S &object_items, AI_Cycle_Attr_t &cycle_attr, JpegDataInfo &JpegInfo) {
    picojson::value obj;

    JpegInfo.eType = JPEG_TYPE_CYCLE;

    cycle_attr.bExist = AX_FALSE;
    cycle_attr.strCycleSubclass = "";

    for (size_t i = 0; i < object_items.meta_info_size; i++) {
        if (!strcmp(object_items.meta_info[i].type, "cycle_attr")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            cycle_attr.bExist = AX_TRUE;
            cycle_attr.strCycleSubclass = PICO_ROOT["classifications"].PICO_OBJECT["name"].PICO_STRING;
        }
    }
}

static void FaceAttrResult(const MCV_OBJECT_ITEM_S &object_items, AI_Face_Attr_t &face_attr, JpegDataInfo &JpegInfo)
{
    picojson::value obj;

    JpegInfo.eType = JPEG_TYPE_FACE;

    face_attr.bExist = AX_FALSE;
    face_attr.nAge = 0;
    face_attr.nGender = 0;
    face_attr.strRespirator = "";

    for (size_t i = 0; i < object_items.meta_info_size; i++) {
        if (!strcmp(object_items.meta_info[i].type, "face_attr")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            face_attr.bExist = AX_TRUE;
            // age
            face_attr.nAge = PICO_ROOT["age"].PICO_VALUE;

            // gender
            if (PICO_ROOT["gender"].PICO_OBJECT["name"].PICO_STRING == "male") {
                face_attr.nGender = 1;
            }
            else {
                face_attr.nGender = 0;
            }

            // respirator
            face_attr.strRespirator = PICO_ROOT["respirator"].PICO_OBJECT["name"].PICO_STRING;

            JpegInfo.tFaceInfo.nAge = face_attr.nAge;
            JpegInfo.tFaceInfo.nGender = face_attr.nGender;
            strncpy(JpegInfo.tFaceInfo.szMask, (const AX_CHAR *)face_attr.strRespirator.c_str(), sizeof(JpegInfo.tFaceInfo.szMask) - 1);

            LOG_M(DETECTION, "Face: age:%d, gender:%s, respirator:%s",
                    face_attr.nAge, face_attr.nGender ? "male":"female",
                    face_attr.strRespirator.c_str());
        }
    }
}

static void PlateAttrResult(const MCV_OBJECT_ITEM_S &object_items, AI_Plat_Attr_t &plat_attr, JpegDataInfo &JpegInfo) {
    DETECTOR_CONFIG_PARAM_T tConf = CDetector::GetInstance()->GetConfig();

    picojson::value obj;

    JpegInfo.eType = JPEG_TYPE_PLATE;

    plat_attr.bExist = AX_FALSE;
    plat_attr.bValid = AX_FALSE;
    plat_attr.strPlateColor = "";
    plat_attr.strPlateType = "";
    plat_attr.strPlateCode = "";

    for (size_t i = 0; i < object_items.meta_info_size; i++) {
        if (!strcmp(object_items.meta_info[i].type, "plate_attr")) {
            std::string value = object_items.meta_info[i].value;
            std::string strParseRet = picojson::parse(obj, value);
            if (!strParseRet.empty() || !obj.is<picojson::object>()) {
                break;
            }

            plat_attr.bExist = AX_TRUE;
            // color
            plat_attr.strPlateColor = "unknown";
            if (PICO_ROOT.end() != PICO_ROOT.find("color")) {
                plat_attr.strPlateColor = PICO_ROOT["color"].PICO_OBJECT["name"].PICO_STRING;
            }

            // style
            plat_attr.strPlateType = "unknown";
            if (PICO_ROOT.end() != PICO_ROOT.find("style")) {
                plat_attr.strPlateType = PICO_ROOT["style"].PICO_OBJECT["name"].PICO_STRING;
            }

            // code
            plat_attr.strPlateCode = PICO_ROOT["code_result"].PICO_STRING;

            if (PICO_ROOT["code_killed"].PICO_BOOL) {
                plat_attr.bValid = AX_FALSE;
            } else {
                plat_attr.bValid = AX_TRUE;
            }

            if (tConf.bPlateIdentify) {
                strncpy(JpegInfo.tPlateInfo.szColor, (const AX_CHAR *)plat_attr.strPlateColor.c_str(), sizeof(JpegInfo.tPlateInfo.szColor) - 1);

                if (plat_attr.bValid) {
                    strncpy(JpegInfo.tPlateInfo.szNum, (const AX_CHAR *)plat_attr.strPlateCode.c_str(), sizeof(JpegInfo.tPlateInfo.szNum) - 1);
                }
                else {
                    strncpy(JpegInfo.tPlateInfo.szNum, "unknown", sizeof(JpegInfo.tPlateInfo.szNum) - 1);
                }
            }

            LOG_M(DETECTION, "Plate: color:%s, style:%s, code:%s, valid:%d",
                    plat_attr.strPlateColor.c_str(), plat_attr.strPlateType.c_str(),
                    plat_attr.strPlateCode.c_str(), plat_attr.bValid);
        }
    }
}

static AX_VOID AsyncRecvAlgorithmResultThread(AX_VOID *__this)
{
    CDetector *pThis = (CDetector *)__this;

    while (pThis->m_bGetResultThreadRunning) {
        if (!pThis->AsyncRecvDetectionResult()) {
            CTimeUtils::msSleep(1);
        }
    }
}

//////////////////////////////////////////////////////////////////
CDetector::CDetector(AX_VOID)
{
    m_bFinished = AX_TRUE;
    m_bForedExit = AX_FALSE;
    m_nFrame_id = 1;
    m_bGetResultThreadRunning = AX_FALSE;
    m_pGetResultThread = nullptr;

    //Search
    m_pObjectSearch = nullptr;

    //AI Perf Test
    memset(&m_perfTestInfo, 0x00, sizeof(PERF_TEST_INFO_T));

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();

    m_perfTestInfo.nWidth = pStageOption->GetAiAttr().tConfig.nWidth;
    m_perfTestInfo.nHeight = pStageOption->GetAiAttr().tConfig.nHeight;
    m_perfTestInfo.nSize = (m_perfTestInfo.nWidth * m_perfTestInfo.nHeight * 3 / 2);

    InitConfigParam();
}

CDetector::~CDetector(AX_VOID)
{

}

AX_VOID CDetector::BindCropStage(CTrackCropStage* pStage)
{
    m_pTrackCropStage = pStage;
}

AX_BOOL CDetector::InitConfigParam(AX_VOID)
{
    //Running config
    m_tConfigParam.tPushStrategy.nPushMode = (AX_U8)DETECTION_DEFAULT_PUSH_MODE;
    m_tConfigParam.tPushStrategy.nInterval = DETECTION_DEFAULT_PUSH_INTERVAL;
    m_tConfigParam.tPushStrategy.nPushCounts = DETECTION_DEFAULT_PUSH_COUNTS;
    m_tConfigParam.tPushStrategy.bPushSameFrame = DETECTION_DEFAULT_PUSH_SAME_FRAME;

    m_tConfigParam.tStreamParam.algo_type = DETECTION_DEFAULT_ALGO_TYPE;
    m_tConfigParam.tStreamParam.config_path = DETECTION_DEFAULT_ALGO_CONFIG_PATH;

    //Track
    m_tConfigParam.nTrackType = 0;
    if (AI_TRACK_BODY_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nTrackType, AI_TRACK_TYPE_BODY);
    }
    if (AI_TRACK_VEHICLE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nTrackType, AI_TRACK_TYPE_VEHICLE);
    }
    if (AI_TRACK_CYCLE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nTrackType, AI_TRACK_TYPE_CYCLE);
    }
    if (AI_TRACK_FACE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nTrackType, AI_TRACK_TYPE_FACE);
    }
    if (AI_TRACK_PLATE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nTrackType, AI_TRACK_TYPE_PLATE);
    }

    //Draw rect
    m_tConfigParam.nDrawRectType = 0;
    if (AI_DRAW_RECT_BODY_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_BODY);
    }
    if (AI_DRAW_RECT_VEHICLE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_VEHICLE);
    }
    if (AI_DRAW_RECT_VEHICLE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_CYCLE);
    }
    if (AI_DRAW_RECT_CYCLE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_FACE);
    }
    if (AI_DRAW_RECT_PLATE_ENABLE) {
        AX_BIT_SET(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_PLATE);
    }

    m_tConfigParam.bPlateIdentify = AX_TRUE;

    // body fliter
    m_tConfigParam.tObjectFliter["body"].nWidth = AI_BODY_FLITER_DEFAULT_WIDTH;
    m_tConfigParam.tObjectFliter["body"].nHeight = AI_BODY_FLITER_DEFAULT_HEIGHT;
    m_tConfigParam.tObjectFliter["body"].fConfidence = AI_BODY_FLITER_DEFAULT_CONFIDENCE;

    // vehicle fliter
    m_tConfigParam.tObjectFliter["vehicle"].nWidth = AI_VEHICLE_FLITER_DEFAULT_WIDTH;
    m_tConfigParam.tObjectFliter["vehicle"].nHeight = AI_VEHICLE_FLITER_DEFAULT_HEIGHT;
    m_tConfigParam.tObjectFliter["vehicle"].fConfidence = AI_VEHICLE_FLITER_DEFAULT_CONFIDENCE;

    // cycle fliter
    m_tConfigParam.tObjectFliter["cycle"].nWidth = AI_CYCLE_FLITER_DEFAULT_WIDTH;
    m_tConfigParam.tObjectFliter["cycle"].nHeight = AI_CYCLE_FLITER_DEFAULT_HEIGHT;
    m_tConfigParam.tObjectFliter["cycle"].fConfidence = AI_CYCLE_FLITER_DEFAULT_CONFIDENCE;

    // face fliter
    m_tConfigParam.tObjectFliter["face"].nWidth = AI_FACE_FLITER_DEFAULT_WIDTH;
    m_tConfigParam.tObjectFliter["face"].nHeight = AI_FACE_FLITER_DEFAULT_HEIGHT;
    m_tConfigParam.tObjectFliter["face"].fConfidence = AI_FACE_FLITER_DEFAULT_CONFIDENCE;

    // plate fliter
    m_tConfigParam.tObjectFliter["plate"].nWidth = AI_PLATE_FLITER_DEFAULT_WIDTH;
    m_tConfigParam.tObjectFliter["plate"].nHeight = AI_PLATE_FLITER_DEFAULT_HEIGHT;
    m_tConfigParam.tObjectFliter["plate"].fConfidence = AI_PLATE_FLITER_DEFAULT_CONFIDENCE;

    return AX_TRUE;
}

AX_BOOL CDetector::Startup(AX_VOID)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    LOG_M_W(DETECTION, "+++");

    if (!gOptions.IsActivedDetect()) {
        return AX_TRUE;
    }

    //Load Api
    if (!LoadApi()) {
        LOG_M_E(DETECTION, "Load Api failed");
        return AX_FALSE;
    }

    //get chn env
    if (!getenv(DETECTION_VENC_CHN_ENV_STR)) {
        setenv(DETECTION_VENC_CHN_ENV_STR, DETECTION_VENC_CHN_DEFAULT, 1);
    }

    //get load cmm env
    if (!getenv(DETECTION_LOAD_MODEL_USE_CMM_ENV_STR)) {
        setenv(DETECTION_LOAD_MODEL_USE_CMM_ENV_STR, DETECTION_LOAD_MODEL_USE_CMM, 1);
    }

    std::string strModPath = gOptions.GetDetectionModelPath();
    LOG_M_W(DETECTION, "Load pro path: %s", strModPath.c_str());

    MCV_INIT_PARAM_S param{0};
    param.model_deployment_path = (char *)strModPath.c_str();
    DETECTOR_API_RUN_START(mcv_init_sdk);
    ret = DETECTOR_API(mcv_init_sdk)(&param);
    DETECTOR_API_RUN_END(mcv_init_sdk);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_init_sdk error");
        return AX_FALSE;
    }

    // get sdk version
    const MCV_VERSION_INFO_S *mcv_version = NULL;
    ret = DETECTOR_API(mcv_get_version)(&mcv_version);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_get_version error");
        return AX_FALSE;
    }

    LOG_M_W(DETECTION, "sdk version: %s", mcv_version->version);
    LOG_M_W(DETECTION, "frame version: %s", mcv_version->frame_version);

    DETECTOR_API(mcv_delete)((void *)mcv_version);

    // set log level
    MCV_LOG_PARAM_S log_param;
    memset(&log_param, 0, sizeof(log_param));
    log_param.log_level = (MCV_LOG_LEVEL_E)3;//MCV_LOG_LEVEL_ERROR; //TODO
    log_param.log_path = (char *)"./output";
    ret = DETECTOR_API(mcv_set_log_config)(&log_param);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_log_config error");
        return AX_FALSE;
    }

    //Stream Param
    m_tConfigParam.tStreamParam.algo_type = gOptions.GetDetectionAlgoType();
    m_tConfigParam.tStreamParam.config_path = gOptions.GetDetectionConfigPath();

    LOG_M(DETECTION, "algo config: %s", m_tConfigParam.tStreamParam.algo_type.c_str());

    transform_algotype_string_to_streamtype(m_tConfigParam.tStreamParam.algo_type);

    if (!InitStreamHandle()) {
        LOG_M_E(DETECTION, "InitStreamHandle error");
        return AX_FALSE;
    }

    //Search
    if (gOptions.IsActivedSearch()) {
        //Search
        m_pObjectSearch = CSearch::GetInstance();

        if (!m_pObjectSearch->Startup()) {
            gOptions.SetSearchActived(AX_FALSE);
            LOG_M_W(DETECTION, "Search init failed");
        }
    }

    //update AI Attribute
    UpdateAiAttr();

    m_bFinished = AX_TRUE;
    m_bForedExit = AX_FALSE;

    //Load perf test info
    LoadPerfTestInfo();

    LOG_M_W(DETECTION, "---");

    return AX_TRUE;
}

AX_VOID CDetector::Cleanup(AX_VOID)
{
    LOG_M_W(DETECTION, "+++");

    m_bForedExit = AX_TRUE;

    WaitForFinish(AX_TRUE);

    if (m_bGetResultThreadRunning) {
        m_bGetResultThreadRunning = AX_FALSE;

        if (m_pGetResultThread && m_pGetResultThread->joinable()) {
            m_pGetResultThread->join();

            delete m_pGetResultThread;
            m_pGetResultThread = nullptr;
        }
    }

    if (m_pObjectSearch) {
        m_pObjectSearch->Cleanup();
    }

    if (m_bApiLoaded) {
        if (m_stream_handle) {
            DETECTOR_API_RUN_START(mcv_release_stream_handle);
            DETECTOR_API(mcv_release_stream_handle)(m_stream_handle);
            DETECTOR_API_RUN_END(mcv_release_stream_handle);
        }

        ClearAlgorithmData();

        DETECTOR_API_RUN_START(mcv_deinit_sdk);
        DETECTOR_API(mcv_deinit_sdk)();
        DETECTOR_API_RUN_END(mcv_deinit_sdk);
    }

    if (m_pDetectLib) {
        dlclose(m_pDetectLib);
        m_pDetectLib = nullptr;
    }

    m_bApiLoaded = AX_FALSE;

    //AI Perf Test
    if (m_perfTestInfo.nPhyAddr != 0) {
        AX_SYS_MemFree(m_perfTestInfo.nPhyAddr, &m_perfTestInfo.pVirAddr);
    }

    LOG_M_W(DETECTION, "---");
}

AX_BOOL CDetector::LoadPerfTestInfo(AX_VOID)
{
    //get perf test env
    const char *perfTestEnableStr = getenv(DETECTION_PERF_TEST_ENABLE_ENV_STR);
    if (perfTestEnableStr) {
        AX_BOOL bAiPerfTest = (AX_BOOL)atoi(perfTestEnableStr);

        if (bAiPerfTest) {
            const char *perfTestYUVPathStr = getenv(DETECTION_PERF_TEST_YUV_PATH_ENV_STR);

            if (perfTestYUVPathStr) {
                FILE *fp = fopen(perfTestYUVPathStr, "rb");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    AX_U32 nFileSize = ftell(fp);
                    fseek(fp, 0, SEEK_SET);

                    if (nFileSize == m_perfTestInfo.nSize) {
                        AX_S32 ret = AX_SYS_MemAlloc(&m_perfTestInfo.nPhyAddr, &m_perfTestInfo.pVirAddr, nFileSize, 256, NULL);

                        if (AX_SDK_PASS == ret) {
                            AX_U64 *pData = (AX_U64 *)m_perfTestInfo.pVirAddr;
                            fread(pData, nFileSize, 1, fp);
                            m_perfTestInfo.m_bAiPerfTest = AX_TRUE;

                            LOG_M_E(DETECTION, "Load YUV file(%s) success.", perfTestYUVPathStr);
                        }
                        else {
                            LOG_M_E(DETECTION, "AX_SYS_MemAlloc() failed, size:0x%x, error: 0x%x", nFileSize, ret);
                        }
                    }
                    else {
                        LOG_M_E(DETECTION, "YUV file(%s) should be %d*%d.", perfTestYUVPathStr, m_perfTestInfo.nWidth, m_perfTestInfo.nHeight);
                    }

                    fclose(fp);
                }
                else {
                    LOG_M_E(DETECTION, "open YUV file(%s) fail.", perfTestYUVPathStr);
                }
            }
        }
    }

    return AX_TRUE;
}

AX_BOOL CDetector::LoadApi(AX_VOID)
{
    if (!m_bApiLoaded) {
        m_pDetectLib = dlopen("libmcvsdk_video.so", RTLD_LAZY);

        if (!m_pDetectLib) {
            LOG_M_E(DETECTION, "Load detect lib failed");
            return AX_FALSE;
        }

        DETECTOR_API_LOAD(mcv_init_sdk, MCV_RET_CODE_E, (const MCV_INIT_PARAM_S *param));
        DETECTOR_API_LOAD(mcv_deinit_sdk, MCV_RET_CODE_E, (void));
        DETECTOR_API_LOAD(mcv_get_version, MCV_RET_CODE_E, (const MCV_VERSION_INFO_S **version));
        DETECTOR_API_LOAD(mcv_set_log_config, MCV_RET_CODE_E, (const MCV_LOG_PARAM_S *log_param));
        DETECTOR_API_LOAD(mcv_get_capability, MCV_RET_CODE_E, (const MCV_CAPABILITY_S **capability));
        DETECTOR_API_LOAD(mcv_query_model_package, MCV_RET_CODE_E, (const MCV_MODEL_PACKAGE_LIST_S **model_package_list));
        DETECTOR_API_LOAD(mcv_create_stream_handle, MCV_RET_CODE_E, (const MCV_STREAM_HANDLE_PARAM_S *handle_param,
                                                                        MCV_STREAM_HANDLE_T *handle));
        DETECTOR_API_LOAD(mcv_release_stream_handle, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
        DETECTOR_API_LOAD(mcv_set_stream_config, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                     const MCV_STREAM_CONFIG_S *config));
        DETECTOR_API_LOAD(mcv_reset, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle));
        DETECTOR_API_LOAD(mcv_send_stream_frame, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, MCV_FRAME_S *frame));
        DETECTOR_API_LOAD(mcv_get_detect_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                 MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                                 int timedwait_millisecond));
        DETECTOR_API_LOAD(mcv_get_algorithm_result, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                                MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                                int timedwait_millisecond));
        DETECTOR_API_LOAD(mcv_register_detect_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle,
                                                               ALGORITHM_RESULT_CALLBACK_FUNC callback,
                                                               void *user_data));
        DETECTOR_API_LOAD(mcv_register_algorithm_result_callback, MCV_RET_CODE_E, (MCV_STREAM_HANDLE_T handle, ALGORITHM_RESULT_CALLBACK_FUNC callback, void *user_data));
        DETECTOR_API_LOAD(mcv_delete, MCV_RET_CODE_E, (void *p));

        m_bApiLoaded = AX_TRUE;
    }

    LOG_M_W(DETECTION, "Load detect API Success");

    return m_bApiLoaded;
}

AX_BOOL CDetector::ProcessDetect(CMediaFrame* pFrame, AX_S32 nGaps /* = -1 */)
{
    if (m_bForedExit) {
        return AX_FALSE;
    }

    AX_BOOL bRet = AX_FALSE;
    AX_U64 nFrame_id = m_nFrame_id ++;

    if (!gOptions.IsActivedDetect() || !gOptions.IsActivedDetectFromWeb()) {
         return bRet;
    }

    if (!m_bApiLoaded) {
        LOG_M_E(DETECTION, "Detect api not loaded");
        return bRet;
    }

    if (!m_stream_handle) {
        LOG_M_E(DETECTION, "Stream handle not init");
        return bRet;
    }
    LOG_M_I(DETECTION, "+++");

    if (!m_bForedExit) {
        AX_VIDEO_FRAME_INFO_S tFrame = {0};
        if (pFrame->bIvpsFrame) {
            tFrame.stVFrame = pFrame->tVideoFrame;
        } else {
            tFrame = pFrame->tFrame.tFrameInfo;
        }

        if (nFrame_id == 0) {
            LOG_M_E(DETECTION, "Skip FrameId(0)");
            return bRet;
        }

        //Get feature info
        if (gOptions.IsActivedSearch() && m_pObjectSearch) {
            if (m_pObjectSearch->ProcessFeature(pFrame)) {
                return AX_TRUE;
            }
        }

        m_bFinished = AX_FALSE;

        MCV_FRAME_S frame;
        memset(&frame, 0, sizeof(MCV_FRAME_S));
        frame.image_type = MCV_IMAGE_TYPE_NV12;
        frame.image_data.addr_type = MCV_IMAGE_SOURCE_ADDR;
        frame.header.extra_type = MCV_FRAME_EXTRA_TYPE_VIDEO;
        frame.header.frame_id = (uint64_t)nFrame_id;

        if (!m_perfTestInfo.m_bAiPerfTest) {
            frame.image_data.addr.phy_addr[0] = (uint64_t)tFrame.stVFrame.u64PhyAddr[0];
            frame.image_data.addr.phy_addr[1] = (uint64_t)tFrame.stVFrame.u64PhyAddr[1];
            frame.image_data.addr.phy_addr[2] = (uint64_t)tFrame.stVFrame.u64PhyAddr[2];
            frame.image_data.addr.vir_addr[0] = (void *)tFrame.stVFrame.u64VirAddr[0];
            frame.image_data.addr.vir_addr[1] = (void *)tFrame.stVFrame.u64VirAddr[1];
            frame.image_data.addr.vir_addr[2] = (void *)tFrame.stVFrame.u64VirAddr[2];

            frame.header.width = tFrame.stVFrame.u32Width;
            frame.header.height = tFrame.stVFrame.u32Height;
            frame.image_data.addr.stride[0] = tFrame.stVFrame.u32PicStride[0];
            frame.image_data.addr.stride[1] = tFrame.stVFrame.u32PicStride[1];
            frame.image_data.addr.stride[2] = tFrame.stVFrame.u32PicStride[2];
        }
        else {
            frame.image_data.addr.phy_addr[0] = (uint64_t)m_perfTestInfo.nPhyAddr;
            frame.image_data.addr.phy_addr[1] = (uint64_t)m_perfTestInfo.nPhyAddr;
            frame.image_data.addr.phy_addr[2] = (uint64_t)m_perfTestInfo.nPhyAddr;
            frame.image_data.addr.vir_addr[0] = (void *)m_perfTestInfo.pVirAddr;
            frame.image_data.addr.vir_addr[1] = (void *)m_perfTestInfo.pVirAddr;
            frame.image_data.addr.vir_addr[2] = (void *)m_perfTestInfo.pVirAddr;

            frame.header.width = m_perfTestInfo.nWidth;
            frame.header.height = m_perfTestInfo.nHeight;
            frame.image_data.addr.stride[0] = m_perfTestInfo.nWidth;
            frame.image_data.addr.stride[1] = m_perfTestInfo.nWidth;
            frame.image_data.addr.stride[2] = m_perfTestInfo.nWidth;
        }

        //if buffer full, retry
        AX_U8 nRetry = 0;
        AX_U8 nRetryTimes = (gOptions.IsActivedOcclusionDetect() && gOptions.IsActivedMotionDetect())
                                ? DETECTION_RETRY_LIMIT_TIMES : DETECTION_RETRY_UNLIMIT_TIMES;
        MCV_RET_CODE_E ret = MCV_RET_FRAME_BUFFER_FULL;

        if (nGaps == 0) {
            m_mutex.lock();
            AX_U64 nFrameRecv = m_tRunningStatus.nFrameRecv;
            AX_U64 nFrameSent = m_tRunningStatus.nFrameSent;
            m_mutex.unlock();

            if (nFrameSent <= nFrameRecv + AX_AI_DETECT_FRAME_DEPTH) {
                ret = DETECTOR_API(mcv_send_stream_frame)(m_stream_handle, &frame);
            }
        }
        else {
            auto startTime = std::chrono::steady_clock::now();

            do {
                m_mutex.lock();
                AX_U64 nFrameRecv = m_tRunningStatus.nFrameRecv;
                AX_U64 nFrameSent = m_tRunningStatus.nFrameSent;
                m_mutex.unlock();

                if (nFrameSent <= nFrameRecv + AX_AI_DETECT_FRAME_DEPTH) {
                    ret = DETECTOR_API(mcv_send_stream_frame)(m_stream_handle, &frame);
                    break;
                }

                auto endTime = std::chrono::steady_clock::now();

                AX_S32 nElapsed = (AX_S32)(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count());

                if ((nGaps > 0) && (nElapsed >= nGaps)) {
                    LOG_M_I(DETECTION, "Timeout skip: retry=%d, gaps=%d", nRetry, nGaps);
                    break;
                }

                ++nRetry;

                CTimeUtils::msSleep(5);
            } while (((nGaps < 0) && (nRetry < nRetryTimes)) || (nGaps > 0));
        }

#if 0
        do {
            ret = DETECTOR_API(mcv_send_stream_frame)(m_stream_handle, &frame);

            if (MCV_RET_SUCCESS == ret) {
                break;
            }
            else {
                //if frame buffer full, retry
                if (MCV_RET_FRAME_BUFFER_FULL == ret) {
                    CTimeUtils::msSleep(5);
                }
                else {
                    break;
                }
            }
        } while ((++nRetry) < nRetryTimes);
#endif

        m_bFinished = AX_TRUE;

        if (MCV_RET_SUCCESS == ret) {
            // here,save track result in m_mapDataResult
            FRAME_ALGORITHM_RESULT_ST stAlgorithmResult;
            stAlgorithmResult.m_sVideoFrame = tFrame.stVFrame;
            stAlgorithmResult.m_tpStart = std::chrono::steady_clock::now();
            m_mutex.lock();
            m_mapDataResult[nFrame_id] = stAlgorithmResult;

            if (tFrame.stVFrame.u32BlkId[0] > 0) {
                AX_POOL_IncreaseRefCnt(tFrame.stVFrame.u32BlkId[0], AX_ID_USER);
            }
            if (tFrame.stVFrame.u32BlkId[1] > 0) {
                AX_POOL_IncreaseRefCnt(tFrame.stVFrame.u32BlkId[1], AX_ID_USER);
            }

            m_tRunningStatus.nFrameSent ++;

            m_mutex.unlock();

            bRet = AX_TRUE;
        }
        else {
            if (MCV_RET_FRAME_BUFFER_FULL == ret) {
                LOG_M_I(DETECTION, "mcv_send_stream_frame(FrameId:%lld): frame buffer is full", nFrame_id);
            }
            else {
                LOG_M_E(DETECTION, "mcv_send_stream_frame(FrameId:%lld): error code(%d)", nFrame_id, ret);
            }

            bRet = AX_FALSE;
        }
    }

    LOG_M_I(DETECTION, "---");

    return bRet;
}

AX_BOOL CDetector::InitStreamHandle(AX_VOID)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    // model query
    const MCV_MODEL_PACKAGE_LIST_S *model_package_list = nullptr;
    ret = DETECTOR_API(mcv_query_model_package)(&model_package_list);
    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_query_model_package error");
        return AX_FALSE;
    }
    for (size_t i = 0; i < model_package_list->model_package_info_size; i++) {
        for (size_t j = 0; j < model_package_list->model_package_info[i].meta_info_size; j++) {
            LOG_M_I(DETECTION, "meta_info value: %s", model_package_list->model_package_info[i].meta_info[j].value);
        }
    }

    DETECTOR_API(mcv_delete)((void *)model_package_list);

    //capability query
    const MCV_CAPABILITY_S *capability = nullptr;
    ret = DETECTOR_API(mcv_get_capability)(&capability);
    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_get_capability error");
        return AX_FALSE;
    }

    LOG_M(DETECTION, "Use algo: %s", m_tConfigParam.tStreamParam.algo_type.c_str());

    AX_BOOL find_algo_type = AX_FALSE;
    for (size_t i = 0; i < capability->pipline_config_size; i++) {
        LOG_M_I(DETECTION, "pipline_config_key: %s", capability->pipline_config[i].pipline_config_key);
        auto type = split(capability->pipline_config[i].pipline_config_key, ":");
        if (type[2] == m_tConfigParam.tStreamParam.algo_type) {
            m_tConfigParam.tStreamParam.algo_type = capability->pipline_config[i].pipline_config_key;
            find_algo_type = AX_TRUE;
        }
    }

    LOG_M(DETECTION, "Use pipeline: %s", m_tConfigParam.tStreamParam.algo_type.c_str());

    DETECTOR_API(mcv_delete)((void *)capability);

    if (!find_algo_type) {
        LOG_M_E(DETECTION, "not find algo_type for %s", m_tConfigParam.tStreamParam.algo_type.c_str());
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
    handle_param.pipline_config_key = (char *)m_tConfigParam.tStreamParam.algo_type.c_str();
    if (m_tConfigParam.tStreamParam.config_path.size() > 0) {
        if (strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "facehuman_video") ||
            strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "facehuman_image") ||
            strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "hvcfp_video") ||
            strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "hvcfp_image")) {
            bRet = get_facehuamn_hvcfp_handle_param(
                handle_param, m_tConfigParam.tStreamParam, object_category, item_setting, analyze_attribute,
                quality_filter_object_category, push_quality_filter, m_tConfigParam);

            m_tConfigParam.tPushStrategy.nPushMode = (AX_U8)handle_param.facehuman_stream_config.push_strategy.push_mode;
            m_tConfigParam.tPushStrategy.nInterval = (AX_U32)handle_param.facehuman_stream_config.push_strategy.interval_times;
            m_tConfigParam.tPushStrategy.nPushCounts = (AX_U32)handle_param.facehuman_stream_config.push_strategy.push_counts;
            m_tConfigParam.tPushStrategy.bPushSameFrame = (AX_BOOL)handle_param.facehuman_stream_config.push_strategy.push_same_frame;
        } else {
            LOG_M_E(DETECTION, "algo_type:%s not support", m_tConfigParam.tStreamParam.algo_type.c_str());
            return AX_FALSE;
        }
    }
    else {
        LOG_M_E(DETECTION, "config_path invalid");
        return AX_FALSE;
    }

    if (!bRet) {
        LOG_M_E(DETECTION, "get handle param error");
        return AX_FALSE;
    }

    handle_param.skip_frame_by_user = true;
    handle_param.buffer_depth = (AX_AI_DETECT_FRAME_DEPTH ? AX_AI_DETECT_FRAME_DEPTH : 1);
    if (m_tConfigParam.tStreamParam.algo_type.find("image") != std::string::npos) {
        handle_param.stream_type = MCV_STREAM_TYPE_IMAGE;
    } else {
        handle_param.stream_type = MCV_STREAM_TYPE_VIDEO;
    }
    handle_param.smooth_result_enabled = false;

    DETECTOR_API_RUN_START(mcv_create_stream_handle);
    ret = DETECTOR_API(mcv_create_stream_handle)(&handle_param, &m_stream_handle);
    DETECTOR_API_RUN_END(mcv_create_stream_handle);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "Init handle error");
        return AX_FALSE;
    } else {
        LOG_M_I(DETECTION, "Init handle success");
    }

    if (strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "facehuman_video") ||
        strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "facehuman_image") ||
        strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "hvcfp_video") ||
        strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "hvcfp_image")) {
        bRet = SetStreamConfig(m_stream_handle, &m_tConfigParam);
    }

    if (!bRet) {
        LOG_M_E(DETECTION, "SetStreamConfig error");
        return AX_FALSE;
    }

    m_bGetResultThreadRunning = AX_TRUE;
    m_pGetResultThread = new thread(AsyncRecvAlgorithmResultThread, this);

    return AX_TRUE;
}

AX_BOOL CDetector::SetStreamConfig(MCV_STREAM_HANDLE_T ptStreamHandle, DETECTOR_CONFIG_PARAM_T *ptConfigParam)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;

    if (ptStreamHandle == nullptr) {
        LOG_M_E(DETECTION, "stream handle null..");

        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    //set frame cache depth
    MCV_COMMON_THRESHOLD_CONFIG_S frame_cache_depth_threshold;
    frame_cache_depth_threshold.value = (float)(AX_AI_DETECT_CACHE_LIST_DEPTH ? AX_AI_DETECT_CACHE_LIST_DEPTH : 1);
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"frame_cache_depth";
    tStreamItem.value = (void *)&frame_cache_depth_threshold;
    vStreamContent.emplace_back(tStreamItem);

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();

    //set jenc attribute
    MCV_VENC_ATTR_CONFIG jencAttr;
    jencAttr.w = pStageOption->GetAiAttr().tConfig.nWidth;
    jencAttr.h = pStageOption->GetAiAttr().tConfig.nHeight;
    jencAttr.buf_size  = (jencAttr.w * jencAttr.h)*3/8;
    tStreamItem.type = (char *)"venc_attr_config";
    tStreamItem.value = (void *)&jencAttr;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    if (tStreamParam.items == nullptr) {
        LOG_M_E(DETECTION, "param item nullpointer...");
        return AX_FALSE;
    }

    ret = DETECTOR_API(mcv_set_stream_config)(ptStreamHandle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_stream_config error: %d", ret);
        return AX_FALSE;
    }

    // update crop qp level
    SetCropEncoderQpLevel(ptConfigParam->fCropEncoderQpLevel);

    // update track size
    SetTrackSize(&ptConfigParam->tTrackSize);

    // update object fliter
    for (auto iter = ptConfigParam->tObjectFliter.begin(); iter != ptConfigParam->tObjectFliter.end();) {
        SetObjectFilter(iter->first, &iter->second);

        ++iter;
    }

    return AX_TRUE;
}

AX_BOOL CDetector::AsyncRecvDetectionResult(AX_VOID)
{
    MCV_RET_CODE_E ret = MCV_RET_SUCCESS;
    MCV_ALGORITHM_RESULT_S *palgorithm_result = nullptr;

    ret = DETECTOR_API(mcv_get_algorithm_result)(m_stream_handle, &palgorithm_result, 0);

    if (MCV_RET_SUCCESS != ret) {
        if (MCV_RET_EMPTY_RESULT != ret) {
            LOG_M_E(DETECTION, "mcv_get_algorithm_result error: %d", ret);
        }

        return AX_FALSE;
    }

    return DetectionResultHandler(palgorithm_result);
}

AX_BOOL CDetector::DetectionResultHandler(MCV_ALGORITHM_RESULT_S *algorithm_result)
{
    if (!algorithm_result) {
        return AX_FALSE;
    }

    AX_U32 frame_id = (AX_U32)algorithm_result->frame_id;
    AX_U32 nWidth = algorithm_result->original_width;
    AX_U32 nHeight = algorithm_result->original_height;
    auto endTime = std::chrono::steady_clock::now();
    DET_PERF_INFO_T tPerfInfo = {0};
    AX_BOOL bAddPerfInfo = AX_FALSE;

    m_mutex.lock();
    AX_U32 nActualFrameId = frame_id;
    m_tRunningStatus.nFrameRecv ++;
    auto iter = m_mapDataResult.find(frame_id);
    if (iter != m_mapDataResult.end()) {
        AX_U32 nElapsed = (AX_U32)(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - iter->second.m_tpStart).count());

        bAddPerfInfo = AX_TRUE;

        tPerfInfo.nElapsed = nElapsed;

        nActualFrameId = (AX_U32)iter->second.m_sVideoFrame.u64SeqNum;

        LOG_M_I(DETECTION, "Frame_id(%d) detect elapsed %d(ms)", frame_id, nElapsed);
    } else {
        LOG_M_I(DETECTION, "m_mapDataResult frame_id(%d) not found", frame_id);
    }
    m_mutex.unlock();

    do {
        if (algorithm_result->object_size > 0
            && nWidth > 0
            && nHeight > 0) {
            // Object Definition
            ObjectDefinition(Face)
            ObjectDefinition(Body)
            ObjectDefinition(Vehicle)
            ObjectDefinition(Plate)
            ObjectDefinition(Cycle)

            AI_Detection_Result_t DetectionResult;
            AI_Detection_Result_t *pDetectionResult = &DetectionResult;
            pDetectionResult->nErrorCode = AX_SUCCESS;
            pDetectionResult->nFrameId = nActualFrameId;

            for (size_t i = 0; i < algorithm_result->object_size; i++) {
                const char *object_category = (const char *)algorithm_result->object_items[i].object_category;
                if (object_category) {
                    // Object Calculate
                    ObjectCalculate(Face);
                    ObjectCalculate(Body);
                    ObjectCalculate(Vehicle);
                    ObjectCalculate(Plate);
                    ObjectCalculate(Cycle);
                }
            }

            // Object Create
            ObjectCreate(Face);
            ObjectCreate(Body);
            ObjectCreate(Vehicle);
            ObjectCreate(Plate);
            ObjectCreate(Cycle);

            for (size_t i = 0; i < algorithm_result->object_size; i++) {
                const char *object_category = (const char *)algorithm_result->object_items[i].object_category;
                if (object_category) {
                    auto left = algorithm_result->object_items[i].detect_rect.left_top.x;
                    auto right = algorithm_result->object_items[i].detect_rect.right_bottom.x;
                    auto top = algorithm_result->object_items[i].detect_rect.left_top.y;
                    auto bottom = algorithm_result->object_items[i].detect_rect.right_bottom.y;

                    if (left >= right || top >= bottom) {
                        continue;
                    }

                    JpegDataInfo JpegInfo;

                    AI_Detection_Box_t tBox = {0};
                    tBox.fX = (float)left/nWidth;
                    tBox.fY = (float)top/nHeight;
                    tBox.fW = (float)(right - left - 1)/nWidth;
                    tBox.fH = (float)(bottom - top - 1)/nHeight;

                    AX_U64 u64TrackId = algorithm_result->object_items[i].track_id;
                    AX_F32 fConfidence = algorithm_result->object_items[i].confidence * 100;

                    if (fConfidence < m_tConfigParam.tObjectFliter[object_category].fConfidence) {
                        continue;
                    }

                    // Object Assign
                    ObjectAssign(Face);
                    ObjectAssign(Body);
                    ObjectAssign(Vehicle);
                    ObjectAssign(Plate);
                    ObjectAssign(Cycle);

                    //search
                    AX_BOOL isObjectTrack = AX_TRUE;

                    if (gOptions.IsActivedSearchFromWeb() && m_pObjectSearch) {
                        isObjectTrack = m_pObjectSearch->SyncSearch(&algorithm_result->object_items[i]);

                        if (isObjectTrack) {
                            strncpy(JpegInfo.tFaceInfo.szInfo, (const AX_CHAR *)"identified", sizeof(JpegInfo.tFaceInfo.szInfo) - 1);
                        }
                        else {
                            strncpy(JpegInfo.tFaceInfo.szInfo, (const AX_CHAR *)"unknown", sizeof(JpegInfo.tFaceInfo.szInfo) - 1);
                        }
                    }

                    // Object Track
                    if (m_pTrackCropStage) {
                        auto DoTracking = [&](MCV_OBJECT_ITEM_S object_items) {
                                if (MCV_TRACK_STATUS_SELECT == object_items.track_state
                                    && object_items.has_crop_frame
                                    && object_items.crop_frame.frame_data
                                    && 0 < object_items.crop_frame.frame_data_size) {
                                    m_pTrackCropStage->DoTracking((AX_VOID *)object_items.crop_frame.frame_data,
                                                                    object_items.crop_frame.frame_data_size,
                                                                    &JpegInfo);
                            }
                        };

                        // Object Track
                        if (isObjectTrack) {
                            if (AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_BODY)) {
                                ObjectTrack(Body);
                            }
                            if (AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_VEHICLE)) {
                                ObjectTrack(Vehicle);
                            }
                            if (AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_CYCLE)) {
                                ObjectTrack(Cycle);
                            }
                            if (AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_FACE)) {
                                ObjectTrack(Face);
                            }
                            if (AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_PLATE)) {
                                ObjectTrack(Plate);
                            }
                        }
                    }
                }
            }

            gOptions.SetDetectResult(0, pDetectionResult);

            // Object Destroy
            ObjectDestroy(Face);
            ObjectDestroy(Body);
            ObjectDestroy(Vehicle);
            ObjectDestroy(Plate);
            ObjectDestroy(Cycle);
        }
        else {
            gOptions.SetDetectResult(0, NULL);
        }
    }while(0);

    if (bAddPerfInfo) {
        gPrintHelper.Add(E_PH_MOD_DET_PERF, (AX_VOID *)&tPerfInfo);
    }

    ClearAlgorithmData(algorithm_result);

    return AX_TRUE;
}

AX_BOOL CDetector::ClearAlgorithmData(MCV_ALGORITHM_RESULT_S *algorithm_result) {
    m_mutex.lock();

    for (auto iter = m_mapDataResult.begin(); iter != m_mapDataResult.end();) {
        if (iter->first > algorithm_result->frame_id) {//TODO
            break;
        }

        AX_BOOL need_clear_algorithm_data = AX_TRUE;
        for (AX_U32 i = 0; i < algorithm_result->cache_list_size; i++) {
            if (iter->first == (AX_U32)algorithm_result->cache_list[i].frame_id) {
                need_clear_algorithm_data = AX_FALSE;
                break;
            }
        }

        if (need_clear_algorithm_data) {
            if (iter->second.m_sVideoFrame.u32BlkId[0] > 0) {
                AX_POOL_DecreaseRefCnt(iter->second.m_sVideoFrame.u32BlkId[0], AX_ID_USER);
                AX_POOL_ReleaseBlock(iter->second.m_sVideoFrame.u32BlkId[0]);
            }
            if (iter->second.m_sVideoFrame.u32BlkId[1] > 0) {
                AX_POOL_DecreaseRefCnt(iter->second.m_sVideoFrame.u32BlkId[1], AX_ID_USER);
                AX_POOL_ReleaseBlock(iter->second.m_sVideoFrame.u32BlkId[1]);
            }

            m_mapDataResult.erase(iter++);
        } else {
            ++iter;
        }
    }

    DETECTOR_API(mcv_delete)(algorithm_result);

    m_mutex.unlock();

    return AX_TRUE;
}

AX_BOOL CDetector::ClearAlgorithmData(void) {
    m_mutex.lock();
    for (auto iter = m_mapDataResult.begin(); iter != m_mapDataResult.end();) {
        if (iter->second.m_sVideoFrame.u32BlkId[0] > 0) {
            AX_POOL_DecreaseRefCnt(iter->second.m_sVideoFrame.u32BlkId[0], AX_ID_USER);
            AX_POOL_ReleaseBlock(iter->second.m_sVideoFrame.u32BlkId[0]);
        }
        if (iter->second.m_sVideoFrame.u32BlkId[1] > 0) {
            AX_POOL_DecreaseRefCnt(iter->second.m_sVideoFrame.u32BlkId[1], AX_ID_USER);
            AX_POOL_ReleaseBlock(iter->second.m_sVideoFrame.u32BlkId[1]);
        }

        m_mapDataResult.erase(iter++);
    }
    m_mutex.unlock();

    return AX_TRUE;
}

AX_BOOL CDetector::WaitForFinish(AX_BOOL bWaitBlock /*= AX_FALSE*/)
{
    //Wait sent finish
    while(!m_bFinished) {
        CTimeUtils::msSleep(10);
    }

    if (bWaitBlock) {
        //Wait handle finish
        auto waitStart = std::chrono::steady_clock::now();
        do {
            m_mutex.lock();
            AX_U64 nFrameRecv = m_tRunningStatus.nFrameRecv;
            AX_U64 nFrameSent = m_tRunningStatus.nFrameSent;
            m_mutex.unlock();

            if (nFrameRecv >= nFrameSent) {
                break;
            }

            auto waitEnd = std::chrono::steady_clock::now();
            AX_U32 nElapsed = (AX_U32)(std::chrono::duration_cast<std::chrono::milliseconds>(waitEnd - waitStart).count());

            if (nElapsed >= DETECTION_WAITING_TIMEOUT) {
                break;
            }

            CTimeUtils::msSleep(10);
        } while(1);
    }

    return AX_TRUE;
}

DETECTOR_CONFIG_PARAM_T CDetector::GetConfig(AX_VOID)
{
    std::lock_guard<std::mutex> lck(m_stMutex);

    return (DETECTOR_CONFIG_PARAM_T)m_tConfigParam;
}

AX_BOOL CDetector::SetConfig(DETECTOR_CONFIG_PARAM_T *pConfig)
{
    if (!pConfig || !m_stream_handle) {
        return AX_FALSE;
    }

    // update param
    {
        std::lock_guard<std::mutex> lck(m_stMutex);

        m_tConfigParam.nTrackType = pConfig->nTrackType;
        m_tConfigParam.nDrawRectType = pConfig->nDrawRectType;
        m_tConfigParam.bPlateIdentify = pConfig->bPlateIdentify;
    }

    // update roi
    SetRoi(&pConfig->tRoi);

    // update push strategy
    SetPushStrategy(&pConfig->tPushStrategy);

    // update object fliter
    for (auto iter = pConfig->tObjectFliter.begin(); iter != pConfig->tObjectFliter.end();) {
        SetObjectFilter(iter->first, &iter->second);

        ++iter;
    }

    return AX_TRUE;
}

AX_BOOL CDetector::SetRoi(DETECTOR_ROI_CONFIG_T *ptRoi)
{
    std::lock_guard<std::mutex> lck(m_stMutex);

    if (!ptRoi || !m_stream_handle) {
        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;
    MCV_ROI_CONFIG_S tRoiParam;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    AX_U32 nWidth = pStageOption->GetAiAttr().tConfig.nWidth;
    AX_U32 nHeight = pStageOption->GetAiAttr().tConfig.nHeight;

    memset(&tRoiParam, 0x00, sizeof(tRoiParam));
    tRoiParam.enable = (bool)ptRoi->bEnable;
    tRoiParam.points_num = 4;
    // points should as clockwise or anti-clockwise
    tRoiParam.points[0].x = (int)(ptRoi->tBox.fX * nWidth);
    tRoiParam.points[0].y = (int)(ptRoi->tBox.fY * nHeight);
    tRoiParam.points[1].x = (int)((ptRoi->tBox.fX + ptRoi->tBox.fW) * nWidth) - 1;
    tRoiParam.points[1].y = (int)(ptRoi->tBox.fY * nHeight);
    tRoiParam.points[2].x = (int)((ptRoi->tBox.fX + ptRoi->tBox.fW) * nWidth) - 1;
    tRoiParam.points[2].y = (int)((ptRoi->tBox.fY + ptRoi->tBox.fH)* nHeight) - 1;
    tRoiParam.points[3].x = (int)(ptRoi->tBox.fX * nWidth);
    tRoiParam.points[3].y = (int)((ptRoi->tBox.fY + ptRoi->tBox.fH)* nHeight) - 1;

    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"detect_roi";
    tStreamItem.value = (void *)&tRoiParam;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    MCV_RET_CODE_E ret = DETECTOR_API(mcv_set_stream_config)(m_stream_handle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_stream_config error: %d", ret);

        return AX_FALSE;
    }

    m_tConfigParam.tRoi = *ptRoi;

    return AX_TRUE;
}

AX_BOOL CDetector::SetPushStrategy(DETECTOR_PUSH_STRATEGY_T *ptPushStrategy)
{
    std::lock_guard<std::mutex> lck(m_stMutex);

    if (!ptPushStrategy || !m_stream_handle) {
        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;
    MCV_PUSH_STRATEGY_CONFIG_S pushStrategy;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    m_tConfigParam.tPushStrategy.nPushMode = ptPushStrategy->nPushMode;
    m_tConfigParam.tPushStrategy.nInterval = ptPushStrategy->nInterval;
    m_tConfigParam.tPushStrategy.nPushCounts = ptPushStrategy->nPushCounts;
    m_tConfigParam.tPushStrategy.bPushSameFrame = ptPushStrategy->bPushSameFrame;

    //set push_strategy
    pushStrategy.push_mode = (MCV_PUSH_MODE_E)ptPushStrategy->nPushMode;
    pushStrategy.interval_times = (uint32_t)ptPushStrategy->nInterval;
    pushStrategy.push_counts = (uint32_t)ptPushStrategy->nPushCounts;
    pushStrategy.push_same_frame = (bool)ptPushStrategy->bPushSameFrame;

    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"push_strategy";
    tStreamItem.value = (void *)&pushStrategy;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    MCV_RET_CODE_E ret = DETECTOR_API(mcv_set_stream_config)(m_stream_handle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_stream_config error: %d", ret);

        return AX_FALSE;
    }

    m_tConfigParam.tPushStrategy = *ptPushStrategy;

    return AX_TRUE;
}

AX_BOOL CDetector::SetObjectFilter(std::string strObject, DETECTOR_OBJECT_FILTER_CONFIG_T *ptObjectFliter)
{
    std::lock_guard<std::mutex> lck(m_stMutex);

    if (!ptObjectFliter || !m_stream_handle) {
        return AX_FALSE;
    }

    if (strObject != "body"
        && strObject != "vehicle"
        && strObject != "cycle"
        && strObject != "face"
        && strObject != "plate") {
        LOG_M_E(DETECTION, "wrong object: %s", strObject.c_str());
        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;
    MCV_OBJECT_SIZE_FILTER_CONFIG_S tObjectSizeParam;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    memset(&tObjectSizeParam, 0x00, sizeof(tObjectSizeParam));
    tObjectSizeParam.width = ptObjectFliter->nWidth;
    tObjectSizeParam.height = ptObjectFliter->nHeight;

    AX_CHAR aObject[20] = {0};
    snprintf(aObject, 20, "min_%s", strObject.c_str());
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)aObject;
    tStreamItem.value = (void *)&tObjectSizeParam;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    MCV_RET_CODE_E ret = DETECTOR_API(mcv_set_stream_config)(m_stream_handle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_stream_config error: %d", ret);

        return AX_FALSE;
    }

    m_tConfigParam.tObjectFliter[strObject] = *ptObjectFliter;

    LOG_M(DETECTION, "%s filter(%d X %d, confidence: %.2f)",
                                    strObject.c_str(),
                                    m_tConfigParam.tObjectFliter[strObject].nWidth,
                                    m_tConfigParam.tObjectFliter[strObject].nHeight,
                                    m_tConfigParam.tObjectFliter[strObject].fConfidence);

    return AX_TRUE;
}

AX_BOOL CDetector::SetTrackSize(DETECTOR_TRACK_SIZE_T *ptTrackSize)
{
    std::lock_guard<std::mutex> lck(m_stMutex);

    if (!ptTrackSize || !m_stream_handle) {
        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    //set max track human size
    MCV_COMMON_THRESHOLD_CONFIG_S max_track_human_size;
    max_track_human_size.value = (float)ptTrackSize->nTrackHumanSize;
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"max_track_human_size";
    tStreamItem.value = (void *)&max_track_human_size;
    vStreamContent.emplace_back(tStreamItem);

    //set max track vehicle size
    MCV_COMMON_THRESHOLD_CONFIG_S max_track_vehicle_size;
    max_track_vehicle_size.value = (float)ptTrackSize->nTrackVehicleSize;
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"max_track_vehicle_size";
    tStreamItem.value = (void *)&max_track_vehicle_size;
    vStreamContent.emplace_back(tStreamItem);

    //set max track cycle size
    MCV_COMMON_THRESHOLD_CONFIG_S max_track_cycle_size;
    max_track_cycle_size.value = (float)ptTrackSize->nTrackCycleSize;
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"max_track_cycle_size";
    tStreamItem.value = (void *)&max_track_cycle_size;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    MCV_RET_CODE_E ret = DETECTOR_API(mcv_set_stream_config)(m_stream_handle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_stream_config error: %d", ret);

        return AX_FALSE;
    }

    m_tConfigParam.tTrackSize = *ptTrackSize;

    LOG_M(DETECTION, "Track size(human: %d, vehicle: %d, cycle: %d)",
                            m_tConfigParam.tTrackSize.nTrackHumanSize,
                            m_tConfigParam.tTrackSize.nTrackVehicleSize,
                            m_tConfigParam.tTrackSize.nTrackCycleSize);

    return AX_TRUE;
}

AX_BOOL CDetector::SetCropEncoderQpLevel(AX_F32 fCropEncoderQpLevel)
{
    std::lock_guard<std::mutex> lck(m_stMutex);

    if (!m_stream_handle) {
        return AX_FALSE;
    }

    std::vector<MCV_CONFIG_ITEM_S> vStreamContent;
    MCV_STREAM_CONFIG_S tStreamParam;
    MCV_CONFIG_ITEM_S tStreamItem;

    vStreamContent.clear();
    memset(&tStreamParam, 0, sizeof(MCV_STREAM_CONFIG_S));

    //set crop encoder qpLevel
    MCV_COMMON_THRESHOLD_CONFIG_S crop_encoder_qpLevel_threshold;
    crop_encoder_qpLevel_threshold.value = (float)fCropEncoderQpLevel;
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"crop_encoder_qpLevel";
    tStreamItem.value = (void *)&crop_encoder_qpLevel_threshold;
    vStreamContent.emplace_back(tStreamItem);

    //set panoramic quality qpLevel
    MCV_COMMON_THRESHOLD_CONFIG_S panoramic_quality_qpLevel_threshold;
    panoramic_quality_qpLevel_threshold.value = (float)fCropEncoderQpLevel;
    memset(&tStreamItem, 0, sizeof(MCV_CONFIG_ITEM_S));
    tStreamItem.type = (char *)"panoramic_quality_qpLevel";
    tStreamItem.value = (void *)&panoramic_quality_qpLevel_threshold;
    vStreamContent.emplace_back(tStreamItem);

    tStreamParam.items = vStreamContent.data();
    tStreamParam.size = vStreamContent.size();

    MCV_RET_CODE_E ret = DETECTOR_API(mcv_set_stream_config)(m_stream_handle, &tStreamParam);

    if (MCV_RET_SUCCESS != ret) {
        LOG_M_E(DETECTION, "mcv_set_stream_config error: %d", ret);

        return AX_FALSE;
    }

    m_tConfigParam.fCropEncoderQpLevel = fCropEncoderQpLevel;

    return AX_TRUE;
}

AX_BOOL CDetector::UpdateAiAttr(AX_VOID)
{
    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    AI_ATTR_T tAiAttr = pStageOption->GetAiAttr();

    tAiAttr.nEnable = AX_TRUE;

    if (strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "facehuman")) {
        if (strstr(m_tConfigParam.tStreamParam.algo_type.c_str(), "detect")) {
            tAiAttr.tHumanFaceSetting.tFace.nEnable = 0;
            tAiAttr.tHumanFaceSetting.tBody.nEnable = 0;
            tAiAttr.tHumanFaceSetting.nEnableFI = 0;
            tAiAttr.nDetectOnly = 1;
        }
        else {
            tAiAttr.tHumanFaceSetting.tFace.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_FACE) ? AX_TRUE : AX_FALSE;
            tAiAttr.tHumanFaceSetting.tBody.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_BODY) ? AX_TRUE : AX_FALSE;
            tAiAttr.tHumanFaceSetting.nEnableFI = gOptions.IsActivedSearchFromWeb();
            tAiAttr.nDetectOnly = 0;
        }

        tAiAttr.eDetectModel = E_AI_DETECT_MODEL_TYPE_FACEHUMAN;
        tAiAttr.tHumanFaceSetting.tFace.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_FACE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHumanFaceSetting.tBody.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_BODY) ? AX_TRUE : AX_FALSE;
    }
    else {
        tAiAttr.eDetectModel = E_AI_DETECT_MODEL_TYPE_HVCFP;
        tAiAttr.nDetectOnly = 0;

        tAiAttr.tHvcfpSetting.tFace.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_FACE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tFace.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_FACE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tBody.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_BODY) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tBody.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_BODY) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tVechicle.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_VEHICLE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tVechicle.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_VEHICLE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tCycle.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_CYCLE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tCycle.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_CYCLE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tPlate.nEnable = AX_BIT_CHECK(m_tConfigParam.nTrackType, AI_TRACK_TYPE_PLATE) ? AX_TRUE : AX_FALSE;
        tAiAttr.tHvcfpSetting.tPlate.nDrawRect = AX_BIT_CHECK(m_tConfigParam.nDrawRectType, AI_DRAW_RECT_TYPE_PLATE) ? AX_TRUE : AX_FALSE;

        tAiAttr.tHvcfpSetting.nEnablePI = (AX_U16)m_tConfigParam.bPlateIdentify;
    }

    tAiAttr.tPushStrgy.eMode = (AI_DETECT_PUSH_MODE_TYPE_E)m_tConfigParam.tPushStrategy.nPushMode;
    tAiAttr.tPushStrgy.nInterval = (AX_U16)m_tConfigParam.tPushStrategy.nInterval;
    tAiAttr.tPushStrgy.nCount = (AX_U16)m_tConfigParam.tPushStrategy.nPushCounts;
    tAiAttr.tPushStrgy.nPushSameFrame = (AX_U8)m_tConfigParam.tPushStrategy.bPushSameFrame;

    pStageOption->SetAiAttr(tAiAttr);

    return AX_TRUE;
}
AX_BOOL CDetector::UpdateConfig(const AI_ATTR_T& tAiAttr)
{
    gOptions.SetDetectActived((AX_BOOL)tAiAttr.nEnable);

    AX_U32 nTrackType = 0;
    AX_U32 nDrawRectType = 0;
    AX_BOOL bPlateIdentify = AX_TRUE;
    DETECTOR_PUSH_STRATEGY_T tPushStrategy = {0};

    if (E_AI_DETECT_MODEL_TYPE_FACEHUMAN == tAiAttr.eDetectModel) {
        //face
        if (tAiAttr.tHumanFaceSetting.tFace.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_FACE);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_FACE);
        }
        if (tAiAttr.tHumanFaceSetting.tFace.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_FACE);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_FACE);
        }

        //body
        if (tAiAttr.tHumanFaceSetting.tBody.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_BODY);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_BODY);
        }
        if (tAiAttr.tHumanFaceSetting.tBody.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_BODY);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_BODY);
        }

        gOptions.SetSearchActived((AX_BOOL)tAiAttr.tHumanFaceSetting.nEnableFI);
    }
    else if (E_AI_DETECT_MODEL_TYPE_HVCFP == tAiAttr.eDetectModel) {
        //face
        if (tAiAttr.tHvcfpSetting.tFace.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_FACE);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_FACE);
        }
        if (tAiAttr.tHvcfpSetting.tFace.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_FACE);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_FACE);
        }

        //body
        if (tAiAttr.tHvcfpSetting.tBody.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_BODY);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_BODY);
        }
        if (tAiAttr.tHvcfpSetting.tBody.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_BODY);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_BODY);
        }

        //vehicle
        if (tAiAttr.tHvcfpSetting.tVechicle.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_VEHICLE);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_VEHICLE);
        }
        if (tAiAttr.tHvcfpSetting.tVechicle.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_VEHICLE);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_VEHICLE);
        }

        //cycle
        if (tAiAttr.tHvcfpSetting.tCycle.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_CYCLE);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_CYCLE);
        }
        if (tAiAttr.tHvcfpSetting.tCycle.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_CYCLE);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_CYCLE);
        }

        //plate
        if (tAiAttr.tHvcfpSetting.tPlate.nEnable) {
            AX_BIT_SET(nTrackType, AI_TRACK_TYPE_PLATE);
        }
        else {
            AX_BIT_CLEAR(nTrackType, AI_TRACK_TYPE_PLATE);
        }
        if (tAiAttr.tHvcfpSetting.tPlate.nDrawRect) {
            AX_BIT_SET(nDrawRectType, AI_DRAW_RECT_TYPE_PLATE);
        }
        else {
            AX_BIT_CLEAR(nDrawRectType, AI_DRAW_RECT_TYPE_PLATE);
        }

        bPlateIdentify = (AX_BOOL)tAiAttr.tHvcfpSetting.nEnablePI;
    }

    tPushStrategy.nPushMode = (AX_U8)tAiAttr.tPushStrgy.eMode;
    tPushStrategy.nInterval = (AX_U32)tAiAttr.tPushStrgy.nInterval;
    tPushStrategy.nPushCounts = (AX_U32)tAiAttr.tPushStrgy.nCount;
    tPushStrategy.bPushSameFrame = (AX_BOOL)tAiAttr.tPushStrgy.nPushSameFrame;

    //update
    DETECTOR_CONFIG_PARAM_T tConf = GetConfig();

    tConf.nTrackType = nTrackType;
    tConf.nDrawRectType = nDrawRectType;
    tConf.bPlateIdentify = bPlateIdentify;
    tConf.tPushStrategy = tPushStrategy;

    SetConfig(&tConf);

    return AX_TRUE;
}
