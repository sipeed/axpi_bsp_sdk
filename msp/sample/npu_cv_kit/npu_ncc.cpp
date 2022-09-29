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
    args.add<std::string> ("image0", '\0', "image file", false);
    args.add<std::string> ("image1", '\0', "image file", false);
    args.add<AX_U32>("height", '\0', "height of image", true);
    args.add<AX_U32>("width", '\0', "width of image", true);
    args.add<std::string> ("color-space", '\0', "GRAY", false, "GRAY");
    args.add<AX_U32>("repeat", '\0', "execution count", false, 1);
    args.add<bool>("check", '\0', "Check outputs", false, false);
    args.add<std::string> ("output", '\0', "output dir", false);

    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_ncc.ax620a  --mode=disable --mode-type=disable --image0 "A.GRAY" --image1 "B.GRAY" --height 128 --width 128 --repeat=100 --check=1

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_U32 width = args.get<AX_U32>("width");
    AX_U32 height = args.get<AX_U32>("height");
    std::string color_space = args.get<std::string>("color-space");
    bool bCheck = args.get<bool>("check");

    if(get_color_space(color_space) != AX_NPU_CV_FDT_GRAY){
        assert(0);
    }

    AX_NPU_CV_Image* src0 = new AX_NPU_CV_Image;
    src0->nWidth = width;
    src0->nHeight = height;
    src0->tStride.nW = src0->nWidth; // TODO
    src0->eDtype = get_color_space(color_space);
    long size = get_image_data_size(src0);
    std::string image_fn0 = args.get<std::string>("image0");
    load_file(image_fn0, (void**) &src0->pVir, reinterpret_cast<AX_U64*>(&src0->pPhy), &size);
    src0->nSize = size;
    assert(get_image_data_size(src0) == size);

    AX_NPU_CV_Image* src1 = new AX_NPU_CV_Image;
    src1->nWidth = width;
    src1->nHeight = height;
    src1->tStride.nW = src1->nWidth; // TODO
    src1->eDtype = get_color_space(color_space);
    size = get_image_data_size(src1);
    std::string image_fn1 = args.get<std::string>("image1");
    load_file(image_fn1, (void**) &src1->pVir, reinterpret_cast<AX_U64*>(&src1->pPhy), &size);
    src1->nSize = size;
    assert(get_image_data_size(src1) == size);
    printf("src image h %d, w %d, color space %s\n", height, width, color_space.c_str());

    AX_NPU_CV_NCC_MEM * dst = new AX_NPU_CV_NCC_MEM;
    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    const int test_count = args.get<AX_U32>("repeat");
    auto task_start= std::chrono::system_clock::now();
    for (int i = 0; i < test_count; ++i) {
        assert(0 == AX_NPU_CV_NCC(virtual_npu_mode_type, src0, src1, dst));
    }
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / test_count;

    AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
    print_work_cycle(cur);
    printf("Run tasks took %d us (%d rounds for average)\n\n", (int)duration.count(), test_count);

    std::string out_dir = args.get<std::string>("output");
    if (!out_dir.empty()) {
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
            return 1;
        }
        AX_NPU_CV_Matrix2D dump_mat = create_matrix_by_size(1,1,AX_NPU_CV_DT_FLOAT32);
        float * pvir = (float*)dump_mat.pVir;
        *pvir = dst->u64Numerator/(sqrt(dst->u64QuadSum1)*sqrt(dst->u64QuadSum2));
        char fn[256];
        sprintf(fn, "%s/npu_ncc.%dx%d.vnpu_%d.%s",
                out_dir.c_str(),
                height, width,
                virtual_npu_mode_type,
                get_data_type_name(AX_NPU_CV_DT_FLOAT32).c_str());
        dump_matrix_to_file(fn, dump_mat);
        release_matrix_memory(dump_mat);
    }

    if(bCheck){
        printf("dumped result =============================\n");
        printf("%lld %lld %lld\n", dst->u64Numerator, dst->u64QuadSum1, dst->u64QuadSum2);
        AX_U8 * data1 = (AX_U8*)src0->pVir;
        AX_U8 * data2 = (AX_U8*)src1->pVir;
        AX_U64 numberator= 0, quadsum1 = 0, quadsum2 = 0;
        for(AX_U32 row = 0; row < height; row++){
            for(AX_U32 col = 0; col < width; col++){
                numberator += (AX_U64)data1[row*width + col] * data2[row*width+col];
                quadsum1 += (AX_U64)data1[row*width + col]*data1[row*width + col];
                quadsum2 += (AX_U64)data2[row*width+col] * data2[row*width+col];
            }
        }
        printf("gt result ===============================\n");
        printf("%lld %lld %lld\n", numberator, quadsum1, quadsum2);

        if(abs((int)(numberator - dst->u64Numerator)) < 1e-4*numberator
            && abs((int)(quadsum1 - dst->u64QuadSum1)) < 1e-4*quadsum1
            && abs((int)(quadsum2 - dst->u64QuadSum2)) < 1e-4*quadsum2
            ){
            printf("The ncc result match with GT!\n");
        }else{
            printf("The ncc result mismatch with GT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        }
    }

    release_image_memory(src0);
    release_image_memory(src1);
    delete dst;
    return 0;
}