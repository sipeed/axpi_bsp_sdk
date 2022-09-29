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

static std::vector<AX_S8> parse_kernel(std::string argv) {
    std::vector<AX_S8> ret;
    if (argv.empty()) return ret;
    auto arr = split_string(argv, ",");
    for (auto& s: arr) {
        ret.push_back(std::stoi(s));
    }
    if (arr.size() != 9) {
        fprintf(stderr, "kernel size must be 9\n");
        exit(1);
    }
    return ret;
}

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<AX_U32>("row", '\0', "Rows of matrix", true);
    args.add<AX_U32>("column", '\0', "Columns of matrix", true);
    args.add<std::string> ("A", '\0', "input matrix file", false);
    args.add<std::string> ("input-data-type", '\0', "uint8", false, "uint8");
    args.add<std::string> ("output-data-type", '\0', "int16, uint16", false, "int16");
    args.add<std::string>("kernel", '\0', "Laplacian 3x3 kernel", false, "2, 0, 2, 0, -8, 0, 2, 0, 2");
    args.add<AX_U32>("repeat", '\0', "Laplacian execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, false);
    args.add<std::string> ("output", '\0', "output path", false);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_laplacian.ax620a  --mode=disable --mode-type=disable --input-data-type=uint8 --output-data-type=int16 --A "a.txt" --repeat=1

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");

    std::string matrix_file = args.get<std::string>("A");
    int nRow = args.get<AX_U32>("row");
    int nColumn = args.get<AX_U32>("column");
    std::string out_put = args.get<std::string>("output");

    auto filter_mat33 = parse_kernel(args.get<std::string>("kernel"));
    srand(time(0));

    AX_NPU_CV_DataType src_data_type = get_data_type(args.get<std::string>("input-data-type"));
    if(src_data_type != AX_NPU_CV_DT_UINT8){
        printf("unsupported data type %d for base matrix\n", (int)src_data_type);
        assert(0);
    }

    AX_NPU_CV_Matrix2D src_matrix_2d = create_matrix_universal(matrix_file, nRow, nColumn, src_data_type);

    printf("Input row %d, column %d, repeat %d\n", nRow, nColumn, nRepeat);

    AX_NPU_CV_DataType result_data_type = get_data_type(args.get<std::string>("output-data-type"));
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nRow, nColumn, result_data_type);

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    AX_NPU_CV_LaplacianContext tContext = nullptr;
    AX_NPU_CV_LaplacianParams tLaplacianParams;
    memcpy(tLaplacianParams.nKernelValue, filter_mat33.data(), sizeof(tLaplacianParams.nKernelValue));

    auto time_start = std::chrono::system_clock::now();
    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_Laplacian(&tContext, virtual_npu_mode_type, &src_matrix_2d, &result_matrix_2d, &tLaplacianParams));
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

    if(!out_put.empty()){
        struct stat info;
        if (stat(out_put.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_put.c_str());
        }else{
            char fn[256];
            sprintf(fn, "%s/result_%dx%d_laplacian.npu_mode_%d.%s",
                out_put.c_str(), nRow, nColumn, virtual_npu_mode_type, get_data_type_name(result_data_type).c_str());
            dump_matrix_to_file(fn, result_matrix_2d);
        }
    }
    if (bCheck) {
        // Calculate GT
        int64_t* gt_data = new int64_t[nRow * nColumn];
        memset(gt_data, 0, nRow * nColumn * sizeof(int64_t));

        uint8_t *src_data = (uint8_t *)src_matrix_2d.pVir;

        for (int i = 0; i < nRow; ++i) {
            for (int j = 0; j < nColumn; ++j) {
                uint8_t input_feature[9] = {0};

                input_feature[4] = src_data[i*nColumn + j];
                if (j == 0) {
                    input_feature[0] = 0;
                    input_feature[3] = 0;
                    input_feature[6] = 0;
                    if (i == 0) {
                        input_feature[0] = 0;
                        input_feature[1] = 0;
                        input_feature[2] = 0;

                        input_feature[5] = src_data[i*nColumn + j + 1];
                        input_feature[7] = src_data[(i+1)*nColumn + j];
                        input_feature[8] = src_data[(i+1)*nColumn + j + 1];
                    } else if (i == nRow - 1) {
                        input_feature[6] = 0;
                        input_feature[7] = 0;
                        input_feature[8] = 0;

                        input_feature[1] = src_data[(i-1)*nColumn + j];
                        input_feature[2] = src_data[(i-1)*nColumn + j + 1];
                        input_feature[5] = src_data[i*nColumn + j + 1];
                    } else {
                        input_feature[1] = src_data[(i-1)*nColumn + j];
                        input_feature[7] = src_data[(i+1)*nColumn + j];

                        input_feature[2] = src_data[(i-1)*nColumn + j + 1];
                        input_feature[5] = src_data[i*nColumn + j + 1];
                        input_feature[8] = src_data[(i+1)*nColumn + j + 1];
                    }

                } else if (j == nColumn - 1) {
                    input_feature[2] = 0;
                    input_feature[5] = 0;
                    input_feature[8] = 0;

                    if (i == 0) {
                        input_feature[0] = 0;
                        input_feature[1] = 0;
                        input_feature[2] = 0;

                        input_feature[3] = src_data[i*nColumn + j - 1];
                        input_feature[6] = src_data[(i+1)*nColumn + j - 1];
                        input_feature[7] = src_data[(i+1)*nColumn + j];
                    } else if (i == nRow - 1) {
                        input_feature[6] = 0;
                        input_feature[7] = 0;
                        input_feature[8] = 0;

                        input_feature[0] = src_data[(i-1)*nColumn + j - 1];
                        input_feature[1] = src_data[(i-1)*nColumn + j];
                        input_feature[3] = src_data[i*nColumn + j - 1];
                    } else {
                        input_feature[0] = src_data[(i-1)*nColumn + j - 1];
                        input_feature[3] = src_data[i*nColumn + j - 1];
                        input_feature[6] = src_data[(i+1)*nColumn + j - 1];

                        input_feature[1] = src_data[(i-1)*nColumn + j];
                        input_feature[7] = src_data[(i+1)*nColumn + j];
                    }
                } else {

                    if (i == 0) {
                        input_feature[0] = 0;
                        input_feature[1] = 0;
                        input_feature[2] = 0;

                        input_feature[3] = src_data[i*nColumn + j - 1];
                        input_feature[5] = src_data[i*nColumn + j + 1];

                        input_feature[6] = src_data[(i+1)*nColumn + j - 1];
                        input_feature[7] = src_data[(i+1)*nColumn + j];
                        input_feature[8] = src_data[(i+1)*nColumn + j + 1];
                    } else if (i == nRow - 1) {
                        input_feature[6] = 0;
                        input_feature[7] = 0;
                        input_feature[8] = 0;

                        input_feature[0] = src_data[(i-1)*nColumn + j - 1];
                        input_feature[1] = src_data[(i-1)*nColumn + j];
                        input_feature[2] = src_data[(i-1)*nColumn + j + 1];

                        input_feature[3] = src_data[i*nColumn + j - 1];
                        input_feature[5] = src_data[i*nColumn + j + 1];
                    } else {
                        input_feature[0] = src_data[(i-1)*nColumn + j - 1];
                        input_feature[1] = src_data[(i-1)*nColumn + j];
                        input_feature[2] = src_data[(i-1)*nColumn + j + 1];

                        input_feature[3] = src_data[i*nColumn + j - 1];
                        input_feature[5] = src_data[i*nColumn + j + 1];

                        input_feature[6] = src_data[(i+1)*nColumn + j - 1];
                        input_feature[7] = src_data[(i+1)*nColumn + j];
                        input_feature[8] = src_data[(i+1)*nColumn + j + 1];
                    }

                }

                for (int k = 0; k < 9; ++k) {
                    gt_data[i*nColumn + j] += input_feature[k] * filter_mat33[k];
                }

                if (result_data_type == AX_NPU_CV_DT_INT16) {
                    // Clip to [INT16_MIN, INT16_MAX]
                    if (gt_data[i*nColumn + j] > INT16_MAX) gt_data[i*nColumn + j] = INT16_MAX;
                    if (gt_data[i*nColumn + j] < INT16_MIN) gt_data[i*nColumn + j] = INT16_MIN;
                } else if (result_data_type == AX_NPU_CV_DT_UINT16) {
                    // Clip to [0, UINT16_MAX]
                    if (gt_data[i*nColumn + j] > UINT16_MAX) gt_data[i*nColumn + j] = UINT16_MAX;
                    if (gt_data[i*nColumn + j] < 0) gt_data[i*nColumn + j] = 0;
                } else {
                    assert(0);
                }
            }
        }
        /*
        printf("src:\n");
        for (int i = 0; i < nRow; ++i) {
            for (int j = 0; j < nColumn; ++j) {
                if (j % 16 == 0) printf("\n");
                printf("%d ", src_data[i * nColumn + j]);
            }
            printf("\n");
        }
        */
        printf("dump laplacian GT results:\n");
        for (int i = 0; i < nRow; ++i) {
            for (int j = 0; j < nColumn; ++j) {
                if (j % 16 == 0) printf("\n");
                printf("%4lld ", gt_data[i * nColumn + j]);
            }
            printf("\n");
        }

        printf("**************************************************\n");
        printf("**************************************************\n");

        printf("dump laplacian results:\n");
        bool match_gt = true;

        if (result_data_type == AX_NPU_CV_DT_INT16) {
            int16_t* result_data = (int16_t*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows; ++i) {
                for (AX_U32 j = 0; j < result_matrix_2d.nColumns; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%4d ", result_data[i * result_matrix_2d.nColumns + j]);
                }
                printf("\n");
            }

            // Check GT with result
            for (int i = 0; i < nRow * nColumn; ++i) {
                if (result_data[i] != gt_data[i]) {
                    match_gt = false;
                    break;
                }
            }

        } else if (result_data_type == AX_NPU_CV_DT_UINT16) {
            uint16_t* result_data = (uint16_t*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows; ++i) {
                for (AX_U32 j = 0; j < result_matrix_2d.nColumns; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%4d ", result_data[i * result_matrix_2d.nColumns + j]);
                }
                printf("\n");
            }

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
            printf("The laplacian result match with GT!\n");
        } else {
            printf("The laplacian result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }
        delete[] gt_data;

    }

    release_matrix_memory(src_matrix_2d);
    AX_NPU_CV_DestroyLaplacianContext(tContext); // 程序退出前释放 Laplacian 算子上下文
    release_matrix_memory(result_matrix_2d);

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
