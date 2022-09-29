/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef __AX_MIPI_API__
#define __AX_MIPI_API__

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    AX_MIPI_TX_DEV_0 = 0,
    AX_MIPI_TX_DEV_1,
    AX_MIPI_TX_DEV_2,
    AX_MIPI_TX_DEV_MAX
} AX_MIPI_TX_DEV_E;

typedef enum {
    AX_MIPI_RX_DEV_0 = 0,
    AX_MIPI_RX_DEV_1,
    AX_MIPI_RX_DEV_2,
    AX_MIPI_RX_DEV_3,
    AX_MIPI_RX_DEV_MAX
} AX_MIPI_RX_DEV_E;

typedef enum {
    AX_MIPI_TX_SRC_SNS_0,
    AX_MIPI_TX_SRC_SNS_1,
    AX_MIPI_TX_SRC_SNS_2,
    AX_MIPI_TX_SRC_RAW_SCALAR_2,
    AX_MIPI_TX_SRC_RAW_SCALAR_3,
    AX_MIPI_TX_SRC_RLTM,
    AX_MIPI_TX_SRC_RLTM_INFO,
} AX_MIPI_TX_SRC_E;

typedef enum {
    AX_MIPI_DATA_RATE_80M = 0,
    AX_MIPI_DATA_RATE_100M,
    AX_MIPI_DATA_RATE_200M,
    AX_MIPI_DATA_RATE_300M,
    AX_MIPI_DATA_RATE_400M,
    AX_MIPI_DATA_RATE_500M,
    AX_MIPI_DATA_RATE_600M,
    AX_MIPI_DATA_RATE_700M,
    AX_MIPI_DATA_RATE_800M,
    AX_MIPI_DATA_RATE_900M,
    AX_MIPI_DATA_RATE_1000M,
    AX_MIPI_DATA_RATE_1100M,
    AX_MIPI_DATA_RATE_1200M,
    AX_MIPI_DATA_RATE_1300M,
    AX_MIPI_DATA_RATE_1400M,
    AX_MIPI_DATA_RATE_1500M,
    AX_MIPI_DATA_RATE_1600M,
    AX_MIPI_DATA_RATE_1700M,
    AX_MIPI_DATA_RATE_1800M,
    AX_MIPI_DATA_RATE_1900M,
    AX_MIPI_DATA_RATE_2000M,
    AX_MIPI_DATA_RATE_2100M,
    AX_MIPI_DATA_RATE_2200M,
    AX_MIPI_DATA_RATE_2300M,
    AX_MIPI_DATA_RATE_2400M,
    AX_MIPI_DATA_RATE_2500M,
} AX_MIPI_DATA_RATE_E;

typedef enum {
    AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3 = 0,
    AX_MIPI_RX_PHY0_SEL_LANE_2_3,
    AX_MIPI_RX_PHY1_SEL_LANE_0_1_2_3,
    AX_MIPI_RX_PHY1_SEL_LANE_2_3,
    AX_MIPI_RX_PHY2_SEL_LANE_0_1_2_3,
    AX_MIPI_RX_PHY2_SEL_LANE_2_3,
} AX_MIPI_RX_PHY_SEL_E;

typedef enum {
    AX_MIPI_LANE_1 = 1,
    AX_MIPI_LANE_2 = 2,
    AX_MIPI_LANE_4 = 4,
} AX_MIPI_LANE_NUM_E;

typedef enum {
    AX_MIPI_VC_0 = 0,
    AX_MIPI_VC_1,
    AX_MIPI_VC_2,
    AX_MIPI_VC_3,
    AX_MIPI_VC_4,
    AX_MIPI_VC_5,
    AX_MIPI_VC_6,
    AX_MIPI_VC_7,
    AX_MIPI_VC_8,
    AX_MIPI_VC_9,
    AX_MIPI_VC_10,
    AX_MIPI_VC_11,
    AX_MIPI_VC_12,
    AX_MIPI_VC_13,
    AX_MIPI_VC_14,
    AX_MIPI_VC_15,
    AX_MIPI_VC_MAX
} AX_MIPI_VC_NUM_E;

