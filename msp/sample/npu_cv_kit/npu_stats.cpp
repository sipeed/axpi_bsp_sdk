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


AX_NPU_CV_StatsMethod get_stats_method(const std::string& stats_method) {
    if (stats_method == "sum") {
        return AX_NPU_CV_SM_SUM;
    } else if (stats_method == "max") {
        return AX_NPU_CV_SM_MAX;
    } else if (stats_method == "argmax") {
        return AX_NPU_CV_SM_ARGMAX;
    } else if (stats_method == "min") {
        return AX_NPU_CV_SM_MIN;
    } else if (stats_method == "argmin") {
        return AX_NPU_CV_SM_ARGMIN;
    } else {
        assert(0 && "stats_method error");
    }
}

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string> ("stats-method", '\0', "sum, max, argmax, min, argmin", false, "sum");
    args.add<std::string> ("A", '\0', "matrix file", false);
    args.add<AX_U32>("row", '\0', "rows of matrix", true);
    args.add<AX_U32>("column", '\0', "columns of matrix", true);
    args.add<std::string> ("data-type", '\0', "float", false, "float");
    args.add<AX_U32>("repeat", '\0', "execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, false);
    args.add<std::string> ("output", '\0', "output dir", false);

    args.parse_check(argc, argv);

    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_stats.ax620a  --mode=disable --mode-type=disable --data-type=float --stats-method=max --row 128 --column 128 --repeat=1 --check=1

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
    std::string method = args.get<std::string>("stats-method");

    AX_NPU_CV_Matrix2D a_matrix_2d = create_matrix_universal(matrix_a, nRow, nColumn, data_type);

    printf("Input row %d, column %d, repeat %d, data-type:%d\n", nRow, nColumn, nRepeat, data_type);

    srand(time(0));

    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, 1, data_type); // CHECK!!!!

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    AX_NPU_CV_StatsMethod stats_metthod = get_stats_method(method);

    if(stats_metthod == AX_NPU_CV_SM_SUM){
        assert(data_type == AX_NPU_CV_DT_FLOAT32);
    }

    if(stats_metthod == AX_NPU_CV_SM_ARGMAX || stats_metthod == AX_NPU_CV_SM_ARGMIN){
        release_matrix_memory(result_matrix_2d);
        result_matrix_2d = create_matrix_by_size(nRow, 1, AX_NPU_CV_DT_UINT16);
    }

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_Stats(virtual_npu_mode_type, stats_metthod, &a_matrix_2d, &result_matrix_2d));
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
            AX_NPU_CV_DataType out_type = data_type;
            if(stats_metthod == AX_NPU_CV_SM_ARGMIN || stats_metthod == AX_NPU_CV_SM_ARGMAX)
                out_type = AX_NPU_CV_DT_UINT16;
            sprintf(fname, "%s/npu_stats.row_%d.column_%d.%s.method_%s.npu_mode_%d.%s",
                output.c_str(),
                nRow, nColumn,
                get_data_type_name(data_type).c_str(),
                method.c_str(),
                virtual_npu_mode_type,
                get_data_type_name(out_type).c_str());
            dump_matrix_to_file(fname, result_matrix_2d);
        }
    }


    if (bCheck) {
        printf("input data:\n");
        print_matrix(a_matrix_2d);

        // Calculate GT
        printf("gt========\n");
        AX_NPU_CV_Matrix2D gt_mat;
        if(stats_metthod == AX_NPU_CV_SM_MAX || stats_metthod == AX_NPU_CV_SM_MIN || stats_metthod == AX_NPU_CV_SM_SUM){
            gt_mat = create_matrix_by_size(nRow, 1, a_matrix_2d.eDataType);
            matrx_stats(a_matrix_2d, gt_mat, method);
        }else if(stats_metthod ==AX_NPU_CV_SM_ARGMAX || stats_metthod == AX_NPU_CV_SM_ARGMIN){
            gt_mat = create_matrix_by_size(nRow, 1, AX_NPU_CV_DT_UINT16);
            matrx_stats(a_matrix_2d, gt_mat, method);
        }
        print_matrix(gt_mat);

        printf("dump result=======\n");
        print_matrix(result_matrix_2d);

        bool match_gt = matrix_cmp(result_matrix_2d, gt_mat);
        if (match_gt) {
            printf("The stats result match with GT!\n");
        } else {
            printf("The stats result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }


        release_matrix_memory(gt_mat);
    }
    release_matrix_memory(a_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
