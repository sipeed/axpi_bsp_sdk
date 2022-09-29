/*!
 *  @file: mcv_config_param.h
 *  @brief: mcv config param file
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
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief object size filter config
 */
typedef struct MCV_OBJECT_SIZE_FILTER_CONFIG_ST {
    uint32_t width;
    uint32_t height;
} MCV_OBJECT_SIZE_FILTER_CONFIG_S;
// cmd:"min_face",  value_type:MCV_OBJECT_SIZE_FILTER_CONFIG_S*
// cmd:"min_body",  value_type:MCV_OBJECT_SIZE_FILTER_CONFIG_S*
// cmd:"min_vehicle",  value_type:MCV_OBJECT_SIZE_FILTER_CONFIG_S*
// cmd:"min_cycle",  value_type:MCV_OBJECT_SIZE_FILTER_CONFIG_S*
// cmd:"min_plate",  value_type:MCV_OBJECT_SIZE_FILTER_CONFIG_S*

/**
 * @brief Set Venc Attr Config
 */
typedef struct {
    int w;
    int h;
    int buf_size;
} MCV_VENC_ATTR_CONFIG;
// cmd:"venc_attr_config",  value_type:MCV_VENC_ATTR_CONFIG*

/**
 * @brief Common Enable Config
 */
typedef struct MCV_COMMON_ENABLE_CONFIG_ST {
    bool enable;
} MCV_COMMON_ENABLE_CONFIG_S;
// cmd:"f2h_kill_body",  value_type:MCV_COMMON_ENABLE_CONFIG_S*

/**
 * @brief Common Threshold Config
 */
typedef struct MCV_COMMON_THRESHOLD_CONFIG_ST {
    float value;
} MCV_COMMON_THRESHOLD_CONFIG_S;
// cmd:"frame_cache_depth",  value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"crop_encoder_qpLevel",  value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"panorama_qpLevel",  value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"crop_encoder_quality",  value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"panorama_quality",  value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"max_track_human_size",   value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"max_track_vehicle_size", value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
// cmd:"max_track_cycle_size",   value_type:MCV_COMMON_THRESHOLD_CONFIG_S*
/**
 * @brief Crop Encoder Threshold Config
 */
typedef struct {
    float scale_left;
    float scale_right;
    float scale_top;
    float scale_bottom;
} MCV_CROP_ENCODER_THRESHOLD_CONFIG_S;
// cmd:"crop_encoder",  value_type:MCV_CROP_ENCODER_THRESHOLD_CONFIG_S*

/**
 * @brief Reszie Config
 */
typedef struct {
    float w;
    float h;
} MCV_RESIZE_CONFIG;
// cmd:"resize_panoramic_encoder_config",  value_type:MCV_RESIZE_CONFIG*

/**
 * @brief Transmission JsonData Config
 */
// any transmission cmd, value_type:char*

/**
 * @brief Point coordinates
 */
typedef struct MCV_POINT2i_ST {
    int x;
    int y;
} MCV_POINT2i_S;

#define ROI_MAX_POINT 10
/**
 * @brief roi config
 */
typedef struct MCV_ROI_CONFIG_ST {
    bool enable;
    int points_num;
    MCV_POINT2i_S points[ROI_MAX_POINT];
} MCV_ROI_CONFIG_S;
// cmd:"detect_roi",  value_type:MCV_ROI_CONFIG_S*

/**
 * @brief pose blur config
 */
typedef struct MCV_POSE_BLUR_ST {
    float pitch;
    float yaw;
    float roll;
    float blur;
} MCV_POSE_BLUR_S;

/**
 * @brief face attr filter config
 */
typedef struct MCV_FACT_ATTR_FILTER_CONFIG_ST {
    uint32_t width;
    uint32_t height;
    MCV_POSE_BLUR_S poseblur;
} MCV_FACT_ATTR_FILTER_CONFIG_S;

/**
 * @brief common attr filter config
 */
typedef struct MCV_COMMON_ATTR_FILTER_CONFIG_ST {
    float quality;
} MCV_COMMON_ATTR_FILTER_CONFIG_S;

/**
 * @brief attr filter config
 */
