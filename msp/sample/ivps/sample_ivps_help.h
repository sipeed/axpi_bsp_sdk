/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _SAMPLE_IVPS_HELP_6AC1E5D7_A840_459E_BB6C_80CB92424A8E_H_
#define _SAMPLE_IVPS_HELP_6AC1E5D7_A840_459E_BB6C_80CB92424A8E_H_

#include "ax_ivps_api.h"
enum
{
    SAMPLE_HELP_MIN = -1,
    SAMPLE_HELP_USERMODE = 0,
    SAMPLE_HELP_LINKMODE = 1,
    SAMPLE_HELP_REGION = 2,
    SAMPLE_HELP_MAX
};

#define SAMPLE_NAME "opt/bin/sample_ivps"

AX_VOID ShowUsage(AX_S32 nHelpIdx);

#endif /* _SAMPLE_IVPS_HELP_6AC1E5D7_A840_459E_BB6C_80CB92424A8E_H_ */
