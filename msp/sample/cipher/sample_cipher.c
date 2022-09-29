/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdio.h>
#include <ax_base_type.h>
#include <ax_cipher_api.h>
#include <string.h>
#include <ax_sys_api.h>

extern int SAMPLE_CIPHER_AES_Encrypt();
extern int SAMPLE_CIPHER_AES_Decrypt();
extern int SAMPLE_CIPHER_AES_EncryptPhy();
extern int SAMPLE_CIPHER_AES_DecryptPhy();
extern int SAMPLE_CIPHER_DES_Encrypt();
extern int SAMPLE_CIPHER_DES_Decrypt();
extern int SAMPLE_CIPHER_Rsa();
extern int SAMPLE_CIPHER_RsaEnc();
extern int SAMPLE_CIPHER_Hash();
extern int SAMPLE_CIPHER_Trng();
extern int SAMPLE_CIPHER_AES_PerformanceTest();
extern int SAMPLE_CIPHER_DES_PerformanceTest();
extern int SAMPLE_CIPHER_Rsa_PerformanceTest();
extern int SAMPLE_CIPHER_Hash_PerformanceTest();
extern int SAMPLE_CIPHER_Trng_PerformanceTest();

static void SAMPLE_CIPHER_FunctionalTest()
{
    SAMPLE_CIPHER_AES_Encrypt();
    SAMPLE_CIPHER_AES_Decrypt();
    SAMPLE_CIPHER_AES_EncryptPhy();
    SAMPLE_CIPHER_AES_DecryptPhy();
    SAMPLE_CIPHER_DES_Encrypt();
    SAMPLE_CIPHER_DES_Decrypt();
    SAMPLE_CIPHER_Rsa();
    SAMPLE_CIPHER_Hash();
    SAMPLE_CIPHER_Trng();
}

static void SAMPLE_CIPHER_PerformanceTest()
{
    SAMPLE_CIPHER_AES_PerformanceTest();
    SAMPLE_CIPHER_DES_PerformanceTest();
}


int main()
{
    int ret;
    AX_SYS_Init();
    ret = AX_CIPHER_Init();
    if (ret < 0) {
        printf("AX_CIPHER_Init failed, ret: %x\n", ret);
        return -1;
    }
    SAMPLE_CIPHER_FunctionalTest();
    SAMPLE_CIPHER_PerformanceTest();
    ret = AX_CIPHER_DeInit();
    if (ret < 0) {
        printf("Deinit failed, ret: %x\n", ret);
        return -1;
    }
    return 0;
}
