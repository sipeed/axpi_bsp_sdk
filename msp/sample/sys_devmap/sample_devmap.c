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
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <ax_base_type.h>
#include <ax_sys_api.h>

#define MMAP_FILE_NAME "/dev/ax_devmap"

#define MMAP_TEST_LEN   0x1200000
void *src_test_virt, *dst_test_virt;
AX_U64 test_addr_src, test_addr_dst;

int main()
{
    AX_U64 ret, fd;
    AX_U64 phyAddr;
    char *addr_base0, *addr_base1;
    struct timeval start, end;

    AX_SYS_Init();
    ret = AX_SYS_MemAlloc(&phyAddr, &src_test_virt, MMAP_TEST_LEN, 0x4, (AX_S8 *)"sys_devmap_test");
    if (ret < 0) {
        printf("AX_MC20_HAL_DMA_tc_init alloc failed %llx\n", ret);
        return -1;
    }
    test_addr_src = phyAddr;
    ret = AX_SYS_MemAlloc(&phyAddr, &dst_test_virt, MMAP_TEST_LEN, 0x4, (AX_S8 *)"sys_devmap_test");
    if (ret < 0) {
        printf("AX_MC20_HAL_DMA_tc_init alloc failed %llx\n", ret);
        return -1;
    }

    // nocache
    test_addr_dst = phyAddr;
    fd = open(MMAP_FILE_NAME, O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("open %s fail!\n", MMAP_FILE_NAME);
        return -1;
    }
    addr_base0 = (char *)mmap(0, MMAP_TEST_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, test_addr_src);
    addr_base1 = (char *)mmap(0, MMAP_TEST_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, test_addr_dst);

    if ((addr_base0 == MAP_FAILED) || (addr_base1 == MAP_FAILED)) {
        printf("map fail\n");
        goto exit0;
    }

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; i++) {
        memcpy(addr_base0, addr_base1, MMAP_TEST_LEN);
    }
    gettimeofday(&end, NULL);
    printf("time : %ld.%ld\n", end.tv_sec - start.tv_sec,
           (end.tv_sec - start.tv_sec) ? (end.tv_usec - start.tv_usec + 1000000) : (end.tv_usec - start.tv_usec));

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; i++) {
        memcpy(addr_base0 + 1, addr_base1 + 1, MMAP_TEST_LEN - 1);
    }
    gettimeofday(&end, NULL);
    printf("time : %ld.%ld\n", end.tv_sec - start.tv_sec,
           (end.tv_sec - start.tv_sec) ? (end.tv_usec - start.tv_usec + 1000000) : (end.tv_usec - start.tv_usec));

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; i++) {
        memcpy(addr_base0 + 2, addr_base1 + 2, MMAP_TEST_LEN - 2);
    }
    gettimeofday(&end, NULL);
    printf("time : %ld.%ld\n", end.tv_sec - start.tv_sec,
           (end.tv_sec - start.tv_sec) ? (end.tv_usec - start.tv_usec + 1000000) : (end.tv_usec - start.tv_usec));

    for (int i = 0; i < 0x20; i++) {
        printf("loop %d\n", i);
        memcpy(addr_base0 + i, addr_base1 + i, MMAP_TEST_LEN - i);
        if (memcmp(addr_base0 + i, addr_base1 + i, MMAP_TEST_LEN - i)) {
            printf("memcpy fail\n");
        }
    }

    if (addr_base0) {
        munmap((void *)addr_base0, MMAP_TEST_LEN);
        addr_base0 = 0;
    }

    if (addr_base1) {
        munmap((void *)addr_base1, MMAP_TEST_LEN);
        addr_base1 = 0;
    }
    close(fd);

    // cache
    fd = open(MMAP_FILE_NAME, O_RDWR);
    if (fd == -1) {
        printf("open %s fail!\n", MMAP_FILE_NAME);
        return -1;
    }
    addr_base0 = (char *)mmap(0, MMAP_TEST_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, test_addr_src);
    addr_base1 = (char *)mmap(0, MMAP_TEST_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, test_addr_dst);

    if ((addr_base0 == MAP_FAILED) || (addr_base1 == MAP_FAILED)) {
        printf("map fail\n");
        goto exit0;
    }

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; i++) {
        memcpy(addr_base0, addr_base1, MMAP_TEST_LEN);
    }
    gettimeofday(&end, NULL);
    printf("cache time : %ld.%ld\n", end.tv_sec - start.tv_sec,
           (end.tv_sec - start.tv_sec) ? (end.tv_usec - start.tv_usec + 1000000) : (end.tv_usec - start.tv_usec));

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; i++) {
        memcpy(addr_base0 + 1, addr_base1 + 1, MMAP_TEST_LEN - 1);
    }
    gettimeofday(&end, NULL);
    printf("cache time : %ld.%ld\n", end.tv_sec - start.tv_sec,
           (end.tv_sec - start.tv_sec) ? (end.tv_usec - start.tv_usec + 1000000) : (end.tv_usec - start.tv_usec));

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; i++) {
        memcpy(addr_base0 + 2, addr_base1 + 2, MMAP_TEST_LEN - 2);
    }
    gettimeofday(&end, NULL);
    printf("cache time : %ld.%ld\n", end.tv_sec - start.tv_sec,
           (end.tv_sec - start.tv_sec) ? (end.tv_usec - start.tv_usec + 1000000) : (end.tv_usec - start.tv_usec));

    for (int i = 0; i < 0x20; i++) {
        memcpy(addr_base0 + i, addr_base1 + i, MMAP_TEST_LEN - i);
        if (memcmp(addr_base0 + i, addr_base1 + i, MMAP_TEST_LEN - i)) {
            printf("memcpy fail\n");
        }
    }
exit0:
    if (addr_base0) {
        munmap((void *)addr_base0, MMAP_TEST_LEN);
        addr_base0 = 0;
    }

    if (addr_base1) {
        munmap((void *)addr_base1, MMAP_TEST_LEN);
        addr_base1 = 0;
    }
    close(fd);
    AX_SYS_MemFree(test_addr_src, src_test_virt);
    AX_SYS_MemFree(test_addr_dst, src_test_virt);

    printf("samp devmap test pass\n");
    return 0;
}
