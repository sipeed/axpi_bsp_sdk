#include <cstdio>
#include <assert.h>
#include <string>
#include <sys/stat.h>
#include <chrono>
#include <memory.h>
#include <math.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"


int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string>("image", '\0', "input image", false);
    args.add<AX_U32>("src_h", '\0', "src height", 128);
    args.add<AX_U32>("src_w", '\0', "src width", 128);
    args.add<AX_U32>("dst_h", '\0', "dst height", 128);
    args.add<AX_U32>("dst_w", '\0', "dst width", 128);
    args.add<std::string>("format", '\0', "image format: BGR/RGB/RGGB/RAW", true);
    args.add<AX_U32>("repeat", '\0', "warp execution count", false, 1);
    args.add<std::string>("output", '\0', "output dir", true, "warp_test");

    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    int src_h = args.get<AX_U32>("src_h");
    int src_w = args.get<AX_U32>("src_w");
    int dst_h = args.get<AX_U32>("dst_h");
    int dst_w = args.get<AX_U32>("dst_w");
    int nRepeat = args.get<AX_U32>("repeat");
    std::string format = args.get<std::string>("format");
    printf("src_h %d, src_w %d, dst_h %d, dst_w %d, format %s, repeat %d\n",
            src_h, src_w, dst_h, dst_w, format.c_str(), nRepeat);

    srand(time(0));

    AX_NPU_CV_Image* src_img = new AX_NPU_CV_Image;
    memset(src_img, 0, sizeof(AX_NPU_CV_Image));
    src_img->nWidth = src_w;
    src_img->nHeight = src_h;
    src_img->tStride.nW = src_w;
    if (!format.compare(std::string("RGBA"))) {
        src_img->eDtype = AX_NPU_CV_FrameDataType::AX_NPU_CV_FDT_RGBA;
    } else if (!format.compare(std::string("BGR"))) {
        src_img->eDtype = AX_NPU_CV_FrameDataType::AX_NPU_CV_FDT_BGR;
    } else if (!format.compare(std::string("RGGB"))) {
        src_img->eDtype = AX_NPU_CV_FrameDataType::AX_NPU_CV_FDT_BAYER_RGGB;
    }else if(!format.compare(std::string("RGB"))){
        src_img->eDtype = AX_NPU_CV_FrameDataType::AX_NPU_CV_FDT_RGB;
    }else if(!format.compare(std::string("NV12"))){
        src_img->eDtype = AX_NPU_CV_FrameDataType::AX_NPU_CV_FDT_NV12;
    }else if(!format.compare(std::string("NV21"))){
        src_img->eDtype = AX_NPU_CV_FrameDataType::AX_NPU_CV_FDT_NV21;
    }else{
        assert(0);
    }

    long size = get_image_data_size(src_img);
    std::string image_fn = args.get<std::string>("image");
    load_file(image_fn, (void**) &src_img->pVir, reinterpret_cast<AX_U64*>(&src_img->pPhy), &size);
    src_img->nSize = size;

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    AX_NPU_CV_Image* dst = new AX_NPU_CV_Image;
    memset(dst, 0, sizeof(AX_NPU_CV_Image));
    dst->nWidth = dst_w;
    dst->nHeight = dst_h;
    dst->tStride.nW = dst_w;
    dst->eDtype = src_img->eDtype;
    dst->nSize = get_image_data_size(dst);
    assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst->pPhy), (void**) &dst->pVir, (size_t) dst->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));
    memset(dst->pVir, 0, dst->nSize);

    const float mat33[3][3] = {
        {1.3130770040162774, 0.17364817766693033, 0},
        {-0.17364817766693033, 1.3130770040162774, 0},
        {0, 0, 1}
    };
    /*const float mat33[3][3] = {
        {0.709382, 0.288826, 107.611304},
        {0.041073, 1.067956, 51.734816},
        {0.000133, 0.000734, 1.0}
    };*/
    AX_NPU_CV_Interp interp = AX_NPU_CV_BILINEAR;

    const int test_count = nRepeat;
    auto task_start= std::chrono::system_clock::now();
    for (int i = 0; i < test_count; ++i) {
        assert(0 == AX_NPU_CV_Warp(virtual_npu_mode_type, src_img, dst, &mat33[0][0], interp, 0));
    }
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / test_count;
    printf("Run tasks took %d us (%d rounds for average)\n\n", (int)duration.count(), test_count);

    std::string out_dir = args.get<std::string>("output");
    if (!out_dir.empty()) {
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
            return 1;
        }
        char fn[256];
        sprintf(fn, "%s/npu_warp.HxW_%dx%d.%s.bin", out_dir.c_str(), dst->nHeight, dst->nWidth, format.c_str());
        dump_image_to_file(fn, dst);
    }

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst->pPhy), (void*) dst->pVir));
    delete dst;

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src_img->pPhy), (void*) src_img->pVir));
    delete src_img;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();
    return 0;
}
