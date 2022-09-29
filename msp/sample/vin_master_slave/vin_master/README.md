1）功能说明：
vin_master_slave/vin_master 文件夹下面的code,仅适用于客户两颗芯片级联的应用场景,作为主芯片的sample code.
示例代码演示的是如下的功能：正常运行isp和vin模块,获取yuv,然后通过tx传输给master端.


2）使用示例：
双摄mipi rx0对接slave的Tx0,获取os08a20 8M的数据, mipi rx1接os04a10对接slave的Tx1, 获取os04a10 4M的数据, bin放在/opt/bin路径下, 运行命令如下：
./sample_vin_master -c 2 -e 2

双摄mipi rx0对接slave的Tx0,获取os08a20 8M的连续raw 数据, mipi rx1接os04a10对接slave的Tx1, 获取os04a10 4M的连续raw数据, bin放在/opt/bin路径下, 运行命令如下：
./sample_vin_master -c 2 -e 2 -d 50

-c: 选择不同的场景：
                  0-单摄os04a10,
                  1-单摄os08a20,
                  2-双摄os08a20+os04a10,
                  3-ISP Bypass, Dump Raw模式

-e: HDR/SDR模式参数设置：
                       默认值: AX_SNS_LINEAR_MODE，
                       SDR设置为 AX_SNS_LINEAR_MODE,
                       HDR设置为:AX_SNS_HDR_2X_MODE

-d: dump 连续帧raw的个数：
                        例如是dump sdr 连续raw 50个，则需要d 50; 
                        hdr 连续raw 50个,则需要d 100

3）示例运行结果：
#./sample_vin_master -c 2 -e 2
AX_POOL_SetConfig success!
[COMMON_SYS_Init]-85: AX_POOL_Init success!
ISP IFE INIT done.
ISP ITP INIT done.
[SAMPLE_VIN_MASTER][main]-477: camera 0 is open
[SAMPLE_VIN_MASTER][main]-477: camera 1 is open
运行结果包含以上内容代表成功运行.

4）注意事项：
（1）master端不需要初始化sensor相关的内容,也不需要运行itp_run线程.
（2）用户如果mipi rx连接的是0和1时，请正确的配置 nRxDev nDevId nPipeId三个字段,请参考sample中的配置.
（3）slave端需要在AX_DEV_ATTR_T结构体中,配置正确的work mode, eDevWorkMode = AX_DEV_WORK_MODE_MASTER.
