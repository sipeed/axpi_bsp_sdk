/*!
 *  @file: mcv_common_struct.h
 *  @brief: mcv common struct file
 *  @version: 1.0.0
 *  @author:
 *  @date:
 */

/******************************************************************************
@note
    Copyright 2017, Megvii Corporation, Limited
                        ALL RIGHTS RESERVED
******************************************************************************/
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mcv_config_param.h"
#include "mcv_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *MCV_STREAM_HANDLE_T;

typedef enum MCV_RET_CODE_ENUM {
    // common
    MCV_RET_SUCCESS = 0,
    MCV_RET_FAILURE,
    MCV_RET_MEM_BAD_ALLOC,
    MCV_RET_INVALID_ARG,
    MCV_RET_INVALID_HANDLE,
    MCV_RET_INTERNAL_ERROR,
    MCV_RET_UNSPECIFIED,
    // model
    MCV_RET_MODEL_NOSUPPORT = 50,
    MCV_RET_MODEL_NOEXIST,
    MCV_RET_FLASH_BAD_ALLOC,
    // frame
    MCV_RET_INVALID_FRAME = 80,
    MCV_RET_FRAME_BUFFER_FULL,
    MCV_RET_INVALID_TRACKID,
    MCV_RET_SUCCESS_AND_WILL_SMOOTH,    // sdk will skip this frame processing and return smooth
                                        // result

    // result
    MCV_RET_EMPTY_RESULT = 100,

    // search
    MCV_RET_SEARCH_OUT_OF_RANGE = 120,

    // param
    MCV_RET_PARAM_ERROR = 150,

} MCV_RET_CODE_E;

typedef struct MCV_UPLOAD_MODEL_PACKAGE_ST {
    char *modelpackage_name;
    char *modelpackage_path;
} MCV_UPLOAD_MODEL_PACKAGE_S;

typedef struct MCV_DELETE_MODEL_PACKAGE_ST {
    char *modelpackage_name;
} MCV_DELETE_MODEL_PACKAGE_S;

/**
 * 模式类型
 * @type  类型名称
 * @value 值
 */
typedef struct MCV_META_INFO_ST {
    char *type;
    char *value;
} MCV_META_INFO_S;

/**
 * model package info
 **/
typedef struct MCV_MODEL_PACKAGE_INFO_ST {
    const char *modelpackage_name;
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
} MCV_MODEL_PACKAGE_INFO_S;

/**
 * model package list
 **/
typedef struct MCV_MODEL_PACKAGE_LIST_ST {
    uint32_t model_package_info_size;
    MCV_MODEL_PACKAGE_INFO_S *model_package_info;
} MCV_MODEL_PACKAGE_LIST_S;

typedef struct MCV_INIT_PARAM_ST {
    char *model_deployment_path;
} MCV_INIT_PARAM_S;

typedef enum MCV_LOG_MODULE_ENUM {
    MCV_LOG_MODULE_ALL = 0,
    MCV_LOG_MODULE_HAL,
    MCV_LOG_MODULE_PLUGINS,
    MCV_LOG_MODULE_CORE,
    MCV_LOG_MODULE_MAX
} MCV_LOG_MODULE_E;

typedef enum MCV_LOG_LEVEL_ENUM {
    MCV_LOG_LEVEL_INFO = 0,
    MCV_LOG_LEVEL_WARN,
    MCV_LOG_LEVEL_ERROR,
} MCV_LOG_LEVEL_E;

typedef struct MCV_LOG_PARAM_ST {
    MCV_LOG_MODULE_E module;
    MCV_LOG_LEVEL_E log_level;
    char *log_path;
    int save_file;
} MCV_LOG_PARAM_S;

typedef struct MCV_VERSION_INFO_ST {
    const char *version;
    const char *frame_version;
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
} MCV_VERSION_INFO_S;

/**
 * rect info
 */
typedef struct MCV_RECT_ST {
    int left;
    int top;
    int right;
    int bottom;
} MCV_RECT_S;

typedef struct MCV_ITEM_SETTING_ST {
    // The desired object will be executed
    char *object_category;
    // fitler min size
    uint32_t width;
    uint32_t height;
    // only for face
    MCV_POSE_BLUR_S pose_blur;
} MCV_ITEM_SETTING_S;

