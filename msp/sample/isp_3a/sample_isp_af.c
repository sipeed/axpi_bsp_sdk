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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "ax_isp_api.h"
#include "ax_mipi_api.h"
#include "ax_sys_api.h"
#include "sample_isp_af.h"
#include <pthread.h>
#include <assert.h>
#include "ax_isp_3a_api.h"
#include "common_cam.h"
#include "common_type.h"

extern CAMERA_T gCams[MAX_CAMERAS];
static AX_ISP_IQ_AF_STAT_PARAM_T af_param_sdr = {
    /* nAfEnable */
    1,
    /* tAfBayer2Y */
    {
        /* nYSel */
        1,
        /* nGrgbSel */
        1,
        /* nCoeffR */
        30,
        /* nCoeffG */
        2726,
        /* nCoeffB */
        1339,
    },
    /* tAfGamma */
    {
        /* nGammaEnable */
        1,
        /* nGammaLut[33] */
        {
            0, 3390, 4646, 5586, 6366, 7046, 7655, 8211, 8724, 9204, 9656, 10083, 10490, 10879, 11251, 11610, 11956, 12290, 12613, 12927, 13232, 13529, 13818, 14100, 14375, 14644, 14908, 15166, 15419, 15667, 15910, 16149,  /* 0 - 31*/
            16383, /*32 - 32*/
        },
    },
    /* tAfScaler */
    {
        /* nScaleEnable */
        1,
        /* nScaleFactor */
        1,
        /* nWeightLut[16] */
        {2048, 2047, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 15*/},
    },
    /* tAfFilter */
    {
        /* nFirEnable */
        0,
        /* nViirRefId */
        0,
        /* nH1IirRefId */
        0,
        /* nH2IirRefId */
        0,
    },
    /* tAfCoring */
    {
        /* nCoringThr */
        0,
        /* nCoringGain */
        2176,
        /* nCoringLut[16] */
        {1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 5, 7, 9, 13, 14, /*0 - 15*/},
    },
    /* tAfRoi */
    {
        /* nRoiOffsetH */
        104,
        /* nRoiOffsetV */
        64,
        /* nRoiRegionNumH */
        1,
        /* nRoiRegionNumV */
        1,
        /* nRoiRegionW */
        512,
        /* nRoiRegionH */
        512,
    },
};
static AX_ISP_IQ_AF_IIR_REF_LIST_T af_iir_ref_list_param =
{
    /* nViirRefNum */
    10,
    /* nH1IirRefNum */
    10,
    /* nH2IirRefNum */
    10,
    /* tVIirRefList[32] */
    {
        /* 0 */
        {
            /* nStartFreq */
            104858,
            /* nEndFreq */
            314573,
            /* nIirCoefList[10] */
            {2147, 4295, 2147, 12251, -4459, 13117, -26234, 13117, 25575, -10508, /*0 - 9*/},
        },
        /* 1 */
        {
            /* nStartFreq */
            314573,
            /* nEndFreq */
            524288,
            /* nIirCoefList[10] */
            {4798, 9597, 4798, 0, -2811, 8273, -16547, 8273, 12251, -4459, /*0 - 9*/},
        },
        /* 2 */
        {
            /* nStartFreq */
            524288,
            /* nEndFreq */
            734003,
            /* nIirCoefList[10] */
            {8273, 16547, 8273, -12251, -4459, 4798, -9597, 4798, 0, -2811, /*0 - 9*/},
        },
        /* 3 */
        {
            /* nStartFreq */
            41943,
            /* nEndFreq */
            104858,
            /* nIirCoefList[10] */
            {329, 658, 329, 25575, -10508, 14991, -29981, 14991, 29863, -13716, /*0 - 9*/},
        },
        /* 4 */
        {
            /* nStartFreq */
            104858,
            /* nEndFreq */
            209715,
            /* nIirCoefList[10] */
            {1105, 2210, 1105, 18726, -6763, 13117, -26234, 13117, 25575, -10508, /*0 - 9*/},
        },
        /* 5 */
        {
            /* nStartFreq */
            209715,
            /* nEndFreq */
            314573,
            /* nIirCoefList[10] */
            {2147, 4295, 2147, 12251, -4459, 10467, -20937, 10467, 18726, -6763, /*0 - 9*/},
        },
        /* 6 */
        {
            /* nStartFreq */
            314573,
            /* nEndFreq */
            419430,
            /* nIirCoefList[10] */
            {3384, 6768, 3384, 6053, -3207, 8273, -16547, 8273, 12251, -4459, /*0 - 9*/},
        },
        /* 7 */
        {
            /* nStartFreq */
            419430,
            /* nEndFreq */
            524288,
            /* nIirCoefList[10] */
            {4798, 9597, 4798, 0, -2811, 6411, -12823, 6411, 6053, -3207, /*0 - 9*/},
        },
        /* 8 */
        {
            /* nStartFreq */
            524288,
            /* nEndFreq */
            629146,
            /* nIirCoefList[10] */
            {6411, 12823, 6411, -6053, -3207, 4798, -9597, 4798, 0, -2811, /*0 - 9*/},
        },
        /* 9 */
        {
            /* nStartFreq */
            41943,
            /* nEndFreq */
            209715,
            /* nIirCoefList[10] */
            {1105, 2210, 1105, 18726, -6763, 14991, -29981, 14991, 29863, -13716, /*0 - 9*/},
        },
        /* 10 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 11 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 12 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 13 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 14 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 15 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 16 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 17 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 18 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 19 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 20 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 21 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 22 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 23 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 24 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 25 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 26 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 27 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 28 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 29 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 30 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 31 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
    },
    /* tH1IirRefList[32] */
    {
        /* 0 */
        {
            /* nStartFreq */
            104858,
            /* nEndFreq */
            314573,
            /* nIirCoefList[10] */
            {2147, 4295, 2147, 12251, -4459, 13117, -26234, 13117, 25575, -10508, /*0 - 9*/},
        },
        /* 1 */
        {
            /* nStartFreq */
            314573,
            /* nEndFreq */
            524288,
            /* nIirCoefList[10] */
            {4798, 9597, 4798, 0, -2811, 8273, -16547, 8273, 12251, -4459, /*0 - 9*/},
        },
        /* 2 */
        {
            /* nStartFreq */
            524288,
            /* nEndFreq */
            734003,
            /* nIirCoefList[10] */
            {8273, 16547, 8273, -12251, -4459, 4798, -9597, 4798, 0, -2811, /*0 - 9*/},
        },
        /* 3 */
        {
            /* nStartFreq */
            41943,
            /* nEndFreq */
            104858,
            /* nIirCoefList[10] */
            {329, 658, 329, 25575, -10508, 14991, -29981, 14991, 29863, -13716, /*0 - 9*/},
        },
        /* 4 */
        {
            /* nStartFreq */
            104858,
            /* nEndFreq */
            209715,
            /* nIirCoefList[10] */
            {1105, 2210, 1105, 18726, -6763, 13117, -26234, 13117, 25575, -10508, /*0 - 9*/},
        },
        /* 5 */
        {
            /* nStartFreq */
            209715,
            /* nEndFreq */
            314573,
            /* nIirCoefList[10] */
            {2147, 4295, 2147, 12251, -4459, 10467, -20937, 10467, 18726, -6763, /*0 - 9*/},
        },
        /* 6 */
        {
            /* nStartFreq */
            314573,
            /* nEndFreq */
            419430,
            /* nIirCoefList[10] */
            {3384, 6768, 3384, 6053, -3207, 8273, -16547, 8273, 12251, -4459, /*0 - 9*/},
        },
        /* 7 */
        {
            /* nStartFreq */
            419430,
            /* nEndFreq */
            524288,
            /* nIirCoefList[10] */
            {4798, 9597, 4798, 0, -2811, 6411, -12823, 6411, 6053, -3207, /*0 - 9*/},
        },
        /* 8 */
        {
            /* nStartFreq */
            524288,
            /* nEndFreq */
            629146,
            /* nIirCoefList[10] */
            {6411, 12823, 6411, -6053, -3207, 4798, -9597, 4798, 0, -2811, /*0 - 9*/},
        },
        /* 9 */
        {
            /* nStartFreq */
            41943,
            /* nEndFreq */
            209715,
            /* nIirCoefList[10] */
            {1105, 2210, 1105, 18726, -6763, 14991, -29981, 14991, 29863, -13716, /*0 - 9*/},
        },
        /* 10 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 11 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 12 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 13 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 14 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 15 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 16 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 17 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 18 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 19 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 20 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 21 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 22 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 23 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 24 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 25 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 26 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 27 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 28 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 29 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 30 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 31 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
    },
    /* tH2IirRefList[32] */
    {
        /* 0 */
        {
            /* nStartFreq */
            104858,
            /* nEndFreq */
            314573,
            /* nIirCoefList[10] */
            {2147, 4295, 2147, 12251, -4459, 13117, -26234, 13117, 25575, -10508, /*0 - 9*/},
        },
        /* 1 */
        {
            /* nStartFreq */
            314573,
            /* nEndFreq */
            524288,
            /* nIirCoefList[10] */
            {4798, 9597, 4798, 0, -2811, 8273, -16547, 8273, 12251, -4459, /*0 - 9*/},
        },
        /* 2 */
        {
            /* nStartFreq */
            524288,
            /* nEndFreq */
            734003,
            /* nIirCoefList[10] */
            {8273, 16547, 8273, -12251, -4459, 4798, -9597, 4798, 0, -2811, /*0 - 9*/},
        },
        /* 3 */
        {
            /* nStartFreq */
            41943,
            /* nEndFreq */
            104858,
            /* nIirCoefList[10] */
            {329, 658, 329, 25575, -10508, 14991, -29981, 14991, 29863, -13716, /*0 - 9*/},
        },
        /* 4 */
        {
            /* nStartFreq */
            104858,
            /* nEndFreq */
            209715,
            /* nIirCoefList[10] */
            {1105, 2210, 1105, 18726, -6763, 13117, -26234, 13117, 25575, -10508, /*0 - 9*/},
        },
        /* 5 */
        {
            /* nStartFreq */
            209715,
            /* nEndFreq */
            314573,
            /* nIirCoefList[10] */
            {2147, 4295, 2147, 12251, -4459, 10467, -20937, 10467, 18726, -6763, /*0 - 9*/},
        },
        /* 6 */
        {
            /* nStartFreq */
            314573,
            /* nEndFreq */
            419430,
            /* nIirCoefList[10] */
            {3384, 6768, 3384, 6053, -3207, 8273, -16547, 8273, 12251, -4459, /*0 - 9*/},
        },
        /* 7 */
        {
            /* nStartFreq */
            419430,
            /* nEndFreq */
            524288,
            /* nIirCoefList[10] */
            {4798, 9597, 4798, 0, -2811, 6411, -12823, 6411, 6053, -3207, /*0 - 9*/},
        },
        /* 8 */
        {
            /* nStartFreq */
            524288,
            /* nEndFreq */
            629146,
            /* nIirCoefList[10] */
            {6411, 12823, 6411, -6053, -3207, 4798, -9597, 4798, 0, -2811, /*0 - 9*/},
        },
        /* 9 */
        {
            /* nStartFreq */
            41943,
            /* nEndFreq */
            209715,
            /* nIirCoefList[10] */
            {1105, 2210, 1105, 18726, -6763, 14991, -29981, 14991, 29863, -13716, /*0 - 9*/},
        },
        /* 10 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 11 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 12 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 13 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 14 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 15 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 16 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 17 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 18 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 19 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 20 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 21 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 22 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 23 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 24 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 25 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 26 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 27 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 28 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 29 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 30 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
        /* 31 */
        {
            /* nStartFreq */
            0,
            /* nEndFreq */
            0,
            /* nIirCoefList[10] */
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*0 - 9*/},
        },
    },
};


#define DEF_AF_GAMMA_LUT_NUM    (33)
#define DEF_AF_WEIGHT_LUT_NUM   (16)
#define DEF_AF_VIIR_LUT_NUM     (10)
#define DEF_AF_H1IIR_LUT_NUM    (10)
#define DEF_AF_H2IIR_LUT_NUM    (10)
#define DEF_AF_H1FIR_LUT_NUM    (13)
#define DEF_AF_H2FIR_LUT_NUM    (13)
#define DEF_AF_CORING_LUT_NUM   (16)
#define DEF_AF_IIR_REF_LIST_SIZE   (32)
#define DEF_AF_IIR_COEF_NUM     (10)
extern AX_S32 g_isp_force_loop_exit;
AX_S32 sample_af_init_afparams
(
    AX_U8                         pipe,
    AX_ISP_IQ_AF_STAT_PARAM_T          *pAfInitPara
)
{
    pAfInitPara->nAfEnable = af_param_sdr.nAfEnable;

    pAfInitPara->tAfBayer2Y.nYSel = af_param_sdr.tAfBayer2Y.nYSel;
    pAfInitPara->tAfBayer2Y.nGrgbSel = af_param_sdr.tAfBayer2Y.nGrgbSel;
    pAfInitPara->tAfBayer2Y.nCoeffR = af_param_sdr.tAfBayer2Y.nCoeffR;
    pAfInitPara->tAfBayer2Y.nCoeffG = af_param_sdr.tAfBayer2Y.nCoeffG;
    pAfInitPara->tAfBayer2Y.nCoeffB = af_param_sdr.tAfBayer2Y.nCoeffB;
    pAfInitPara->tAfGamma.nGammaEnable = af_param_sdr.tAfGamma.nGammaEnable;

    for (int i = 0; i < DEF_AF_GAMMA_LUT_NUM; i++)
    {
        pAfInitPara->tAfGamma.nGammaLut[i] = af_param_sdr.tAfGamma.nGammaLut[i];
    }

    pAfInitPara->tAfScaler.nScaleEnable = af_param_sdr.tAfScaler.nScaleEnable;
    pAfInitPara->tAfScaler.nScaleFactor = af_param_sdr.tAfScaler.nScaleFactor;
    for (int i = 0; i < DEF_AF_WEIGHT_LUT_NUM; i++)
    {
        pAfInitPara->tAfScaler.nWeightLut[i] = af_param_sdr.tAfScaler.nWeightLut[i];
    }

    pAfInitPara->tAfFilter.nFirEnable = af_param_sdr.tAfFilter.nFirEnable;
    pAfInitPara->tAfFilter.nViirRefId = af_param_sdr.tAfFilter.nViirRefId;
    pAfInitPara->tAfFilter.nH1IirRefId = af_param_sdr.tAfFilter.nH1IirRefId;
    pAfInitPara->tAfFilter.nH2IirRefId = af_param_sdr.tAfFilter.nH2IirRefId;

    pAfInitPara->tAfCoring.nCoringThr = af_param_sdr.tAfCoring.nCoringThr;
    pAfInitPara->tAfCoring.nCoringGain = af_param_sdr.tAfCoring.nCoringGain;

    for (int i = 0; i < DEF_AF_CORING_LUT_NUM; ++i)
    {
        pAfInitPara->tAfCoring.nCoringLut[i] = af_param_sdr.tAfCoring.nCoringLut[i];
    }

    pAfInitPara->tAfRoi.nRoiOffsetH = af_param_sdr.tAfRoi.nRoiOffsetH;
    pAfInitPara->tAfRoi.nRoiOffsetV = af_param_sdr.tAfRoi.nRoiOffsetV;
    pAfInitPara->tAfRoi.nRoiRegionNumH = af_param_sdr.tAfRoi.nRoiRegionNumH;
    pAfInitPara->tAfRoi.nRoiRegionNumV = af_param_sdr.tAfRoi.nRoiRegionNumV;
    pAfInitPara->tAfRoi.nRoiRegionW = af_param_sdr.tAfRoi.nRoiRegionW;
    pAfInitPara->tAfRoi.nRoiRegionH = af_param_sdr.tAfRoi.nRoiRegionH;

    return 0;

}


AX_S32 sample_af_init_afiirref
(
    AX_U8                         pipe,
    AX_ISP_IQ_AF_IIR_REF_LIST_T *pAfRefListPara
)
{
    AX_S32 axRet;
    memcpy(pAfRefListPara, &af_iir_ref_list_param, sizeof(AX_ISP_IQ_AF_IIR_REF_LIST_T));

    axRet = AX_ISP_IQ_SetAFIirRefList(pipe, pAfRefListPara);
    if (0 != axRet)
    {
        printf("AX_ISP_GetAfAttr failed!\n");
        return -1;
    }
    return 0;
}

AX_S32 sample_af_init
(
    AX_U8                         pipe,
    AX_ISP_IQ_AF_STAT_PARAM_T     *pAfInitPara,
    AX_ISP_IQ_AF_IIR_REF_LIST_T   *pAfRefListPara
)
{
    AX_S32 axRet;

    axRet = sample_af_init_afiirref(pipe, pAfRefListPara);
    if (0 != axRet)
    {
        printf("sample_af_init_afiirref failed!\n");
        return -1;
    }
    axRet = sample_af_init_afparams(pipe, pAfInitPara);
    if (0 != axRet)
    {
        printf("sample_af_init_afparams failed!\n");
        return -1;
    }

    axRet = AX_ISP_IQ_SetAfStatParam(pipe, pAfInitPara);
    if (0 != axRet)
    {
        printf("AX_ISP_IQ_SetAfStatParam failed!\n");
        return -1;
    }
    printf("sample_af_init end\n");

    return 0;
}

AX_S32 sample_af_stats
(
    AX_U8                         pipe,
    AX_ISP_AF_STAT_INFO_T *pAfStats,
    AX_ISP_IQ_AF_STAT_PARAM_T          *pAfParams,
    AfRes                         *Res
)
{
    AX_S32 axRet = 0;
    AX_U64 regions = 0;
    AX_U64 V_v, H1_v, H2_v, V_y, H1_y, H2_y, V_p, H1_p, H2_p;
    regions = pAfParams->tAfRoi.nRoiRegionNumH * pAfParams->tAfRoi.nRoiRegionNumV;
    V_v = 0;
    H1_v = 0;
    H2_v = 0;
    V_y = 0;
    H1_y = 0;
    H2_y = 0;
    V_p = 0;
    H1_p = 0;
    H2_p = 0;

    axRet = AX_ISP_IQ_GetAFStatistics(gCams[pipe].nPipeId, pAfStats);
    if (0 != axRet)
    {
        COMM_ISP_PRT("get pipe: 0 statistics failed!\n");
        return -1;
    }
    for (int k = 0; k < regions; k++) {
        V_p += pAfStats->tAfStats[0].tAfRoiV[k].nPixCount;
        V_y  +=  pAfStats->tAfStats[0].tAfRoiV[k].nPixSum;
        V_v += pAfStats->tAfStats[0].tAfRoiV[k].nSharpness;

        H1_p += pAfStats->tAfStats[0].tAfRoiH1[k].nPixCount;
        H1_y  +=  pAfStats->tAfStats[0].tAfRoiH1[k].nPixSum;
        H1_v += pAfStats->tAfStats[0].tAfRoiH1[k].nSharpness;

        H2_p += pAfStats->tAfStats[0].tAfRoiH2[k].nPixCount;
        H2_y  +=  pAfStats->tAfStats[0].tAfRoiH2[k].nPixSum;
        H2_v += pAfStats->tAfStats[0].tAfRoiH2[k].nSharpness;
    }

    // COMM_ISP_PRT("V_p:%llu, V_y:%llu, V_v:%llu\n", V_p, V_y, V_v);
    // COMM_ISP_PRT("H1_p:%llu, H1_y:%llu, H1_v:%llu\n", H1_p, H1_y, H1_v);
    // COMM_ISP_PRT("H2_p:%llu, H2_y:%llu, H2_v:%llu\n", H2_p, H2_y, H2_v);

    Res->V = (V_v + H1_v + H2_v)/(3*regions);
    if (0 == V_p || 0 == H1_p || 0 == H2_p) {
        Res->Y = 1;
    } else {
        Res->Y = (V_y/V_p + H1_y/H1_p + H2_y/H2_p)/3;
    }

    Res->Y>>=3;

    return 0;
}


AX_S32 sample_af_deinit(AX_U8 pipe)
{
    return 0;
}

void *AfRun(void *args)
{
    AX_U32 i = (AX_U32)args;
    AX_ISP_AF_STAT_INFO_T afStat = {0};
    AX_S32 axRet = 0;
    AX_ISP_IQ_AF_STAT_PARAM_T tParams;
    AX_ISP_IQ_AF_IIR_REF_LIST_T tAfRefListPara;
    AfRes Res;
    AX_U8 error_flag = 0;

    AX_U32 count =0;
    if (gCams[i].bOpen) {
        axRet = sample_af_init(gCams[i].nPipeId, &tParams, &tAfRefListPara);
        if (0 != axRet)
        {
            printf("Init AF Failed, ret = %d\n", axRet);
            return NULL;
        }
    }
    COMM_ISP_PRT("3A %d is running...\n", i);

    while (!g_isp_force_loop_exit) {
        if (gCams[i].bOpen) {
            axRet = AX_ISP_GetIRQTimeOut(gCams[i].nPipeId, 0, 200);
            if(error_flag == 0) {
                if (axRet == 0) {
                    axRet = sample_af_stats(gCams[i].nPipeId, &afStat, &tParams, &Res);
                    if (0 == axRet) {
                        count ++;
                        if (count % 16 == 0)
                        {
                            COMM_ISP_PRT("AF:  Y: %llu   FV: %llu\n", Res.Y, Res.V);
                            count = 0;
                        }
                    } else {
                        COMM_ISP_PRT("sample_af_stats failed\n");
                        error_flag = 1;
                    }
                }
                else {
                    COMM_ISP_PRT(" isp wait frame done timeout, ret=0x%x.\n", axRet);
                }
            }
        }

    }
    return NULL;
}