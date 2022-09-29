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
#include <cfenv>
#include <cmath>
#include "img_helper.h"

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string> ("A", '\0', "matrix file", false);
    args.add<AX_U32>("row", '\0', "rows of matrix", true);
    args.add<AX_U32>("column", '\0', "columns of matrix", true);
    args.add<std::string> ("data-type", '\0', "float", false, "float");
    args.add<AX_U32>("repeat", '\0', "execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, false);
    args.add<std::string> ("output", '\0', "output dir", false);

    args.parse_check(argc, argv);

    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_softmax.ax620a  --mode=disable --mode-type=disable --data-type=float --row 128 --column 128 --repeat=100 --check=1

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");
    std::string matrix_a = args.get<std::string>("A");
    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");
    AX_NPU_CV_DataType data_type = get_data_type(args.get<std::string>("data-type"));
    std::string output = args.get<std::string>("output");

    if(data_type != AX_NPU_CV_DT_FLOAT32){
        printf("unsupported data type %d for matrix\n", (int)data_type);
        assert(0);
    }

    AX_NPU_CV_Matrix2D a_matrix_2d = create_matrix_universal(matrix_a, nRow, nColumn, data_type);

    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    srand(time(0));

    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_Softmax(virtual_npu_mode_type, &a_matrix_2d, &result_matrix_2d));
        if(nRepeat > 1 && i == 0){
            time_start = std::chrono::system_clock::now();
        }
    }
    auto time_end = std::chrono::system_clock::now();
    if (nRepeat > 1) nRepeat--;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - time_start) / nRepeat;
    printf("\n**************************************************\n");
    printf("Run task took %lld us (%d rounds for average)\n", duration.count(), nRepeat);
    printf("**************************************************\n");

     if(!output.empty()){
        struct stat info;
        if (stat(output.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", output.c_str());
        }else{
            char fname[256];
            sprintf(fname, "%s/npu_softmax.row_%d.column_%d.npu_mode_%d.%s",
                output.c_str(),
                nRow, nColumn,
                virtual_npu_mode_type,
                get_data_type_name(data_type).c_str());
            dump_matrix_to_file(fname, result_matrix_2d);
        }
    }


    if (bCheck) {
        printf("src data =================================\n");
        float * src = (float*)a_matrix_2d.pVir;
        for (int r = 0; r < nRow; ++r) {
            for (int c = 0; c < nColumn; ++c) {
                if (c % 16 == 0) printf("\n");
                printf("%8.4f ", src[r*nColumn + c]);
            }
            printf("\n");
        }

        // Calculate GT
        float* gt_data = new float[nRow * nColumn];
        memset(gt_data, 0, nRow * nColumn * sizeof(float));

        float * uniform_data = (float*)malloc(nRow*nColumn*sizeof(float));
        float * line_max = (float*)malloc(nRow * sizeof(float));
        float *a_data = (float *)a_matrix_2d.pVir;
        for (int r = 0; r < nRow; ++r) {
            float max = a_data[r*nColumn];
            for (int c = 0; c < nColumn; ++c) {
                if(max < a_data[r*nColumn+c])
                    max = a_data[r*nColumn+c];
            }
            line_max[r] = max;
            printf("r = %d, max = %8.3f\n", r, max);
        }
        printf("uniform data============\n");
        for(int r = 0; r < nRow; r++){
            for(int c = 0; c < nColumn; c++){
                uniform_data[r*nColumn+c] = a_data[r*nColumn+c]-line_max[r];
                if (c % 16 == 0) printf("\n");
                printf("%8.4f ", uniform_data[r*nColumn + c]);
            }
            printf("\n");
        }
        free(line_max);

        printf("exp data============\n");
        for(int r = 0; r < nRow; r++){
            for(int c = 0; c < nColumn; c++){
                float exp_data = std::exp(uniform_data[r*nColumn+c]);
                if (c % 16 == 0) printf("\n");
                printf("%8.4f ",exp_data);
            }
            printf("\n");
        }

        float *line_sum = (float*)malloc(nRow*sizeof(float));
        for(int r=0; r<nRow; r++){
            float sum = 0.0;
            for(int c=0; c<nColumn; c++){
                sum += std::exp(uniform_data[r*nColumn+c]);
            }
            line_sum[r] = sum;
            printf("r=%d, line_sum=%8.4f\n", r, sum);
        }

        printf("gt==================================================\n");
        for (int r = 0; r < nRow; ++r) {
            for (int c = 0; c < nColumn; ++c) {
                gt_data[r*nColumn+c] = std::exp(uniform_data[r*nColumn + c])/line_sum[r];
                if (c % 16 == 0) printf("\n");
                printf("%8.4f ", gt_data[r*nColumn + c]);
            }
            printf("\n");
        }

        free(uniform_data);
        free(line_sum);
        printf("**************************************************\n");
        printf("**************************************************\n");

        float diff = 0.0;
        printf("dump exp results:\n");
        float* result_data = (float*)result_matrix_2d.pVir;
        for (int r = 0; r < nRow; ++r) {
            for (int c = 0; c < nColumn; ++c) {
                if (c % 16 == 0) printf("\n");
                printf("%8.4f ", result_data[r * nColumn + c]);
                float aa = abs(gt_data[r*nColumn+c] - result_data[r*nColumn+c]);
                if(aa > diff) diff = aa;
            }
            printf("\n");
        }

        printf("max diff = %f\n", diff);

        // Check GT with result
        bool match_gt = true;
        for (int i = 0; i < nRow * nColumn; ++i) {
            if (fabs(result_data[i] - gt_data[i]) > 1e-3) {
                match_gt = false;
                break;
            }
        }
        if (match_gt) {
            printf("The exp result match with GT!\n");
        } else {
            printf("The exp result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }

        delete[] gt_data;
    }

    release_matrix_memory(a_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
