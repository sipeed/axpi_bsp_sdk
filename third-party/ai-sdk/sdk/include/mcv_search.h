#ifndef MCV_SEARCH_H_
#define MCV_SEARCH_H_
#include "mcv_common_struct.h"
#include "mcv_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * type of search supported
 */
typedef enum MCV_SEARCH_TYPE_ENUM {
    MCV_SEARCH_TYPE_NPU = 0,
    MCV_SEARCH_TYPE_CPU,
    MCV_SEARCH_TYPE_GPU,    // mc40 not support
    MCV_SEARCH_TYPE_MAX
} MCV_SEARCH_TYPE_E;

/**
 * tag filter param
 */
typedef enum MCV_SEARCH_FILTER_TYPE_ENUM {
    MCV_SEARCH_FILTER_TYPE_USER_VERDICT_TAG = 1,    // the user defines the tag content
    MCV_SEARCH_FILTER_TYPE_MAX
} MCV_SEARCH_FILTER_TYPE_E;

typedef struct MCV_SEARCH_FILTER_ST {
    MCV_SEARCH_FILTER_TYPE_E type;
    uint32_t tag_size;    // if tag_size > 0 Enable filtering policy.
} MCV_SEARCH_FILTER_S;

typedef struct MCV_SEARCH_CREATE_GROUP_OPTPARAM_ST {
    MCV_SEARCH_TYPE_E search_type;
    char* model_path;    // mc40 ignore it
    MCV_SEARCH_FILTER_S filter_opt;
} MCV_SEARCH_CREATE_GROUP_OPTPARAM_S;

/**
 * group insert feature param
 */
typedef struct MCV_SEARCH_INSERT_FEATURE_OPTPARAM_ST {
    uint32_t batch_size;
    uint64_t* object_ids;    // 1 dimensional[batch_size] continuous storage
    uint8_t* features;       // 1 dimensional[batch_size*feature_size] continuous storage
    void** object_infos;    // 1 dimensional[batch_size] continuous storage // 'object_infos' can be
                            // null
    uint8_t* base_tags;     // 1 dimensional[batch_size*tag_size] continuous storage //
                            // 'base_tag' must not be empty if Enable filtering policy
} MCV_SEARCH_INSERT_FEATURE_OPTPARAM_S;

// if `USER_VERDICT_FUNC(verdict_userdatas[search_index], base_tags[base_id]) >0` , it's going to
// search.
typedef int (*USER_VERDICT_FUNC)(void* verdict_userdata, const uint8_t* base_tag);

typedef struct MCV_SEARCH_RUN_OPTPARAM_ST {
    uint32_t batch_size;
    int top_k;
    uint8_t* features;    // 1 dimensional[batch_size*feature_size] continuous storage
    // If the following types are valid, the filter policy will be activated. If not, the filter
    // policy will not be performed
    void** verdict_userdatas;    // 1 dimensional[batch_size] continuous storage
    USER_VERDICT_FUNC verdict_func;
} MCV_SEARCH_RUN_OPTPARAM_S;

/**
 * result of search
 */
typedef struct MCV_GROUP_SEARCH_RESULT_ST {
    size_t batch_size;      // object number of search
    size_t top_k;           // object with k highest scores in object group
    float* scores;          // scorces of object matching in object group
    uint64_t* object_id;    // object id of object matching in object group
    void** info;            // information of object matching in object group(pass-through by user)
} MCV_GROUP_SEARCH_RESULT_S;

/**
 * @fn          mcv_search_init
 * @brief       initialize search
 * @param[in]   null
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search_init();

/**
 * @fn          mcv_search_deinit
 * @brief       deinitialize search
 * @param[in]   null
 * @param[out]  null
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search_deinit();
/**
 * @fn          mcv_search_create
 * @brief       create group in memory
 * @param[in]   groupid //group ID
 *              optparam.search_type //search_type type
 *              optparam.model_path //model path  //mc40 ignore it
 *              optparam.filter_opt //filter policy
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search_create(uint64_t groupid,
                                         MCV_SEARCH_CREATE_GROUP_OPTPARAM_S* optparam);

/**
 * @fn          mcv_search_destroy
 * @brief       destroy group in memory
 * @param[in]   groupid //group ID
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search_destroy(uint64_t groupid);

/**
 * @fn          mcv_search_feature_insert
 * @brief       insert object to the group and add the binding of the object to the group
 * @param[in]   groupid  //group ID
 *              optparam.batch_size   //batch_size
                optparam.object_ids   //object ID
                optparam.features  //feature to search
                optparam.object_infos //pass-through information when searching
                optparam.base_tags //user defines the tag content
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search_feature_insert(uint64_t groupid,
                                                 MCV_SEARCH_INSERT_FEATURE_OPTPARAM_S* optparam);

/**
 * @fn          mcv_search_feature_delete
 * @brief       delete object from group
 * @param[in]   groupid  //group ID
                object_id   //object ID
 * @param[out]  info     //pass-through information when searching
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search_feature_delete(uint64_t groupid, uint64_t object_id, void** info);
/**
 * @fn          mcv_search
 * @brief       search object information in the groups
 * @param[in]   groupid     //group ID
                optparam.features           //feature information of face
                optparam.batch_size         //number of feature information
                optparam.top_k              //faces with k highest scores in face group
                optparam.verdict_userdatas  //filter verdict func param
                optparam.verdict_func       //user defines filter verdict func
 * @param[out]  result //return result of algorithm processing //call mcv_delete to be released
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_search(uint64_t groupid, MCV_SEARCH_RUN_OPTPARAM_S* optparam,
                                  MCV_GROUP_SEARCH_RESULT_S** result);
#ifdef __cplusplus
}
#endif

#endif
