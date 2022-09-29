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
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */
static const char default_rtc[] = "/dev/rtc0";


int main(int argc, char **argv)
{
    int i, fd, retval, irqcount = 0;
    unsigned long tmp, data;
    struct rtc_time rtc_tm;
    const char *rtc = default_rtc;

    switch (argc) {
    case 2:
        rtc = argv[1];
    /* FALLTHROUGH */
    case 1:
        break;
    default:
        fprintf(stderr, "usage:  rtctest [rtcdev]\n");
        return 1;
    }

    fd = open(rtc, O_RDONLY);

    if (fd ==  -1) {
        perror(rtc);
        exit(errno);
    }

    fprintf(stderr, "\n\t\t\tRTC Driver Test Example.\n\n");

test_READ:
    /* Read the RTC time/date */
    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (retval == -1) {
        perror("RTC_RD_TIME ioctl");
        goto fail;
    }

    fprintf(stderr, "\n\nCurrent RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
            rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    /* Set the alarm to 5 sec in the future, and check for rollover */
    rtc_tm.tm_sec += 5;
    if (rtc_tm.tm_sec >= 60) {
        rtc_tm.tm_sec %= 60;
        rtc_tm.tm_min++;
    }
    if (rtc_tm.tm_min == 60) {
        rtc_tm.tm_min = 0;
        rtc_tm.tm_hour++;
    }
    if (rtc_tm.tm_hour == 24)
        rtc_tm.tm_hour = 0;

    retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
    if (retval == -1) {
        if (errno == ENOTTY) {
            fprintf(stderr,
                    "\n...Alarm IRQs not supported.\n");
        }
        perror("RTC_ALM_SET ioctl");
        goto fail;
    }

    /* Read the current alarm settings */
    retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
    if (retval == -1) {
        perror("RTC_ALM_READ ioctl");
        goto fail;
    }

    fprintf(stderr, "Alarm time now set to %02d:%02d:%02d.\n",
            rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);

    /* Enable alarm interrupts */
    retval = ioctl(fd, RTC_AIE_ON, 0);
    if (retval == -1) {
        perror("RTC_AIE_ON ioctl");
        goto fail;
    }

    fprintf(stderr, "Waiting 5 seconds for alarm...");
    fflush(stderr);
    /* This blocks until the alarm ring causes an interrupt */
    retval = read(fd, &data, sizeof(unsigned long));
    if (retval == -1) {
        perror("read");
        goto fail;
    }
    irqcount++;
    fprintf(stderr, " okay. Alarm rang.\n");

    /* Disable alarm interrupts */
    retval = ioctl(fd, RTC_AIE_OFF, 0);
    if (retval == -1) {
        perror("RTC_AIE_OFF ioctl");
        goto fail;
    }

done:
    fprintf(stderr, "\n\n\t\t\t *** Test complete ***\n");

    close(fd);

    return 0;
fail:
    fprintf(stderr, "\n\n\t\t\t *** Test fail ***\n");

    close(fd);

    return 0;
}