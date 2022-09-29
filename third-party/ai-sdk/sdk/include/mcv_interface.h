/*!
 *  @file: mcv_interface.h
 *  @brief: mcv interface file
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

#include "mcv_common_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @fn          mcv_init_sdk
 * @brief       initialize SDK
 * @param[in]   param //initialize param
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_init_sdk(const MCV_INIT_PARAM_S *param);

/**
 * @fn          mcv_deinit_sdk
 * @brief       deinitialize SDK //Please make sure mcv_release_stream_handle has
 *              been called to release initialized handle.
 * @param[in]   null
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_deinit_sdk(void);

/**
 * @fn          mcv_upload_model_package
 * @brief       upload model package
 * @param[in]   upload_model_package //model package info
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E
mcv_upload_model_package(const MCV_UPLOAD_MODEL_PACKAGE_S *upload_model_package);

/**
 * @fn          mcv_delete_model_package
 * @brief       delete model package
 * @param[in]   delete_model_package //model package info
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E
mcv_delete_model_package(const MCV_DELETE_MODEL_PACKAGE_S *delete_model_package);
/**
 * @fn          mcv_query_model_package
 * @brief       query model package
 * @param[in]   null
 * @param[out]  model_package_list // model package list
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_query_model_package(const MCV_MODEL_PACKAGE_LIST_S **model_package_list);

/**
 * @fn          mcv_set_log_config
 * @brief       set log config
 * @param[in]   log_param // log param
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_set_log_config(const MCV_LOG_PARAM_S *log_param);

/**
 * @fn          mcv_get_version
 * @brief       get sdk version
 * @param[in]   null
 * @param[out]  version // sdk version
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_get_version(const MCV_VERSION_INFO_S **version);

/**
 * @fn          mcv_get_capability
 * @brief       get sdk capability
 * @param[in]   null
 * @param[out]  capability // sdk capability
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_get_capability(const MCV_CAPABILITY_S **capability);

/**
 * @fn          mcv_create_stream_handle
 * @brief       create stream handle
 * @param[in]   handle_param //stream handle param
 * @param[out]  handle // stream handle
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_create_stream_handle(const MCV_STREAM_HANDLE_PARAM_S *handle_param,
                                                MCV_STREAM_HANDLE_T *handle);

/**
 * @fn          mcv_release_stream_handle
 * @brief       release stream handle
 * @param[in]   handle //stream handle
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_release_stream_handle(MCV_STREAM_HANDLE_T handle);

/**
 * @fn          mcv_set_stream_config
 * @brief       set stream config
 * @param[in]   handle //stream handle
 *              config //stream config,Implementation reference config_param.h
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_set_stream_config(MCV_STREAM_HANDLE_T handle,
                                             const MCV_STREAM_CONFIG_S *config);

/**
 * @fn          mcv_get_stream_config
 * @brief       get stream config
 * @param[in]   handle //stream handle
 * @param[out]  config //stream config
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_get_stream_config(MCV_STREAM_HANDLE_T handle,
                                             const MCV_STREAM_CONFIG_S **config);

/**
 * @fn          mcv_interrupt_track
 * @brief       interrupt track
 * @param[in]   handle //stream handle
 *              interrupt_track //interrupt track
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_interrupt_track(MCV_STREAM_HANDLE_T handle,
                                           const MCV_INTERRUPT_TRACK_S *interrupt_track);

/**
 * @fn          mcv_reset
 * @brief       track reset
 * @param[in]   handle //stream handle
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_reset(MCV_STREAM_HANDLE_T handle);
/**
 * @fn          mcv_send_stream_frame
 * @brief       send stream frame
 * @param[in]   handle //stream handle
 *              frame //video/image frame
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure,MCV_RET_FRAME_BUFFER_FULL:frame
 * buffer full
 */
MCV_API MCV_RET_CODE_E mcv_send_stream_frame(MCV_STREAM_HANDLE_T handle, MCV_FRAME_S *frame);

/**
 * @fn          mcv_get_detect_result
 * @brief       get detect result
 *              if you want to get detect result,you must set
 * MCV_HVCFP_STREAM_CONFIG_S::detect_result_interface_enabled true
 * @param[in]   handle //stream handle
 * @param[out]  algorithm_result //algorithm result
 * @param[in]   timedwait_millisecond // >0:timed_wait;   =0: do not wait;   <0: wait forever
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure,MCV_RET_EMPTY_RESULT:result is empty
 */
MCV_API MCV_RET_CODE_E mcv_get_detect_result(MCV_STREAM_HANDLE_T handle,
                                             MCV_ALGORITHM_RESULT_S **algorithm_result,
                                             int timedwait_millisecond);

/**
 * @fn          mcv_get_algorithm_result
 * @brief       get algorithm result
 * @param[in]   handle //stream handle
 * @param[out]  algorithm_result //algorithm result
 * @param[in]   timedwait_millisecond // >0:timed_wait;   =0: do not wait;   <0: wait forever
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure,MCV_RET_EMPTY_RESULT:result is empty
 */
MCV_API MCV_RET_CODE_E mcv_get_algorithm_result(MCV_STREAM_HANDLE_T handle,
                                                MCV_ALGORITHM_RESULT_S **algorithm_result,
                                                int timedwait_millisecond);

/**
 * @fn          mcv_register_detect_result_callback
 * @brief       register detect result callback
 * @param[in]   handle //stream handle
 *              callback //result callback func
 *              user_data //user data
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_register_detect_result_callback(MCV_STREAM_HANDLE_T handle,
                                                           ALGORITHM_RESULT_CALLBACK_FUNC callback,
                                                           void *user_data);

/**
 * @fn          mcv_register_algorithm_result_callback
 * @brief       register algorithm result callback
 * @param[in]   handle //stream handle
 *              callback //result callback func
 *              user_data //user data
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_register_algorithm_result_callback(
    MCV_STREAM_HANDLE_T handle, ALGORITHM_RESULT_CALLBACK_FUNC callback, void *user_data);
/**
 * @fn          mcv_delete
 * @brief       free memory
 * @param[in]   p //sdk returned pointer address
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_delete(void *p);

/**
 * @fn          mcv_check_framework_and_plugin_version
 * @brief       check framework version and plugin version
 * @param[in]   p //sdk returned pointer address
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_check_framework_and_plugin_version(const char *library_name);

#ifdef __cplusplus
}
#endif
