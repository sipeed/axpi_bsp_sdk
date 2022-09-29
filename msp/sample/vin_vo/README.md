
1）功能说明：
vin_vo文件夹下面的代码, 是爱芯SDK包提供的示例参考代码, 方便客户快速的理解VIN/VO二个模块的串连流程.
示例代码演示的是如下的功能：VIN/VO模块初始化、单摄SDR/HDR初始化、VIN到VO之间的LINK.
我司提供三个厂家的sensor初始化配置: OV-OS04A10、SONY-IMX334和GC-gc4653


2）使用示例：
单摄os04a10，SDR模式，通道0显示, 运行命令如下：
./sample_vin_vo -c 0 -e 1 -s 0 -v dsi0@1920x1080@60

-c： 选择不同的场景, 0-单摄os04a10, 1-单摄imx334, 2-单摄GC4653, 3-双摄os04a10+os04a10
-e:  选择SDR/HDR模式, 1-SDR, 2-HDR 2DOL, 默认跑SDR
-v:  选择显示设备的类型、分辨率及刷新率，如dsi0@1920x1080@60
-s:  选择sensor输出的哪个通道送到VO去显示，默认通道0
注意：以上-v参数的使用可以参考sample_vo, 其它参数可以参考sample_vin

3）注意事项：
（1）common pool内存块如何申请？我司提供了一个内存配置表（详见sample_vin_vo.c文件中的定义）,
	 用户可根据自己的业务,在此处配置整个系统各个模块的buf,这样的设计更有利于理解和使用.
（2）用户如果需要配置其他参数,如rawType,修改sample代码即可.
（3）本sample只支持一路视频流的显示,在MIPI双屏的情况下,dsi0显示视频,dsi1显示彩条.
