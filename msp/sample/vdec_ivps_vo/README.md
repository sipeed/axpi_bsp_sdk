1）功能说明：
该模块是爱芯SDK包提供的视频解码单元link模式示例代码，方便客户快速理解和掌握视频解码相关接口的使用。
代码演示了如下功能：解码模块初始化、解码Send线程解析和发送码流、IVPS/VO模块初始化、VO offline模式
下获得YUV图像线程，各模块退出功能。
编译后得到可执行程序sample_vdec_ivps_vo,位于/opt/bin目录，可用于VDEC link IVPS，IVPS link VO功能验证。

使用示例：
举例一：查看help信息
/opt/bin/sample_vdec  -h

H264协议：
./sample_vdec_ivps_vo /opt/data/vdec/1080p.h264 -T 96

jpeg:
./sample_vdec_ivps_vo /opt/data/vdec/1080P_qp9.jpg -T 26

参数说明：
-v:  选择显示设备的类型、分辨率及刷新率，如dsi0@1920x1080@60
-w : vo输出YUV的帧数，默认值是1，此值只有在没有接显示屏，即-v没有
选择的情况下有效，用于设备没有接入外设使用vo offline功能获取YUV图像。
注意：由于sample中vo offline输出YUV getframe在一个线程中执行，如果w值
大于实际帧数，线程不能退出，需要Ctrl +C退出。
-T ：解码类型，26：jpegdec  96：vdec


3）注意事项：
（1）common pool内存块如何申请？我司提供了一个内存配置表（详见vdec_ivps_vo中文件的定义）,
        用户可根据自己的业务,在此处配置整个系统各个模块的buf,这样的设计更有利于理解和使用.
（2）用户如果需要配置其他参数,如rawType,修改sample代码即可.
（3）本sample只支持一路视频流的显示,在MIPI双屏的情况下,dsi0显示视频,dsi1显示彩条.
