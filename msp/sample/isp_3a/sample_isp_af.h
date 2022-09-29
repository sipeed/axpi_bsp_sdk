/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _SAMPLE_ISP_AF_H_
#define _SAMPLE_ISP_AF_H_

typedef struct {
    AX_U64 V;
    AX_U64 Y;
} AfRes;
AX_S32 sample_af_init_afparams
(
    AX_U8                         pipe,
    AX_ISP_IQ_AF_STAT_PARAM_T     *pAfInitPara
);
AX_S32 sample_af_init_afiirref
(
    AX_U8                         pipe,
    AX_ISP_IQ_AF_IIR_REF_LIST_T *pAfRefListPara
);
AX_S32 sample_af_init
(
    AX_U8                         pipe,
    AX_ISP_IQ_AF_STAT_PARAM_T     *pAfInitPara,
    AX_ISP_IQ_AF_IIR_REF_LIST_T   *pAfRefListPara
);
AX_S32 sample_af_stats
(
    AX_U8                         pipe,
    AX_ISP_AF_STAT_INFO_T *pAfStats,
    AX_ISP_IQ_AF_STAT_PARAM_T          *pAfParams,
    AfRes                         *Res
);
AX_S32 sample_af_run
(
    AX_U8                         pipe,
    AX_ISP_AF_STAT_INFO_T *pAfStats,
    AX_ISP_IQ_AF_STAT_PARAM_T          *pAfParams,
    AfRes                         *Res
);
AX_S32 sample_af_deinit(AX_U8 pipe);
void *AfRun(void *args);
void *Isp3ARun(void *args);
#endif