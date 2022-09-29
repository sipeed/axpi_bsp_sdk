1.本测试用来运行一次模型并保存结果，模型为face landmark model，测试数据为128x128 NV12 face image.

2.模型和图片数据默认已经放到rootfs /opt/data/npu目录.

3.运行方法：
#sample_npu /opt/data/npu/sinopec_nv12_extra.neu /opt/data/npu/image_128x128.nv12

4.程序执行完成输出结果会保存到当前目录下面.
before_cpu_tail.bin为npu直接输出结果
after_cpu_tail.bin为经过cpu后处理结果

5.可以用sha1sham验证一下本次执行正确性，正确结果如下：
# sha1sum *
761282bfdcb64bfa42c8f2d1380974f61066f4ae  after_cpu_tail.bin
4744b78a12815ea47b054538d6382aa239963282  before_cpu_tail.bin



