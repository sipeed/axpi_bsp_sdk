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
#include <ax_efuse_api.h>
static int testData[32] = {
    0x2D87C4C1, 0xD6A5F7FA, 0xAEA2B8BF, 0xF355ED13,
    0xAE31B2F8, 0x09E6718A, 0xF0995D80, 0x81AA21CF,
    0x72D11E34, 0x4468DE11, 0x248D690E, 0x6895C85B,
    0x00116468, 0x9842F634, 0xFC8D7A19, 0x1603033B,
    0x0A956A67, 0x1FF28E31, 0xA38178CC, 0xA218BAEC,
    0x11714E78, 0x0871D5B0, 0xF4952499, 0xF5B6315E,
    0x3691D253, 0xD73849F9, 0xE6985469, 0x19F656EE,
    0x3C0C6B8E, 0xD32C56A4, 0x274BDF4C, 0xCA93B7C6,
};
#define TEST_BLK_START 14
#define TEST_BLK_END 20
int main()
{
    AX_S32 i;
    AX_S32 ret;
    AX_S32 data;
    printf("This will write the Efuse, please confirm you want Do it!!!\n");
    ret = AX_EFUSE_Init();
    if (ret != AX_SUCCESS) {
        printf("Efuse init failed %x\n", ret);
        return -1;
    }
    for (i = TEST_BLK_START; i <= TEST_BLK_END; i++) {
        ret = AX_EFUSE_Write(i, testData[i]);
        if (ret != 0) {
            printf("Efuse test write error %x, %x\n", i, ret);
            return -1;
        }
    }
    for (i = TEST_BLK_START; i <= TEST_BLK_END; i++) {
        ret = AX_EFUSE_Read(i, &data);
        if (ret != 0) {
            printf("efuse test read error %x, %x\n", i, ret);
            return -1;
        }
        if (data != testData[i]) {
            printf("Efuse test data compare error %x, %x, %x\n", i, data, testData[i]);
            return -1;
        }
    }
    ret = AX_EFUSE_Deinit();
    if (ret != AX_SUCCESS) {
        printf("Efuse Deinit failed %x\n", ret);
        return -1;
    }
    printf("Efuse test PASS!\n");
    return 0;
}
