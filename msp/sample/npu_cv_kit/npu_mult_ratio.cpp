#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <memory.h>
#include <math.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<AX_U32>("row", '\0', "rows of matrix A", true);
    args.add<AX_U32>("column", '\0', "columns of matrix A", true);
    args.add<std::string>("A", '\0', "matrix file name", false);
    args.add<std::string> ("input-data-type", '\0', "int16, uint16", false, "uint16");
    args.add<std::string> ("output-data-type", '\0', "int16, uint16", false, "uint16");
    args.add<AX_F64>("ratio", '\0', "Scale ratio", true);
    args.add<AX_U32>("repeat", '\0', "Affine execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, true);
    args.add<std::string>("output", '\0', "output directory", false);

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");
    std::string matrix_file = args.get<std::string>("A");
    AX_NPU_CV_DataType src_data_type = get_data_type(args.get<std::string>("input-data-type"));
    AX_NPU_CV_Matrix2D src_matrix_2d = create_matrix_universal(matrix_file, nRow, nColumn, src_data_type);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");
    double ratio = args.get<AX_F64>("ratio");
    std::string output_dir = args.get<std::string>("output");

    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    srand(time(0));

    if(src_data_type != AX_NPU_CV_DT_UINT16 and src_data_type != AX_NPU_CV_DT_INT16) {
        printf("unsupported data type %d for base matrix\n", (int)src_data_type);
        assert(0);
    }

    AX_NPU_CV_DataType result_data_type = get_data_type(args.get<std::string>("output-data-type"));
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, result_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_MultRatio(virtual_npu_mode_type, &src_matrix_2d, &result_matrix_2d, ratio));
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

    {
        char fname[128];
        sprintf(fname, "%s/mult_ratio.%dx%d.%s.ratio_%.2f.%s",
            output_dir.c_str(),
            nRow, nColumn, get_data_type_name(src_data_type).c_str(),
            ratio,
            get_data_type_name(result_data_type).c_str());
        dump_matrix_to_file(fname, result_matrix_2d);
    }

    if (bCheck) {
        // Calculate GT
        int64_t* gt_data = new int64_t[nRow * nColumn];
        memset(gt_data, 0, nRow * nColumn * sizeof(int64_t));
        ratio = (int64_t)(ratio * 4096) / 4096.0;
        if (src_data_type == AX_NPU_CV_DT_UINT16) {
            uint16_t *src_data = (uint16_t *)src_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                gt_data[i] = floor(src_data[i] * ratio + 0.5);
                if (result_data_type == AX_NPU_CV_DT_INT16) {
                    // Clip to [INT16_MIN, INT16_MAX]
                    if (gt_data[i] > INT16_MAX) gt_data[i] = INT16_MAX;
                    if (gt_data[i] < INT16_MIN) gt_data[i] = INT16_MIN;
                } else if (result_data_type == AX_NPU_CV_DT_UINT16) {
                    // Clip to [0, UINT16_MAX]
                    if (gt_data[i] > UINT16_MAX) gt_data[i] = UINT16_MAX;
                    if (gt_data[i] < 0) gt_data[i] = 0;
                } else {
                    assert(0);
                }
            }
        } else if (src_data_type == AX_NPU_CV_DT_INT16) {
            int16_t *src_data = (int16_t *)src_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                gt_data[i] = floor(src_data[i] * ratio + 0.5);
                if (result_data_type == AX_NPU_CV_DT_INT16) {
                    // Clip to [INT16_MIN, INT16_MAX]
                    if (gt_data[i] > INT16_MAX) gt_data[i] = INT16_MAX;
                    if (gt_data[i] < INT16_MIN) gt_data[i] = INT16_MIN;
                } else if (result_data_type == AX_NPU_CV_DT_UINT16) {
                    // Clip to [0, UINT16_MAX]
                    if (gt_data[i] > UINT16_MAX) gt_data[i] = UINT16_MAX;
                    if (gt_data[i] < 0) gt_data[i] = 0;
                } else {
                    assert(0);
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
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