typedef enum MCV_STREAM_TYPE_ENUM {
    MCV_STREAM_TYPE_VIDEO = 1,
    MCV_STREAM_TYPE_IMAGE,
} MCV_STREAM_TYPE_E;

typedef enum MCV_PIPELINE_TYPE_ENUM {
    MCV_PIPELINE_TYPE_FACEHUMAN = 1,
    MCV_PIPELINE_TYPE_HVCFP,
    MCV_PIPELINE_TYPE_ALGORITHM_STORE,
} MCV_PIPELINE_TYPE_E;

typedef struct MCV_PIPELINE_CONFIG_ST {
    char *pipline_config_key;             // The key is used to create the stream
    MCV_STREAM_TYPE_E stream_type;        // The value is to configure the pipline
    MCV_PIPELINE_TYPE_E pipeline_type;    // The value is to configure the pipline
} MCV_PIPELINE_CONFIG_S;

typedef enum MCV_BIND_MODE_ENUM {
    MCV_BIND_MODE_SAME_FRAME = 1,
    MCV_BIND_MODE_CROSS_FRAME,
} MCV_BIND_MODE_E;

typedef struct MCV_PUSH_STRATEGY_ST {
    MCV_PUSH_MODE_E push_mode;
    uint32_t interval_times;    // only for MCV_PUSH_MODE_INTERVAL or MCV_PUSH_MODE_FAST
    uint32_t push_counts;       // only for MCV_PUSH_MODE_INTERVAL or MCV_PUSH_MODE_FAST
    bool push_same_frame;       // false: push cross frame true: push same frame
} MCV_PUSH_STRATEGY_S;

typedef struct MCV_FACE_PUSH_QUALITY_FILTER_ST {
    uint32_t width;
    uint32_t height;
    MCV_POSE_BLUR_S pose_blur;
} MCV_FACE_PUSH_QUALITY_FILTER_S;

typedef struct MCV_COMMON_PUSH_QUALITY_FILTER_ST {
    float quality;
} MCV_COMMON_PUSH_QUALITY_FILTER_S;

typedef struct MCV_PUSH_QUALITY_FILTER_ST {
    // The desired object will be executed
    char *object_category;
    union {
        MCV_FACE_PUSH_QUALITY_FILTER_S face_quality;        // only for face
        MCV_COMMON_PUSH_QUALITY_FILTER_S common_quality;    // body,vehicle,cycle,plate
    };
} MCV_PUSH_QUALITY_FILTER_S;

typedef struct MCV_CORP_ENCODER_CONFIG_ST {
    // The desired object will be executed
    char *object_category;
    float scale_left;
    float scale_right;
    float scale_top;
    float scale_bottom;
} MCV_CORP_ENCODER_CONFIG_S;

typedef struct MCV_PANORAMA_RESIZE_CONFIG_ST {
    int w;
    int h;
} MCV_PANORAMA_RESIZE_CONFIG_S;

typedef enum MCV_ANALYZE_ATTR_ENUM {
    MCV_ANALYZE_ATTR_FACE_LIVENESS = 1,
    MCV_ANALYZE_ATTR_FACE_FEATURE,
    MCV_ANALYZE_ATTR_FACE_EMOTION,
    MCV_ANALYZE_ATTR_FACE_ATTRIBUTE,
    MCV_ANALYZE_ATTR_HUMAN_ACTION,
    MCV_ANALYZE_ATTR_HUMAN_ATTRIBUTE,
    MCV_ANALYZE_ATTR_HUMAN_FEATURE,
    MCV_ANALYZE_ATTR_MOTOR_VEHICLE_FEATURE,
    MCV_ANALYZE_ATTR_MOTOR_VEHICLE_ATTRIBUTE,
    MCV_ANALYZE_ATTR_NON_MOTOR_VEHICLE_FEATURE,
    MCV_ANALYZE_ATTR_NON_MOTOR_VEHICLE_ATTRIBUTE,
    MCV_ANALYZE_ATTR_PLATE_ATTRIBUTE,
} MCV_ANALYZE_ATTR_E;

