#include <cstdio>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory.h>
#include <math.h>
#include <sys/stat.h>
#include <float.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"


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
    args.add<std::string> ("output", '\0', "Output directory for result", false, "");
    args.add<AX_F32>("Alpha", '\0', "scalar coefficient", false, 1);
    args.add<AX_U32>("repeat", '\0', "MatMul execution count", false, 1);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_matmul_precision.ax620a --mode=disable --mode-type=disable --A A.txt --B B.txt --output .

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_F32 fAlpha = args.get<AX_F32>("Alpha");
    int nRepeat = args.get<AX_U32>("repeat");
    std::string out_dir = args.get<std::string>("output");

    int A_rows = args.get<AX_U32>("A_rows");
    int A_columns = args.get<AX_U32>("A_column");
    int B_rows = args.get<AX_U32>("B_rows");
    std::string input_a = args.get<std::string>("A");
    std::string input_b = args.get<std::string>("B");
    AX_NPU_CV_Matrix2D base_matrix_2d = create_matrix_universal(input_a, A_rows, A_columns, AX_NPU_CV_DT_FLOAT32);
    AX_NPU_CV_Matrix2D query_matrix_2d = create_matrix_universal(input_b, B_rows, A_columns, AX_NPU_CV_DT_FLOAT32);
    assert(base_matrix_2d.nColumns == query_matrix_2d.nColumns);

    int nM = base_matrix_2d.nRows;
    int nK = base_matrix_2d.nColumns;
    int nN = query_matrix_2d.nRows;

    AX_NPU_CV_DataType result_data_type = AX_NPU_CV_DT_FLOAT32;
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

    if (!out_dir.empty()) {
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
            delete[] gt_data;
            return 1;
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

        {
            char path[256];
            sprintf(path, "%s/tolerance_%dx%d.txt", out_dir.c_str(), result_matrix_2d.nRows, result_matrix_2d.nColumns);

            std::ofstream ofs(path);
            if (!ofs.is_open() || ofs.fail()) {
                printf("cannot open file \"%s\" for writing\n", path);
                assert(0);
            }
            ofs << result_matrix_2d.nRows << " ";
            ofs << result_matrix_2d.nColumns << std::endl;
            float max_tolerance = FLT_MIN;
            float* result_data = (float*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows * result_matrix_2d.nColumns; ++i) {
                if (fabs(result_data[i] - gt_data[i]) > max_tolerance) {
                    max_tolerance = fabs(result_data[i] - gt_data[i]);
                }
                ofs << result_data[i] - gt_data[i] << " ";
            }

            printf("The matmul result max tolerance with GT is %f, dumpped tolerance as '%s'!\n", max_tolerance, path);
        }
    }

    release_matrix_memory(base_matrix_2d);
    release_matrix_memory(query_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    delete[] gt_data;
    gt_data = nullptr;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
