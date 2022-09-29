/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/



#ifndef _AX_EFUSE_API_H_
#define _AX_EFUSE_API_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define EFSC_MAX_BLK 96
#ifndef AX_SUCCESS
#define AX_SUCCESS                          0
#endif

typedef enum {
    AX_ERR_EFUSE_ILLEGAL_PARAM = 0x8005000a,
    AX_ERR_EFUSE_SYS_NOTREADY = 0x80050010,
    AX_ERR_EFUSE_READ_FAIL = 0x80050080,
    AX_ERR_EFUSE_WRITE_FAIL = 0x80050081,
    AX_ERR_EFUSE_MMAP_FAIL = 0x80050082,
} EFUSE_STAUS_E;

AX_S32 AX_EFUSE_Init(void);
AX_S32 AX_EFUSE_Deinit(void);
AX_S32 AX_EFUSE_Read(AX_S32 blk, AX_S32 *data);
AX_S32 AX_EFUSE_Write(AX_S32 blk, AX_S32 data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