typedef struct MCV_HVCFP_STREAM_CONFIG_ST {
    bool panorama_enabled;        // panoramic push enabled
    uint32_t panorama_quality;    // panoramic push image quality
    bool roi_enabled;
    MCV_RECT_S roi_rect;    // the ROI region for detect filter
    uint32_t item_setting_size;
    MCV_ITEM_SETTING_S *item_setting;     // target filter
    MCV_PUSH_STRATEGY_S push_strategy;    // push strategy
    uint32_t push_quality_filter_size;
    MCV_PUSH_QUALITY_FILTER_S *push_quality_filter;         // push_quality_filter
    MCV_PANORAMA_RESIZE_CONFIG_S panorama_resize_config;    // corp encoder config
    uint32_t analyze_attribute_size;
    MCV_ANALYZE_ATTR_E *analyze_attribute;    // target attribute mask
    bool push_bind_enabled;    // push image and push target bind info .only for video pipline.
} MCV_HVCFP_STREAM_CONFIG_S, MCV_FACEHUMAN_STREAM_CONFIG_S;

typedef struct MCV_ALGORITHM_STORE_STREAM_CONFIG_ST {
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
} MCV_ALGORITHM_STORE_STREAM_CONFIG_S;

typedef struct MCV_STREAM_HANDLE_PARAM_ST {
    char *pipline_config_key;
    MCV_STREAM_TYPE_E stream_type;
    MCV_PIPELINE_TYPE_E pipeline_type;
    union {
        MCV_FACEHUMAN_STREAM_CONFIG_S facehuman_stream_config;
        MCV_HVCFP_STREAM_CONFIG_S hvcfp_stream_config;
        MCV_ALGORITHM_STORE_STREAM_CONFIG_S algorithm_store_stream_config;
    };
    bool skip_frame_by_user;                 // if =false, sdk will do internal frame skipping
    uint32_t buffer_depth;                   // buffer depth
    bool detect_result_interface_enabled;    // is enabled mcv_get_detect_result interface
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
    bool smooth_result_enabled;    // when sdk do internal frame skipping
                                   // generate result for the skipped frames
} MCV_STREAM_HANDLE_PARAM_S;

/*
 reference mcv_config_param.h
 */
typedef struct MCV_CONFIG_ITEM_ST {
    char *type;     // mcv_config_param.h::cmd
    void *value;    // mcv_config_param.h::valuetype
} MCV_CONFIG_ITEM_S;

/**
 * set config param
 */
typedef struct MCV_STREAM_CONFIG_ST {
    uint32_t size;
    MCV_CONFIG_ITEM_S *items;
} MCV_STREAM_CONFIG_S;

typedef enum MCV_IMAGE_TYPE_ENUM {
    MCV_IMAGE_TYPE_PACKET_H264 = 1,
    MCV_IMAGE_TYPE_PACKET_H265,
    MCV_IMAGE_TYPE_JPEG,
    MCV_IMAGE_TYPE_GRAY,
    MCV_IMAGE_TYPE_NV12,
    MCV_IMAGE_TYPE_NV21,
    MCV_IMAGE_TYPE_BGR,
    MCV_IMAGE_TYPE_RGB,
} MCV_IMAGE_TYPE_E;

typedef enum MCV_IMAGE_SOURCE_TYPE_ENUM {
    MCV_IMAGE_SOURCE_ADDR = 0x00,
    MCV_IMAGE_SOURCE_DATA = 0x01,
    MCV_IMAGE_SOURCE_MAX,
} MCV_IMAGE_SOURCE_TYPE_E;

typedef struct MCV_IMAGE_ADDR_ST {
    uint32_t stride[3];    // Y U V stride
    uint64_t phy_addr[3];
    void *vir_addr[3];    // pointer of Y U V
} MCV_IMAGE_ADDR_S;

typedef struct MCV_IMAGE_DATA_ST {
    MCV_IMAGE_SOURCE_TYPE_E addr_type;    // type of frame address
    union {
        MCV_IMAGE_ADDR_S addr;
        unsigned char *data;
    };
} MCV_IMAGE_DATA_S;

typedef struct MCV_POINT2f_ST {
    float x;
    float y;
} MCV_POINT2f_S;

typedef struct MCV_LANDMARK_ST {
    uint32_t point_nums;
    MCV_POINT2f_S *points;
} MCV_LANDMARK_S;

typedef struct MCV_RECT4f_ST {
    MCV_POINT2f_S left_top;
    MCV_POINT2f_S right_top;
    MCV_POINT2f_S left_bottom;
    MCV_POINT2f_S right_bottom;
} MCV_RECT4f_S;

