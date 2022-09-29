当前版本的底层显示可以支持minigui原生的fbcon，使用前确保FB功能已使能（具体可参考FB开发指南），使用时只需在MiniGui.cfg中选择fbcon。

sample_minigui程序依赖libminigui_ths.a，目前libminigui在third-party/minigui目录下，需要手动编译。步骤如下:

1.在third-party/libminigui目录下执行：
	./build/./build/buildlib-linux-ax620a

2.在third-party/minigui-res目录下执行(--prefix后跟的路径为自己代码的路径):
	./configure --prefix=/home/zhengwanhu/work/mc20-0113/rootfs/rootfs/usr/local
	make
	make install

3.在当前目录下执行make p=AX620_demo
4.将当前目录下MiniGUI.cfg拷贝到rootfs/rootfs/etc目录下
5.重新打包
