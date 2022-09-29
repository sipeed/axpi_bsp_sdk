1）功能说明：
该模块是爱芯SDK包提供的视频(h264/h265)编码动态切换分辨率示例代码，方便客户快速理解和掌握视频编码动态切换分辨率功能的使用。
代码演示了如下动态切换分辨率的流程：编码通道Stop停止接受数据、编码通道设置属性切换新分辨率、编码通道Start重新开始接受数据。
编译后得到可执行程序sample_resolution_venc,位于/opt/bin目录，可用于视频编码动态切换分辨率功能验证。

直接运行可得到配置参数的help说明。

-w: Raw input frame width.(MAX 5584)
-h: Raw input frame height.(MAX 4188)
-d: Raw input frame stride.(MAX 5584)
-i: yuv file to encode.
-t: input frame format, 0: yuv420p 1: nv12 2: nv21 3: YUYV422 4: UYVY422.(default 0)
-n: total encoder number change to next resolution(Max support 1 way 4k@60fps).(default 50)
-b: enable encoder type change, 0: disable, 1: enable
-W: Next resolution raw input frame width.(MAX 5584)
-H: Next resolution raw input frame height.(MAX 4188)
-D: Next resolution raw input frame stride.(MAX 5584)
-I: Next resolution yuv file to encode.
-T: Next resolution input frame format, 0: yuv420p 1: nv12 2: nv21 3: YUYV422 4: UYVY422.(default 0)

2）使用示例：
举例一：查看help信息
sample_resolution_venc

举例二：1080P切换720P分辨率（通道0：h264，通道1：h265）
sample_resolution_venc -w 1920 -h 1080 -i 1080p_i420.yuv -n 10 -W 1280 -H 720 -I 720p_i420.yuv

3）运行结果：
运行成功后，执行Ctrl+C退出，在当前目录应生成码流文件，名称如enc_0.264、enc_1.265，用户可打开看实际切换分辨率效果。

4）注意事项：
     a)示例代码中部分参数可能不是最优，仅作为API演示。实际开发中，用户需结合具体业务场景配置参数。
     b)支持最大分辨率为5584x4188