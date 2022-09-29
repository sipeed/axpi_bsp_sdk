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

#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((a)<(b)?(b):(a))

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<AX_U32>("row", '\0', "rows of matrix A", true);
    args.add<AX_U32>("column", '\0', "columns of matrix A", true);
    args.add<std::string> ("A", '\0', "Input file of matrix A", false);
    args.add<std::string> ("B", '\0', "Input file of matrix B", false);
    args.add<std::string> ("input-data-type", '\0', "input data type", false, "uint8");
    args.add<std::string> ("output-data-type", '\0', "output data type(uint8,int8)", false, "uint8");
    args.add<std::string> ("arith", '\0', "arith method(add,sub)", true, "add");
    args.add<std::string> ("output", '\0', "Output directory for result", false, "");
    args.add<AX_U32>("repeat", '\0', "MatArith execution count", false, 1);
    args.add<AX_U32>("check", '\0', "check", false, 0);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_matarithax620a --mode=disable --mode-type=disable --M 100 --N 100 --output .

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRepeat = args.get<AX_U32>("repeat");
    int bCheck = args.get<AX_U32>("check");
    std::string out_dir = args.get<std::string>("output");

    int M = args.get<AX_U32>("row");
    int N = args.get<AX_U32>("column");
    std::string input_a = args.get<std::string>("A");
    std::string input_b = args.get<std::string>("B");
    std::string arith_method = args.get<std::string>("arith");
    std::string input_type = args.get<std::string>("input-data-type");
    std::string output_type = args.get<std::string>("output-data-type");

    AX_NPU_CV_DataType in_type = get_data_type(input_type);
    AX_NPU_CV_DataType out_type = get_data_type(output_type);

    if(out_type == AX_NPU_CV_DT_INT8){
        assert(arith_method.compare("sub") == 0);
    }

    AX_NPU_CV_Matrix2D mat_a = create_matrix_universal(input_a, M, N, in_type);
    AX_NPU_CV_Matrix2D mat_b = create_matrix_universal(input_b, M, N, in_type);
    assert(mat_a.nColumns == mat_b.nColumns);

    AX_NPU_CV_DataType result_data_type = out_type;
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(M, N, result_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    AX_NPU_CV_ConvArithContext context = nullptr;

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        if(arith_method == "add")
            assert(0 == AX_NPU_CV_MatAdd(&context, virtual_npu_mode_type, &mat_a, &mat_b, &result_matrix_2d));
        else if(arith_method == "sub")
            assert(0 == AX_NPU_CV_MatSub(&context, virtual_npu_mode_type, &mat_a, &mat_b, &result_matrix_2d));
    }

    AX_NPU_CV_DestroyConvArithContext(context);
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start) / nRepeat;
    printf("\n**************************************************\n");
    printf("Run task took %lld us (%d rounds for average)\n", duration.count(), nRepeat);
    printf("**************************************************\n");

    if (!out_dir.empty()) {
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
            return 1;
        }

        {
            char fn[256];
            sprintf(fn, "%s/result.MatArith.%s.%dx%d.npu_mode_%d.%s",
                out_dir.c_str(),
                arith_method.c_str(),
                M, N,
                virtual_npu_mode_type,
                get_data_type_name(result_data_type).c_str());
            dump_matrix_to_file(fn, result_matrix_2d);
        }

    }

    // Calculate GT
    if(bCheck){
        printf("gt=======================\n");
        AX_S32* gt_data = new AX_S32[M * N];
        memset(gt_data, 0, M * N * sizeof(AX_U8));

        AX_U8 *a_data = (AX_U8  *)mat_a.pVir;
        AX_U8  *b_data = (AX_U8 *)mat_b.pVir;
        for (int m = 0; m < M; ++m) {
            for (int n = 0; n < N; ++n) {
                AX_S32 res = (AX_U32)a_data[m*N+n] + (AX_U32)b_data[m*N+n];
                if(arith_method == "sub")
                    res =(AX_S32)(AX_U32)a_data[m*N+n] - (AX_S32)(AX_U32)b_data[m*N+n];
                gt_data[m*N + n]  = res;
                if(out_type == AX_NPU_CV_DT_UINT8){
                    gt_data[m*N + n] = max(0,gt_data[m*N + n]);
                    gt_data[m*N + n] = min(255,gt_data[m*N + n]);
                }else if(out_type == AX_NPU_CV_DT_INT8){
                    gt_data[m*N + n] = max(-128,gt_data[m*N + n]);
                    gt_data[m*N + n] = min(127,gt_data[m*N + n]);
                }
                if(n % 16 == 0) printf("\n");
                printf("%d ", gt_data[m*N+n]);
            }
            printf("\n");
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
            AX_U8 max_tolerance = 0;
            AX_U8* result_data = (AX_U8*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows * result_matrix_2d.nColumns; ++i) {
                if (abs(result_data[i] - gt_data[i]) > max_tolerance) {
                    max_tolerance = abs(result_data[i] - gt_data[i]);
                }
                ofs << result_data[i] - gt_data[i] << " ";
            }

            printf("The matmul result max tolerance with GT is %d, dumpped tolerance as '%s'!\n", max_tolerance, path);
        }

        delete[] gt_data;
        gt_data = nullptr;
    }
    release_matrix_memory(mat_a);
    release_matrix_memory(mat_b);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}