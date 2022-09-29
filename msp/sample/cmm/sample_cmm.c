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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include <pthread.h>
#include <poll.h>
#include <semaphore.h>
#include "ax_sys_api.h"
#include "ax_base_type.h"
#include "ax_sys_dma_api.h"


#define APP_PAGE_SIZE (4096)
#define DEFAULT_1_M 1024 * 1024
#define DEFAULT_ALLOC_TIME 1000

/*
test MemAlloc/MemFree
*/
static int ax_mem_cmm_test_001(void)
{
    AX_U64 PhyAddr[10]= {0};
    AX_VOID *pVirAddr[10] = {0};
    AX_U32 BlockSize = 1*1024*1024;
    AX_U32 align = 0x1000;
    AX_S8 blockname[20];
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_001 begin\n");

    for(i=0;i<10;i++)
    {
        sprintf((char *)blockname,"nc_block_%d",i);
        nRet = AX_SYS_MemAlloc(&PhyAddr[i], (AX_VOID **)&pVirAddr[i], BlockSize, align, blockname);
        if (nRet != 0) {
        	printf("AX_SYS_MemAlloc failed\n");
        	return -1;
        }

        printf("alloc PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr[i],pVirAddr[i]);
    }

    for(i=0;i<10;i++)
    {
        nRet = AX_SYS_MemFree(PhyAddr[i], (AX_VOID *)pVirAddr[i]);
        if (nRet != 0) {
        	printf("AX_SYS_MemFree failed\n");
        	return -1;
        }
        printf("free PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr[i],pVirAddr[i]);
    }

    printf("ax_mem_cmm_test_001 end success\n");
    return 0;
}

/*
test MemAllocCached/MemFree
*/
static int ax_mem_cmm_test_002(void)
{
    AX_U64 PhyAddr[10]= {0};
    AX_VOID *pVirAddr[10] = {0};
    AX_U32 BlockSize = 1*1024*1024;
    AX_U32 align = 0x1000;
    AX_S8 blockname[20];
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_002 begin\n");

    for(i=0;i<10;i++)
    {
        sprintf((char *)blockname,"c_block_%d",i);
        nRet = AX_SYS_MemAllocCached(&PhyAddr[i], (AX_VOID **)&pVirAddr[i], BlockSize, align, blockname);
        if (nRet != 0) {
        	printf("AX_SYS_MemAllocCached failed\n");
        	return -1;
        }
        printf("alloc PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr[i],pVirAddr[i]);
    }

    for(i=0;i<10;i++)
    {
        nRet = AX_SYS_MemFree(PhyAddr[i], (AX_VOID *)pVirAddr[i]);
        if (nRet != 0) {
        	printf("AX_SYS_MemFree failed\n");
        	return -1;
        }
        printf("free PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr[i],pVirAddr[i]);
    }

    printf("ax_mem_cmm_test_002 end success\n");
    return 0;
}

/*
 test GetBlockInfoByVirt/GetBlockInfoByPhy
*/
static int ax_mem_cmm_test_003(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_U64 PhyAddr2;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize;
    AX_S32 CacheType;
    AX_S32 nRet;

    printf("\nax_mem_cmm_test_003 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, 1*1024*1024, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
    	printf("AX_SYS_MemAlloc failed\n");
    	return -1;
    }

    printf("PhyAddr= 0x%llx,non-cached pVirAddr=%p\n",PhyAddr,pVirAddr);

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr + 0x1000, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr + 0x1000,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByPhy(PhyAddr, &CacheType,&pVirAddr2, &BlockSize);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByPhy failed\n");
        return -1;
    }else{
        printf("PhyAddr:0x%llx --> pVirAddr2:%p ,CacheType:%d,BlockSize=%d\n",PhyAddr, pVirAddr2, CacheType,BlockSize);
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
	if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_003 end success\n");
    return 0;
}

/*
 test AX_SYS_Mmap/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_004(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_004 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_Mmap(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_Mmap failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr2, 0x78, BlockSize);

    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_004 failed,i=%d\n",i);
            return -1;
        }
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_004 end success\n");
    return 0;
}

/*
 test AX_SYS_MmapCache/AX_SYS_MflushCache/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_005(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_005 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    memset((AX_U8 *)pVirAddr, 0xfd, BlockSize);

    pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCache failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr2, 0xfe, BlockSize);

    nRet = AX_SYS_MflushCache(PhyAddr, pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_MflushCache failed\n");
        return -1;
    }

    printf("pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_005 failed,i=%d\n",i);
            return -1;
        }
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_005 end success\n");
    return 0;
}

/*
 test AX_SYS_MmapCache/AX_SYS_MinvalidateCache/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_006(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_006 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCache failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr, 0xbc, BlockSize);
    memset((AX_U8 *)pVirAddr2, 0xfa, BlockSize);

    printf("before invalidate pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("before invalidate pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    nRet = AX_SYS_MinvalidateCache(PhyAddr, pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_MinvalidateCache failed\n");
        return -1;
    }

    memset((AX_U8 *)pVirAddr, 0xbb, BlockSize);

    printf("after invalidate pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("after invalidate pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_006 failed,i=%d\n",i);
            return -1;
        }
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_006 end success\n");
    return 0;
}

/*
 test MemFlushCache
 Note:insmod /soc/ko/ax_dma_hal.ko before testing
*/
static int ax_mem_cmm_test_007(void)
{
    AX_S32 l_size = 1*1024*1024;
    AX_S32 i = 0;
    AX_S32 j = 0;
    AX_S32 nRet = 0;
    AX_S8 str[DEFAULT_ALLOC_TIME][60];
    AX_U64 src_phyaddr[DEFAULT_ALLOC_TIME];
    AX_U8 *src_pviraddr[DEFAULT_ALLOC_TIME];
    AX_U64 dst_phyaddr[DEFAULT_ALLOC_TIME];
    AX_U8 *dst_pviraddr[DEFAULT_ALLOC_TIME];
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;
    AX_U32 TEST_TIME = 32;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_007 begin\n");

    while (1) {
        j++;
        l_size = j*DEFAULT_1_M;
        printf("ax_mem_cmm_flush start j:%d size:%d\n", j, l_size);
        sprintf((char *)str[j], "mem%d", j);
        nRet = AX_SYS_MemAllocCached(&src_phyaddr[j], (AX_VOID **)&src_pviraddr[j], l_size, APP_PAGE_SIZE, str[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemAllocCached failed\n");
            return -1;
        }

        memset(((AX_S8 *)src_pviraddr[j]), 0x78, l_size);
        for(i=0; i<=255; i++) {
            src_pviraddr[j][i] = 255-i;
        }

        nRet = AX_SYS_MemFlushCache(src_phyaddr[j], (AX_VOID *)src_pviraddr[j], l_size);
        if (nRet != 0) {
            printf("AX_SYS_MemFlushCache failed\n");
            return -1;
        }

        nRet = AX_SYS_MemAlloc(&dst_phyaddr[j], (AX_VOID **)&dst_pviraddr[j], l_size, APP_PAGE_SIZE, str[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc failed\n");
            return -1;
        }
        memset(((AX_S8 *)dst_pviraddr[j]), 0x39, l_size);

        //printf("begin DMA transfer: src_phyaddr:0x%llx dst_phyaddr:0x%llx size:%d\n",src_phyaddr[j], dst_phyaddr[j], l_size);

        dma_handle = AX_HAL_DMA_ChnOpen();
        if (dma_handle == AX_ERR_DMA_IO_FAIL) {
            printf("AX_HAL_DMA_ChnOpen error!Please insmod /soc/ko/ax_dma_hal.ko first!\n");
            return -1;
        }

        dma_cfg.direction = AX_DMA_MEM_TO_MEM;
        dma_cfg.lli_num = 1;
        dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
        dma_xfer_cfg.dst_addr = (u32)dst_phyaddr[j];
        dma_xfer_cfg.src_addr = (u32)src_phyaddr[j];
        dma_xfer_cfg.xfer_len = l_size;
        AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);

        AX_HAL_DMA_ChnStart(dma_handle);
        while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
            //printf("dma done\n");
        }
        //printf("after DMA transfer:\n");

        for (i = 0; i < l_size; i++) {
            if (dst_pviraddr[j][i] != src_pviraddr[j][i]) {
                printf("test MemFlushCache failed ! read data is not equel to write after flush cache\n");
                printf("src_data[%d][%d]:0x%x != dst_data[%d][%d]:0x%x\n", j, i, ((AX_S8 *)src_pviraddr[j])[i], j, i, ((AX_S8 *)dst_pviraddr[j])[i]);
                fail_count++;
                break;
            }
        }

        //printf("ax_mem_cmm_flush free j:%d\n", j);
        nRet = AX_SYS_MemFree(src_phyaddr[j], (AX_VOID *)src_pviraddr[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        nRet = AX_SYS_MemFree(dst_phyaddr[j], (AX_VOID *)dst_pviraddr[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        AX_HAL_DMA_ChnClose(dma_handle);

        if (j >= TEST_TIME) {
            //printf("ax_mem_cmm_flush end success j:%d\n", j);
            break;
        }else{
            //printf("ax_mem_cmm_flush success j:%d\n\n", j);
        }
    }
    printf("\nax_mem_cmm_test_007 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test MemInvalidateCache
 Note:insmod /soc/ko/ax_dma_hal.ko before testing
*/
static int ax_mem_cmm_test_008(void)
{
    AX_S32 l_size = 1*1024*1024;
    AX_S32 i = 0;
    AX_S32 j = 0;
    AX_S32 nRet = 0;
    AX_S8 str[DEFAULT_ALLOC_TIME][60];
    AX_U64 dst_phyaddr[DEFAULT_ALLOC_TIME];
    AX_U8 *dst_pviraddr[DEFAULT_ALLOC_TIME];
    AX_U64 src_phyaddr[DEFAULT_ALLOC_TIME];
    AX_U8 *src_pviraddr[DEFAULT_ALLOC_TIME];
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;
    AX_U32 TEST_TIME = 32;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_008 begin\n");

    while (1) {
        j++;
        l_size = j*DEFAULT_1_M;
        printf("ax_mem_cmm_invalidate start j:%d size:%d\n", j, l_size);
        sprintf((char *)str[j], "mem%d", j);

        nRet = AX_SYS_MemAllocCached(&dst_phyaddr[j], (AX_VOID **)&dst_pviraddr[j], l_size, APP_PAGE_SIZE, str[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemAllocCached failed\n");
            return -1;
        }

        memset(((AX_S8 *)dst_pviraddr[j]), 0xee, l_size);
        for(i=0; i<=255; i++) {
            dst_pviraddr[j][i] = i;
        }

        nRet = AX_SYS_MemFlushCache(dst_phyaddr[j], (AX_VOID *)dst_pviraddr[j], l_size);
        if (nRet != 0) {
            printf("AX_SYS_MemFlushCache dst failed\n");
            return -1;
        }

        nRet = AX_SYS_MemAlloc(&src_phyaddr[j], (AX_VOID **)&src_pviraddr[j], l_size, APP_PAGE_SIZE, str[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc failed\n");
            return -1;
        }

        memset(((AX_S8 *)src_pviraddr[j]), 0xff, l_size);

        for(i=0; i<=255; i++) {
            src_pviraddr[j][i] = 255-i;
        }

        //printf("begin DMA transfer:dst_phyaddr:0x%llx s_phyaddr:0x%llx size:%d\n",dst_phyaddr[j], src_phyaddr[j], l_size);

        dma_handle = AX_HAL_DMA_ChnOpen();
        if (dma_handle == AX_ERR_DMA_IO_FAIL) {
            printf("AX_HAL_DMA_ChnOpen error!Please insmod /soc/ko/ax_dma_hal.ko first!\n");
            return -1;
        }

        dma_cfg.direction = AX_DMA_MEM_TO_MEM;
        dma_cfg.lli_num = 1;
        dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
        dma_xfer_cfg.dst_addr = (u32)dst_phyaddr[j];
        dma_xfer_cfg.src_addr = (u32)src_phyaddr[j];
        dma_xfer_cfg.xfer_len = l_size;

        AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);
        AX_HAL_DMA_ChnStart(dma_handle);
        while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
            //printf("dma done\n");
        }
        //printf("after DMA transfer:\n");

        nRet = AX_SYS_MemInvalidateCache(dst_phyaddr[j], (AX_VOID *)dst_pviraddr[j], l_size);
        if (nRet != 0) {
            printf("AX_SYS_MemInvalidateCache failed\n");
            AX_HAL_DMA_ChnClose(dma_handle);
            return -1;
        }

        for (i = 0; i < l_size; i++) {
            if(dst_pviraddr[j][i] != src_pviraddr[j][i]) {
                printf("test MemInvalidateCache failed !\n");
                printf("dst_data[%d][%d]:0x%x != src_data[%d][%d]:0x%x\n", j, i, ((AX_S8 *)dst_pviraddr[j])[i], j, i, ((AX_S8 *)src_pviraddr[j])[i]);
                fail_count++;
                break;
            }
        }
        memset(((AX_S8 *)src_pviraddr[j]), 0xbc, l_size);

        //printf("ax_mem_cmm_invalidate free j:%d\n", j);
        nRet = AX_SYS_MemFree(dst_phyaddr[j], (AX_VOID *)dst_pviraddr[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        nRet = AX_SYS_MemFree(src_phyaddr[j], (AX_VOID *)src_pviraddr[j]);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        AX_HAL_DMA_ChnClose(dma_handle);
        if (j >= TEST_TIME) {
            //printf("ax_mem_cmm_invalidate end success j:%d\n", j);
            break;
        }else{
            //printf("ax_mem_cmm_invalidate success j:%d\n\n", j);
        }
    }
    printf("\nax_mem_cmm_test_008 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test MemFlushCache with offset address
 Note:insmod /soc/ko/ax_dma_hal.ko before testing

 should pass
*/
static int ax_mem_cmm_test_009(void)
{
    AX_S32 l_size = 1*1024*1024;
    AX_S32 i = 0,j = 0;
    AX_S32 nRet = 0;
    AX_S8 str[20];
    AX_U64 src_phyaddr;
    AX_VOID *src_pviraddr;
    AX_U64 dst_phyaddr;
    AX_VOID *dst_pviraddr;
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;
    AX_U32 offset = 0;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_009 begin\n");

    for(j=0;j<TEST_TIME;j++)
    {

        dma_handle = AX_HAL_DMA_ChnOpen();
        if (dma_handle == AX_ERR_DMA_IO_FAIL) {
            printf("AX_HAL_DMA_ChnOpen error!Please insmod /soc/ko/ax_dma_hal.ko first!\n");
            return -1;
        }

        l_size = 4*DEFAULT_1_M;

        sprintf((char *)str, "mem%d", j);
        nRet = AX_SYS_MemAllocCached(&src_phyaddr, (AX_VOID **)&src_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAllocCached j:%d failed\n",j);
            return -1;
        }

        memset((AX_U8 *)src_pviraddr, 0x78, l_size);
        for(i=0; i<=255; i++) {
          ((AX_U8 *)src_pviraddr)[i] = 255-i;
        }

        offset = 2*DEFAULT_1_M;
        nRet = AX_SYS_MemFlushCache(src_phyaddr+offset, (AX_VOID *)(src_pviraddr+offset), l_size);
        if (nRet != 0) {
            printf("AX_SYS_MemFlushCache j:%d failed\n",j);
            AX_HAL_DMA_ChnClose(dma_handle);
            return -1;
        }

        nRet = AX_SYS_MemAlloc(&dst_phyaddr, (AX_VOID **)&dst_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc j:%d failed\n",j);
            return -1;
        }
        memset(((AX_U8 *)dst_pviraddr), 0x39, l_size);

        dma_cfg.direction = AX_DMA_MEM_TO_MEM;
        dma_cfg.lli_num = 1;
        dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
        dma_xfer_cfg.dst_addr = (u32)dst_phyaddr;
        dma_xfer_cfg.src_addr = (u32)src_phyaddr;
        dma_xfer_cfg.xfer_len = l_size;
        AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);

        AX_HAL_DMA_ChnStart(dma_handle);
        while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
            //printf("dma done\n");
        }

        for (i = offset; i < l_size; i++) {
            if ((((AX_U8 *)dst_pviraddr)[i]) != (((AX_U8 *)src_pviraddr)[i])) {
                printf("test MemFlushCache failed %d! read data is not equel to write after flush cache\n",j);
                printf("src_data[%d]:0x%x != dst_data[%d]:0x%x\n", i, ((AX_U8 *)src_pviraddr)[i], i, ((AX_U8 *)dst_pviraddr)[i]);
                fail_count++;
                break;
            }
        }
        memset(((AX_U8 *)dst_pviraddr), 0x93, l_size);
        memset(((AX_U8 *)src_pviraddr), 0x98, l_size);

        nRet = AX_SYS_MemFree(src_phyaddr, (AX_VOID *)src_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        nRet = AX_SYS_MemFree(dst_phyaddr, (AX_VOID *)dst_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }
        AX_HAL_DMA_ChnClose(dma_handle);

    }//end of for

    printf("\nax_mem_cmm_test_009 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test MemFlushCache with offset address
 Note:insmod /soc/ko/ax_dma_hal.ko before testing

 should pass
*/
static int ax_mem_cmm_test_010(void)
{
    AX_S32 l_size = 1*1024*1024;
    AX_S32 i = 0;
    AX_S32 j = 0;
    AX_S32 nRet = 0;
    AX_S8 str[20];
    AX_U64 src_phyaddr;
    AX_VOID *src_pviraddr;
    AX_U64 dst_phyaddr;
    AX_VOID *dst_pviraddr;
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;
    AX_U32 offset = 0;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_010 begin\n");


    for(j=0;j<TEST_TIME;j++)
    {
        dma_handle = AX_HAL_DMA_ChnOpen();
        if (dma_handle == AX_ERR_DMA_IO_FAIL) {
            printf("AX_HAL_DMA_ChnOpen error!Please insmod /soc/ko/ax_dma_hal.ko first!\n");
            return -1;
        }

        l_size = 4*DEFAULT_1_M;

        sprintf((char *)str, "mem%d", j);
        nRet = AX_SYS_MemAllocCached(&src_phyaddr, (AX_VOID **)&src_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAllocCached j:%d failed\n",j);
            return -1;
        }

        memset((AX_U8 *)src_pviraddr, 0x78, l_size);
        for(i=0; i<=255; i++) {
            ((AX_U8 *)src_pviraddr)[i] = 255-i;
        }

        offset = 2*DEFAULT_1_M;
        nRet = AX_SYS_MemFlushCache(src_phyaddr+offset, (AX_VOID *)(src_pviraddr+offset), l_size*2);
        if (nRet != 0) {
            printf("AX_SYS_MemFlushCache j:%d failed\n",j);
            AX_HAL_DMA_ChnClose(dma_handle);
            return -1;
        }

        nRet = AX_SYS_MemAlloc(&dst_phyaddr, (AX_VOID **)&dst_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc j:%d failed\n",j);
            return -1;
        }
        memset(((AX_U8 *)dst_pviraddr), 0x39, l_size);

        dma_cfg.direction = AX_DMA_MEM_TO_MEM;
        dma_cfg.lli_num = 1;
        dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
        dma_xfer_cfg.dst_addr = (u32)dst_phyaddr;
        dma_xfer_cfg.src_addr = (u32)src_phyaddr;
        dma_xfer_cfg.xfer_len = l_size;
        AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);

        AX_HAL_DMA_ChnStart(dma_handle);
        while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
            //printf("dma done\n");
        }

        for (i = offset; i < l_size; i++) {
            if (((AX_U8 *)dst_pviraddr)[i] != ((AX_U8 *)src_pviraddr)[i]) {
                printf("test MemFlushCache failed %d! read data is not equel to write after flush cache\n",j);
                printf("src_data[%d]:0x%x != dst_data[%d]:0x%x\n", i, ((AX_U8 *)src_pviraddr)[i], i, ((AX_U8 *)dst_pviraddr)[i]);
                fail_count++;
                break;
            }
        }
        memset(((AX_U8 *)dst_pviraddr), 0x93, l_size);
        memset(((AX_U8 *)src_pviraddr), 0x98, l_size);

        nRet = AX_SYS_MemFree(src_phyaddr, (AX_VOID *)src_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        nRet = AX_SYS_MemFree(dst_phyaddr, (AX_VOID *)dst_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        AX_HAL_DMA_ChnClose(dma_handle);

    }//end of for
    printf("\nax_mem_cmm_test_010 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test MemFlushCache with offset address
 Note:insmod /soc/ko/ax_dma_hal.ko before testing

 should pass
*/
static int ax_mem_cmm_test_011(void)
{
    AX_S32 l_size = 1*1024*1024;
    AX_S32 i = 0;
    AX_S32 j = 0;
    AX_S32 nRet = 0;
    AX_S8 str[20];
    AX_U64 src_phyaddr;
    AX_VOID *src_pviraddr;
    AX_U64 dst_phyaddr;
    AX_VOID *dst_pviraddr;
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;
    AX_U32 offset = 0;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_011 begin\n");


    for(j=0;j<TEST_TIME;j++)
    {
        dma_handle = AX_HAL_DMA_ChnOpen();
        if (dma_handle == AX_ERR_DMA_IO_FAIL) {
            printf("AX_HAL_DMA_ChnOpen error!Please insmod /soc/ko/ax_dma_hal.ko first!\n");
            return -1;
        }

        l_size = 4*DEFAULT_1_M;

        sprintf((char *)str, "mem%d", j);
        nRet = AX_SYS_MemAllocCached(&src_phyaddr, (AX_VOID **)&src_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAllocCached j:%d failed\n",j);
            return -1;
        }

        memset((AX_U8 *)src_pviraddr, 0x88, l_size);
        for(i=0; i<=255; i++) {
            ((AX_U8 *)src_pviraddr)[i] = 255-i;
        }

        offset = 1*DEFAULT_1_M;
        nRet = AX_SYS_MemFlushCache(src_phyaddr+offset, (AX_VOID *)(src_pviraddr+offset), l_size/4);
        if (nRet != 0) {
            printf("AX_SYS_MemFlushCache j:%d failed\n",j);
            AX_HAL_DMA_ChnClose(dma_handle);
            return -1;
        }

        nRet = AX_SYS_MemAlloc(&dst_phyaddr, (AX_VOID **)&dst_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc j:%d failed\n",j);
            return -1;
        }
        memset(((AX_U8 *)dst_pviraddr), 0x49, l_size);

        dma_cfg.direction = AX_DMA_MEM_TO_MEM;
        dma_cfg.lli_num = 1;
        dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
        dma_xfer_cfg.dst_addr = (u32)dst_phyaddr;
        dma_xfer_cfg.src_addr = (u32)src_phyaddr;
        dma_xfer_cfg.xfer_len = l_size;
        AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);

        AX_HAL_DMA_ChnStart(dma_handle);
        while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
            //printf("dma done\n");
        }

        for (i = offset; i < l_size/4+offset; i++) {
            if (((AX_U8 *)dst_pviraddr)[i] != ((AX_U8 *)src_pviraddr)[i]) {
                printf("test MemFlushCache failed %d! read data is not equel to write after flush cache\n",j);
                printf("src_data[%d]:0x%x != dst_data[%d]:0x%x\n", i, ((AX_U8 *)src_pviraddr)[i], i, ((AX_U8 *)dst_pviraddr)[i]);
                fail_count++;
                break;
            }
        }
        memset(((AX_U8 *)dst_pviraddr), 0x93, l_size);
        memset(((AX_U8 *)src_pviraddr), 0x98, l_size);

        nRet = AX_SYS_MemFree(src_phyaddr, (AX_VOID *)src_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        nRet = AX_SYS_MemFree(dst_phyaddr, (AX_VOID *)dst_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        AX_HAL_DMA_ChnClose(dma_handle);

    }//end of for
    printf("\nax_mem_cmm_test_011 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test MemFlushCache with offset address
 Note:insmod /soc/ko/ax_dma_hal.ko before testing

 We expect this case to fail
*/
static int ax_mem_cmm_test_012(void)
{
    AX_S32 l_size = 1*1024*1024;
    AX_S32 i = 0;
    AX_S32 j = 0;
    AX_S32 nRet = 0;
    AX_S8 str[20];
    AX_U64 src_phyaddr;
    AX_VOID *src_pviraddr;
    AX_U64 dst_phyaddr;
    AX_VOID *dst_pviraddr;
    AXI_DMA_HAND_T dma_handle;
    AXI_DMA_CFG_T dma_cfg;
    AXI_DMA_XFER_T dma_xfer_cfg;
    AX_U32 offset = 0;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_012 begin (We expect this case to fail)\n");


    for(j=0;j<TEST_TIME;j++)
    {
        dma_handle = AX_HAL_DMA_ChnOpen();
        if (dma_handle == AX_ERR_DMA_IO_FAIL) {
            printf("AX_HAL_DMA_ChnOpen error!Please insmod /soc/ko/ax_dma_hal.ko first!\n");
            return -1;
        }

        l_size = 4*DEFAULT_1_M;

        sprintf((char *)str, "mem%d", j);
        nRet = AX_SYS_MemAllocCached(&src_phyaddr, (AX_VOID **)&src_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAllocCached j:%d failed\n",j);
            return -1;
        }

        memset((AX_U8 *)src_pviraddr, 0x88, l_size);
        for(i=0; i<=255; i++) {
            ((AX_U8 *)src_pviraddr)[i] = 255-i;
        }

        offset = 1*DEFAULT_1_M;
        nRet = AX_SYS_MemFlushCache(src_phyaddr+offset, (AX_VOID *)(src_pviraddr+offset), l_size/4);
        if (nRet != 0) {
            printf("AX_SYS_MemFlushCache j:%d failed\n",j);
            AX_HAL_DMA_ChnClose(dma_handle);
            return -1;
        }

        nRet = AX_SYS_MemAlloc(&dst_phyaddr, (AX_VOID **)&dst_pviraddr, l_size, APP_PAGE_SIZE, str);
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc j:%d failed\n",j);
            return -1;
        }
        memset(((AX_U8 *)dst_pviraddr), 0x49, l_size);

        dma_cfg.direction = AX_DMA_MEM_TO_MEM;
        dma_cfg.lli_num = 1;
        dma_cfg.xfer_mode = AX_DMA_TRF_SINGLE;
        dma_xfer_cfg.dst_addr = (u32)dst_phyaddr;
        dma_xfer_cfg.src_addr = (u32)src_phyaddr;
        dma_xfer_cfg.xfer_len = l_size;
        AX_HAL_DMA_ChnCfg(dma_handle, &dma_cfg, &dma_xfer_cfg);

        AX_HAL_DMA_ChnStart(dma_handle);
        while (AX_HAL_DMA_ChnWaitDone(dma_handle, AX_DMA_BROKED_MODE, AX_DMA_WAITING_FOREVER) != AX_SUCCESS) {
            //printf("dma done\n");
        }

        //should fail
        for (i = offset; i < l_size/2+offset; i++) {
            if (((AX_U8 *)dst_pviraddr)[i] != ((AX_U8 *)src_pviraddr)[i]) {
                printf("test MemFlushCache failed %d! read data is not equel to write after flush cache\n",j);
                printf("src_data[%d]:0x%x != dst_data[%d]:0x%x\n", i, ((AX_U8 *)src_pviraddr)[i], i, ((AX_U8 *)dst_pviraddr)[i]);
                fail_count++;
                break;
            }
        }
        memset(((AX_U8 *)dst_pviraddr), 0x93, l_size);
        memset(((AX_U8 *)src_pviraddr), 0x98, l_size);

        nRet = AX_SYS_MemFree(src_phyaddr, (AX_VOID *)src_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        nRet = AX_SYS_MemFree(dst_phyaddr, (AX_VOID *)dst_pviraddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree j:%d failed\n", j);
            return -1;
        }

        AX_HAL_DMA_ChnClose(dma_handle);

    }//end of for
    printf("\nax_mem_cmm_test_012 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test AX_SYS_MmapCache/AX_SYS_MflushCache/AX_SYS_Munmap with offset address
 should pass
*/
static int ax_mem_cmm_test_013(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 4*DEFAULT_1_M;
    AX_S32 nRet,i,j;
    AX_U32 offset = 0;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count =0;

    printf("\nax_mem_cmm_test_013 begin\n");

    for(j=0;j<TEST_TIME;j++)
    {
        nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc failed\n");
            return -1;
        }
        memset((AX_U8 *)pVirAddr, 0xfd, BlockSize);
        //printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

        pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
        if(pVirAddr2 == NULL){
            printf("AX_SYS_MmapCache failed\n");
            return -1;
        }else{
            //printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
        }

        memset((AX_U8 *)pVirAddr2, 0xfe, BlockSize);

        offset = 1*DEFAULT_1_M;
        nRet = AX_SYS_MflushCache(PhyAddr+offset, pVirAddr2+offset, BlockSize/2);
        if (nRet != 0) {
            printf("AX_SYS_MflushCache failed\n");
            return -1;
        }

        for(i=offset; i<BlockSize/2+offset; i++) {
            if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
                printf("ax_mem_cmm_test_013 failed %d\n",j);
                printf("pVirAddr[%d]:0x%x != pVirAddr2[%d]:0x%x\n", i, ((AX_U8 *)pVirAddr)[i], i, ((AX_U8 *)pVirAddr2)[i]);
                fail_count++;
                break;
            }
        }

        nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
        if (nRet != 0) {
            printf("AX_SYS_Munmap failed\n");
            return -1;
        }

        nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree failed\n");
            return -1;
        }
    }
    printf("\nax_mem_cmm_test_013 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test AX_SYS_MmapCache/AX_SYS_MflushCache/AX_SYS_Munmap with offset address
 We expect this case to fail
*/
static int ax_mem_cmm_test_014(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 4*DEFAULT_1_M;
    AX_S32 nRet,i,j;
    AX_U32 offset = 0;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count = 0;

    printf("\nax_mem_cmm_test_014 begin (We expect this case to fail)\n");

    for(j=0;j<TEST_TIME;j++)
    {
        nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
        if (nRet != 0) {
            printf("AX_SYS_MemAlloc failed\n");
            return -1;
        }

        memset((AX_U8 *)pVirAddr, 0x85, BlockSize);
        //printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

        pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
        if(pVirAddr2 == NULL){
            printf("AX_SYS_MmapCache failed\n");
            return -1;
        }else{
            //printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
        }

        memset((AX_U8 *)pVirAddr2, 0x84, BlockSize);

        offset = 1*DEFAULT_1_M;
        nRet = AX_SYS_MflushCache(PhyAddr+offset, pVirAddr2+offset, BlockSize/4);
        if (nRet != 0) {
            printf("AX_SYS_MflushCache failed\n");
            return -1;
        }

        //should fail
        for(i=offset; i<BlockSize; i++) {
            if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
                printf("ax_mem_cmm_test_014 failed %d\n",j);
                printf("pVirAddr[%d]:0x%x != pVirAddr2[%d]:0x%x\n", i, ((AX_U8 *)pVirAddr)[i], i, ((AX_U8 *)pVirAddr2)[i]);
                fail_count++;
                break;
            }
        }

        memset((AX_U8 *)pVirAddr, 0x88, BlockSize);
        memset((AX_U8 *)pVirAddr2, 0x94, BlockSize);

        nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
        if (nRet != 0) {
            printf("AX_SYS_Munmap failed\n");
            return -1;
        }

        nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
        if (nRet != 0) {
            printf("AX_SYS_MemFree failed\n");
            return -1;
        }
    }
    printf("\nax_mem_cmm_test_014 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test AX_SYS_MmapCache/AX_SYS_MflushCache/AX_SYS_Munmap 非托管模式
 should pass
*/
static int ax_mem_cmm_test_015(void)
{
    AX_U64 PhyAddr = 0x90000000;//该物理地址需没有被当前进程使用
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*DEFAULT_1_M;
    AX_S32 nRet,i,j;
    AX_U32 TEST_TIME = 100;
    AX_U32 fail_count =0;

    printf("\nax_mem_cmm_test_015 begin\n");

    for(j=0;j<TEST_TIME;j++)
    {
        pVirAddr = AX_SYS_Mmap(PhyAddr, BlockSize);
        if(pVirAddr == NULL){
            printf("AX_SYS_Mmap failed\n");
            return -1;
        }else{
            //printf("PhyAddr= 0x%llx,noncached pVirAddr=%p\n",PhyAddr,pVirAddr);
        }

        memset((AX_U8 *)pVirAddr, 0xdf, BlockSize);

        pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
        if(pVirAddr2 == NULL){
            printf("AX_SYS_MmapCache failed\n");
            return -1;
        }else{
            //printf("PhyAddr= 0x%llx,cached pVirAddr2=%p\n",PhyAddr,pVirAddr2);
        }

        memset((AX_U8 *)pVirAddr2, 0xde, BlockSize);

        nRet = AX_SYS_MflushCache(PhyAddr, pVirAddr2, BlockSize);
        if (nRet != 0) {
            printf("AX_SYS_MflushCache failed\n");
            return -1;
        }

        for(i=0; i<BlockSize; i++) {
            if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
                printf("ax_mem_cmm_test_015 failed %d\n",j);
                printf("pVirAddr[%d]:0x%x != pVirAddr2[%d]:0x%x\n", i, ((AX_U8 *)pVirAddr)[i], i, ((AX_U8 *)pVirAddr2)[i]);
                fail_count++;
                break;
            }
        }

        nRet = AX_SYS_Munmap(pVirAddr, BlockSize);
        if (nRet != 0) {
            printf("AX_SYS_Munmap pVirAddr failed\n");
            return -1;
        }

        nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
        if (nRet != 0) {
            printf("AX_SYS_Munmap pVirAddr2 failed\n");
            return -1;
        }

    }
    printf("\nax_mem_cmm_test_015 end.Total:%d, Pass:%d, Fail:%d\n",TEST_TIME,TEST_TIME-fail_count,fail_count);
    return 0;
}

/*
 test AX_SYS_MmapCache/AX_SYS_MinvalidateCache/AX_SYS_Munmap 非托管模式
 should pass
*/
static int ax_mem_cmm_test_016(void)
{
    AX_U64 PhyAddr = 0x90000000;//该物理地址需没有被当前进程使用
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_016 begin\n");

    pVirAddr = AX_SYS_Mmap(PhyAddr, BlockSize);
    if(pVirAddr == NULL){
        printf("AX_SYS_Mmap failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,noncached pVirAddr=%p\n",PhyAddr,pVirAddr);
    }

    pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCache failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,cached pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr, 0xbc, BlockSize);
    memset((AX_U8 *)pVirAddr2, 0xfa, BlockSize);

    printf("before invalidate pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("before invalidate pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    nRet = AX_SYS_MinvalidateCache(PhyAddr, pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_MinvalidateCache failed\n");
        return -1;
    }

    memset((AX_U8 *)pVirAddr, 0xbb, BlockSize);

    printf("after invalidate pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("after invalidate pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_016 failed,i=%d\n",i);
            return -1;
        }
    }

    nRet = AX_SYS_Munmap(pVirAddr, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap pVirAddr failed\n");
        return -1;
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap pVirAddr2 failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_016 end success\n");
    return 0;
}

/*
 test GetBlockInfoByVirt/GetBlockInfoByPhy,cached
*/
static int ax_mem_cmm_test_017(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_U64 PhyAddr2;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize;
    AX_S32 CacheType;
    AX_S32 nRet;

    printf("\nax_mem_cmm_test_017 begin\n");

    nRet = AX_SYS_MemAllocCached(&PhyAddr, (AX_VOID **)&pVirAddr, 1*1024*1024, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAllocCached failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,cached pVirAddr=%p\n",PhyAddr,pVirAddr);

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr + 0x1000, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr + 0x1000,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByPhy(PhyAddr, &CacheType,&pVirAddr2, &BlockSize);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByPhy failed\n");
        return -1;
    }else{
        printf("PhyAddr:0x%llx --> pVirAddr2:%p ,CacheType:%d,BlockSize=%d\n",PhyAddr, pVirAddr2, CacheType,BlockSize);
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
	if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_017 end success\n");
    return 0;
}

/*
 test GetBlockInfoByVirt/GetBlockInfoByPhy,mapped non-cached addr
*/
static int ax_mem_cmm_test_018(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_U64 PhyAddr2;
    AX_VOID *pVirAddr2;
    AX_VOID *pVirAddr3;
    AX_U32 BlockSize = 1*1024*1024;
    AX_S32 CacheType;
    AX_S32 nRet;

    printf("\nax_mem_cmm_test_018 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, 1*1024*1024, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_Mmap(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_Mmap failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new non-cached pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr2, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr2,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr2 + 0x1000, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr2 + 0x1000,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByPhy(PhyAddr, &CacheType,&pVirAddr3, &BlockSize);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByPhy failed\n");
        return -1;
    }else{
        printf("PhyAddr:0x%llx --> pVirAddr3:%p ,CacheType:%d,BlockSize=%d\n",PhyAddr, pVirAddr3, CacheType,BlockSize);
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap pVirAddr2 failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
	if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_018 end success\n");
    return 0;
}

/*
 test GetBlockInfoByVirt/GetBlockInfoByPhy,mapped cache addr
*/
static int ax_mem_cmm_test_019(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_U64 PhyAddr2;
    AX_VOID *pVirAddr2;
    AX_VOID *pVirAddr3;
    AX_U32 BlockSize = 1*1024*1024;
    AX_S32 CacheType;
    AX_S32 nRet;

    printf("\nax_mem_cmm_test_019 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, 1*1024*1024, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_MmapCache(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCache failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new cached pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr2, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr2,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByVirt(pVirAddr2 + 0x1000, &PhyAddr2, &CacheType);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByVirt failed\n");
        return -1;
    }else{
        printf("pVirAddr:%p --> PhyAddr2:0x%llx ,CacheType:%d\n",pVirAddr2 + 0x1000,PhyAddr2,CacheType);
    }

    nRet = AX_SYS_MemGetBlockInfoByPhy(PhyAddr, &CacheType,&pVirAddr3, &BlockSize);
    if(nRet != 0){
        printf("AX_SYS_MemGetBlockInfoByPhy failed\n");
        return -1;
    }else{
        printf("PhyAddr:0x%llx --> pVirAddr3:%p ,CacheType:%d,BlockSize=%d\n",PhyAddr, pVirAddr3, CacheType,BlockSize);
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap pVirAddr2 failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
	if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_019 end success\n");
    return 0;
}

static int ax_mem_cmm_test_020(void)
{
    AX_S32 ret=0,i=0;
    AX_POOL_FLOORPLAN_T PoolFloorPlan;
    AX_U32 BlkSize;
    AX_BLK BlkId;
    AX_U64 PhysAddr;
    AX_VOID *blockVirAddr;
    AX_VOID *pVirAddr;

    printf("\nax_mem_cmm_test_020 begin\n");

    ret = AX_POOL_Exit();

    if(ret){
        printf("AX_POOL_Exit fail!!Error Code:0x%X\n",ret);
        return -1;
    }else{
        printf("AX_POOL_Exit success!\n");
    }


    memset(&PoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));
    PoolFloorPlan.CommPool[0].MetaSize   = 0x1000;
    PoolFloorPlan.CommPool[0].BlkSize   = 3*1024*1024;
    PoolFloorPlan.CommPool[0].BlkCnt    = 5;
    PoolFloorPlan.CommPool[0].CacheMode = POOL_CACHE_MODE_NONCACHE;

    memset(PoolFloorPlan.CommPool[0].PartitionName,0,sizeof(PoolFloorPlan.CommPool[0].PartitionName));
    strcpy((char *)PoolFloorPlan.CommPool[0].PartitionName,"anonymous");

    ret = AX_POOL_SetConfig(&PoolFloorPlan);

    if(ret){
        printf("AX_POOL_SetConfig fail!Error Code:0x%X\n",ret);
        return -1;
    }else{
        printf("AX_POOL_SetConfig success!\n");
    }

    ret = AX_POOL_Init();
    if(ret){
        printf("AX_POOL_Init fail!!Error Code:0x%X\n",ret);
        return -1;
    }else{
        printf("AX_POOL_Init success!\n");
    }

    BlkSize = 3*1024*1024;//期望获取的buffer大小
    BlkId = AX_POOL_GetBlock(AX_INVALID_POOLID,BlkSize,NULL);

    if(BlkId == AX_INVALID_BLOCKID){
        printf("AX_POOL_GetBlock fail!\n");
        goto EXIT;
    }else{
        printf("AX_POOL_GetBlock success!BlkId:0x%X\n",BlkId);
    }

    PhysAddr = AX_POOL_Handle2PhysAddr(BlkId);

    if(!PhysAddr){
        printf("AX_POOL_Handle2PhysAddr fail!\n");
        goto EXIT;
    }else{
        printf("AX_POOL_Handle2PhysAddr success!(Blockid:0x%X --> PhyAddr=0x%llx)\n",BlkId,PhysAddr);
    }

    blockVirAddr = AX_POOL_GetBlockVirAddr(BlkId);

    if(!blockVirAddr){
        printf("AX_POOL_GetBlockVirAddr fail!\n");
        goto EXIT;
    }else{
        printf("AX_POOL_GetBlockVirAddr success!blockVirAddr=0x%x\n",(AX_U32)blockVirAddr);
    }

    //write data
    for(i = 0;i < 20; i++){
        ((AX_S32 *)blockVirAddr)[i] = i;
    }

    //read data
    for(i = 0;i < 20; i++){
        printf("%d,",((AX_S32 *)blockVirAddr)[i]);
    }
    printf("\n");


    //对common pool中单个block做映射
    pVirAddr = AX_SYS_Mmap(PhysAddr, BlkSize);
    if(pVirAddr == NULL){
        printf("AX_SYS_Mmap failed\n");
        goto EXIT;
    }else{
        printf("PhysAddr= 0x%llx,new pVirAddr=%p,size=%d\n",PhysAddr,pVirAddr,BlkSize);
    }
    //read data
    for(i = 0;i < 20; i++){
        printf("%d,",((AX_S32 *)pVirAddr)[i]);
    }
    printf("\n");

    ret = AX_SYS_Munmap(pVirAddr, BlkSize);
    if (ret != 0) {
        printf("AX_SYS_Munmap pVirAddr2 failed\n");
        goto EXIT;
    }else{
        printf("AX_SYS_Munmap success!pVirAddr=%p,size=%d\n",pVirAddr,BlkSize);
    }

    pVirAddr = AX_SYS_MmapCache(PhysAddr, BlkSize);
    if(pVirAddr == NULL){
        printf("AX_SYS_MmapCache failed\n");
        goto EXIT;
    }else{
        printf("PhysAddr= 0x%llx,new cached pVirAddr=%p,size=%d\n",PhysAddr,pVirAddr,BlkSize);
    }
    //read data
    for(i = 0;i < 20; i++){
        printf("%d,",((AX_S32 *)pVirAddr)[i]);
    }
    printf("\n");

    ret = AX_SYS_MflushCache(PhysAddr, pVirAddr, BlkSize);
    if (ret != 0) {
        printf("AX_SYS_MflushCache failed\n");
        goto EXIT;
    }else{
        printf("AX_SYS_MflushCache success!pVirAddr=%p,size=%d\n",pVirAddr,BlkSize);
    }

    ret = AX_SYS_MinvalidateCache(PhysAddr, pVirAddr, BlkSize);
    if (ret != 0) {
        printf("AX_SYS_MinvalidateCache failed\n");
        goto EXIT;
    }else{
        printf("AX_SYS_MinvalidateCache success!pVirAddr=%p,size=%d\n",pVirAddr,BlkSize);
    }

    ret = AX_SYS_Munmap(pVirAddr, BlkSize);
    if (ret != 0) {
        printf("AX_SYS_Munmap pVirAddr2 failed\n");
        goto EXIT;
    }else{
        printf("AX_SYS_Munmap success!pVirAddr=%p,size=%d\n",pVirAddr,BlkSize);
    }

    ret = AX_POOL_ReleaseBlock(BlkId);

    if(ret){
        printf("AX_POOL_ReleaseBlock fail!Error Code:0x%X\n",ret);
        goto EXIT;
    }else{
        printf("AX_POOL_ReleaseBlock success!Blockid=0x%X\n",BlkId);
    }

EXIT:
    ret = AX_POOL_Exit();

    if(ret){
        printf("AX_POOL_Exit fail!!Error Code:0x%X\n",ret);
        return -1;
    }else{
        printf("AX_POOL_Exit success!\n");
    }

    printf("ax_mem_cmm_test_020 end success\n");
    return 0;
}

/*
 test AX_SYS_MmapFast/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_021(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_VOID *pVirAddr3;
    AX_U32 BlockSize= 4*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_021 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_MmapFast(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapFast failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr2, 0x78, BlockSize);

    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_021 failed,i=%d\n",i);
            return -1;
        }
    }

    pVirAddr3 = AX_SYS_MmapFast(PhyAddr, BlockSize);
    if(pVirAddr3 == NULL){
        printf("AX_SYS_MmapFast failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new pVirAddr3=%p\n",PhyAddr,pVirAddr3);
    }

    if(pVirAddr2 != pVirAddr3){
        printf("failed!pVirAddr2 != pVirAddr3\n");
        return -1;
    }

    nRet = AX_SYS_Munmap(pVirAddr3, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_021 end success\n");
    return 0;
}

/*
 test AX_SYS_MmapCacheFast/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_022(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_VOID *pVirAddr3;
    AX_U32 BlockSize= 4*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_022 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_MmapCacheFast(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCacheFast failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new cached pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr2, 0x78, BlockSize);

    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    pVirAddr3 = AX_SYS_MmapCacheFast(PhyAddr, BlockSize);
    if(pVirAddr3 == NULL){
        printf("AX_SYS_MmapCacheFast failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new cached pVirAddr3=%p\n",PhyAddr,pVirAddr3);
    }

    if(pVirAddr2 != pVirAddr3){
        printf("failed!pVirAddr2 != pVirAddr3\n");
        return -1;
    }

    nRet = AX_SYS_Munmap(pVirAddr3, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_022 end success\n");
    return 0;
}

/*
 test AX_SYS_MmapCacheFast/AX_SYS_MflushCache/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_023(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_023 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    memset((AX_U8 *)pVirAddr, 0xfd, BlockSize);

    pVirAddr2 = AX_SYS_MmapCacheFast(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCacheFast failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr2, 0xfe, BlockSize);

    nRet = AX_SYS_MflushCache(PhyAddr, pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_MflushCache failed\n");
        return -1;
    }

    printf("pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_023 failed,i=%d\n",i);
            return -1;
        }
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_023 end success\n");
    return 0;
}

/*
 test AX_SYS_MmapCacheFast/AX_SYS_MinvalidateCache/AX_SYS_Munmap
*/
static int ax_mem_cmm_test_024(void)
{
    AX_U64 PhyAddr;
    AX_VOID *pVirAddr;
    AX_VOID *pVirAddr2;
    AX_U32 BlockSize= 1*1024*1024;
    AX_S32 nRet,i;

    printf("\nax_mem_cmm_test_024 begin\n");

    nRet = AX_SYS_MemAlloc(&PhyAddr, (AX_VOID **)&pVirAddr, BlockSize, 0x1000, (AX_S8 *)"mem_test");
    if (nRet != 0) {
        printf("AX_SYS_MemAlloc failed\n");
        return -1;
    }

    printf("PhyAddr= 0x%llx,pVirAddr=%p\n",PhyAddr,pVirAddr);

    pVirAddr2 = AX_SYS_MmapCacheFast(PhyAddr, BlockSize);
    if(pVirAddr2 == NULL){
        printf("AX_SYS_MmapCacheFast failed\n");
        return -1;
    }else{
        printf("PhyAddr= 0x%llx,new cached pVirAddr2=%p\n",PhyAddr,pVirAddr2);
    }

    memset((AX_U8 *)pVirAddr, 0xbc, BlockSize);
    memset((AX_U8 *)pVirAddr2, 0xfa, BlockSize);

    printf("before invalidate pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("before invalidate pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    nRet = AX_SYS_MinvalidateCache(PhyAddr, pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_MinvalidateCache failed\n");
        return -1;
    }

    memset((AX_U8 *)pVirAddr, 0xbb, BlockSize);

    printf("after invalidate pVirAddr  data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr)[i]);
    }
    printf("\n");

    printf("after invalidate pVirAddr2 data:");
    for(i=0; i<20; i++) {
        printf("0x%x,",((AX_U8 *)pVirAddr2)[i]);
    }
    printf("\n");

    for(i=0; i<BlockSize; i++) {
        if(((AX_U8 *)pVirAddr)[i] != ((AX_U8 *)pVirAddr2)[i]){
            printf("ax_mem_cmm_test_024 failed,i=%d\n",i);
            return -1;
        }
    }

    nRet = AX_SYS_Munmap(pVirAddr2, BlockSize);
    if (nRet != 0) {
        printf("AX_SYS_Munmap failed\n");
        return -1;
    }

    nRet = AX_SYS_MemFree(PhyAddr, (AX_VOID *)pVirAddr);
    if (nRet != 0) {
        printf("AX_SYS_MemFree failed\n");
        return -1;
    }
    printf("ax_mem_cmm_test_024 end success\n");
    return 0;
}

int main(AX_S32 argc, char *argv[])
{
    AX_SYS_Init();

    ax_mem_cmm_test_001();
    ax_mem_cmm_test_002();
    ax_mem_cmm_test_003();
    ax_mem_cmm_test_004();
    ax_mem_cmm_test_005();
    ax_mem_cmm_test_006();
    ax_mem_cmm_test_007();
    ax_mem_cmm_test_008();
    ax_mem_cmm_test_009();
    ax_mem_cmm_test_010();
    ax_mem_cmm_test_011();
    ax_mem_cmm_test_012();
    ax_mem_cmm_test_013();
    ax_mem_cmm_test_014();
    ax_mem_cmm_test_015();
    ax_mem_cmm_test_016();
    ax_mem_cmm_test_017();
    ax_mem_cmm_test_018();
    ax_mem_cmm_test_019();
    ax_mem_cmm_test_020();
    ax_mem_cmm_test_021();
    ax_mem_cmm_test_022();
    ax_mem_cmm_test_023();
    ax_mem_cmm_test_024();

    AX_SYS_Deinit();
    return 0;
}
