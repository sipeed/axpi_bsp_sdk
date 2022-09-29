#include <cstdio>
#include <assert.h>
#include <string>
#include <chrono>
#include <string.h>
#include "ax_sys_api.h"
#include "ax_npu_imgproc.h"
#include "cmdline.h"
#include <sys/stat.h>
#include "img_helper.h"

static void test_crop(AX_NPU_CV_Image* src, AX_NPU_CV_Box** boxes, AX_U32 dest_size, AX_NPU_SDK_EX_MODEL_TYPE_T npu_mode_type, int repeat, std::string& outdir) {
    AX_NPU_CV_Image** dest_images = new AX_NPU_CV_Image*[dest_size];
    for (AX_U32 i = 0; i < dest_size; ++i) {
        AX_NPU_CV_Image* dst = new AX_NPU_CV_Image;
        dst->nWidth = boxes == nullptr ?src->nWidth : boxes[i]->fW;
        dst->nHeight = boxes == nullptr ? src->nHeight : boxes[i]->fH;
        dst->tStride.nW = dst->nWidth; // TODO
        dst->eDtype = src->eDtype;
        dst->nSize = get_image_data_size(dst);

        assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst->pPhy),
                    (void**) &dst->pVir, (size_t) dst->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));

        dest_images[i] = dst;
    }

    auto task_start= std::chrono::system_clock::now();
    for (int i = 0; i < repeat; ++i) {
        assert(0 == AX_NPU_CV_CropImage(npu_mode_type, src, dest_size, dest_images, boxes));
    }
    auto time_end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(time_end - task_start) / repeat;

    AX_NPU_SDK_EX_WORK_CYCLE_T cur = get_current_work_cycle();
    print_work_cycle(cur);
    printf("Run tasks took %llu us (%d rounds for average)\n\n", duration.count(), repeat);

    if (!outdir.empty()) {
        struct stat info;
        if (stat(outdir.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
            fprintf(stderr, "cannot access directory '%s'\n", outdir.c_str());
        } else {
            for (AX_U32 i = 0; i < dest_size; ++i) {
                AX_NPU_CV_Image* dst = dest_images[i];
                char fn[128];
                sprintf(fn, "%s/crop_%d.vnpu_%d.src_%dx%d.%dx%d.color_space_%d.bin",
                    outdir.c_str(), i,
                    npu_mode_type,
                    src->nWidth, src->nHeight,
                    dst->nWidth, dst->nHeight,
                    dst->eDtype);
                dump_image_to_file(fn, dst);
            }
        }
    }

    for (AX_U32 i = 0; i < dest_size; ++i) {
        AX_NPU_CV_Image* dst = dest_images[i];
        assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst->pPhy), (void*) dst->pVir));
        delete dst;
    }
    delete[] dest_images;
}

int main(int argc, char* argv[]) {

    printf("NPU-CV-KIT Version:%s\n", AX_NPU_CV_Version());

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string>("image", '\0', "input image", false);
    args.add<AX_U32>("width", '\0', "width of image", true);
    args.add<AX_U32>("height", '\0', "height of image", true);
    args.add<std::string>("color-space", '\0', "color-space for image, one of NV12/NV21/RGB/BGR/GRAY", true);
    args.add<std::string>("boxes", '\0', "Input coordinates of bounding box split by colon. The format is x1,y1,w1,h1:x2,y2,w2,h2... to specify input boxes, default box is 0,0,width,height", false);
    args.add<int>("repeat", '\0', "repeat times", false, 1);
    args.add<std::string>("output", '\0', "Dump output results. The user ensures that the file path exists.", false);

    // example:
    // LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH ./bin/npu_crop.ax620a  --mode disable --mode-type disable --image img/640x640.bgr  --width 640 --height 640 --color-space BGR --boxes 0,0,128,128
    args.parse_check(argc, argv);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));

    assert(AX_SYS_Init() == 0);
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_NPU_CV_Image* src = new AX_NPU_CV_Image;
    src->nWidth = args.get<AX_U32>("width");
    src->nHeight = args.get<AX_U32>("height");
    src->tStride.nW = src->nWidth; // TODO
    src->eDtype = get_color_space(args.get<std::string>("color-space"));
    long size = get_image_data_size(src);
    std::string image_fn = args.get<std::string>("image");
    load_file(image_fn, (void**) &src->pVir, reinterpret_cast<AX_U64*>(&src->pPhy), &size);
    src->nSize = size;
    assert(get_image_data_size(src) == size);
    printf("src image h %d, w %d, color space %s\n", src->nHeight, src->nWidth, args.get<std::string>("color-space").c_str());

    int dest_size = 1;
    AX_NPU_CV_Box** boxes = nullptr;
    std::string str_boxes = args.get<std::string>("boxes");
    if (!str_boxes.empty()) {
        std::vector<std::string> input_boxes = split_string(str_boxes, ":");
        dest_size = input_boxes.size();
        if (dest_size > 0) {
            boxes = new AX_NPU_CV_Box*[dest_size];
            for (int i = 0; i < dest_size; ++i) {
                int _x, _y, _w, _h;
                sscanf(input_boxes[i].c_str(), "%d,%d,%d,%d", &_x, &_y, &_w, &_h);
                AX_NPU_CV_Box* box = new AX_NPU_CV_Box;
                box->fX = _x;
                box->fY = _y;
                box->fW = _w;
                box->fH = _h;
                boxes[i] = box;
                printf("box[%d] xywh: %f, %f, %f, %f\n", i, box->fX, box->fY, box->fW, box->fH);
            }
        } else {
            dest_size = 1;
            boxes = nullptr;
        }
    }

    AX_NPU_SDK_EX_MODEL_TYPE_T npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));
    if (dest_size > 0 && src) {
        const int repeat = args.get<int>("repeat");
        std::string output_dir = args.get<std::string>("output");
        test_crop(src, boxes, dest_size, npu_mode_type, repeat, output_dir);
    }

    if (boxes) {
        for (int i = 0; i < dest_size; ++i) delete boxes[i];
        delete[] boxes;
    }

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src->pPhy), (void*) src->pVir));
    delete src;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}