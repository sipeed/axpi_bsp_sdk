#IPCDemo
## 如何执行？
1. cd /opt/bin/IPCDemo
2. vi config/ipc_demo.conf 配置编码通道以及检测开关等。[参数说明](#配置参数)
3. 执行./run.sh config/os08a20_config.json
>   ./gdb.sh用于gdb调试，IPCDemo内部使用了SIGUSR2信号作为定时器，因此gdb调试时输入**handle SIGUSR2 nostop noprint**忽略SIGUSR2信号。

## 如何编译？
1. cd app/IPCDemo
2. make p=xxx clean
3. make p=xxx
4. make p=xxx install
> p=xxx 指定编译项目名，示例：make p=AX620_demo

# <a href="#配置参数">配置参数</a>

|   #   |          参数         |    参数范围   |       说明                        |
| ----- | --------------------- | ------------ | --------------------------------- |
|   1   | ActiveDetect          |              | 0：关闭AI检测 1:开启AI检测         |
|   2   | DetectModel           |              | AI检测模型文件位置，默认"/opt/etc/models"                 |
|   3   | DetectAlgoType        |              | AI检测模型类型选择(脸人/结构化/识别)                  |
|   4   | DetectConfigPath      |              | AI检测模型配置文件路径(脸人/结构化/人脸识别)              |
|   5   | ActiveDetectTrack     |              | 0：关闭AI检测推图 1:开启AI检测推图         |
|   6   | ActiveDetectSearch    |              | 0：关闭AI人脸识别 1:开启AI人脸识别         |
|   7   | LogTarget             | [0 - 3]      | 应用日志打印目的地，默认1           |
|   8   | LogLevel              | [0 - 4]      | 应用日志级别：默认2                 |
|   9   | RTSPMaxFrmSize        |              | 单位：KB                           |
|   10  | PrintFPS              |              | 0：不打印 1:打印                    |
|   11  | EnableCoreDump        |              | 0：关闭 1:开启                      |
|   12  | RunInBackground       |              | 0：非后台运行 1:后台运行             |
|   13  | EnableCmmCopy         |              | 是否开启CmmCopy验证功耗             |
|   14  | DetectInputStrgy      | [0 - 2]      | 伪检测模型输入数据来源类型           |
|   15  | EnableEIS             |              | 0：关闭EIS 1:开启EIS|
|       |                       |              |【功能开关生效前提】|
|       |                       |              | 1、非低内存版本（AX620U不支持）；|
|       |                       |              | 2、EIS模型支持并将对应bin配置到config目录下对应sensor json|
|       |                       |              | 配置文件中，同时将json文件中eis_enable置为1|
|   16  | EISDelayNum           | [1 - 4]      | EIS延迟帧数：帧数越大，内存消耗越大（需缓存对应数量的YUV帧）|
|       |                       |              | 延迟越大，理论上防抖效果越好|
|   17  | EISCropW              | [0 - 64]     | 宽裁剪比例：左右分别裁剪EISCropW/255|
|   18  | EISCropH              | [0 - 64]     | 高裁剪比例：上下分别裁剪EISCropH/255|
|   19  | EISEffectComp         |              |  0：开启EIS效果对比功能 1:关闭EIS效果对比功能|
|       |                       |              |【功能说明】|
|       |                       |              |功能开启后，web端开启两个IPCDemo窗口，两个窗口分别选择“主码流|
|       |                       |              |0”（EIS），“子码流1”(无EIS)即可进行效果对比|
|   20  | EnableOSD             | [0 | 1]      | 0：关闭 1:开启             |

[========]
#修改历史
- 2022-07-06
1. 适配IVPS旋转180°及mirror场景，宽8对齐要求

- 2022-06-30
1. 支持WEB预览页面显示当前码流的分辨率和实时码率

- 2022-06-21
1. 支持动态P/N制切换

- 2022-06-20
1. 修改Detector以及Search部分接口名称以及相关实现
2. AI检测新增接口: SetPushStrategy, SetObjectFilter, SetTrackSize, SetCropEncoderQpLevel
3. AI检测配置json文件新增配置: 目标大小限制，可信度
4. 配置文件ipc_demo.conf支持Rotation/Mirror/Flip
5. AI检测支持Rotation/Mirror/Flip
6. 修正webapp版本不匹配问题导致crash
7. 实现抓拍功能

- 2022-06-15
1. 修正mirror功能开启场景，OSD显示位置错误问题

- 2022-05-26
1. AX620SW-5294 - 支持EIS相关功能

- 2022-02-24
1. AX170SW-596 - 支持可通过配置文件配置AI检测相关功能

- 2021-11-03
1. AX620SW-1446 - 支持可通过配置文件配置ISP是否加载降噪模型

- 2021-10-22
1. AX620SW-806 - 支持加载伪检测模型进行检测状态下NPU负载验证

- 2021-10-13
1. AX620SW-1027 - 修正IMX334 HDR模式不出图问题

- 2021-10-12 （Ver: v1.21.42.1）
1. AX620SW-1135 - 修改VENC默认初始码率配置

- 2021-10-08
1. AX620SW-1076 - 优化退出流程各ISP接口调用顺序
2. AX620SW-1047 - 优化AX_POOL配置可按pipe独立配置

- 2021-09-29
1. AX620SW-1071 - 支持HDR显示

- 2021-09-26
1. AX620SW-1015 - 增加OSD时间显示功能
2. AX620SW-1013 - 修改录屏功能视频按帧保存

- 2021-09-23
1. AX620SW-971 - SDR/HDR可配置化

- 2021-09-22
1. AX620SW-943 - 适配VENC的AX_VENC_CreateChn接口入参中bitrate单位由bps改为kbps
2. AX620SW-976 - 适配ISP释放帧策略，需由上层调用AX_VIN_ReleaseYuvFrame来释放帧资源

- 2021-09-16 （Ver: v1.21.38.1）
1. AX620SW-68 - 支持3路ISP输出

- 2021-08-18 （Ver: v1.21.34.1）
1. 初版
