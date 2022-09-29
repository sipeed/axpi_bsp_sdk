/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_VIN_SLAVE_H_
#define _SAMPLE_VIN_SLAVE_H_

#define COMM_ISP_PRT(fmt...)   \
do {\
    printf("[SAMPLE_VIN_SLAVE][%s][%d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)

#endif
