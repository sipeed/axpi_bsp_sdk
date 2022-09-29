1）功能说明
该模块是爱芯SDK包提供的jpeg编码单元示例代码，方便客户快速理解和掌握jpeg编码相关接口的使用。
代码演示了如下流程：jpeg编码模块初始化、编码Send线程解析和发送帧数据、编码Get线程获取和保存码流数据、jpeg编码模块去初始化。
编译后得到可执行程序sample_jpegenc,位于/opt/bin目录，可用于jpeg编码功能验证。

-w：配置源数据width
-h：配置源数据height
-i ：输入的源数据路径
-g：输入源数据YUV格式,（0:420p ,1:NV12 ,2:NV21）默认0
-j ：配置编码通道数目，偶数通道编码JPEG，奇数通道编码MJPEG，默认开启一路通道编码JPEG。

2）使用示例：
举例一：查看help信息
sample_jpegenc -H

举例二：开启两路通道编码1080p NV12格式（通道0：JPEG，通道1：MJPEG）
sample_jpegenc -w 1920 -h 1080 -i  1080p_nv12_1frm.yuv  -j 2  -g 1

举例三：开启两路通道编码3840x2160 NV21格式（通道0：JPEG，通道1：MJPEG）
sample_jpegenc -w 3830 -h 2160 -i  3840x2160_nv21_1frm.yuv  -j 2  -g 2

3）运行结果：
运行成功后，执行Ctrl+C退出，在当前目录应生成码流文件，名称如jenc_0.jpg、jenc_1.mjpeg，用户可打开看实际效果。

4）注意事项：
     a)示例代码中部分参数可能不是最优，比如量化表、码控参数，仅作为API演示。实际开发中，用户需结合具体业务场景配置参数。

