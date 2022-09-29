
1）功能说明：

该模块是爱芯SDK包提供的IVPS(视频处理系统)单元示例代码，方便客户快速理解和掌握IVPS相关接口的使用。
sample_ivps,位于/opt/bin目录，可用于IVPS 接口示例。


-v：输入源图像路径及描述信息。

参数内容描述如下：
-v <IMG PATH>@<FMT>@<srcW>x<srcH>@<cropW>x<cropH>[+<srcX0>+<srcY0>]

<IMG PATH>    :   输入源图像路径
<FMT>         :   图像格式
    1         :   NV12
    2         :   NV21
    65        :   RGB888
    64        :   RGB565
    69        :   ARGB4444
    80        :   RGBA4444
    71        :   ARGB8888
    73        :   RGBA8888
    70        :   ARGB1555
    81        :   RGBA5551
    72        :   ARGB8565
    82        :   BITMAP
<srcW>x<srcH>               : 输入源图像宽和高，宽范围64~4096，高范围64~4096
<cropW>x<cropH>             : CROP 窗口的宽和高
+<srcX0>+<srcY0> (optional) : CROP 窗口的左边起始点
注意：
CROP窗口需在源图像宽高范围内，即
srcX0+cropW <= srcW, srcY0+cropH <= srcH
若不做CROP，则cropW = srcW, cropH = srcH, 并且去除参数+<srcX0>+<srcY0>


-n：对源数据指定处理次数，若不设置-n，则会一直循环执行。

若用户想要查看IVPS的proc信息，则需要将处理次数设置较大或一直循环；
查看IVPS proc信息方法：/cat/ax_proc/ivps


-r：叠加REGION个数，目前最大为4。

IVPS PIPELINE上叠加REGION是异步操作，需等几帧以后才会真正的叠加到输入源图像上。
故用户想要验证REGION功能，需要将-n后参数设置大些，建议大于3.



2）使用示例：

举例一：查看help信息
./opt/bin/sample_ivps -h

举例二：对源图像（3840x2160 NV12格式）进行1次处理。
./opt/bin/sample_ivps -v /opt/data/ivps/3840x2160.nv12@1@3840x2160@3840x2160 -n 1

举例三：对源图像（800x480 RGB888格式）先CROP(X0=128 Y0=50 W=400 H=200),再进行3次处理。
./opt/bin/sample_ivps -v /opt/data/ivps/800x480logo.rgb24@65@800x480@400x200+128+50  -n 3
对于TDP硬件，X0需16 bytes对齐。

举例四：对源图像（3840x2160 NV12格式）进行5次处理，并叠加3个REGION。
./opt/bin/sample_ivps -v /opt/data/ivps/3840x2160.nv12@1@3840x2160@3840x2160 -n 5 -r 3



3）运行结果：

运行成功后，会在和源图像相同目录（/opt/data/ivps）下产生类似以下图像，可通过工具打开查看：
FlipMirrorRotate_chn0_480x800.fmt_41
OSD_chn0_3840x2160.fmt_1
AlphaBlend_chn0_3840x2160.fmt_1
Rotate_chn0_1088x1920.fmt_1
CSC_chn0_3840x2160.fmt_1
CropResize_chn0_1280x720.fmt_1
output_grp1chn0_1920x1080.fmt_1
output_grp1chn1_2688x1520.fmt_41
output_grp1chn2_768x1280.fmt_41
fmt_1 ：表示NV12格式
fmt_41：表示RGB888格式

执行Ctrl+C退出。



4）注意事项：

     a)示例代码仅作为API演示；在实际开发中，用户需结合具体业务场景配置参数。
     b)输入图像和输出图像最大分辨率为4096x4096。
     c)需在根目录下执行以上bin文件。





