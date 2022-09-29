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


//dst = alpha*mat1 + mat2
void matrix_calc(float alpha, AX_NPU_CV_Matrix2D mat1, AX_NPU_CV_Matrix2D mat2,  AX_NPU_CV_Matrix2D dst){
    AX_U32 nRow = mat1.nRows;
    AX_U32 nColumn = mat1.nColumns;
    assert(nRow == mat2.nRows && nColumn == mat2.nColumns);
    for(AX_U32 i = 0; i < nRow; i++){
        for(AX_U32 j = 0; j < nColumn; j++){
            if(mat1.eDataType == AX_NPU_CV_DT_FLOAT32
                && mat2.eDataType == AX_NPU_CV_DT_FLOAT32
                && dst.eDataType == AX_NPU_CV_DT_FLOAT32){
                    float * in1 = (float*)mat1.pVir+(i*nColumn)+j;
                    float * in2 = (float*)mat2.pVir+(i*nColumn)+j;
                    float * out = (float*)dst.pVir+(i*nColumn)+j;
                    *out = alpha*(*in1)+(*in2);
            }
            if(mat1.eDataType == AX_NPU_CV_DT_INT8
                && mat2.eDataType == AX_NPU_CV_DT_INT8
                && dst.eDataType == AX_NPU_CV_DT_INT8){
                    int8_t * in1 = (int8_t*)mat1.pVir+(i*nColumn)+j;
                    int8_t * in2 = (int8_t*)mat2.pVir+(i*nColumn)+j;
                    int8_t * out = (int8_t*)dst.pVir+(i*nColumn)+j;
                    int32_t res = alpha*(*in1)+(*in2);
                    if(res > INT8_MAX) res = INT8_MAX;
                    if(res < INT8_MIN) res = INT8_MIN;
                    *out = (int8_t)res;
                }
                if(mat1.eDataType == AX_NPU_CV_DT_INT8
                    && mat2.eDataType == AX_NPU_CV_DT_INT8
                    && dst.eDataType == AX_NPU_CV_DT_INT16){
                        int8_t * in1 = (int8_t*)mat1.pVir+(i*nColumn)+j;
                        int8_t * in2 = (int8_t*)mat2.pVir+(i*nColumn)+j;
                        int16_t * out = (int16_t*)dst.pVir+(i*nColumn)+j;
                        int32_t res = alpha*(*in1)+(*in2);
                        if(res > INT16_MAX) res = INT16_MAX;
                        if(res < INT16_MIN) res = INT16_MIN;
                        *out = (int16_t)res;
                    }
        }
    }
}
int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string> ("A", '\0', "matrix file", false);
    args.add<std::string> ("B", '\0', "matrix file", false);
    args.add<AX_U32>("row", '\0', "rows of matrix", true);
    args.add<AX_U32>("column", '\0', "columns of matrix", true);
    args.add<std::string> ("input-data-type", '\0', "float,int8", false, "float");
    args.add<std::string> ("output-data-type", '\0', "float,int8,int16", false, "float");
    args.add<AX_F32>("alpha", '\0', "scalar coefficient", false, 1);
    args.add<AX_U32>("repeat", '\0', "execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, true);
    args.add<std::string> ("output", '\0', "output dir", false);

    args.parse_check(argc, argv);

    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_saxpy.ax620a  --mode=disable --mode-type=disable --input-data-type=float --row 128 --column 128 --repeat=100 --alpha=1 --check=1

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");
    std::string matrix_a = args.get<std::string>("A");
    std::string matrix_b = args.get<std::string>("B");
    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");
    AX_NPU_CV_DataType input_data_type = get_data_type(args.get<std::string>("input-data-type"));
    AX_NPU_CV_DataType output_data_type = get_data_type(args.get<std::string>("output-data-type"));
    AX_F32 fAlpha = args.get<AX_F32>("alpha");
    std::string output = args.get<std::string>("output");

    // if(input_data_type != AX_NPU_CV_DT_FLOAT32){
    //     printf("unsupported data type %d for matrix\n", (int)input_data_type);
    //     assert(0);
    // }

    AX_NPU_CV_Matrix2D a_matrix_2d = create_matrix_universal(matrix_a, nRow, nColumn, input_data_type);
    AX_NPU_CV_Matrix2D b_matrix_2d = create_matrix_universal(matrix_b, nRow, nColumn, input_data_type);
    assert(a_matrix_2d.nRows == b_matrix_2d.nRows);
    assert(a_matrix_2d.nColumns == b_matrix_2d.nColumns);

    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    srand(time(0));

    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, output_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_Saxpy(virtual_npu_mode_type, fAlpha, &a_matrix_2d, &b_matrix_2d, &result_matrix_2d));
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

     if(!output.empty()){
        struct stat info;
        if (stat(output.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", output.c_str());
        }else{
            char fname[256];
            sprintf(fname, "%s/npu_saxpy.row_%d.column_%d.alpha_%.2f.npu_mode_%d.%s",
                output.c_str(),
                nRow, nColumn, fAlpha,
                virtual_npu_mode_type,
                get_data_type_name(output_data_type).c_str());
            dump_matrix_to_file(fname, result_matrix_2d);
        }
    }

    if (bCheck) {
        // Calculate GT
        AX_NPU_CV_Matrix2D gt_matrix = create_matrix_by_size(nRow, nColumn, output_data_type);
        matrix_calc(fAlpha, a_matrix_2d, b_matrix_2d, gt_matrix);
        printf("dump gt data:\n");
        print_matrix(gt_matrix);
        printf("**************************************************\n");
        printf("**************************************************\n");

        printf("dump results:\n");
        print_matrix(result_matrix_2d);

        // Check GT with result
        bool match_gt = true;
        match_gt = matrix_cmp(gt_matrix, result_matrix_2d);
        if (match_gt) {
            printf("The result match with GT!\n");
        } else {
            printf("The result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }

        release_matrix_memory(gt_matrix);
    }

    release_matrix_memory(a_matrix_2d);
    release_matrix_memory(b_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
