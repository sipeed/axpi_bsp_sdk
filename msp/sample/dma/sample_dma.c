/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <ax_sys_dma_api.h>
#include <ax_sys_api.h>
#include "ax_efuse_api.h"

void *src_test_virt, *dst_test_virt;

u64 test_addr_src, test_addr_dst;

static void AX_MC20_HAL_DMA_FUNC_001_001(void)
{
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;

    memset(src_test_virt, 0xa, 0x10000);
    if (!(dma_handle = AX_HAL_DMA_ChnOpen())) {
        printf("open error\n");
    }

    dma_cfg.direction = AX_DMA_MEM_TO_MEM;
    dma_cfg.lli_num = 1;
    dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
    dma_xfer_cfg.dst_addr = test_addr_dst;
    dma_xfer_cfg.src_addr = test_addr_src;
    dma_xfer_cfg.xfer_len = 0x10000;
    AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);
    AX_HAL_DMA_ChnStart(dma_handle);
    while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
        printf("dma done\n");

    }

    if (memcmp(dst_test_virt, src_test_virt, dma_xfer_cfg.xfer_len) != 0) {
        printf("dma mem fail\n");
    }
    AX_HAL_DMA_ChnClose(dma_handle);

}

static void AX_MC20_HAL_DMA_FUNC_001_002(void)
{
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;

    memset((void *)src_test_virt, 0x5, 0x10000);

    if (!(dma_handle = AX_HAL_DMA_ChnOpen())) {
        printf("open error\n");
    }
    dma_cfg.direction = AX_DMA_MEM_TO_MEM;
    dma_cfg.lli_num = 1;
    dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
    dma_xfer_cfg.dst_addr = test_addr_dst;
    dma_xfer_cfg.src_addr = test_addr_src;
    dma_xfer_cfg.xfer_len = 0x10000;
    AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);
    AX_HAL_DMA_ChnStart(dma_handle);
    while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_POLL_MODE, 0) != AX_SUCCESS);

    if (memcmp(dst_test_virt, src_test_virt, dma_xfer_cfg.xfer_len) != 0) {
        printf("dma mem fail\n");
    }
    AX_HAL_DMA_ChnClose(dma_handle);

}

static void AX_MC20_HAL_DMA_FUNC_001_003(void)
{
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;

    memset((void *)src_test_virt, 0x3, 0x1200000);
    if (!(dma_handle = AX_HAL_DMA_ChnOpen())) {
        printf("open error\n");
    }

    dma_cfg.direction = AX_DMA_MEM_TO_MEM;
    dma_cfg.lli_num = 1;
    dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
    dma_xfer_cfg.dst_addr = test_addr_dst;
    dma_xfer_cfg.src_addr = test_addr_src;
    dma_xfer_cfg.xfer_len = 0x1200000;
    AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);
    AX_HAL_DMA_ChnStart(dma_handle);
    while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
        printf("dma done\n");
    }

    if (memcmp(dst_test_virt, src_test_virt, dma_xfer_cfg.xfer_len) != 0) {
        printf("dma mem fail\n");
    }
    AX_HAL_DMA_ChnClose(dma_handle);

}

static void AX_MC20_HAL_DMA_FUNC_002_001(void)
{
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;

    memset((void *)src_test_virt, 0x1, 0x10000);
    if (!(dma_handle = AX_HAL_DMA_ChnOpen())) {
        printf("open error\n");
    }

    AX_HAL_DMA_ChnLliNumCfg(dma_handle, 32);
    dma_cfg.direction = AX_DMA_MEM_TO_MEM;
    dma_cfg.lli_num = 1;
    dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
    dma_xfer_cfg.dst_addr = test_addr_dst;
    dma_xfer_cfg.src_addr = test_addr_src;
    dma_xfer_cfg.xfer_len = 0x10000;
    AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);
    AX_HAL_DMA_ChnStart(dma_handle);
    while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
        printf("dma done\n");
    }

    if (memcmp(dst_test_virt, src_test_virt, dma_xfer_cfg.xfer_len) != 0) {
        printf("dma mem fail\n");
    }
    AX_HAL_DMA_ChnClose(dma_handle);


}

static void AX_MC20_HAL_DMA_FUNC_003_001(void)
{
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg[2];

    memset((void *)src_test_virt, 0xa, 0x10000);
    memset((void *)src_test_virt + 0x10000, 0x5, 0x10000);

    if (!(dma_handle = AX_HAL_DMA_ChnOpen())) {
        printf("open error\n");
    }
    dma_cfg.lli_num = 2;
    dma_cfg.xfer_mode = AX_DMA_TRF_LLI;
    dma_xfer_cfg[0].dst_addr = (u32)test_addr_dst;
    dma_cfg.direction = AX_DMA_MEM_TO_MEM;
    dma_xfer_cfg[0].src_addr = test_addr_src;
    dma_xfer_cfg[0].xfer_len = 0x10000;
    dma_xfer_cfg[1].dst_addr = test_addr_dst + 0x10000;
    dma_xfer_cfg[1].src_addr = test_addr_src + 0x10000;
    dma_xfer_cfg[1].xfer_len = 0x20000;
    AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, dma_xfer_cfg);
    AX_HAL_DMA_ChnStart(dma_handle);
    if (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
        printf("dma done\n");
    }

    if (memcmp(dst_test_virt, src_test_virt, dma_xfer_cfg[0].xfer_len) != 0) {
        printf("dma mem fail\n");
    }


    if (memcmp(dst_test_virt + 0x10000, src_test_virt + 0x10000, dma_xfer_cfg[1].xfer_len) != 0) {
        printf("dma mem fail\n");
    }
    AX_HAL_DMA_ChnClose(dma_handle);

}

static void AX_MC20_HAL_DMA_tc_init(void)
{
    u32 ret;
    AX_U64 phyAddr;

    AX_SYS_Init();
    ret = AX_SYS_MemAlloc(&phyAddr, &src_test_virt, 0x1200000, 0x4, (AX_S8 *)"dma_hal_test");
    if (ret < 0) {
        printf("AX_MC20_HAL_DMA_tc_init alloc failed %x\n", ret);
        return;
    }
    test_addr_src = phyAddr;
    ret = AX_SYS_MemAlloc(&phyAddr, &dst_test_virt, 0x1200000, 0x4, (AX_S8 *)"dma_hal_test");
    if (ret < 0) {
        printf("AX_MC20_HAL_DMA_tc_init alloc failed %x\n", ret);
        return;
    }
    test_addr_dst = phyAddr;
    if (!test_addr_src | !test_addr_dst) {
        printf("malloc error\n");

    }

    return;
}

static void AX_MC20_HAL_DMA_tc_cleanup(void)
{
    if (!test_addr_src | !test_addr_dst) {
        printf("clean error\n");
    }

    AX_SYS_MemFree(test_addr_src, src_test_virt);
    AX_SYS_MemFree(test_addr_dst, dst_test_virt);

    return;
}

int main()
{
    AX_MC20_HAL_DMA_tc_init();
    AX_MC20_HAL_DMA_FUNC_001_001();
    AX_MC20_HAL_DMA_FUNC_001_002();
    AX_MC20_HAL_DMA_FUNC_001_003();
    AX_MC20_HAL_DMA_FUNC_002_001();
    AX_MC20_HAL_DMA_FUNC_003_001();
    AX_MC20_HAL_DMA_tc_cleanup();
    printf("TEST DONE!!!!\n");
    return 0;
}

