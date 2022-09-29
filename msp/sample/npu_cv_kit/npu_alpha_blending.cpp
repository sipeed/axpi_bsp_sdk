#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <sys/stat.h>
#include <string.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"

int main(int argc, char *argv[])
{

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

#ifdef X86

    printf("only supports ax620a\n");
    return 1;

#else
    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");

    args.add<std::string> ("bg", '\0', "input background image, only support NV12/NV21", false);
    args.add<AX_U32> ("bg-width", '\0', "background image width", true);
    args.add<AX_U32>("bg-height", '\0', "background image height", true);
    args.add<AX_U32>("bg-stride", '\0', "background image stride", false, 0);
    args.add<std::string>("bg-color-space", '\0', "color space for background image, NV12 or RGBA", true);

    args.add<std::string>("fg", '\0', "input foreground image, only support RGBA", false);
    args.add<AX_U32> ("fg-width", '\0', "foreground image width", true);
    args.add<AX_U32>("fg-height", '\0', "foreground image height", true);
    args.add<AX_U32>("fg-stride", '\0', "foreground image stride", false, 0);

    args.add<AX_U32>("dst-stride", '\0', "foreground image stride", false, 0);

    args.add<AX_U32>("offset-width", '\0', "offset Width for foreground", false, 0);
    args.add<AX_U32>("offset-height", '\0', "offset height for foreground", false, 0);

    args.add<int>("repeat", '\0', "repeat times", false, 1);
    args.add<std::string>("output", '\0', "Dump output results. The user ensures that the file path exists.", false);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_alpha_blend_dst.ax620a --bg back_1024x768.rgba --bg-width 1024 --bg-height 768 --bg-color-space rgba --fg front_640x600.rgba --fg-width 640 --fg-height 600 --output . --offset-width=20 --offset-height=20
    args.parse_check(argc, argv);

    assert(AX_SYS_Init() == 0);
    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    //load bg image
    AX_NPU_CV_Image *bg = new AX_NPU_CV_Image;
    bg->nWidth = args.get<AX_U32>("bg-width");
    bg->nHeight = args.get<AX_U32>("bg-height");
    bg->tStride.nW = bg->nWidth;
    bg->eDtype = get_color_space(args.get<std::string>("bg-color-space"));

    long size = get_image_data_size(bg);
    load_file(args.get<std::string>("bg"), (void **) &bg->pVir, reinterpret_cast<AX_U64 *>(&bg->pPhy), &size);
    bg->nSize = size;
    assert(get_image_data_size(bg) == size);
    printf("background image h %d, w %d, color space %d. addr=%llu\n", bg->nHeight, bg->nWidth, bg->eDtype, bg->pPhy);

    //load fg image
    AX_NPU_CV_Image *fg = new AX_NPU_CV_Image;
    fg->nWidth = args.get<AX_U32>("fg-width");
    fg->nHeight = args.get<AX_U32>("fg-height");
    fg->tStride.nW = fg->nWidth;
    fg->eDtype = AX_NPU_CV_FDT_RGBA;

    size = get_image_data_size(fg);
    load_file(args.get<std::string>("fg"), (void **) &fg->pVir, reinterpret_cast<AX_U64 *>(&fg->pPhy), &size);
    fg->nSize = size;
    assert(get_image_data_size(fg) == size);
    printf("foreground image h %d, w %d, color space %d. addr=%llu\n", fg->nHeight, fg->nWidth, fg->eDtype, fg->pPhy);

    int offset_w = args.get<AX_U32>("offset-width");
    int offset_h = args.get<AX_U32>("offset-height");

    AX_NPU_CV_Image *dst = new AX_NPU_CV_Image;
    dst->nWidth = args.get<AX_U32>("bg-width");
    dst->nHeight = args.get<AX_U32>("bg-height");
    dst->tStride.nW = bg->nWidth;
    dst->eDtype = AX_NPU_CV_FDT_RGBA;
    dst->tStride.nW = args.get<AX_U32>("dst-stride") <= 0 ? dst->nWidth : args.get<AX_U32>("dst-stride");

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    if (bg->eDtype == AX_NPU_CV_FDT_RGBA) {

        size = get_image_data_size(dst);
        load_file(args.get<std::string>("bg"), (void **) &dst->pVir, reinterpret_cast<AX_U64 *>(&dst->pPhy), &size);
        dst->nSize = size;
        assert(get_image_data_size(dst) == size);

        const int test_count = args.get<int>("repeat");
        auto task_start = std::chrono::system_clock::now();
        for (int i = 0; i < test_count; ++i) {
            assert(0 == AX_NPU_CV_AlphaBlending(virtual_npu_mode_type, bg, fg, dst, offset_w, offset_h));
        }
        auto time_end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / test_count;
        AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
        print_work_cycle(cur);
        printf("Run tasks took %lld us (%d rounds for average)\n\n", duration.count(), test_count);

        std::string out_dir = args.get<std::string>("output");
        if (!out_dir.empty()) {
            struct stat info;
            if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
                fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
                assert(0);
            }

            // dump result
            char fn[256];
            sprintf(fn, "%s/alpha_blending_bg_rgba_%dx%d_fg_%dx%d_dtype_%d.bin", out_dir.c_str(), bg->tStride.nW, bg->nHeight, fg->nWidth, fg->nHeight, (int)dst->eDtype);
            FILE *fout = fopen(fn, "wb");
            fwrite(dst->pVir, 1, dst->nSize, fout);
            fclose(fout);
            printf("dumpped image as '%s'\n", fn);
        }
    } else if (bg->eDtype == AX_NPU_CV_FDT_NV12) {
        bg->tStride.nW = args.get<AX_U32>("bg-stride") <= 0 ? bg->nWidth : args.get<AX_U32>("bg-stride");
        fg->tStride.nW = args.get<AX_U32>("fg-stride") <= 0 ? fg->nWidth : args.get<AX_U32>("fg-stride");

        size = get_image_data_size(dst);
        dst->nSize = size;
        assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst->pPhy), (void**) &dst->pVir, size, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));
        dst->tStride.nW = args.get<AX_U32>("dst-stride") <= 0 ? dst->nWidth : args.get<AX_U32>("dst-stride");

        const int test_count = args.get<int>("repeat");
        auto task_start= std::chrono::system_clock::now();
        for (int i = 0; i < test_count; ++i) {
            assert(0 == AX_NPU_CV_AlphaBlending(virtual_npu_mode_type, bg, fg, dst, offset_w, offset_h));
        }
        auto time_end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / test_count;
        AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
        print_work_cycle(cur);
        printf("Run tasks took %lld us (%d rounds for average)\n\n", duration.count(), test_count);

        std::string out_dir = args.get<std::string>("output");
        if (!out_dir.empty()) {
            struct stat info;
            if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
                fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
                assert(0);
            }

            // dump result
            char fn[256];
            sprintf(fn, "%s/alpha_blending_bg_nv12_%dx%d_fg_%dx%d_dtype_%d.bin", out_dir.c_str(), bg->tStride.nW, bg->nHeight, fg->nWidth, fg->nHeight, (int)bg->eDtype);
            dump_image_to_file(fn, dst);
        }
    }

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(bg->pPhy), (void *) bg->pVir));
    delete bg;
    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(fg->pPhy), (void *) fg->pVir));
    delete fg;
    if (dst != nullptr) {
        assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst->pPhy), (void *) dst->pVir));
        delete dst;
    }

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;

#endif  // _PLATFORM_ARM_
}
