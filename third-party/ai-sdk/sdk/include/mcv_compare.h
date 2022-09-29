#ifndef MCV_COMPARE_H_
#define MCV_COMPARE_H_
#include "mcv_common_struct.h"
#include "mcv_macro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCV_COMPARE_SCORE_ST {
    size_t top_k;      // the top k scores
    float *scores;     // scores, top_k
    int *object_id;    // object_id, top_k
} MCV_COMPARE_SCORE_S;

typedef struct MCV_FEATURE_COMPARE_RESULT_ST {
    MCV_COMPARE_SCORE_S *score_list;    // list of comparison results
    int score_list_size;                // number of results
} MCV_FEATURE_COMPARE_RESULT_S;

/**
 * @fn          mcv_compare_feature
 * @brief       object comparison 1:1
 * @param[in]   features_1  //feature of object 1
                features_2  //feature of object 2
 * @param[out]  score  //score of comparing two faces(0-100) //the higher the score is,
 * the more similar the objects are.
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_compare_feature(MCV_FEATURE_S *features_1, MCV_FEATURE_S *features_2,
                                           float *score);

/**
 * @fn          mcv_compare_feature_mxn
 * @brief       object comparison M:N
 * @param[in]   features_array1  //feature array of object 1
                array1_count     //number of elements in features_array1
                features_array2  //feature array of object 2
                array2_count     //number of elements in features_array2
                top_k            //the top k scores
 * @param[out]  compare_result   //return the result of the algorithm processing
 //call mcv_delete to be released
 * @return      MCV_RET_SUCCESS:success,MCV_RET_FAILURE:failure
 */
MCV_API MCV_RET_CODE_E mcv_compare_feature_mxn(MCV_FEATURE_S *features_array1, int array1_count,
                                               MCV_FEATURE_S *features_array2, int array2_count,
                                               int top_k,
                                               MCV_FEATURE_COMPARE_RESULT_S **compare_result);

#ifdef __cplusplus
}
#endif

#endif
