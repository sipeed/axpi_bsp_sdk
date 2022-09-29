#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <memory.h>
#include <math.h>
#include <float.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<AX_U32>("A_rows", '\0', "rows of matrix A", true);
    args.add<AX_U32>("A_columns", '\0', "columns of matrix A", true);
    args.add<std::string> ("A", '\0', "Input file of matrix A", false);
    args.add<AX_U32>("B_rows", '\0', "rows of matrix B", true);
    args.add<std::string> ("B", '\0', "Input file of matrix B", false);
    args.add<std::string> ("input-data-type", '\0', "int8, float", false, "int8");
    args.add<std::string> ("output-data-type", '\0', "int16, float", false, "int16");
    args.add<AX_F32>("Alpha", '\0', "scalar coefficient", false, 1);
    args.add<AX_U32>("repeat", '\0', "MatMul execution count", false, 1);
    args.add<std::string> ("output", '\0', "Output directory for result", false, "");
    args.add<bool>("check", '\0', "check gt", false, false);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_matmul.ax620a  --mode=disable --mode-type=disable --input-data-type=float --output-data-type=float --A a.txt --B b.txt --repeat=1
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_matmul.ax620a  --mode=1_1 --mode-type=1_1_1 --input-data-type=int8 --output-data-type=int16 --A a.txt --B b.txt --repeat=1
    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int A_rows = args.get<AX_U32>("A_rows");
    int A_columns = args.get<AX_U32>("A_columns");
    int B_rows = args.get<AX_U32>("B_rows");
    std::string matrix_a = args.get<std::string>("A");
    std::string matrix_b = args.get<std::string>("B");
    AX_NPU_CV_DataType base_data_type = get_data_type(args.get<std::string>("input-data-type"));
    AX_NPU_CV_DataType query_data_type = get_data_type(args.get<std::string>("input-data-type"));
    std::string out_dir = args.get<std::string>("output");
    bool bcheck = args.get<bool>("check");

    AX_NPU_CV_Matrix2D base_matrix_2d = create_matrix_universal(matrix_a, A_rows,  A_columns, base_data_type);
    AX_NPU_CV_Matrix2D query_matrix_2d = create_matrix_universal(matrix_b, B_rows, A_columns, query_data_type);
    int nM = base_matrix_2d.nRows;
    int nK = base_matrix_2d.nColumns;
    int nN = query_matrix_2d.nRows;

    AX_F32 fAlpha = args.get<AX_F32>("Alpha");
    int nRepeat = args.get<AX_U32>("repeat");
    printf("Input M %d, K %d, N %d, Alpha %f, repeat %d\n", nM, nK, nN, fAlpha, nRepeat);

    srand(time(0));

    if(base_data_type != AX_NPU_CV_DT_INT8 && base_data_type != AX_NPU_CV_DT_FLOAT32){
        printf("unsupported data type %d for base matrix\n", (int)base_data_type);
        assert(0);
    }

    if(query_data_type != AX_NPU_CV_DT_INT8 && query_data_type != AX_NPU_CV_DT_FLOAT32){
        printf("unsupported data type %d for query matrix\n", (int)query_data_type);
        assert(0);
    }

    AX_NPU_CV_DataType result_data_type = get_data_type(args.get<std::string>("output-data-type"));

    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nM, nN, result_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_MatMul(virtual_npu_mode_type, fAlpha, &base_matrix_2d, &query_matrix_2d, &result_matrix_2d));
    }
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start) / nRepeat;
    AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
    print_work_cycle(cur);
    printf("\n**************************************************\n");
    printf("Run task took %lld us (%d rounds for average)\n", duration.count(), nRepeat);
    printf("**************************************************\n");

    if (!out_dir.empty()) {
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
            assert(0);
        }

        {
            char fn[256];
            sprintf(fn, "%s/result.a_%dx%d.b_%dx%d.c_%dx%d.alfa_%.2f.npu_mode_%d.%s",
                out_dir.c_str(),
                nM, nK, nN, nK, nM, nN,
                fAlpha, virtual_npu_mode_type,
                get_data_type_name(result_data_type).c_str());
            dump_matrix_to_file(fn, result_matrix_2d);
        }
    }

    if(bcheck){
        if (base_data_type == AX_NPU_CV_DT_INT8) {
            // Calculate GT
            int32_t* gt_data = new int32_t[nM * nN];
            memset(gt_data, 0, nM * nN * sizeof(int32_t));

            int8_t *base_data = (int8_t *)base_matrix_2d.pVir;
            int8_t *query_data = (int8_t *)query_matrix_2d.pVir;
            for (int m = 0; m < nM; ++m) {
                for (int n = 0; n < nN; ++n) {
                    for (int k = 0; k < nK; ++k) {
                        gt_data[m*nN + n] += (int32_t)base_data[m*nK + k] * (int32_t)query_data[n*nK + k];
                    }
                    gt_data[m*nN + n] *= fAlpha;

                    // Clip to [INT16_MIN, INT16_MAX]
                    if (gt_data[m*nN + n] > INT16_MAX) gt_data[m*nN + n] = INT16_MAX;
                    if (gt_data[m*nN + n] < INT16_MIN) gt_data[m*nN + n] = INT16_MIN;
                }
            }

            printf("dump matmul GT results:\n");
            for (int i = 0; i < nM; ++i) {
                for (int j = 0; j < nN; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%d ", gt_data[i * nN + j]);
                }
                printf("\n");
            }

            printf("**************************************************\n");
            printf("**************************************************\n");

            printf("dump matmul results:\n");
            int16_t* result_data = (int16_t*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows; ++i) {
                for (AX_U32 j = 0; j < result_matrix_2d.nColumns; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%d ", result_data[i * result_matrix_2d.nColumns + j]);
                }
                printf("\n");
            }

            // Check GT with result
            bool match_gt = true;
            for (int i = 0; i < nM * nN; ++i) {
                if (result_data[i] != gt_data[i]) {
                    match_gt = false;
                    break;
                }
            }
            if (match_gt) {
                printf("The matmul result match with GT!\n");
            } else {
                printf("The matmul result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            delete[] gt_data;

        } else if (base_data_type == AX_NPU_CV_DT_FLOAT32) {
            // Calculate GT
            float* gt_data = new float[nM * nN];
            memset(gt_data, 0, nM * nN * sizeof(float));

            float *base_data = (float *)base_matrix_2d.pVir;
            float *query_data = (float *)query_matrix_2d.pVir;
            for (int m = 0; m < nM; ++m) {
                for (int n = 0; n < nN; ++n) {
                    for (int k = 0; k < nK; ++k) {
                        gt_data[m*nN + n] += base_data[m*nK + k] * query_data[n*nK + k];
                    }
                    gt_data[m*nN + n] *= fAlpha;
                }
            }

            printf("dump matmul GT results:\n");
            for (int i = 0; i < nM; ++i) {
                for (int j = 0; j < nN; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%.3f ", gt_data[i * nN + j]);
                }
                printf("\n");
            }

            printf("**************************************************\n");
            printf("**************************************************\n");

            printf("dump matmul results:\n");
            float* result_data = (float*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows; ++i) {
                for (AX_U32 j = 0; j < result_matrix_2d.nColumns; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%.3f ", result_data[i * result_matrix_2d.nColumns + j]);
                }
                printf("\n");
            }

            // Check GT with result
            bool match_gt = true;
            for (int i = 0; i < nM * nN; ++i) {
                if (fabs(result_data[i] - gt_data[i]) > 1e-2) {
                    match_gt = false;
                    break;
                }
            }
            if (match_gt) {
                printf("The matmul result match with GT!\n");
            } else {
                printf("The matmul result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            delete[] gt_data;
        }
    }
    release_matrix_memory(base_matrix_2d);
    release_matrix_memory(query_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
