#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <memory.h>
#include <math.h>
#include <sys/stat.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<AX_U32>("row", '\0', "Rows of matrix", true);
    args.add<AX_U32>("column", '\0', "Columns of matrix", true);
    args.add<std::string>("A", '\0', "input matrix file name", false);
    args.add<std::string> ("input-data-type", '\0', "uint8, int16, uint16", false, "uint16");
    args.add<std::string> ("output-data-type", '\0', "uint8, int16, uint16", false, "uint16");
    args.add<AX_S32>("low", '\0', "Low threshold", true);
    args.add<AX_S32>("high", '\0', "High threshold", true);
    args.add<AX_S32>("min", '\0', "Minimum value", true);
    args.add<AX_S32>("mid", '\0', "Middle value", true);
    args.add<AX_S32>("max", '\0', "Maximum value", true);
    args.add<std::string>("thresh_mode", '\0', "Thresh mode", false, "binary");
    args.add<AX_U32>("repeat", '\0', "Thresh execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, true);
    args.add<std::string>("output", '\0', "output directory", false);

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_NPU_CV_DataType src_data_type = get_data_type(args.get<std::string>("input-data-type"));
    std::string matrix_file = args.get<std::string>("A");
    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");
    AX_NPU_CV_Matrix2D src_matrix_2d = create_matrix_universal(matrix_file, nRow, nColumn, src_data_type);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");
    int low = args.get<AX_S32>("low");
    int high = args.get<AX_S32>("high");
    int min = args.get<AX_S32>("min");
    int mid = args.get<AX_S32>("mid");
    int max = args.get<AX_S32>("max");
    std::string thresh_mode = args.get<std::string>("thresh_mode");
    std::string output_dir = args.get<std::string>("output");

    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    srand(time(0));

    if(src_data_type != AX_NPU_CV_DT_UINT8 && src_data_type != AX_NPU_CV_DT_UINT16 && src_data_type != AX_NPU_CV_DT_INT16) {
        printf("unsupported data type %d for base matrix\n", (int)src_data_type);
        assert(0);
    }

    AX_NPU_CV_DataType result_data_type = get_data_type(args.get<std::string>("output-data-type"));
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, result_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    AX_NPU_CV_ThreshContext tContext = nullptr;
    AX_NPU_CV_ThreshParams tThreshParams;
    if (thresh_mode == "binary") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_BINARY;
    else if (thresh_mode == "clip_min") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_CLIP_MIN;
    else if (thresh_mode == "clip_max") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_CLIP_MAX;
    else if (thresh_mode == "min_mid_max") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_MIN_MID_MAX;
    else if (thresh_mode == "ori_mid_max") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_ORI_MID_MAX;
    else if (thresh_mode == "min_ori_max") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_MIN_ORI_MAX;
    else if (thresh_mode == "min_mid_ori") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_MIN_MID_ORI;
    else if (thresh_mode == "ori_mid_ori") tThreshParams.eMode = AX_NPU_CV_THRESH_MODE_ORI_MID_ORI;
    tThreshParams.iLowThresh = low;
    tThreshParams.iHighThresh = high;
    tThreshParams.iMinValue = min;
    tThreshParams.iMidValue = mid;
    tThreshParams.iMaxValue = max;
    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_Thresh(&tContext, virtual_npu_mode_type, &src_matrix_2d, &tThreshParams, &result_matrix_2d));
        if(nRepeat > 1 && i ==0 ){
            time_start = std::chrono::system_clock::now();
        }
    }
    auto time_end = std::chrono::system_clock::now();
    if (nRepeat > 1) nRepeat--;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start) / nRepeat;
    AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
    print_work_cycle(cur);
    printf("\n**************************************************\n");
    printf("Run task took %lld us (%d rounds for average)\n", duration.count(), nRepeat);
    printf("**************************************************\n");

    if(!output_dir.empty()){
        struct stat info;
        if (stat(output_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", output_dir.c_str());
        }else{
            char fname[256];
            sprintf(fname, "%s/npu_thresh.%dx%d.%s.low-%d_hight-%d.%d_%d_%d.%s.npu_mode_%d.%s",
                output_dir.c_str(),
                nRow, nColumn, get_data_type_name(src_data_type).c_str(),
                low, high,
                min, mid, max,
                thresh_mode.c_str(),
                virtual_npu_mode_type,
                get_data_type_name(result_data_type).c_str());
            dump_matrix_to_file(fname, result_matrix_2d);
        }
    }

    if (bCheck) {
        // Calculate GT
        int64_t* gt_data = new int64_t[nRow * nColumn];
        memset(gt_data, 0, nRow * nColumn * sizeof(int64_t));
        if (src_data_type == AX_NPU_CV_DT_UINT16) {
            uint16_t *src_data = (uint16_t *)src_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_BINARY) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_CLIP_MAX) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_CLIP_MIN) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else gt_data[i] = src_data[i];
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_MID_MAX) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_ORI_MID_MAX) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_ORI_MAX) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = src_data[i];
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_MID_ORI) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = src_data[i];
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_ORI_MID_ORI) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = src_data[i];
                }
            }
        } else if (src_data_type == AX_NPU_CV_DT_INT16) {
            int16_t *src_data = (int16_t *)src_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_BINARY) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_CLIP_MAX) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_CLIP_MIN) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else gt_data[i] = src_data[i];
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_MID_MAX) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_ORI_MID_MAX) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_ORI_MAX) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = src_data[i];
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_MID_ORI) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = src_data[i];
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_ORI_MID_ORI) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = src_data[i];
                }
            }
        } else if (src_data_type == AX_NPU_CV_DT_UINT8) {
            uint8_t *src_data = (uint8_t *)src_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_BINARY) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_CLIP_MAX) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_CLIP_MIN) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else gt_data[i] = src_data[i];
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_MID_MAX) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_ORI_MID_MAX) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_ORI_MAX) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = src_data[i];
                    else gt_data[i] = max;
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_MIN_MID_ORI) {
                    if (src_data[i] <= low) gt_data[i] = min;
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = src_data[i];
                } else if (tThreshParams.eMode == AX_NPU_CV_THRESH_MODE_ORI_MID_ORI) {
                    if (src_data[i] <= low) gt_data[i] = src_data[i];
                    else if (src_data[i] <= high) gt_data[i] = mid;
                    else gt_data[i] = src_data[i];
                }
            }
        }
        printf("**************************************************\n");

        bool match_gt = true;

        if (result_data_type == AX_NPU_CV_DT_INT16) {
            int16_t* result_data = (int16_t*)result_matrix_2d.pVir;
            // Check GT with result
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (result_data[i] != gt_data[i]) {
                    match_gt = false;
                    break;
                }
            }
        } else if (result_data_type == AX_NPU_CV_DT_UINT16) {
            uint16_t* result_data = (uint16_t*)result_matrix_2d.pVir;
            // Check GT with result
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (result_data[i] != gt_data[i]) {
                    match_gt = false;
                    break;
                }
            }
        } else if (result_data_type == AX_NPU_CV_DT_UINT8) {
            uint8_t* result_data = (uint8_t*)result_matrix_2d.pVir;
            // Check GT with result
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (result_data[i] != gt_data[i]) {
                    match_gt = false;
                    break;
                }
            }
        } else {
            assert(0);
        }

        if (match_gt) {
            printf("The result match with GT!\n");
        } else {
            printf("The result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }
        delete[] gt_data;

    }

    release_matrix_memory(src_matrix_2d);
    AX_NPU_CV_DestroyThreshContext(tContext); // 程序退出前释放 Thresh 算子上下文
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
