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

    printf("only supports ax630a\n");
    return 1;

#else

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string>("image", '\0', "input image for testing NPU-CV", false);
    args.add<AX_U32>("width", '\0', "width of image", true);
    args.add<AX_U32>("height", '\0', "height of image", true);
    args.add<AX_U32>("src-stride", '\0', "stride of src image, currently only supports set the stride of width", false, 0);

    args.add<std::string>("boxes", '\0', "Input coordinates of bounding box split by colon. The format is x1,y1,w1,h1:x2,y2,w2,h2... to specify input boxes", true);

    args.add<AX_U32>("oh", '\0', "height of output image", false, 128);
    args.add<AX_U32>("ow", '\0', "width of output image", false, 128);
    args.add<AX_U32>("dst-stride", '\0', "stride of dst image, currently only supports set the stride of width", false, 0);
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
    args.add<AX_U32>("row", '\0', "width of output image", false, 1);
    args.add<AX_U32>("column", '\0', "width of output image", false, 1);

    args.add<std::string>("output", '\0', "Dump output results. The user ensures that the file path exists.", false);

    args.add<int>("repeat", '\0', "Repeat times", false, 1);

    // example:
    // ./bin/npu_crop_resize_for_split_yuv.ax620a  --mode disable --mode-type disable --image 4096x2048.nv12.bin  --width 4096 --height 2048 --boxes 0,0,512,512:512,512,512,512:0,512,512,512:1146,630,512,512 --oh 998 --ow 1000 --vertical 0 --horizontal 0 --output . --repeat 1 --src-stride 4094 --dst-stride 1022 --row 3 --column 3
    args.parse_check(argc, argv);

    assert(AX_SYS_Init() == 0);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_U32 width = args.get<AX_U32>("width");
    AX_U32 height = args.get<AX_U32>("height");
    AX_U32 src_stride = args.get<AX_U32>("src-stride");

    // load source NV12 image
    AX_NPU_CV_Image* src_img = new AX_NPU_CV_Image;
    src_img->eDtype = AX_NPU_CV_FDT_NV12;
    src_img->nWidth = width;
    src_img->nHeight = height;
    src_img->tStride.nW = src_stride > width ? src_stride : width;
    long size = get_image_data_size(src_img);
    std::string image_fn = args.get<std::string>("image");
    load_file(image_fn, (void**) &src_img->pVir, reinterpret_cast<AX_U64*>(&src_img->pPhy), &size);

    // store Y into new image(Gray)
    AX_NPU_CV_Image* src_y_img = new AX_NPU_CV_Image;
    src_y_img->nWidth = width;
    src_y_img->nHeight = height;
    src_y_img->tStride.nW = src_stride > width ? src_stride : width;
    src_y_img->eDtype = AX_NPU_CV_FDT_GRAY;
    src_y_img->nSize = get_image_data_size(src_y_img);
    assert(size == long(src_y_img->nSize * 3 / 2));
    assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&src_y_img->pPhy),
            (void**) &src_y_img->pVir, (size_t) src_y_img->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));
    memcpy(src_y_img->pVir, src_img->pVir, src_y_img->nSize);

    // store UV into new image(UV)
    // FIXME 将UV 整改成独立的通道
    AX_NPU_CV_Image* src_uv_img = new AX_NPU_CV_Image;
    src_uv_img->nWidth = src_y_img->nWidth / 2;
    src_uv_img->nHeight = src_y_img->nHeight / 2;
    src_uv_img->tStride.nW = src_y_img->tStride.nW / 2;
    src_uv_img->eDtype = AX_NPU_CV_FDT_UV;
    src_uv_img->nSize = get_image_data_size(src_uv_img);
    assert(size == long(src_uv_img->nSize * 3));
    assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&src_uv_img->pPhy),
                (void**) &src_uv_img->pVir, (size_t) src_uv_img->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));
    memcpy(src_uv_img->pVir, src_img->pVir + src_y_img->nSize, src_uv_img->nSize);

    printf("src_img image h %d, w %d, stride w %d\n", src_y_img->nHeight, src_y_img->nWidth, src_y_img->tStride.nW);

    std::string str_boxes = args.get<std::string>("boxes");
    std::vector<std::string> input_boxes = split_string(str_boxes, ":");
    int dest_size = input_boxes.size();
    assert (dest_size > 0);

    AX_NPU_CV_Box** boxes = new AX_NPU_CV_Box*[dest_size];  // Y and UV share the same boxes
    for (int i = 0; i < dest_size; ++i) {
        int _x, _y, _w, _h;
        sscanf(input_boxes[i].c_str(), "%d,%d,%d,%d", &_x, &_y, &_w, &_h);

        AX_NPU_CV_Box* box = new AX_NPU_CV_Box;
        box->fX = _x;
        box->fY = _y;
        box->fW = _w;
        box->fH = _h;
        boxes[i] = box;
    }

    AX_U32 row = args.get<AX_U32>("row");
    AX_U32 column = args.get<AX_U32>("column");
    AX_U32 dst_block_h = args.get<AX_U32>("oh") / row / 2 * 2;
    AX_U32 dst_block_w = args.get<AX_U32>("ow") / column / 2 * 2;
    printf("row %d, clomn %d, dst block hxw:%dx%d, size %d\n", row, column, dst_block_h, dst_block_w, dest_size);

    // 模拟拼图
    AX_NPU_CV_Image* dst_img = new AX_NPU_CV_Image;
    dst_img->nHeight = args.get<AX_U32>("oh");
    dst_img->nWidth = args.get<AX_U32>("ow");
    AX_U32 dst_stride = args.get<AX_U32>("dst-stride");
    dst_img->tStride.nW = dst_stride > dst_img->nWidth ? dst_stride : dst_img->nWidth;
    dst_img->eDtype = AX_NPU_CV_FDT_NV12;
    dst_img->nSize = get_image_data_size(dst_img);
    assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst_img->pPhy),
                (void**) &dst_img->pVir, (size_t) dst_img->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));

    // 填充黑色背景
    memset(dst_img->pVir, 0x00 , dst_img->tStride.nW * dst_img->nHeight);
    memset(dst_img->pVir + dst_img->tStride.nW * dst_img->nHeight, 0x80, dst_img->tStride.nW * dst_img->nHeight / 2);

    AX_NPU_CV_Image** dest_y_images = new AX_NPU_CV_Image*[dest_size];
    AX_NPU_CV_Image** dest_uv_images = new AX_NPU_CV_Image*[dest_size];
    for (AX_U32 i = 0; i < row; i++) {
        for (AX_U32 j = 0; i < column; j++) {
            int block_index = i * column + j;
            if (block_index >= dest_size) break;

            // 计算 Y 偏移地址
            AX_NPU_CV_Image* dst_y_img = new AX_NPU_CV_Image;
            dst_y_img->nHeight = dst_block_h;
            dst_y_img->nWidth =dst_block_w;
            dst_y_img->tStride.nW = dst_img->tStride.nW;
            dst_y_img->eDtype = AX_NPU_CV_FDT_GRAY;
            dst_y_img->nSize = get_image_data_size(dst_y_img);
            dst_y_img->pVir = dst_img->pVir + (i * dst_block_h * dst_img->tStride.nW + j * dst_block_w);
            dst_y_img->pPhy = dst_img->pPhy + (i * dst_block_h * dst_img->tStride.nW + j * dst_block_w);
            dest_y_images[block_index] = dst_y_img;  // Y

            // 计算 UV 偏移地址
            AX_NPU_CV_Image* dst_uv_img = new AX_NPU_CV_Image;
            dst_uv_img->nHeight = dst_block_h / 2;
            dst_uv_img->nWidth = dst_block_w / 2;
            dst_uv_img->tStride.nW = dst_img->tStride.nW / 2;
            dst_uv_img->eDtype = AX_NPU_CV_FDT_UV;
            dst_uv_img->nSize = get_image_data_size(dst_uv_img);
            dst_uv_img->pVir = dst_img->pVir + dst_img->nHeight * dst_img->tStride.nW + (i * dst_block_h / 2 * dst_img->tStride.nW + j * dst_block_w);
            dst_uv_img->pPhy = dst_img->pPhy + dst_img->nHeight * dst_img->tStride.nW + (i * dst_block_h / 2 * dst_img->tStride.nW + j * dst_block_w);
            dest_uv_images[block_index] = dst_uv_img; // UV
        }
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
        assert(0 == AX_NPU_CV_CropResizeImageForSplitYUV(
            virtual_npu_mode_type, src_y_img, src_uv_img, dest_size, dest_y_images, dest_uv_images, boxes, horizontal, vertical, tColorBlack));
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

        char fn[256];
        sprintf(fn, "%s/crop_resize_dst_yuv.vnpu_%d.horizontal_%d_vertical_%d.%dx%d.bin",
                out_dir.c_str(),
                virtual_npu_mode_type,
                horizontal, vertical,
                dst_img->tStride.nW, dst_img->nHeight);
        FILE* fout = fopen(fn, "wb");
        fwrite(dst_img->pVir, 1, dst_img->nSize, fout);
        fclose(fout);
        printf("dumpped image as '%s'\n", fn);

        for (int i = 0; i < dest_size; i++) {
            AX_NPU_CV_Image* dst_y_img = dest_y_images[i];
            char fn[256];
            sprintf(fn, "%s/crop_resize_y_%d.vnpu_%d.src_%dx%d_box_%d_%d_%d_%d.horizontal_%d_vertical_%d.%dx%d.bin",
                    out_dir.c_str(), i,
                    virtual_npu_mode_type,
                    src_img->nWidth, src_img->nHeight,
                    (int32_t)boxes[i]->fX, (int32_t)boxes[i]->fY, (int32_t)boxes[i]->fW, (int32_t)boxes[i]->fH,
                    horizontal, vertical,
                    dst_y_img->tStride.nW, dst_y_img->nHeight);
            dump_image_to_file(fn, dst_y_img);

            AX_NPU_CV_Image* dst_uv_img = dest_uv_images[i];
            memset(fn, 0, 256);
            sprintf(fn, "%s/crop_resize_uv_%d.vnpu_%d.src_%dx%d_box_%d_%d_%d_%d.horizontal_%d_vertical_%d.%dx%d.bin",
                    out_dir.c_str(), i,
                    virtual_npu_mode_type,
                    src_img->nWidth, src_img->nHeight,
                    (int32_t)boxes[i]->fX, (int32_t)boxes[i]->fY, (int32_t)boxes[i]->fW, (int32_t)boxes[i]->fH,
                    horizontal, vertical,
                    dst_uv_img->tStride.nW, dst_uv_img->nHeight);
            dump_image_to_file(fn, dst_uv_img);

            // concat Y and UV
            AX_NPU_CV_Image* dst_yuv_img = new AX_NPU_CV_Image;
            dst_yuv_img->nWidth = dst_y_img->nWidth;
            dst_yuv_img->nHeight = dst_y_img->nHeight;
            dst_yuv_img->tStride.nW = dst_y_img->tStride.nW;
            dst_yuv_img->eDtype = AX_NPU_CV_FDT_NV12;
            dst_yuv_img->nSize = get_image_data_size(dst_yuv_img);
            assert(0 == AX_SYS_MemAlloc(reinterpret_cast<AX_U64*>(&dst_yuv_img->pPhy),
                        (void**) &dst_yuv_img->pVir, (size_t) dst_yuv_img->nSize, PHY_MEM_ALIGN_SIZE, (AX_S8*)"NPU-CV"));
            memcpy(dst_yuv_img->pVir, dst_y_img->pVir, dst_y_img->nSize);
            memcpy(dst_yuv_img->pVir + dst_y_img->nSize, dst_uv_img->pVir, dst_uv_img->nSize);

            memset(fn, 0, 256);
            sprintf(fn, "%s/crop_resize_yuv_%d.vnpu_%d.src_%dx%d_box_%d_%d_%d_%d.horizontal_%d_vertical_%d.%dx%d.bin",
                out_dir.c_str(), i,
                virtual_npu_mode_type,
                src_img->nWidth, src_img->nHeight,
                (int32_t)boxes[i]->fX, (int32_t)boxes[i]->fY, (int32_t)boxes[i]->fW, (int32_t)boxes[i]->fH,
                horizontal, vertical,
                dst_yuv_img->tStride.nW, dst_yuv_img->nHeight);

            dump_image_to_file(fn, dst_yuv_img);

            assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst_yuv_img->pPhy), (void*) dst_yuv_img->pVir));
            delete dst_yuv_img;
        }
    }

    for (int i = 0; i < dest_size; i++) {
        delete dest_y_images[i];
        delete dest_uv_images[i];
    }
    delete[] dest_y_images;
    delete[] dest_uv_images;

    for (int i = 0; i < dest_size; ++i) delete boxes[i];
    delete[] boxes;

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src_img->pPhy), (void*) src_img->pVir));
    delete src_img;

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src_y_img->pPhy), (void*) src_y_img->pVir));
    delete src_y_img;

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(src_uv_img->pPhy), (void*) src_uv_img->pVir));
    delete src_uv_img;

    assert(0 == AX_SYS_MemFree(reinterpret_cast<AX_U64>(dst_img->pPhy), (void*) dst_img->pVir));
    delete dst_img;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;

#endif  // _PLATFORM_ARM_
}
