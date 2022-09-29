#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <string.h>
#include <sys/stat.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include "img_helper.h"

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n",AX_NPU_CV_Version());

#ifdef X86

    printf("only supports ax620a\n");
    return 1;

#else

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string>("image", '\0', "input image for testing NPU-CV", false);
    args.add<AX_U32>("width", '\0', "width of image", true);
    args.add<AX_U32>("height", '\0', "height of image", true);
    args.add<AX_U32>("src-stride", '\0', "stride of src image, currently only supports set the stride of width", false);
    args.add<std::string>("color-space", '\0', "NV12/NV21/RGB/BGR/GRAY", true);

    args.add<std::string>("boxes", '\0', "Input coordinates of bounding box split by colon. The format is x1,y1,w1,h1:x2,y2,w2,h2... to specify input boxes", false);

    args.add<AX_U32>("oh", '\0', "height of output image", false, 128);
    args.add<AX_U32>("ow", '\0', "width of output image", false, 128);
    args.add<AX_U32>("dst-stride", '\0', "stride of dst image, currently only supports set the stride of width", false);
    args.add<AX_U32>("horizontal", '\0', "Image alignment on horizontal:\n\
            \t\t 0 means without keep aspect ratio, others keep aspect ratio\n\
            \t\t 1 means border on the right of the image\n\
            \t\t 2 means border on both sides of the image\n\
            \t\t 3 means border on the left of the image", false, 0, cmdline::range(0,3));

    args.add<AX_U32>("vertical", '\0', "Image alignment on vertical:\n\
            \t\t 0 means without keep aspect ratio, others keep aspect ratio\n\
            \t\t 1 means border on the bottom of the image\n\
            \t\t 2 means border on both sides of the image\n\
            \t\t 3 means border on the top of the image", false, 0, cmdline::range(0,3));

    args.add<std::string>("output", '\0', "Dump output results. The user ensures that the file path exists.", false);

    args.add<int>("repeat", '\0', "Repeat times", false, 1);

    // example:
    // ./bin/npu_crop_resize.ax620a --mode disable --mode-type disable --image 640x480.nv12.bin --width 640 --height 480 --color-space NV12 --boxes 0,0,300,200:100,100,200,300 --oh 128 --ow 128 --vertical 2 --horizontal 2 --output output --repeat 100
    args.parse_check(argc, argv);

    assert(AX_SYS_Init() == 0);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_NPU_CV_Image* src_img = new AX_NPU_CV_Image;
    src_img->nWidth = args.get<AX_U32>("width");
    src_img->nHeight = args.get<AX_U32>("height");
    src_img->tStride.nW = args.get<AX_U32>("src-stride");
    if (src_img->tStride.nW <= 0) src_img->tStride.nW = src_img->nWidth;
    std::string src_img_cs = args.get<std::string>("color-space");
    src_img->eDtype = get_color_space(src_img_cs);

    long size = get_image_data_size(src_img);
    std::string image_fn = args.get<std::string>("image");
    load_file(image_fn, (void**) &src_img->pVir, reinterpret_cast<AX_U64*>(&src_img->pPhy), &size);

    src_img->nSize = size;
    assert(get_image_data_size(src_img) == size);
    printf("src_img image h %d, w %d, stride w %d, color space: %s\n", src_img->nHeight, src_img->nWidth, src_img->tStride.nW, src_img_cs.c_str());

    std::string str_boxes = args.get<std::string>("boxes");
    AX_NPU_CV_Box** boxes = nullptr;
    AX_U32 dest_size = 1;
    if (!str_boxes.empty()) {
        std::vector<std::string> input_boxes = split_string(str_boxes, ":");
        dest_size = input_boxes.size();
        if (dest_size > 0) {
            boxes = new AX_NPU_CV_Box*[dest_size];
            for (AX_U32 i = 0; i < dest_size; ++i) {
                int _x, _y, _w, _h;
                sscanf(input_boxes[i].c_str(), "%d,%d,%d,%d", &_x, &_y, &_w, &_h);

                AX_NPU_CV_Box* box = new AX_NPU_CV_Box;
                box->fX = _x;
                box->fY = _y;
                box->fH = _h;
                box->fW = _w;
                boxes[i] = box;
                printf("box[%d] xywh: %f, %f, %f, %f\n", i, box->fX, box->fY, box->fW, box->fH);
            }
        } else {
            dest_size = 1;
            boxes = nullptr;
        }
    }

    AX_NPU_CV_Image** dest_images = new AX_NPU_CV_Image*[dest_size];
    for (AX_U32 i = 0; i < dest_size; ++i) {
        AX_NPU_CV_Image* dst_img = new AX_NPU_CV_Image;
        dst_img->nHeight = args.get<AX_U32>("oh");
        dst_img->nWidth = args.get<AX_U32>("ow");
        dst_img->tStride.nW = args.get<AX_U32>("dst-stride");
        if (dst_img->tStride.nW <= 0) dst_img->tStride.nW = dst_img->nWidth;
        dst_img->eDtype = src_img->eDtype;
        dst_img->nSize = get_image_data_size(dst_img);
        assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst_img->pPhy),
                    (void**) &dst_img->pVir, (size_t) dst_img->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));
        dest_images[i] = dst_img;
    }

    AX_NPU_CV_Color tColorBlack;
    tColorBlack.nYUVColorValue[0] = 0;
    tColorBlack.nYUVColorValue[1] = 128;
    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));
    AX_NPU_CV_ImageResizeAlignParam horizontal = (AX_NPU_CV_ImageResizeAlignParam)args.get<AX_U32>("horizontal");
    AX_NPU_CV_ImageResizeAlignParam vertical = (AX_NPU_CV_ImageResizeAlignParam)args.get<AX_U32>("vertical");
    const int test_count = args.get<int>("repeat");
    auto task_start= std::chrono::system_clock::now();
    for (int i = 0; i < test_count; ++i) {
        assert(0 == AX_NPU_CV_CropResizeImage(
            virtual_npu_mode_type, src_img, dest_size, dest_images, boxes, horizontal, vertical, tColorBlack));
    }
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / test_count;

    AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
    print_work_cycle(cur);
    printf("Run task took %lld us (%d rounds for average)\n\n", duration.count(), test_count);

    std::string out_dir = args.get<std::string>("output");
    if (!out_dir.empty()) {
        struct stat info;
        if (stat(out_dir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", out_dir.c_str());
            return 1;
        }
        for (AX_U32 i = 0; i < dest_size; ++i) {
            AX_NPU_CV_Image* dst_img = dest_images[i];
            char fn[256];
            sprintf(fn, "%s/crop_resize_%d.vnpu_%d.src_%dx%d_box_%d_%d_%d_%d.horizontal_%d_vertical_%d.%dx%d.%s.bin",
                out_dir.c_str(), i,
                virtual_npu_mode_type,
                src_img->nWidth, src_img->nHeight,
                 (int32_t)boxes[i]->fX,  (int32_t)boxes[i]->fY,  (int32_t)boxes[i]->fW,  (int32_t)boxes[i]->fH,
                horizontal, vertical,
                dst_img->nWidth, dst_img->nHeight,
                src_img_cs.c_str());
            dump_image_to_file(fn, dst_img);
        }
    }

    for (AX_U32 i = 0; i < dest_size; ++i) {
        AX_NPU_CV_Image* dst_img = dest_images[i];
        assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst_img->pPhy), (void*) dst_img->pVir));
        delete dst_img;
    }
    delete[] dest_images;
    if (boxes) {
        for (AX_U32 i = 0; i < dest_size; ++i) delete boxes[i];
        delete[] boxes;
    }

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src_img->pPhy), (void*) src_img->pVir));
    delete src_img;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;

#endif  // _PLATFORM_ARM_
}
