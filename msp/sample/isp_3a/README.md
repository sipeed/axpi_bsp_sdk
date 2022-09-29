sample_isp_3a

1）功能说明：
isp_3a文件夹下面的代码, 是爱芯SDK包提供的示例参考代码, 方便客户快速的理解客制化3A配置流程.
示例代码提供了注册客制化3A算法的参考源码，演示如何将客制化3A算法的回调函数注册进爱芯3A框架。


2）使用示例：
单摄os04a10 4M的sensor, 运行sample_isp_3a的命令如下
./opt/bin/sample_isp_3a -c0 -e1

-c： 选择不同的场景, 0-单摄os04a10, 1-单摄imx334, 2-单摄GC4653, 3-双摄os04a10+os04a10
-e:  选择SDR/HDR模式, 1-SDR, 2-HDR 2DOL, 默认跑SDR

3）示例运行结果：
AE:  串口输出 AE result(eg:AE Result: shutter=30000, Again=62.78, ispGain=3.23, lux=1.32, Luma=44.72),
     也可以通过pc工具tuning tool进行网络图像预览
AWB: 串口输出 AWB结果(eg:AWB: Gains (R, Gr, Gb, B) = (1.758, 1.000, 1.000, 2.300),  CCT = 4832.58 ),
		 也可以通过pc工具tuning tool进行网络图像预览
AF:  串口输出 AF结果(eg:[sample_isp][AfRun][1191] AF:  Y: 512   FV: 1889813086),
	   也可以通过pc工具tuning tool进行网络图像预览
LSC: 通过pc工具tuning tool进行网络图像预览

4）注意事项：
（1）sample_isp_3a 包括 AE AWB AF LSC，并不是常规意思上的3A.
（2）Online模式获取af统计值的 sample_af_stats 接口暂时不支持，后续会支持Online模式.

