使用示例：
H264协议：
./sample_vdec_ivps /opt/data/vdec/1080p.h264 -T 96

jpeg:
./sample_vdec_ivps /opt/data/vdec/1080P_qp9.jpg -T 26

参数说明：
w : ivps输出YUV写入文件的帧数，默认值是1，即写入1帧yuv到文件。
注意：由于sample中ivps输出getframe在一个线程中执行，如果w值
大于实际帧数，线程不能退出，需要Ctrl +C退出。


