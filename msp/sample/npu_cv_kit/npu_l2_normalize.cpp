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
#include <sys/stat.h>


int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string> ("A", '\0', "matrix file", false);
    args.add<AX_U32>("row", '\0', "rows of matrix", true);
    args.add<AX_U32>("column", '\0', "columns of matrix", true);
    args.add<std::string> ("data-type", '\0', "float", false, "float");
    args.add<AX_U32>("repeat", '\0', "HadamardProduct execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, true);
    args.add<std::string> ("output", '\0', "output directory", false);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_l2_normalize.ax620a  --mode disable --mode-type disable --row 128  --column 128 --check 1 --repeat 1

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");
    std::string matrix_file = args.get<std::string>("A");
    std::string out_dir = args.get<std::string>("output");
    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");

    srand(time(0));

    AX_NPU_CV_DataType data_type = get_data_type(args.get<std::string>("data-type"));
    AX_NPU_CV_Matrix2D src_matrix_2d = create_matrix_universal(matrix_file, nRow, nColumn, data_type);
    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    if (data_type != AX_NPU_CV_DT_FLOAT32) {
        printf("unsupported data type %d for input matrix\n", (int)data_type);
        assert(0);
    }

    AX_NPU_CV_DataType result_data_type = data_type;
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, result_data_type);
    AX_VOID *result_vir_addr = result_matrix_2d.pVir;

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));
    AX_NPU_CV_L2NormalizeContext l2Context = nullptr;

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_L2Normalize(&l2Context, virtual_npu_mode_type, &src_matrix_2d, &result_matrix_2d));
        if(nRepeat > 1 && i == 0){
            time_start = std::chrono::system_clock::now();
        }
    }
    auto time_end = std::chrono::system_clock::now();
    if (nRepeat > 1) nRepeat--;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start) / nRepeat;

    AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
    print_work_cycle(cur);

    AX_NPU_CV_DestroyL2NormalizeContext(l2Context);
    printf("\n**************************************************\n");
    printf("Run task took %lld us (%d rounds for average)\n", duration.count(), nRepeat);
    printf("**************************************************\n");

    if(!out_dir.empty()){
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
        }else{
            char fn[256];
            sprintf(fn, "%s/result_%dx%d_l2_normalize.npu_mode_%d.%s",
                out_dir.c_str(), nRow, nColumn, virtual_npu_mode_type, get_data_type_name(result_data_type).c_str());
            dump_matrix_to_file(fn, result_matrix_2d);
        }
    }
    if (bCheck) {

        if (data_type == AX_NPU_CV_DT_FLOAT32) {
            // Calculate GT
            float* gt_data = new float[nRow * nColumn];
            memset(gt_data, 0, nRow * nColumn * sizeof(float));

            double my_gt = 0.0;

            float *src_data = (float *)src_matrix_2d.pVir;
            for (int i = 0; i < nRow; ++i) {
                for (int j = 0; j < nColumn; ++j) {
                    // gt_data[i*nColumn + j] = src_data[i*nColumn + j] * src_data[i*nColumn + j];
                    my_gt += src_data[i*nColumn + j] * src_data[i*nColumn + j];
                }
            }

            float ratio = sqrt(my_gt);
            for (int i = 0; i < nRow; ++i) {
                for (int j = 0; j < nColumn; ++j) {
                    gt_data[i*nColumn + j] = src_data[i*nColumn + j] / ratio;
                }
            }

            // printf("--------------------------gt %f\n", sqrt(my_gt));
            printf("--------------------------my gt %f\n", sqrt(my_gt));

            printf("dump matmul GT results:\n");
            for (int i = 0; i < nRow; ++i) {
                for (int j = 0; j < nColumn; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%.3f ", gt_data[i * nColumn + j]);
                }
                printf("\n");
            }

            printf("**************************************************\n");
            printf("**************************************************\n");

            printf("dump matmul results:\n");
            float* result_data = (float*)result_vir_addr;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows; ++i) {
                for (AX_U32 j = 0; j < result_matrix_2d.nColumns; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%.3f ", result_data[i * result_matrix_2d.nColumns + j]);
                }
                printf("\n");
            }

            // Check GT with result
            bool match_gt = true;
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (fabs(result_data[i] - gt_data[i]) > 1e-2) {
                    match_gt = false;
                    break;
                }
            }
            if (match_gt) {
                printf("The result match with GT!\n");
            } else {
                printf("The result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            delete[] gt_data;
        }
    }

    release_matrix_memory(src_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