typedef enum MCV_FACE_ANALYSIS_TYPE_ENUM {
    MCV_FACE_ANALYSIS_TYPE_FRMO_RECT = 1,
    MCV_FACE_ANALYSIS_TYPE_FRMO_LANDMARK,
} MCV_FACE_ANALYSIS_TYPE_E;

typedef struct MCV_INPUT_OBJECT_ST {
    char *object_category;
    MCV_RECT4f_S rect;
    MCV_FACE_ANALYSIS_TYPE_E face_analysis_type;    // only for face
    MCV_LANDMARK_S face_landmark;
} MCV_INPUT_OBJECT_S;

typedef struct MCV_IMAGE_EXTRA_ST {
    char *pipline_config_key;
    uint32_t analyze_attribute_size;
    MCV_ANALYZE_ATTR_E *analyze_attribute;    // target attribute mask
    uint32_t item_setting_size;
    MCV_ITEM_SETTING_S *item_setting;    // target filter
    uint32_t input_object_size;
    MCV_INPUT_OBJECT_S
    *input_object;    // direct input target , instead of run model. eg: detect/landmark model
} MCV_IMAGE_EXTRA_S;

typedef struct MCV_VIDEO_EXTRA_ST {
    uint64_t pts;
    uint64_t dts;
} MCV_VIDEO_EXTRA_S;

typedef enum MCV_FRAME_EXTRA_TYPE_ENUM {
    MCV_FRAME_EXTRA_TYPE_IMAGE = 1,
    MCV_FRAME_EXTRA_TYPE_VIDEO,
} MCV_FRAME_EXTRA_TYPE_E;

typedef struct MCV_FRAME_HEADER_ST {
    uint64_t frame_id;    // frame id //seq of the frame for input
    uint32_t width;
    uint32_t height;
    uint32_t data_len;    // Number of bytes when format is 264 package
    MCV_FRAME_EXTRA_TYPE_E extra_type;
    union {
        MCV_IMAGE_EXTRA_S image_extra;
        MCV_VIDEO_EXTRA_S video_extra;
    };
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
} MCV_FRAME_HEADER_S;

typedef struct MCV_FRAME_ST {
    MCV_FRAME_HEADER_S header;
    MCV_IMAGE_TYPE_E image_type;
    MCV_IMAGE_DATA_S image_data;
    void *user_data;     // private data used to return result
    int reserved[16];    // reserved
} MCV_FRAME_S;

typedef enum MCV_TRACK_STATUS_ENUM {
    MCV_TRACK_STATUS_NEW = 0x00,    // TRACK new
    MCV_TRACK_STATUS_UPDATE,        // TRACK need to be updated
    MCV_TRACK_STATUS_DIE,           // TRACK died
    MCV_TRACK_STATUS_FILTERD,
    MCV_TRACK_STATUS_SELECT,    // SELECT track
    MCV_TRACK_STATUS_MAX,
} MCV_TRACK_STATUS_E;

typedef struct MCV_CROP_FRAME_ST {
    uint64_t match_frame_id;      // The frame ID from the cropping match panorama_frame
    unsigned char *frame_data;    ////only is jpg format
    uint32_t frame_data_size;
    uint32_t frame_width;
    uint32_t frame_height;
} MCV_CROP_FRAME_S;

typedef enum MCV_OBJECT_BIND_TYPE_ENUM {
    MCV_OBJECT_BIND_FACE_HUMAN = 1,
    MCV_OBJECT_BIND_HUMAN_CYCLE,
    MCV_OBJECT_BIND_VEHICLE_PLATE,
} MCV_OBJECT_BIND_TYPE_E;

typedef struct MCV_BIND_ID_ST {
    uint32_t first_idx;
    uint32_t second_idx;
} MCV_BIND_ID_S;

typedef struct MCV_OBJECT_BIND_ITEM_ST {
    MCV_OBJECT_BIND_TYPE_E bind_type;
    MCV_BIND_ID_S bind_id;
} MCV_OBJECT_BIND_ITEM_S;

typedef struct MCV_ATTRIBUTE_ST {
    char *attribute_json;
} MCV_ATTRIBUTE_S;

typedef struct MCV_FEATURE_ST {
    char *type;
    char *data_ptr;
    uint32_t data_size;
} MCV_FEATURE_S;

