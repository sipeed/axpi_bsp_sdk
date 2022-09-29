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
    args.add<bool>("trans-A", '\0', "tranpose matrix A", false, true);
    args.add<bool>("trans-B", '\0', "tranpose matrix B", false, true);
    args.add<AX_U32>("M", '\0', "M is rows of A and D", true);
    args.add<AX_U32>("K", '\0', "K is columns of A and B", true);
    args.add<AX_U32>("N", '\0', "N is rows of  B and columns of D", true);
    args.add<std::string> ("data-type", '\0', "float", false, "float");
    args.add<AX_F32>("alpha", '\0', "scalar coefficient", false, 1);
    args.add<AX_U32>("repeat", '\0', "execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, true);

    args.parse_check(argc, argv);

    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_sgemm.ax620a  --mode=disable --mode-type=disable --data-type=float --trans-A=1 --trans-B=1 --M 1000 --K 256 --N 512 --repeat=100 --alpha=1 --check=0

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_BOOL bTransA = (AX_BOOL)args.get<bool>("trans-A");
    AX_BOOL bTransB = (AX_BOOL)args.get<bool>("trans-B");
    int nM = args.get<AX_U32>("M");
    int nK = args.get<AX_U32>("K");
    int nN = args.get<AX_U32>("N");
    AX_F32 fAlpha = args.get<AX_F32>("alpha");
    int nRepeat = args.get<AX_U32>("repeat");
    bool bCheck = args.get<bool>("check");

    printf("Input M %d, K %d, N %d, transpose A %d, transpose B %d, repeat %d\n", nM, nK, nN, bTransA, bTransB, nRepeat);

    AX_NPU_CV_DataType data_type = get_data_type(args.get<std::string>("data-type"));
    if(data_type != AX_NPU_CV_DT_FLOAT32){
        printf("unsupported data type %d for matrix\n", (int)data_type);
        assert(0);
    }

    AX_NPU_CV_Matrix2D a_matrix_2d = bTransA ? create_random_matrix(nK, nM, data_type) : create_random_matrix(nM, nK, data_type) ;
    AX_NPU_CV_Matrix2D b_matrix_2d = bTransB ? create_random_matrix(nK, nN, data_type) : create_random_matrix(nN, nK, data_type) ;
    AX_NPU_CV_Matrix2D result_matrix_2d = create_matrix_by_size(nM, nN, data_type);

    srand(time(0));

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    AX_NPU_CV_SgemmContext tContext = NULL;
    auto time_start = std::chrono::system_clock::now();

    for (int i = 0; i < nRepeat; ++i) {
        assert(0 == AX_NPU_CV_Sgemm(&tContext, virtual_npu_mode_type, bTransA, bTransB,
            fAlpha, &a_matrix_2d, &b_matrix_2d, 0, nullptr, &result_matrix_2d));
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

    if (bCheck) {
        // Calculate GT
            if(bTransA){
                transposeMat(a_matrix_2d);
                nM = a_matrix_2d.nRows;
            }
            if(bTransB){
                transposeMat(b_matrix_2d);
                nN = b_matrix_2d.nRows;
            }
            assert(a_matrix_2d.nColumns == b_matrix_2d.nColumns);
            nK = a_matrix_2d.nColumns;

            float* gt_data = new float[nM * nN];
            memset(gt_data, 0, nM * nN * sizeof(float));

            float *base_data = (float *)a_matrix_2d.pVir;
            float *query_data = (float *)b_matrix_2d.pVir;
            for (int m = 0; m < nM; ++m) {
                for (int n = 0; n < nN; ++n) {
                    for (int k = 0; k < nK; ++k) {
                        gt_data[m*nN + n] += base_data[m*nK + k] * query_data[n*nK + k];
                    }
                    gt_data[m*nN + n] *= fAlpha;
                }
            }

            printf("dump Sgemm GT results:\n");
            for (int i = 0; i < nM; ++i) {
                for (int j = 0; j < nN; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%8.3f ", gt_data[i * nN + j]);
                }
                printf("\n");
            }

            printf("**************************************************\n");
            printf("**************************************************\n");

            printf("dump Sgemm results:\n");
            float* result_data = (float*)result_matrix_2d.pVir;
            for (AX_U32 i = 0; i < result_matrix_2d.nRows; ++i) {
                for (AX_U32 j = 0; j < result_matrix_2d.nColumns; ++j) {
                    if (j % 16 == 0) printf("\n");
                    printf("%8.3f ", result_data[i * result_matrix_2d.nColumns + j]);
                }
                printf("\n");
            }

            // Check GT with result
            bool match_gt = true;
            for (int i = 0; i < nM * nN; ++i) {
                if (fabs(result_data[i] - gt_data[i]) > 1e-3) {
                    match_gt = false;
                    break;
                }
            }
            if (match_gt) {
                printf("The Sgemm result match with GT!\n");
            } else {
                printf("The Sgemm result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
            }

            delete[] gt_data;
    }

    release_matrix_memory(a_matrix_2d);
    release_matrix_memory(b_matrix_2d);
    release_matrix_memory(result_matrix_2d);

    AX_NPU_CV_DestroySgemmContext(tContext);  // 程序退出前释放算子上下文

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
