#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <memory.h>
#include <math.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include <cfenv>
#include <cmath>
#include "img_helper.h"

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string> ("A", '\0', "matrix file", false);
    args.add<std::string> ("B", '\0', "matrix file", false);
    args.add<AX_U32>("row", '\0', "rows of matrix", true);
    args.add<AX_U32>("column", '\0', "columns of matrix", true);
    args.add<std::string> ("input0-data-type", '\0', "int16, uint16", false, "uint16");
    args.add<std::string> ("input1-data-type", '\0', "uint8", false, "uint8");
    args.add<std::string> ("output-data-type", '\0', "int16, uint16", false, "uint16");
    args.add<AX_U32>("repeat", '\0', "HadamardProduct execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, true);
    args.add<std::string> ("output", '\0', "output directory", false);

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");
    std::string matrix_a = args.get<std::string>("A");
    std::string matrix_b = args.get<std::string>("B");
    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");
    AX_NPU_CV_DataType src0_data_type = get_data_type(args.get<std::string>("input0-data-type"));
    AX_NPU_CV_DataType src1_data_type = get_data_type(args.get<std::string>("input1-data-type"));
    std::string output = args.get<std::string>("output");

    if(src0_data_type != AX_NPU_CV_DT_UINT16 && src0_data_type != AX_NPU_CV_DT_INT16){
        printf("unsupported data type %d for base matrix\n", (int)src0_data_type);
        assert(0);
    }
    if(src1_data_type != AX_NPU_CV_DT_UINT8){
        printf("unsupported data type %d for base matrix\n", (int)src1_data_type);
        assert(0);
    }
    AX_NPU_CV_Matrix2D src0_matrix_2d = create_matrix_universal(matrix_a, nRow, nColumn, src0_data_type);
    AX_NPU_CV_Matrix2D src1_matrix_2d = create_matrix_universal(matrix_b, nRow, nColumn, src1_data_type);
    assert(src0_matrix_2d.nRows == src1_matrix_2d.nRows);
    assert(src0_matrix_2d.nColumns == src1_matrix_2d.nColumns);

    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    srand(time(0));

    AX_NPU_CV_DataType result_data_type = get_data_type(args.get<std::string>("output-data-type"));
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, result_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_HadamardProduct(virtual_npu_mode_type, &src0_matrix_2d, &src1_matrix_2d, &result_matrix_2d));
        if(nRepeat > 1 && i == 0){
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
        sprintf(fname, "%s/result_%dx%d.%s_hadamard_%dx%d.%s.npu_mode_%d.%s",
            output.c_str(),
            nRow, nColumn, get_data_type_name(src0_data_type).c_str(),
            nRow, nColumn, get_data_type_name(src1_data_type).c_str(),
            virtual_npu_mode_type,
            get_data_type_name(result_data_type).c_str()
            );
        dump_matrix_to_file(fname, result_matrix_2d);
    }

    if (bCheck) {
        // Calculate GT
        int64_t* gt_data = new int64_t[nRow * nColumn];
        memset(gt_data, 0, nRow * nColumn * sizeof(int64_t));
        if (src0_data_type == AX_NPU_CV_DT_UINT16) {
            uint16_t *src0_data = (uint16_t *)src0_matrix_2d.pVir;
            uint8_t *src1_data = (uint8_t *)src1_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (virtual_npu_mode_type == AX_NPU_MODEL_TYPE_1_1_1) {
                    // The rounding method in mode-type_1_1_1 is round_half_to_even
                    std::fesetround(FE_TONEAREST);
                    gt_data[i] = std::nearbyint(src0_data[i] * src1_data[i] / 128.0);
                }
                else
                    // The rounding method in other mode-types is round_half_up
                    gt_data[i] = floor(src0_data[i] * src1_data[i] / 128.0 + 0.5);
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
        } else if (src0_data_type == AX_NPU_CV_DT_INT16) {
            int16_t *src0_data = (int16_t *)src0_matrix_2d.pVir;
            uint8_t *src1_data = (uint8_t *)src1_matrix_2d.pVir;
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (virtual_npu_mode_type == AX_NPU_MODEL_TYPE_1_1_1) {
                    std::fesetround(FE_TONEAREST);
                    gt_data[i] = std::nearbyint(src0_data[i] * src1_data[i] / 128.0);
                }
                else
                    gt_data[i] = floor(src0_data[i] * src1_data[i] / 128.0 + 0.5);
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

    release_matrix_memory(src0_matrix_2d);
    release_matrix_memory(src1_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
