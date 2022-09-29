1）功能说明：
vin_master_slave/isp_slave 文件夹下面的code,仅适用于客户两颗芯片级联的应用场景,作为从芯片的sample code.
示例代码演示的是如下的功能：正常运行isp和vin模块,运行isp处理线程,
创建各自的数据传输线程,两个线程分别获取两路sensor的yuv数据,然后通过tx传输给master端.


2）使用示例：
双摄mipi rx0接os08a20, mipi rx1接os04a10, HDR 2DOL mode, bin放在/opt/bin路径下, 运行命令如下：
./sample_vin_slave -c 2 -e 2

-c: 选择不同的场景：
                  0-单摄os04a10,
                  1-单摄os08a20,
                  2-双摄os08a20+os04a10,
                  3-ISP Bypass, Dump Raw模式

-e: HDR/SDR模式参数设置:
                       默认值: AX_SNS_HDR_2X_MODE,
                       SDR设置为 AX_SNS_LINEAR_MODE,
                       HDR设置为:AX_SNS_HDR_2X_MODE

3）示例运行结果：
#./sample_vin_slave -c 2 -e 2
AX_POOL_SetConfig success!
[COMMON_SYS_Init]-85: AX_POOL_Init success!
ISP IFE INIT done.
ISP ITP INIT done.
[SAMPLE_VIN_SLAVE][IspRun][106] cam 0  pipe 0 is running...
[SAMPLE_VIN_SLAVE][main][440] camera 0 is open
[SAMPLE_VIN_SLAVE][IspRun][106] cam 1  pipe 1 is running...
[SAMPLE_VIN_SLAVE][main][440] camera 1 is open
运行结果包含以上内容代表成功运行.

4）注意事项：
（1）slave端在调用COMMON_CAM_Open()接口之前，应该先配置mipi tx相关的参数,并打开tx设备.
（2）用户如果mipi rx连接的是0和2时，请正确的配置 nRxDev nDevId nPipeId三个字段,请参考sample中的配置.
（3）务必在AX_TX_IMG_INFO_T中将TX模块使能 bMipiTxEnable  = AX_TRUE.
（4）slave端需要在AX_DEV_ATTR_T结构体中国,配置正确的work mode, eDevWorkMode = AX_DEV_WORK_MODE_SLAVE.

