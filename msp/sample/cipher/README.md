1）测试用来验证cipher模块的基本功能：

2）运行方法：
#insmod /soc/ko/ax_cipher.ko
#sample_cipher

3）示例运行结果：
AES_Encrypt PASS
AES_Decrypt PASS
Phy addr AES_Encrypt_Phy PASS
Phy addr AES_Decrypt_Phy PASS
DES_Encrypt PASS
DES_Decrypt PASS
test_rsa_pss PASS
test_rsa_pss PASS
test_rsa_pkcs1v15 PASS
test_rsa_pkcs1v15 PASS
test_hash PASS
SAMPLE_CIPHER_Trng PASS
AES Encrypt ECB Mode, key bits F_1_1_key 128 bits  Mode: perf encryt:10.766854MB/s
AES Encrypt ECB Mode, key bits F_1_3_key 192 bits  Mode: perf encryt:9.067710MB/s
AES Encrypt ECB Mode, key bits F_1_5_key 256 bits  Mode: perf encryt:7.831735MB/s
AES Encrypt CBC Mode, key bits F_2_1_key 128 bits  Mode: perf encryt:10.440149MB/s
AES Encrypt CBC Mode, key bits F_2_3_key 192 bits  Mode: perf encryt:8.834891MB/s
AES Encrypt CBC Mode, key bits F_2_5_key 256 bits  Mode: perf encryt:7.657473MB/s
AES Encrypt CTR Mode, key bits F_5_1_key 128 bits  Mode: perf encryt:10.762494MB/s
AES Encrypt CTR Mode, key bits F_5_3_key 192 bits  Mode: perf encryt:9.067235MB/s
AES Encrypt CTR Mode, key bits F_5_5_key 256 bits  Mode: perf encryt:7.831404MB/s
AES Encrypt ICM Mode, key bits F_4_1_key 128 bits  Mode: perf encryt:10.766118MB/s
AES Encrypt ICM Mode, key bits F_4_2_key 128 bits  Mode: perf encryt:10.766165MB/s
AES_Encrypt_Phy PASS

AES Decrypt ECB Mode, key bits F_1_1_key 128 bits  Mode: perf decrypt:10.766754MB/s
AES Decrypt ECB Mode, key bits F_1_3_key 192 bits  Mode: perf decrypt:9.067612MB/s
AES Decrypt ECB Mode, key bits F_1_5_key 256 bits  Mode: perf decrypt:7.831616MB/s
AES Decrypt CBC Mode, key bits F_2_1_key 128 bits  Mode: perf decrypt:10.766071MB/s
AES Decrypt CBC Mode, key bits F_2_3_key 192 bits  Mode: perf decrypt:9.067125MB/s
AES Decrypt CBC Mode, key bits F_2_5_key 256 bits  Mode: perf decrypt:7.831267MB/s
AES Decrypt CTR Mode, key bits F_5_1_key 128 bits  Mode: perf decrypt:10.766214MB/s
AES Decrypt CTR Mode, key bits F_5_3_key 192 bits  Mode: perf decrypt:9.067247MB/s
AES Decrypt CTR Mode, key bits F_5_5_key 256 bits  Mode: perf decrypt:7.831413MB/s
AES Decrypt ICM Mode, key bits F_4_1_key 128 bits  Mode: perf decrypt:10.766126MB/s
AES Decrypt ICM Mode, key bits F_4_2_key 128 bits  Mode: perf decrypt:10.766198MB/s
AES_Decrypt_Phy PASS

DES Encrypt ECB Mode, key bits App_A_key 64 bits  Mode: perf encryt:10.134185MB/s
DES Encrypt ECB Mode, key bits IP_and_E_Test_key 64 bits  Mode: perf encryt:10.134193MB/s
DES Encrypt ECB Mode, key bits FIPS81_B1_key 64 bits  Mode: perf encryt:10.134107MB/s
DES Encrypt CBC Mode, key bits FIPS81_C1_key 64 bits  Mode: perf encryt:10.133587MB/s
DES Encrypt ECB Mode, key bits (B_1_3DES_ECB_key 192 bits  Mode: perf encryt:3.517042MB/s
DES Encrypt ECB Mode, key bits TDES_ECB_Ex1_key 192 bits  Mode: perf encryt:3.517205MB/s
DES Encrypt ECB Mode, key bits TDES_ECB_Ex2_key 192 bits  Mode: perf encryt:3.517205MB/s
DES Encrypt CBC Mode, key bits TDES_CBC_Ex1_key 192 bits  Mode: perf encryt:3.517131MB/s
DES Encrypt CBC Mode, key bits TDES_CBC_Ex2_key 192 bits  Mode: perf encryt:3.517131MB/s
DES_Phy_Encrypt PASS

DES Decrypt ECB Mode, key bits App_A_key 64 bits  Mode: perf decrypt:10.134184MB/s
DES Decrypt ECB Mode, key bits IP_and_E_Test_key 64 bits  Mode: perf decrypt:10.134206MB/s
DES Decrypt ECB Mode, key bits FIPS81_B1_key 64 bits  Mode: perf decrypt:10.134148MB/s
DES Decrypt CBC Mode, key bits FIPS81_C1_key 64 bits  Mode: perf decrypt:10.133574MB/s
DES Decrypt ECB Mode, key bits (B_1_3DES_ECB_key 192 bits  Mode: perf decrypt:3.517199MB/s
DES Decrypt ECB Mode, key bits TDES_ECB_Ex1_key 192 bits  Mode: perf decrypt:3.517209MB/s
DES Decrypt ECB Mode, key bits TDES_ECB_Ex2_key 192 bits  Mode: perf decrypt:3.517204MB/s
DES Decrypt CBC Mode, key bits TDES_CBC_Ex1_key 192 bits  Mode: perf decrypt:3.517131MB/s
DES Decrypt CBC Mode, key bits TDES_CBC_Ex2_key 192 bits  Mode: perf decrypt:3.517047MB/s
DES_Phy_Decrypt PASS
4）注意事项：
测试时间较长，运行到后面的性能case时候可以ctrl+c退出；