typedef struct MCV_VEHICLE_ST {
    uint32_t point_nums;
    MCV_POINT2f_S *bev_vehicle_box;
    float vehicle_speed;
} MCV_VEHICLE_S;

typedef struct MCV_OBJECT_BINDING_ST {
    char *bind_type;      // object type of binding
    uint64_t track_id;    // track_id of binding
} MCV_OBJECT_BINDING_S;

typedef struct MCV_OBJECT_ITEM_ST {
    char *object_category;
    uint64_t frame_id;
    MCV_RECT4f_S detect_rect;
    uint64_t track_id;
    MCV_RECT4f_S track_rect;
    MCV_TRACK_STATUS_E track_state;    // track_state is an emun of MCV_TRACK_STATUS_E for a track
    float confidence;                  // confidence score. Quality score.
    MCV_POSE_BLUR_S face_pose_blur;    // only for face
    MCV_LANDMARK_S object_landmark;    // only for face,vehicle
    MCV_VEHICLE_S vehicle_info;
    uint32_t object_bind_size;            // bind size
    MCV_OBJECT_BINDING_S *object_bind;    // object binding
    /*******************************/
    // the following are mainly the results of the push image
    /*******************************/
    bool has_crop_frame;
    MCV_PUSH_MODE_E push_mode;
    MCV_RECT4f_S crop_box;
    MCV_CROP_FRAME_S crop_frame;
    bool has_panora_frame;
    MCV_CROP_FRAME_S panora_frame;

    // feature result
    uint32_t feature_list_size;
    MCV_FEATURE_S *feature_list;
    // other kind result, data transmission
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
} MCV_OBJECT_ITEM_S;

typedef struct MCV_BIND_ITEM_RESULT_ST {
    uint32_t track_id_size;
    uint64_t *track_id;
    uint32_t bind_id_size;
    MCV_BIND_ID_S *bind_id;
} MCV_BIND_ITEM_RESULT_S;

typedef struct MCV_FRAME_CACHE_LIST_ST {
    uint64_t frame_id;
    uint32_t track_id_size;
    uint64_t *track_id;
} MCV_FRAME_CACHE_LIST_S;

typedef struct MCV_ALGORITHM_RESULT_ST {
    uint64_t frame_id;
    uint32_t object_size;
    MCV_OBJECT_ITEM_S *object_items;
    uint32_t bind_size;
    MCV_OBJECT_BIND_ITEM_S *item_binds;
    // if push_bind_enabled == true, push_bind_result could be valid.
    MCV_BIND_ITEM_RESULT_S push_bind_result;
    // If push image is enabled, the list of frames currently being cached by the push module
    uint32_t cache_list_size;
    MCV_FRAME_CACHE_LIST_S *cache_list;
    // the size of the original received image
    uint32_t original_width;
    uint32_t original_height;
    // the size of the algorithm processe image
    uint32_t processed_width;
    uint32_t processed_height;
    // other kind result, data transmission
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
    void *user_data;    // user data
    bool is_smooth_result;
    bool is_skip_result;
    int reserved[15];    // reserved
} MCV_ALGORITHM_RESULT_S;

/**
 * authorization capability
 */
typedef struct MCV_AUTHORIZATION_CAPABILITY_ST {
    char *auth_description;    // authorization to describe.
    int reserved[10];          // reserved.
} MCV_AUTHORIZATION_CAPABILITY_S;

/**
 * handle capability
 */
typedef struct MCV_HANDLE_CAPABILITY_ST {
    int video_handle_num;    // number of video handle.
    int image_handle_num;    // number of image handle.
} MCV_HANDLE_CAPABILITY_S;

typedef struct MCV_CAPABILITY_ST {
    MCV_AUTHORIZATION_CAPABILITY_S authorization_capability;    // authorization capability.
    MCV_HANDLE_CAPABILITY_S handle_capability;                  // handle capability.
    uint32_t pipline_config_size;
    MCV_PIPELINE_CONFIG_S *pipline_config;
    uint32_t meta_info_size;
    MCV_META_INFO_S *meta_info;
} MCV_CAPABILITY_S;

typedef void (*ALGORITHM_RESULT_CALLBACK_FUNC)(MCV_STREAM_HANDLE_T handle,
                                               MCV_ALGORITHM_RESULT_S *algorithm_result,
                                               void *user_data);

#ifdef __cplusplus
}
#endif
