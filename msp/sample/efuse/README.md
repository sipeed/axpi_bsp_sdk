1.本测试用来验证efuse模块的读写功能，由于该case会写芯片内部的OTP单元，建议慎重测试（可以只用一台机器测试，不要每台机器都测试）；
AX620X内部的efuse有96个block，每个block是32bit，该测试程序测试的对于block14和block20的读写操作；

2.运行方法：
#sample_efuse

3.正常运行log输出结果如下：
This will write the Efuse, please confirm you want Do it!!!
Efuse test PASS!

4.如果该芯片的efuse已经执行过该测试，再次执行该测试则会报下面的错误，这个是正常现象，每个芯片的OTP只能写一次；
This will write the Efuse, please confirm you want Do it!!!
Efuse test write error 1b, 80050004