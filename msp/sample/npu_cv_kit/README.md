1. npu_crop_resize
usage: ./npu_crop_resize.ax620a --width=unsigned int --height=unsigned int --color-space=string [options] ...
options:
      --mode           NPU hard mode: disable, 1_1 (string [=disable])
      --mode-type      Virtual NPU mode type: disable， 1_1_1， 1_1_2 (string [=disable])
      --image          input image for testing NPU-CV (string [=])
      --width          width of image (unsigned int)
      --height         height of image (unsigned int)
      --src-stride     stride of src image, currently only supports set the stride of width (unsigned int [=0])
      --color-space    NV12/NV21/RGB/BGR/GRAY (string)
      --boxes          Input coordinates of bounding box split by colon. The format is x1,y1,h1,w1:x2,y2,h2,w2... to specify input boxes (string [=])
      --oh             height of output image (unsigned int [=128])
      --ow             width of output image (unsigned int [=128])
      --dst-stride     stride of dst image, currently only supports set the stride of width (unsigned int [=0])
      --horizontal     Image alignment on horizontal:
                         0 means without keep aspect ratio, others keep aspect ratio
                         1 means border on the right of the image
                         2 means border on both sides of the image
                         3 means border on the left of the image (unsigned int [=0])
      --vertical       Image alignment on vertical:
                         0 means without keep aspect ratio, others keep aspect ratio
                         1 means border on the bottom of the image
                         2 means border on both sides of the image
                         3 means border on the top of the image (unsigned int [=0])
      --output         Dump output results. The user ensures that the file path exists. (string [=])
      --repeat         Repeat times (int [=1])
  -?, --help           print this message

example: 

./npu_crop_resize.ax620a   --mode 1_1 --mode-type 1_1_2 --image 3840x2160.nv12   --color-space NV12  --width 3840 --heigh
t 2160 --oh 1080 --ow 1920 --vertical 1 --horizontal 1 --output ./ --repeat 1

2. npu_crop_resize_for_split_yuv
usage: ./npu_crop_resize_for_split_yuv.ax620a --width=unsigned int --height=unsigned int --boxes=string [options] ...
options:
      --mode          NPU hard mode: disable, 1_1 (string [=disable])
      --mode-type     Virtual NPU mode type: disable， 1_1_1， 1_1_2 (string [=disable])
      --image         input image for testing NPU-CV (string [=])
      --width         width of image (unsigned int)
      --height        height of image (unsigned int)
      --src-stride    stride of src image, currently only supports set the stride of width (unsigned int [=0])
      --boxes         Input coordinates of bounding box split by colon. The format is x1,y1,h1,w1:x2,y2,h2,w2... to specify input boxes (string)
      --oh            height of output image (unsigned int [=128])
      --ow            width of output image (unsigned int [=128])
      --dst-stride    stride of dst image, currently only supports set the stride of width (unsigned int [=0])
      --horizontal    Image alignment on horizontal:
                         0 means without keep aspect ratio, others keep aspect ratio
                         1 means border on the right of the image
                         2 means border on both sides of the image
                         3 means border on the left of the image (unsigned int [=0])
      --vertical      Image alignment on vertical:
                         0 means without keep aspect ratio, others keep aspect ratio
                         1 means border on the bottom of the image
                         2 means border on both sides of the image
                         3 means border on the top of the image (unsigned int [=0])
      --row           width of output image (unsigned int [=1])
      --column        width of output image (unsigned int [=1])
      --output        Dump output results. The user ensures that the file path exists. (string [=])
      --repeat        Repeat times (int [=1])
  -?, --help          print this message

example: 

./npu_crop_resize_for_split_yuv.ax620a  --mode disable --mode-type disable --image 4096x2048.nv12.bin  --width 4096 --height 2048 --boxes 0,0,512,512:512,512,512,512:0,512,512,512:1146,630,512,512 --oh 998 --ow 1000 --vertical 0 --horizontal 0 --output . --repeat 1 --src-stride 4094 --dst-stride 1022 --row 3 --column 3


