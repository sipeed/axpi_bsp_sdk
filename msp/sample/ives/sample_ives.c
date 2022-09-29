/**********************************************************************************
 *
 * Copyright (c) 2019-2022 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ax_sys_api.h"
#include "sample_md.h"
#include "sample_od.h"
#include "trace.h"

enum { SAMPLE_TASK_MD = 0, SAMPLE_TASK_OD = 1, SAMPLE_TASK_BUTT };

static AX_VOID ShowUsage(AX_VOID) {
    printf("usage: ./%s [options] ...\n", SAMPLE_IVES_NAME);
    printf("options:\n");
    printf("-t, \t task (unsigned int [=%d])\n", SAMPLE_TASK_MD);
    printf("-h, \t print this message\n");
}

AX_S32 main(AX_S32 argc, AX_CHAR *argv[]) {
    AX_S32 ret = 0;
    AX_S32 c;
    AX_S32 isExit = 0;
    AX_S32 task = SAMPLE_TASK_MD;

#if defined(SAMPLE_IVES_BUILD_VERSION)
    printf("IVES sample: %s build: %s %s\n", SAMPLE_IVES_BUILD_VERSION, __DATE__, __TIME__);
#endif

    while ((c = getopt(argc, argv, "t:h::")) != -1) {
        isExit = 0;
        switch (c) {
            case 't':
                task = atoi(optarg);
                if (task >= SAMPLE_TASK_BUTT) {
                    isExit = 1;
                }
                break;
            case 'h':
                isExit = 1;
                break;
            default:
                isExit = 1;
                break;
        }
    }

    if (isExit) {
        ShowUsage();
        exit(0);
    }

    ret = AX_SYS_Init();
    if (0 != ret) {
        ALOGE("AX_SYS_Init() fail, ret = 0x%x", ret);
        return -1;
    }

    ret = AX_IVES_Init();
    if (0 != ret) {
        ALOGE("AX_IVES_Init() fail, ret = 0x%x", ret);
        goto EXIT0;
    }

    switch (task) {
        case SAMPLE_TASK_MD:
            ret = SAMPLE_IVES_MD_ENTRY();
            break;
        case SAMPLE_TASK_OD:
            ret = SAMPLE_IVES_OD_ENTRY();
            break;
        default:
            break;
    }

    if (0 != ret) {
        goto EXIT1;
    }

EXIT1:
    AX_IVES_DeInit();
EXIT0:
    AX_SYS_Deinit();
    return (0 != ret) ? -1 : 0;
}