typedef struct MCV_ATTR_FILTER_CONFIG_ST {
    union {
        MCV_FACT_ATTR_FILTER_CONFIG_S face_attr_filter_config;      // face
        MCV_COMMON_ATTR_FILTER_CONFIG_S common_attr_filter_config;  // body,vehicle,cycle,plate
    };
} MCV_ATTR_FILTER_CONFIG_S, MCV_QUALITY_FILTER_CONFIG_S;
// cmd:"push_quality_face",  value_type:MCV_QUALITY_FILTER_CONFIG_S*
// cmd:"push_quality_body",  value_type:MCV_QUALITY_FILTER_CONFIG_S*
// cmd:"push_quality_vehicle",  value_type:MCV_QUALITY_FILTER_CONFIG_S*
// cmd:"push_quality_cycle",  value_type:MCV_QUALITY_FILTER_CONFIG_S*
// cmd:"push_quality_plate",  value_type:MCV_QUALITY_FILTER_CONFIG_S*

/**
 * @brief push mode
 */
typedef enum MCV_PUSH_MODE_ENUM {
    MCV_PUSH_MODE_FAST = 1,
    MCV_PUSH_MODE_INTERVAL,
    MCV_PUSH_MODE_BEST,
} MCV_PUSH_MODE_E;

/**
 * @brief push strategy config
 */
typedef struct MCV_PUSH_STRATEGY_CONFIG_ST {
    MCV_PUSH_MODE_E push_mode;
    uint32_t interval_times;  // only for MCV_PUSH_MODE_INTERVAL or MCV_PUSH_MODE_FAST
    uint32_t push_counts;     // only for MCV_PUSH_MODE_INTERVAL or MCV_PUSH_MODE_FAST
    bool push_same_frame;     // false: CROSS_FRAME true: SAME_FRAME
} MCV_PUSH_STRATEGY_CONFIG_S;
// cmd:"push_strategy",  value_type:MCV_PUSH_STRATEGY_CONFIG_S*

/**
 * @brief push panorama config
 */
typedef struct MCV_PUSH_PANORAMA_CONFIG_ST {
    bool panorama_enabled;      // panoramic push enabled
    uint32_t panorama_quality;  // panoramic push image quality
} MCV_PUSH_PANORAMA_CONFIG_S;
// cmd:"push_panorama",  value_type:MCV_PUSH_PANORAMA_CONFIG_S*

typedef enum MCV_INTERRUPT_CTRL_ENUM {
    MCV_INTERRUPT_CTRL_REMOVE = 1,
    MCV_INTERRUPT_CTRL_RESTART,
} MCV_INTERRUPT_CTRL_E;

/**
 * @brief interrupt track config
 */
typedef struct MCV_INTERRUPT_TRACK_ST {
    uint64_t track_id;
    MCV_INTERRUPT_CTRL_E ctrl;
} MCV_INTERRUPT_TRACK_S;
// cmd:"interrupt_track",  value_type:MCV_INTERRUPT_TRACK_S*

/**
 * @brief Feature Encrypt type
 */
typedef enum MCV_FEATURE_ENCRYPT_TYPE_ENUM {
    MCV_FEATURE_ENCRYPT_TYPE_RAW = 0,
    MCV_FEATURE_ENCRYPT_TYPE_MEGFACE,
} MCV_FEATURE_ENCRYPT_TYPE_E;

/**
 * @brief Feature value Storage type
 */
typedef enum MCV_FEATURE_VAULE_TYPE_ENUM {
    MCV_FEATURE_VAULE_TYPE_UINT8 = 0,
    MCV_FEATURE_VAULE_TYPE_FLOAT32,
    MCV_FEATURE_VAULE_TYPE_INT8,
} MCV_FEATURE_VAULE_TYPE_E;

/**
 * @brief Feature Quant type
 */
typedef enum MCV_FEATURE_QUANT_TYPE_ENUM {
    MCV_FEATURE_QUANT_TYPE_NO = 0,
    MCV_FEATURE_QUANT_TYPE_YES,
} MCV_FEATURE_QUANT_TYPE_E;

typedef struct MCV_FEATURE_META_CONFIG_ST {
    bool is_append_feature_head;
    MCV_FEATURE_ENCRYPT_TYPE_E encrypt_state;
    MCV_FEATURE_VAULE_TYPE_E vaule_type;
    MCV_FEATURE_QUANT_TYPE_E quant_state;
} MCV_FEATURE_META_CONFIG_S;
// cmd:"face_feature_meta",  value_type:MCV_FEATURE_META_CONFIG_S*
// cmd:"human_feature_meta", value_type:MCV_FEATURE_META_CONFIG_S*
// cmd:"vehicle_feature_meta", value_type:MCV_FEATURE_META_CONFIG_S*
// cmd:"cycle_feature_meta",  value_type:MCV_FEATURE_META_CONFIG_S*

#ifdef __cplusplus
}
#endif