/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include "ax_base_type.h"

#define SPECIAL_REG_FILE "/dev/ax_special_reg"
#define AX_REG_READ 0
#define AX_REG_WRITE 1
typedef struct {
    unsigned int addr;
    unsigned int val;
} ax_special_reg;
#define PIN_MCK_2_REG (0x1002023c)

static int SAMPLE_SPECIAL_Reg(void)
{
    int fd;
    int ret;
    int times = 0;
    unsigned int val;
    ax_special_reg regs;
    fd = open(SPECIAL_REG_FILE, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("open fail %s\n", SPECIAL_REG_FILE);
        return -1;
    }
    regs.addr = PIN_MCK_2_REG;
    regs.val = 0;
    ret = ioctl(fd, AX_REG_READ, &regs);
    if (ret < 0) {
        printf("AX_REG_READ fail: %d\n", ret);
        close(fd);
        return -1;
    }
    val = regs.val & 0xf;
    val ^= 0xf;
    regs.val = (regs.val & ~0xf) | val;
    val = regs.val;
    while (1) {
        ret = ioctl(fd, AX_REG_WRITE, &regs);
        if (ret < 0) {
            printf("AX_REG_WRITE fail: %d\n", ret);
            close(fd);
            return -1;
        }
        usleep(100);
        regs.val = 0;
        ret = ioctl(fd, AX_REG_READ, &regs);
        if (ret < 0) {
            printf("AX_REG_READ fail: %d\n", ret);
            close(fd);
            return -1;
        }
        if (val != regs.val) {
            printf("write val and read val not same, reg addr: %x, write val: %x, read val: %x\n", regs.addr, regs.val, val);
        }
        printf("reg addr: %x, val: %x\n", regs.addr, regs.val);
        usleep(100);
        times++;
        if ((times % 1000) == 0) {
            printf("test times: %d\n", times);
        }
        val = regs.val & 0xf;
        val ^= 0xf;
        regs.val = (regs.val & ~0xf) | val;
        val = regs.val;
    }
    close(fd);
}
int main(int argc, char *argv[])
{
    SAMPLE_SPECIAL_Reg();
    return 0;
}
