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
#include <stdlib.h>
#include <ax_base_type.h>
#include <ax_cipher_api.h>
#include <string.h>
#include <ax_sys_api.h>

static int trngSize[] = {4, 8, 16, 32, 100, 200, 400, 600, 768, 1024};

int SAMPLE_CIPHER_TrngPrintBuffer(char *buf, int size)
{
    int i = 0;
    for (i = 0 ; i < size; i++) {
        if ((i % 16) == 0) {
            printf("\n");
        }
        printf("%02x ", buf[i]);
    }
    printf("\n");
    return 0;
}
int SAMPLE_CIPHER_Trng_PerformanceTest()
{
    return 0;
}
int SAMPLE_CIPHER_Trng(void)
{
    int ret;
    unsigned int *buf;
    int size;
    int i;
    size = 1024;
    buf = malloc(1024);
    for (i = 0; i < 10; i++) {
        size = trngSize[i];
        ret = AX_CIPHER_GetRandomNumber(buf, size);
        if (ret != 0) {
            free(buf);
            printf("SAMPLE_CIPHER_Trng failed ret= %d, size: %d\n", ret, size);
            return -1;
        }
        //printf("dump i= %d, size = %d", i, size);
        //SAMPLE_CIPHER_TrngPrintBuffer((char *)buf,size);
    }
    free(buf);
    printf("SAMPLE_CIPHER_Trng PASS\n");
    return 0;
}