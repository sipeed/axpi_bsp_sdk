
1）功能说明：
此sample是爱芯SDK包提供的示例参考代码, 方便客户快速的理解如何使用Link机制建立sensor到venc的链路。
sample默认演示的是VIN --> IVPS --> VENC的完整pipeline，链路关系如下：

--------------------Link Table--------------------
         Src            |          Dst
(ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
--------------------------------------------------
(VIN        0       2) -> (IVPS     2       0)
(VIN        0       1) -> (IVPS     1       0)
(VIN        0       0) -> (IVPS     0       0)
(IVPS       2       0) -> (VENC     0       2)
(IVPS       1       0) -> (VENC     0       1)
(IVPS       0       0) -> (VENC     0       0)

2）使用示例：
举例一：查看help信息
/ # /opt/bin/sample_vin_ivps_venc  -h
Usage:./opt/bin/sample_vin_ivps_venc -h for help

        -c: ISP Test Case:
                0: Single OS04A10
                1: Single IMX334
                2: Single GC4653
                3: DUAL OS04A10
                4: Single OS08A20
                5: Single OS04A10 Online
        -e: SDR/HDR Mode:
                1: SDR
                2: HDR 2DOL

-c： 选择不同的场景, 0-单摄os04a10, 1-单摄imx334, 2-单摄GC4653, 3-双摄os04a10+os04a10，4-单摄os08a20, 5-单摄os04a10 online模式
-e:  选择SDR/HDR模式, 1-SDR, 2-HDR 2DOL, 默认跑SDR

举例二：单摄os04a10，SDR模式，运行命令如下：
/opt/bin/sample_vin_ivps_venc -c 0 -e 1


3）运行结果：
通过cat  /proc/ax_proc/vin_status 观察VIN模块帧率是否正常。
通过cat  /proc/ax_proc/ivps 观察IVPS模块帧率是否正常。
通过cat  /proc/ax_proc/venc 观察VENC模块各通道帧率是否正常。

sample默认每个编码通道保存前30帧数据，生成的文件保存在当前执行命令的目录下，文件名为enc_0.264、enc_1.264、enc_2.264。

4）注意事项：
     a)sample中缓存池并不是最优配置，仅供参考。实际开发中，用户需要结合具体业务配置缓存池。
     b)如果希望测试VIN --> VENC，不经过IVPS，则只需修改sample代码，打开SAMPLE_VIN_LINK_VENC_WITHOUT_IVPS宏重新编译即可。链路关系如下：
--------------------Link Table--------------------
         Src            |          Dst
(ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
--------------------------------------------------
(VIN        0       2) -> (VENC     0       2)
(VIN        0       1) -> (VENC     0       1)
(VIN        0       0) -> (VENC     0       0)

