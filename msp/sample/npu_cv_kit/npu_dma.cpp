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

    cmdline::parser args;
    args.add<std::string> ("mode", '\0', "NPU hard mode: disable, 1_1", false, "disable");
    args.add<std::string> ("mode-type", '\0', "Virtual NPU mode type: disable， 1_1_1， 1_1_2", false, "disable");
    args.add<std::string>("image", '\0', "input image for testing NPU-CV", false);
    args.add<AX_U32>("width", '\0', "width of image", true);
    args.add<AX_U32>("height", '\0', "height of image", true);
    args.add<AX_U32>("src-stride", '\0', "stride of src image, currently only supports set the stride of width", false);
    args.add<std::string>("color-space", '\0', "NV12/NV21/RGB/BGR/GRAY", true);
    args.add<std::string>("dma-mode", '\0', "direct-copy/interval-copy", true);
    args.add<AX_U32>("dst-stride", '\0', "stride of dst image, currently only supports set the stride of width", false);
    args.add<std::string>("box", '\0', "Input coordinate of bounding box. The format is x1,y1,w1,h1", false);
    args.add<AX_U32>("seg-size", '\0', "size of splitted segment of each line", false);
    args.add<AX_U32>("ele-size", '\0', "size of which need to be written", false);
    args.add<AX_U32>("seg-rows", '\0', "interval rows of which need to be written", false);

    args.add<std::string>("output", '\0', "Dump output results. The user ensures that the file path exists.", false);

    args.add<int>("repeat", '\0', "Repeat times", false, 1);

    // example:
    // ./bin/npu_crop_resize.ax620a --mode disable --mode-type disable --image 640x480.nv12.bin --width 640 --height 480 --color-space NV12 --boxes 0,0,300,200:100,100,200,300 --oh 128 --ow 128 --vertical 2 --horizontal 2 --output output --repeat 100
    args.parse_check(argc, argv);

    assert(AX_SYS_Init() == 0);

    AX_NPU_SDK_EX_ATTR_T hard_mode = get_npu_hard_mode(args.get<std::string>("mode"));
    assert(AX_NPU_SDK_EX_Init_with_attr(&hard_mode) == 0);

    AX_NPU_CV_DMA_CTRL * pstCtrl = new AX_NPU_CV_DMA_CTRL;

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

    AX_NPU_CV_Image* dst_img = new AX_NPU_CV_Image;
    dst_img->eDtype = src_img->eDtype;

    AX_NPU_SDK_EX_MODEL_TYPE_T virtual_npu_mode_type = get_npu_mode_type(args.get<std::string>("mode-type"));

    std::string dma_mode = args.get<std::string>("dma-mode");
    if(dma_mode.compare("direct-copy") == 0){
        pstCtrl->mode = AX_NPU_CV_DMA_MODE_DIRECT_COPY;
    }else if(dma_mode.compare("interval-copy") == 0){
        pstCtrl->mode = AX_NPU_CV_DMA_MODE_INTERVAL_COPY;
    }else{
        assert(0);
    }
    std::string str_box = args.get<std::string>("box");

    if(!str_box.empty() && pstCtrl->mode == AX_NPU_CV_DMA_MODE_DIRECT_COPY){
        int x,y,w,h;
        sscanf(str_box.c_str(), "%d,%d,%d,%d", &x, &y, &w, &h);
        pstCtrl->box.fX = x;
        pstCtrl->box.fY = y;
        pstCtrl->box.fW = w;
        pstCtrl->box.fH = h;

        dst_img->nWidth = w;
        dst_img->nHeight = h;
    }

    if(pstCtrl->mode == AX_NPU_CV_DMA_MODE_INTERVAL_COPY){
        pstCtrl->u8HorSegSize = args.get<AX_U32>("seg-size");
        pstCtrl->u8VerSegRows = args.get<AX_U32>("seg-rows");
        pstCtrl->u8ElemSize = args.get<AX_U32>("ele-size");
        assert(pstCtrl->u8HorSegSize != 0 && pstCtrl->u8VerSegRows != 0 && pstCtrl->u8ElemSize != 0);
        assert(src_img->nWidth % pstCtrl->u8HorSegSize == 0);
        dst_img->nWidth = src_img->nWidth/pstCtrl->u8HorSegSize * pstCtrl->u8ElemSize;
        dst_img->nHeight = src_img->nHeight / pstCtrl->u8VerSegRows;
    }

    dst_img->tStride.nW = args.get<AX_U32>("dst-stride");
    if(dst_img->tStride.nW <= 0) dst_img->tStride.nW = dst_img->nWidth;
    size = get_image_data_size(dst_img);
    load_file("", (void**)&(dst_img->pVir), &(dst_img->pPhy), &size);
    dst_img->nSize = size;

    const int test_count = args.get<int>("repeat");
    auto task_start= std::chrono::system_clock::now();
    for (int i = 0; i < test_count; ++i) {
        assert(0 == HI_MPI_IVE_DMA(
            virtual_npu_mode_type, src_img, dst_img, pstCtrl));
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
        sprintf(fn, "%s/dma.vnpu_%d.src_%dx%d.mode_%s.%dx%d.%s.bin",
            out_dir.c_str(),
            virtual_npu_mode_type,
            src_img->nWidth, src_img->nHeight,
            dma_mode.c_str(),
            dst_img->nWidth, dst_img->nHeight,
            src_img_cs.c_str());
        dump_image_to_file(fn, dst_img);
    }


    release_image_buffer(src_img);
    release_image_buffer(dst_img);

    delete pstCtrl;

    AX_NPU_SDK_EX_Deinit();
    AX_SYS_Deinit();

    return 0;
}
