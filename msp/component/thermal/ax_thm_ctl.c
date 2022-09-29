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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#if defined(CHIP_AX620)
#define THM_DEV_NAME "/sys/class/thermal/thermal_zone0/temp"
#define PWM_DUTY_DEV_NAME "/sys/class/pwm/pwmchip4/pwm0/duty_cycle"
#define PWM_EXPORT_DEV_NAME "/sys/class/pwm/pwmchip4/export"
#define PWM_PERIOD_DEV_NAME "/sys/class/pwm/pwmchip4/pwm0/period"
#define PWM_ENABLE_DEV_NAME "/sys/class/pwm/pwmchip4/pwm0/enable"
#elif defined(CHIP_AX630A)
#define PWM_DUTY_DEV_NAME "/sys/class/pwm/pwmchip0/pwm1/duty_cycle"
#define PWM_EXPORT_DEV_NAME "/sys/class/pwm/pwmchip0/export"
#define PWM_PERIOD_DEV_NAME "/sys/class/pwm/pwmchip0/pwm1/period"
#define PWM_ENABLE_DEV_NAME "/sys/class/pwm/pwmchip0/pwm1/enable"
#endif
#define PWM_DEFAULT_PERIOD_CYCLE 4167
#define THM_PERIOD_SECOND (1)

#define AX_LOG_ERR(fmt,...) fprintf(stderr, "[E][%s:%d] "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define INVALID_THM (0xffffffff)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef enum {
    AX_NN_VDD = 0,
    AX_NN_MEM = 1,
    AX_CORE_VDD = 2,
} AX_DCDC_T;
typedef struct {
    int tempLow;
    int tempHigh;
    int nnVdd;
    int nnMem;
    int coreVdd;
} AX_THM_CTRL_VDD_POINT;

typedef struct {
    int vol;
    int duty;
} AX_VDD2PWM;
#ifdef AX_THM_CTL_ENABLE
#if defined(CHIP_AX620)
static AX_THM_CTRL_VDD_POINT thmCtrlVddTable[] = {
    {-273, 0, 650, 750, 800},
    {-5, 200, 630,750, 800},
};
#elif defined(CHIP_AX630A)
static AX_THM_CTRL_VDD_POINT thmCtrlVddTable[] = {
    {-273, 50, 680, 750, 800},
    {40, 200, 650,750, 800},
};
#endif
static pthread_t axThermalThread;
static AX_VDD2PWM npuVddDuty[] = {
    {750, 0},
    {740, 3},
    {730, 6},
    {720, 10},
    {710, 13},
    {700, 16},
    {690, 20},
    {680, 23},
    {670, 26},
    {660, 30},
    {650, 33},
    {640, 36},
    {630, 40},
    {620, 43},
    {610, 46},
    {600, 50},
    {590, 53},
    {580, 56},
    {570, 60},
    {560, 63},
    {550, 66},
    {540, 70},
    {530, 73},
};

static AX_THM_CTRL_VDD_POINT *AX_THM_CTRL_GetThmCtrlVddTable(void)
{
    return thmCtrlVddTable;
}