typedef enum {
    AX_MIPI_DT_RAW8     = 8,
    AX_MIPI_DT_RAW10    = 10,
    AX_MIPI_DT_RAW12    = 12,
    AX_MIPI_DT_RAW14    = 14,
    AX_MIPI_DT_RAW16    = 16,
    AX_MIPI_DT_YUV420   = 20,
    AX_MIPI_DT_MAX
} AX_MIPI_DT_E;

typedef enum {
    AX_MIPI_DOL_1 = 1,
    AX_MIPI_DOL_2,
    AX_MIPI_DOL_3,
    AX_MIPI_DOL_MAX
} AX_MIPI_DOL_NUM_E;

typedef struct {
    AX_MIPI_LANE_NUM_E              eLaneNum;
    AX_MIPI_DATA_RATE_E             eDataRate;
    AX_MIPI_RX_PHY_SEL_E            ePhySel;
    AX_U8                           nLaneMap[5];
} AX_MIPI_RX_ATTR_S;

typedef struct {
    AX_BOOL                          bIspBypass;
    AX_MIPI_TX_SRC_E                 eInputSrc;
    AX_MIPI_LANE_NUM_E               eLaneNum;                /*only 4*/
    AX_MIPI_DATA_RATE_E              eDataRate;               /*dphy datarate(Mbps)*/
    AX_MIPI_DOL_NUM_E                eDolSplitNum;            /*val(1, 2, 3)*/
    AX_U32                           nImgWidth;
    AX_U32                           nImgHeight;
    AX_MIPI_DT_E                     eImgDataType;            /*val(8, 10, 12, 14)*/
    AX_MIPI_VC_NUM_E                 eImgVC;
    AX_U32                           nNonImgWidth;
    AX_U32                           nNonImgHeight;
    AX_U32                           eNonImgDataType;
    AX_MIPI_DT_E                     eNonImgDT;               /*val(8)*/
    AX_MIPI_VC_NUM_E                 eNonImgVC;
    AX_U8                            nLaneMap[5];
} AX_MIPI_TX_ATTR_S;

AX_S32 AX_MIPI_TX_Init();
AX_S32 AX_MIPI_TX_DeInit();
AX_S32 AX_MIPI_TX_Reset(AX_MIPI_TX_DEV_E eMipiDev);
AX_S32 AX_MIPI_TX_SetAttr(AX_MIPI_TX_DEV_E eMipiDev, AX_MIPI_TX_ATTR_S *pMipiAttr);
AX_S32 AX_MIPI_TX_GetAttr(AX_MIPI_TX_DEV_E eMipiDev, AX_MIPI_TX_ATTR_S *pMipiAttr);
AX_S32 AX_MIPI_TX_Start(AX_MIPI_TX_DEV_E eMipiDev);
AX_S32 AX_MIPI_TX_Stop(AX_MIPI_TX_DEV_E eMipiDev);


AX_S32 AX_MIPI_RX_Init();
AX_S32 AX_MIPI_RX_DeInit();
AX_S32 AX_MIPI_RX_Reset(AX_MIPI_RX_DEV_E eMipiDev);
AX_S32 AX_MIPI_RX_SetAttr(AX_MIPI_RX_DEV_E eMipiDev, AX_MIPI_RX_ATTR_S *pMipiAttr);
AX_S32 AX_MIPI_RX_GetAttr(AX_MIPI_RX_DEV_E eMipiDev, AX_MIPI_RX_ATTR_S *pMipiAttr);
AX_S32 AX_MIPI_RX_Start(AX_MIPI_RX_DEV_E eMipiDev);
AX_S32 AX_MIPI_RX_Stop(AX_MIPI_RX_DEV_E eMipiDev);

#ifdef __cplusplus
}
#endif

#endif // __AX_MIPI_API__
