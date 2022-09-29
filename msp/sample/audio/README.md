sample_audio

1）功能说明：
audio文件夹下面的代码, 是爱芯SDK包提供的示例参考代码, 方便客户快速的理解audio整个模块的配置流程.
示例代码演示的是如下的功能：cap录音、play播放、aenc编码、adec解码.
我司提供everest厂家的codec驱动: es8388、es7210和es8311


2）使用示例：
举例一：查看help信息
sample_audio -h

举例二：录制8kHz音频
sample_audio cap -D 0 -d 0 -r 8000 -w 1

举例三：启用FIXED模式回声消除录制8kHz音频
sample_audio cap -D 0 -d 0 -r 8000 -p 160 --aec-mode 2 --routing-mode 0 -w 1

举例四：播放8kHz音频
sample_audio play -D 0 -d 0 -r 8000

举例五：使用HDMI播放48kHz音频（AX630A）
sample_audio play -D 0 -d 0 -r 48000

举例六：录制8kHz音频并且编码
sample_audio aenc -D 0 -d 0 -r 8000 -w 1

举例七：录制8kHz音频并且单声道编码
sample_audio aenc -D 0 -d 0 -r 8000 --aenc-chns 1 -w 1

举例八：解码并且播放8kHz音频
sample_audio adec -D 0 -d 0 -r 8000

3）示例运行结果：
运行成功后，执行Ctrl+C退出，在当前目录应生成wave文件，用户可打开看实际效果。

4）注意事项：
（1）声卡号和设备号，请参考/dev/snd/，举例说明:
   pcmC0D0p: card 0, device 0, playback device
   pcmC0D1c: card 0, device 1, capture device
（2）AX630A HDMI仅支持48kHz采样率
（3）若播放HDMI音频, 需要在运行sample_audio之前运行sample_dpu，具体参考sample_dpu的README