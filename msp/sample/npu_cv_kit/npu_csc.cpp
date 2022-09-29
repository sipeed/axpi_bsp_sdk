#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <string.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "npu_common.h"
#include <sys/stat.h>
#include "img_helper.h"

#define max(a,b) ((a)>(b) ? (a): (b))
std::string g_csc_string[][2] = {
    {"NV12/NV21", "BGR/YUV444/LAB"},
    {"YUV444", "GRAY"},
    {"RAW10", "RAW16"},
    {"RAW12", "RAW16"},
    {"RGB", "BGR/LAB"},
    {"BGR", "NV12/YUV444/RGB"},
    {"YUYV/UYVY", "NV12/NV21"},
    {"YUV420_LEGACY", "NV12"},
};

std::string format_csc_string(int n){
    int i = 0;
    std::string ret = "";
    while(!g_csc_string[i][n].empty()){
        int ll = max(g_csc_string[i][0].size(), g_csc_string[i][1].size());
        ll += 2;
        std::string fill(ll-g_csc_string[i][n].size(), ' ');
        ret += "|-"+g_csc_string[i][n]+fill;
        i++;
    }
    return ret;
}
int main(int argc, char* argv[]) {

#ifdef X86

    printf("only supports ax620a\n");
    return 1;

#else

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string>("image", '\0', "input image", false);
    args.add<AX_U32>("width", '\0', "width of image", true);
    args.add<AX_U32>("height", '\0', "height of image", true);
    args.add<AX_U32>("src-stride", '\0', "src stride", false, 0);
    args.add<AX_U32>("dst-stride", '\0', "dst stride", false, 0);
    args.add<std::string>("from", '\0', format_csc_string(0), true);
    args.add<std::string>("to", '\0',   format_csc_string(1), true);
    args.add<int>("repeat", '\0', "repeat times", false, 1);
    args.add<std::string>("output", '\0', "Dump output results. The user ensures that the file path exists.", false);


    // example:
    // ./bin/npu_csc.ax620a --mode 3_1 --mode-type 3_1_2 --image 640x480.bgr --width 640 --height 480 --from BGR --to NV12
    args.parse_check(argc, argv);

    assert(AX_SYS_Init() == 0);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_NPU_CV_Image* src_img = new AX_NPU_CV_Image;
    memset(src_img, 0, sizeof(AX_NPU_CV_Image));
    src_img->nWidth = args.get<AX_U32>("width");
    src_img->nHeight = args.get<AX_U32>("height");
    src_img->tStride.nW = args.get<AX_U32>("src-stride") ? args.get<AX_U32>("src-stride") : src_img->nWidth;

    std::string src_img_cs = args.get<std::string>("from");
    std::string dst_img_cs = args.get<std::string>("to");
    src_img->eDtype = get_color_space(src_img_cs);

    long size = get_image_data_size(src_img);
    std::string image_fn = args.get<std::string>("image");
    load_file(image_fn, (void**) &src_img->pVir, reinterpret_cast<AX_U64*>(&src_img->pPhy), &size);
    src_img->nSize = size;

    assert(get_image_data_size(src_img) == size);
    printf("src_img image h %d, w %d, color space: %s\n", src_img->nHeight, src_img->nWidth, src_img_cs.c_str());

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    {
        AX_NPU_CV_Image* dst = new AX_NPU_CV_Image;

        memset(dst, 0, sizeof(AX_NPU_CV_Image));
        dst->nWidth = src_img->nWidth;
        dst->nHeight = src_img->nHeight;
        dst->tStride.nW = args.get<AX_U32>("dst-stride") ? args.get<AX_U32>("dst-stride") : dst->nWidth;
        dst->eDtype = get_color_space(dst_img_cs);
        dst->nSize = get_image_data_size(dst);
        //dst->nSize = src_img->nWidth*src_img->nHeight*4*4;

        assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst->pPhy), (void**) &dst->pVir, (size_t) dst->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));

        const int test_count = args.get<int>("repeat");
        auto task_start= std::chrono::system_clock::now();
        for (int i = 0; i < test_count; ++i) {
            assert(0 == AX_NPU_CV_CSC(virtual_npu_mode_type, src_img, dst));
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
            char fn[256];
            sprintf(fn, "%s/npu_csc_from_%s_to_%s.vnpu_%d.HxW_%dx%d.bin",
                    out_dir.c_str(),
                    src_img_cs.c_str(), dst_img_cs.c_str(),
                    virtual_npu_mode_type,
                    dst->nHeight, dst->tStride.nW);
            dump_image_to_file(fn, dst);
        }

        assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst->pPhy), (void*) dst->pVir));
        delete dst;
    }

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src_img->pPhy), (void*) src_img->pVir));
    delete src_img;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;

#endif  // _PLATFORM_ARM_
}