3. crop
usage: ./npu_crop.ax620a --width=unsigned int --height=unsigned int --color-space=string [options] ...
options:
      --mode           NPU hard mode: disable, 1_1 (string [=disable])
      --mode-type      Virtual NPU mode type: disable， 1_1_1， 1_1_2 (string [=disable])
      --image          input image (string [=])
      --width          width of image (unsigned int)
      --height         height of image (unsigned int)
      --color-space    color-space for image, one of NV12/NV21/RGB/BGR/GRAY (string)
      --boxes          Input coordinates of bounding box split by colon. The format is x1,y1,w1,h1:x2,y2,w2,h2... to specify input boxes, default box is 0,0,width,height (string [=])
      --repeat         repeat times (int [=1])
      --output         Dump output results. The user ensures that the file path exists. (string [=])
  -?, --help           print this message

example:

./npu_crop.ax620a  --mode disable --mode-type disable --image img/640x640.bgr  --width 640 --height 640 --color-space BGR --boxes 0,0,128,128

4. alpha_blending_with_mask
usage: ./npu_alpha_blending_with_mask.ax620a --bg-width=unsigned int --bg-height=unsigned int --fg-width=unsigned int --fg-height=unsigned int --color-space=string --mask-width=unsigned int --mask-height=unsigned int [options] ...
options:
      --mode             NPU hard mode: disable, 1_1 (string [=disable])
      --mode-type        Virtual NPU mode type: disable， 1_1_1， 1_1_2 (string [=disable])
      --bg               input background image, only support NV12/NV21 (string [=])
      --bg-width         background image width (unsigned int)
      --bg-height        background image height (unsigned int)
      --bg-stride        background image stride (unsigned int [=0])
      --fg               input foreground image, only support NV12/NV21 (string [=])
      --fg-width         foreground image width (unsigned int)
      --fg-height        foreground image height (unsigned int)
      --fg-stride        foreground image stride (unsigned int [=0])
      --color-space      color space for background and foreground image, one of NV12/NV21 (string)
      --mask             input mask image, only support Gray (string [=])
      --mask-width       mask image width (unsigned int)
      --mask-height      mask image height (unsigned int)
      --mask-stride      mask image stride (unsigned int [=0])
      --offset-width     offset Width for foreground (unsigned int [=0])
      --offset-height    offset height for foreground (unsigned int [=0])
      --repeat           repeat times (int [=1])
      --output           Dump output results. The user ensures that the file path exists. (string [=])
  -?, --help             print this message

example:

./npu_alpha_blending_with_mask.ax620a --mode disable --mode-type disable --color-space nv12 --bg blend/ori_R.yuv --bg-width 3840 --bg-height 1920  --fg blend/ori_L.yuv --fg-width 3840 --fg-height 1920 --mask blend/mask_y.bin --mask-width 3840 --mask-height 1920 --output .

5. csc
usage: ./npu_csc.ax620a --width=unsigned int --height=unsigned int --from=string --to=string [options] ...
options:
      --mode          NPU hard mode: disable, 1_1 (string [=disable])
      --mode-type     Virtual NPU mode type: disable， 1_1_1， 1_1_2 (string [=disable])
      --image         input image (string [=])
      --width         width of image (unsigned int)
      --height        height of image (unsigned int)
      --src-stride    src stride (unsigned int [=0])
      --dst-stride    dst stride (unsigned int [=0])
      --from          color space for source image, one of NV12/NV21/RGB/BGR/YUV444/RAW12 (string)
      --to            color space for destination image, one of NV12/NV21/RGB/BGR/YUV444/GRAY/RAW16 (string)
      --repeat        repeat times (int [=1])
      --output        Dump output results. The user ensures that the file path exists. (string [=])
  -?, --help          print this message
example:

./npu_csc.ax620a --mode disable --mode-type disable --image 640x480.bgr --width 640 --height 480 --from BGR --to NV12 --output .
