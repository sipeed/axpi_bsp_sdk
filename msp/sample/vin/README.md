
1）功能说明：
vin文件夹下面的代码, 是爱芯SDK包提供的示例参考代码, 方便客户快速的理解VIN整个模块的配置流程.
示例代码演示的是如下的功能：VIN模块初始化、单摄SDR/HDR初始化、双摄SDR/HDR初始化.
我司提供三个厂家的sensor初始化配置: OV-OS04A10 & OV-OS08A20、SONY-IMX334和GC-gc4653


2）使用示例：
单摄一路os08a20 8M的sensor, 跑一路HDR, 运行命令如下：
./sample_vin -c 4 -e 2

-c： 选择不同的场景：
                    0-单摄os04a10,
                    1-单摄imx334,
                    2-单摄GC4653,
                    3-双摄os04a10+os04a10，
                    4-单摄os08a20,
                    5-单摄os04a10 online模式，
                    6-单摄DVP，
                    7-单摄BT601，
                    8-单摄BT656，
                    9-单摄BT1120，
                    10-MIPI YUV
-e:  选择SDR/HDR模式：
                    1-SDR,
                    2-HDR 2DOL,
                    --默认跑SDR

3）示例运行结果：
#./sample_vin -c 4 -e 2
AX_POOL_SetConfig success!
[COMMON_SYS_Init]-85: AX_POOL_Init success!
ISP IFE INIT done.
ISP ITP INIT done.
[main]-458: camera 0 is open
[main]-479: tuning runing.
[IspRun]-184: cam 0 is running...
运行结果包含以上内容代表成功运行，
也可以通过pc工具tuning tool进行网络图像参数控制和预览


4）注意事项：
（1）common pool内存块如何申请？我司提供了一个内存配置表（详见sample_vin.c文件中的定义）,
	 用户可根据自己的业务,在此处配置整个系统各个模块的buf,这样的设计更有利于理解和使用.
（2）用户如果需要配置其他参数,如rawType,修改sample代码即可.
（3）sample是否支持预览和调试参数？默认就支持，如果想关闭,请在vin目录下执行编译命令（demo板）make p=AX620_demo preview=no,
     sample会集成tuning service，可通过pc工具tuning tool进行网络图像参数控制和预览.