static int AX_THM_CTRL_Vdd2Duty(AX_VDD2PWM *table, int num, int vol)
{
    int i;
    for (i = 0; i < num; i++) {
        if (table[i].vol  <= vol) {
            return table[i].duty;
        }
    }
    return -1;
}
static int AX_THM_CTRL_SetVoltage(AX_DCDC_T dcdc, int voltage)
{
    int fd;
    char str[16];
    int ret;
    int duty = -1;
    if (dcdc == AX_NN_VDD) {
        duty = AX_THM_CTRL_Vdd2Duty(npuVddDuty, ARRAY_SIZE(npuVddDuty), voltage);
    }
    if (duty == -1) {
        AX_LOG_ERR("dcdc: %d, voltage: %d\n", dcdc, voltage);
        return -1;
    }

    fd = open(PWM_DUTY_DEV_NAME, O_RDWR | O_SYNC);
    if (fd < 0) {
        AX_LOG_ERR("open fail %s\n", PWM_DUTY_DEV_NAME);
        return -1;
    }
    duty = duty * PWM_DEFAULT_PERIOD_CYCLE / 100;
    sprintf(str, "%d", duty);
    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        AX_LOG_ERR("write file %s string %s fail\n", PWM_DUTY_DEV_NAME, str);
        close(fd);
        return -1;
    }
    close(fd);

    fd = open(PWM_ENABLE_DEV_NAME, O_RDWR | O_SYNC);
    if (fd < 0) {
        AX_LOG_ERR("open fail %s\n", PWM_ENABLE_DEV_NAME);
        return -1;
    }
    sprintf(str, "%d", 1);
    ret = write(fd, str, strlen(str));
    if (ret < 0) {
        AX_LOG_ERR("write file %s string %s fail\n", PWM_ENABLE_DEV_NAME, str);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
static int AX_THM_CTRL_GetTemp(void)
{
    int fd;
    int temp;
    int ret;
    char curTemp[8];
    fd = open(THM_DEV_NAME, O_RDONLY | O_SYNC);
    if (fd < 0) {
        AX_LOG_ERR("open Thm dev error: %d\n", fd);
        return -1;
    }
    ret = read(fd, curTemp, 8);
    if (ret < 0) {
        close(fd);
        AX_LOG_ERR("open Thm dev error: %d\n", ret);
        return -1;
    }
    temp = atoi(curTemp) / 1000;
    close(fd);
    return temp;
}

static int AX_THM_CTRL_GetNewRange(int oldTemp, int newTemp, int oldRange, AX_THM_CTRL_VDD_POINT *ctrlTable)
{
    int maxRange;
    maxRange = sizeof(thmCtrlVddTable) / sizeof(thmCtrlVddTable[0]);
    if ((newTemp < ctrlTable[oldRange].tempLow) && (oldRange >= 1)) {
        return (oldRange - 1);
    }
    if ((newTemp > ctrlTable[oldRange].tempHigh) && ((oldRange + 1) < maxRange)) {
        return (oldRange + 1);
    }
    return oldRange;
}
static int AX_THM_CTRL_GetInitRange(int temp, AX_THM_CTRL_VDD_POINT *ctrlTable)
{
    int i;
    int maxRange;
    maxRange = sizeof(thmCtrlVddTable) / sizeof(thmCtrlVddTable[0]);
    for (i = 0; i < maxRange; i++) {
        if (temp < ctrlTable[i].tempHigh) {
            break;
        }
    }
    return i;
}
static void *AX_THM_CTRL_Thread(void *arg)
{
    int tempCur;
    int tempOld;
    int newRange;
    int oldRange;
    AX_THM_CTRL_VDD_POINT *ctrlVddTable;
    tempOld = tempCur = AX_THM_CTRL_GetTemp();
    ctrlVddTable = AX_THM_CTRL_GetThmCtrlVddTable();
    oldRange = AX_THM_CTRL_GetInitRange(tempOld, ctrlVddTable);
    AX_THM_CTRL_SetVoltage(AX_NN_VDD, ctrlVddTable[oldRange].nnVdd);
    while (1) {
        sleep(THM_PERIOD_SECOND);
        tempCur = AX_THM_CTRL_GetTemp();
        newRange = AX_THM_CTRL_GetNewRange(tempOld, tempCur, oldRange, ctrlVddTable);
        if (newRange != oldRange) {
            AX_THM_CTRL_SetVoltage(AX_NN_VDD, ctrlVddTable[newRange].nnVdd);
            oldRange = newRange;
        }
        tempOld = tempCur;
    }
    return 0;
}
static int AX_THM_CTRL_Init()
{
    int fdExport;
    int fdPeriod;
    char str[8];
    int ret;
    fdPeriod = open(PWM_PERIOD_DEV_NAME, O_RDWR | O_SYNC);
    if (fdPeriod < 0) {
        fdExport = open(PWM_EXPORT_DEV_NAME, O_WRONLY | O_SYNC);
        if (fdExport < 0) {
            AX_LOG_ERR("%d\n", fdExport);
            return -1;
        }
        sprintf(str, "0");
        ret = write(fdExport, str, 1);
        if (ret < 0) {
            close(fdExport);
            AX_LOG_ERR("%d\n", ret);
            return -1;
        }
        close(fdExport);
        fdPeriod = open(PWM_PERIOD_DEV_NAME, O_RDWR | O_SYNC);
        if (fdPeriod < 0) {
            AX_LOG_ERR("%d\n", fdPeriod);
            return -1;
        }
    }
    sprintf(str, "%d", PWM_DEFAULT_PERIOD_CYCLE);
    ret = write(fdPeriod, str, strlen(str));
    if (ret < 0) {
        close(fdPeriod);
        AX_LOG_ERR("%d\n", ret);
        return -1;
    }
    close(fdPeriod);

    return 0;
}
#endif
int main(int argc, char *argv[])
{
#ifdef AX_THM_CTL_ENABLE
    int ret;
    ret = AX_THM_CTRL_Init();
    if (ret < 0) {
        AX_LOG_ERR("%d\n", ret);
        return -1;
    }
    pthread_create(&axThermalThread, NULL, AX_THM_CTRL_Thread, NULL);
#endif
    while (1) {
        sleep(10);
    }
    return 0;
}
