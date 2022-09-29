/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _AX_SENSOR_STRUCT_H_
#define _AX_SENSOR_STRUCT_H_

#include "ax_base_type.h"
#include "ax_isp_common.h"
#include "ax_isp_iq_api.h"
#include "ax_isp_3a_struct.h"

typedef AX_S32 SENSOR_DEV;
typedef AX_U8  ISP_PIPE_ID;

#define HDR_MAX_FRAME_NUM               (4)
#define ISP_MAX_SNS_REGISTER_NUM        (32)
#define SNS_MAX_FRAME_RATE              (30)

typedef enum _AX_SNS_ADGAIN_MODE_E_ {
    AX_AGAIN_ONLY           = 0,
    AX_ADGAIN_SEPARATION    = 1,
    AX_ADGAIN_COMBINED      = 2,
} AX_SNS_ADGAIN_MODE_E;

typedef enum _AX_SNS_HCGLCG_MODE_E_ {
    AX_HCG_MODE             = 0,
    AX_LCG_MODE             = 1,
    AX_LCG_NOTSUPPORT_MODE  = 2,
} AX_SNS_HCGLCG_MODE_E;

typedef enum _AX_SNS_MASTER_SLAVE_E_ {
    AX_SNS_MASTER   = 0,
    AX_SNS_SLAVE    = 1,
} AX_SNS_MASTER_SLAVE_E;

typedef struct _AX_SNS_CAP_T_ {
    AX_U32 nSnsMode_caps;
    AX_U32 nSnsRawType_caps;
    AX_U32 nSnsFps_caps;
    AX_U32 nSnsResolution_caps;
} AX_SNS_CAP_T;

typedef enum _AX_SNS_CLK_RATE_E_ {
    AX_SNS_CLK_NOT_CFG  = -1,    /* not config, The user completes the configuration himself */
    AX_SNS_CLK_24M      = 24000000,     /* 24MHz, Default value of chip configuration */
    AX_SNS_CLK_27M      = 27000000,     /* 27MHz */
    AX_SNS_CLK_37_125M  = 37125000,     /* 37.125MHz */
    AX_SNS_CLK_74_25M   = 74250000,     /* 74.25MHz */
} AX_SNS_CLK_RATE_E;

typedef enum _AX_SNS_MIRRORFLIP_TYPE_E_{
    AX_SNS_MF_NORMAL      = 0,
    AX_SNS_MF_MIRROR      = 1,
    AX_SNS_MF_FLIP        = 2,
    AX_SNS_MF_MIRROR_FLIP = 3,
    AX_SNS_MF_BUTT
} AX_SNS_MIRRORFLIP_TYPE_E;

typedef struct _AX_SNS_ATTR_T_ {
    AX_U32                  nWidth;
    AX_U32                  nHeight;
    AX_U32                  nFrameRate;             /* 25/30/50/60fps */
    AX_SNS_HDR_MODE_E       eSnsMode;
    AX_RAW_TYPE_E           eRawType;               /* supported raw type  AX_RAW_TYPE_E*/
    AX_SNS_HCGLCG_MODE_E    eSnsHcgLcg;             /* supported hcg/lcg mode  AX_SNS_HCGLCG_MODE_E*/
    AX_BAYER_PATTERN_E      eBayerPattern;
    AX_BOOL                 bTestPatternEnable;     /* Active Sensor Test Pattern */
    AX_SNS_MASTER_SLAVE_E   eMasterSlaveSel;
} AX_SNS_ATTR_T;

typedef struct _AX_SNS_AE_GAIN_TABLE_T_ {
    AX_S32  nAgainTableSize;
    AX_F32 *pAgainTable;
    AX_S32  nDgainTableSize;
    AX_F32 *pDgainTable;
} AX_SNS_AE_GAIN_TABLE_T;

typedef struct _AX_SNS_AE_SLOW_SHUTTER_TBL_T_ {
    AX_U32      nMaxIntTime;        /* unit : us */
    AX_F32      fRealFps;
} AX_SNS_AE_SLOW_SHUTTER_TBL_T;

typedef struct _AX_SNS_AE_SLOW_SHUTTER_PARAM_T_ {
    AX_SNS_AE_SLOW_SHUTTER_TBL_T    tSlowShutterTbl[SNS_MAX_FRAME_RATE];
    AX_F32                          fMaxFps;
    AX_F32                          fMinFps;
    AX_S32                          nGroupNum;
} AX_SNS_AE_SLOW_SHUTTER_PARAM_T;

typedef struct _AX_SNS_AE_LIMIT_T_ {
    AX_F32 nMingain;
    AX_F32 nMaxgain;
    AX_F32 nMinAgain[HDR_MAX_FRAME_NUM];    /* minimum:0-long analog gain, 1- medium analog gain, 2- short analog gain, 3-*/
    AX_F32 nMaxAgain[HDR_MAX_FRAME_NUM];    /* max mum:0-long analog gain, 1- medium analog gain, 2- short analog gain, 3-*/
    AX_F32 nMinDgain[HDR_MAX_FRAME_NUM];    /* minimum:0-long digital gain, 1- medium digital gain, 2- short digital gain, 3-*/
    AX_F32 nMaxDgain[HDR_MAX_FRAME_NUM];    /* max mum:0-long digital gain, 1- medium digital gain, 2- short digital gain, 3-*/
    AX_F32 nMinratio;
    AX_F32 nMaxratio;
    AX_U32 nInitMinIntegrationTime;                    /* Initial value ; unit-us */
    AX_U32 nInitMaxIntegrationTime;                    /* Initial value ; unit-us */
    AX_U32 nMinIntegrationTime[HDR_MAX_FRAME_NUM];     /* minimum:0-long frame IntegrationTime, 1- medium frame IntegrationTime, 2- short frame IntegrationTime, 3- ; unit-us*/
    AX_U32 nMaxIntegrationTime[HDR_MAX_FRAME_NUM];     /* max mum:0-long frame IntegrationTime, 1- medium frame IntegrationTime, 2- short frame IntegrationTime, 3- ; unit-us*/
} AX_SNS_AE_LIMIT_T;

typedef struct _AX_SNS_AE_PARAM_T_ {
    AX_U32 nGainMode;                                   /* AX_SNS_ADGAIN_MODE_E */

    /* current ae param */
    AX_F32 nCurAGain[HDR_MAX_FRAME_NUM];                /* current analog gain: 0-long frame, 1- medium frame, 2- short frame, 3- */
    AX_F32 nCurDGain[HDR_MAX_FRAME_NUM];                /* current digital gain: 0-long frame, 1- medium frame, 2- short frame, 3- */
    AX_F32 nCurIspDGain[HDR_MAX_FRAME_NUM];             /* current isp digital gain: 0-long frame, 1- medium frame, 2- short frame, 3- */
    AX_U32 nCurIntegrationTime[HDR_MAX_FRAME_NUM];      /* current Integration time: 0-long frame , 1- medium frame, 2- short frame, 3- ; unit-us */

    /* ae step param */
    AX_F32 nAGainIncrement[HDR_MAX_FRAME_NUM];          /* analog gain step: 0-long frame, 1- medium frame, 2- short frame, 3- */
    AX_F32 nDGainIncrement[HDR_MAX_FRAME_NUM];          /* digital gain step: 0-long frame, 1- medium frame, 2- short frame, 3- */
    AX_F32 nIspDGainIncrement[HDR_MAX_FRAME_NUM];       /* isp digital gain step: 0-long frame, 1- medium frame, 2- short frame, 3- */
    AX_U32 nIntegrationTimeIncrement[HDR_MAX_FRAME_NUM];  /* Integration time step: 0-long frame, 1- medium frame, 2- short frame, 3- ; unit-us */

    AX_F32 nCurFps;
    AX_F32 nSnsHcgLcgRatio;
    AX_F32 nHcgLcgRatio;
} AX_SNS_AE_PARAM_T;

typedef struct _AX_SNS_AE_GAIN_CFG_T_ {
    /* gain: 0-long frame, 1- medium frame, 2- short frame, 3-very short frame */
    AX_F32 nGain[HDR_MAX_FRAME_NUM];

    /* ratio: index=0: longFrame/mediumFrame, index=1: mediumFrame/shortFrame, index=2: shortFrame/veryShortFrame, index=3:Not used */
    AX_F32 nHdrRatio[HDR_MAX_FRAME_NUM];
} AX_SNS_AE_GAIN_CFG_T;

typedef struct _AX_SNS_AE_SHUTTER_CFG_T_ {
    /* integration time: 0-long frame, 1- medium frame, 2- short frame, 3-very short frame ; unit-us */
    AX_U32 nIntTime[HDR_MAX_FRAME_NUM];

    /* integration time ratio: index=0: longFrame/mediumFrame, index=1: mediumFrame/shortFrame, index=2: shortFrame/veryShortFrame, index=3:Not used */
    AX_F32 nHdrRatio[HDR_MAX_FRAME_NUM];
} AX_SNS_AE_SHUTTER_CFG_T;

typedef struct _AX_SNS_PARAMS_T_ {
    AX_SNS_ATTR_T           sns_dev_attr;
    AX_SNS_AE_LIMIT_T       sns_ae_limit;
    AX_SNS_AE_PARAM_T       sns_ae_param;
} AX_SNS_PARAMS_T;

typedef struct _AX_SNS_ATTR_INFO_T_ {
    SENSOR_DEV            nSensorId;
} AX_SNS_ATTR_INFO_T;

typedef struct _AX_SNS_BLACK_LEVEL_T_ {
    AX_U16  nBlackLevel[AX_ISP_BAYER_CHN_NUM];
} AX_SNS_BLACK_LEVEL_T;

typedef union _AX_SNS_COMMBUS_T_ {
    AX_S8   I2cDev;
    struct {
        AX_S8  bit4SpiDev       : 4;
        AX_S8  bit4SpiCs        : 4;
    } SpiDev;
} AX_SNS_COMMBUS_T;

/*
IIC/SPI connection is used between sensor and ISP
*/
typedef enum _AX_SNS_CONNECT_TYPE_E_ {
    ISP_SNS_CONNECT_I2C_TYPE = 0,
    ISP_SNS_CONNECT_SPI_TYPE,
    ISP_SNS_CONNECT_TYPE_BUTT,
} AX_SNS_CONNECT_TYPE_E;

typedef struct _AX_SNS_ISP_I2C_DATA_T_ {
    AX_BOOL bUpdate;            /* AX_TRUE: The sensor registers are written, AX_FALSE: The sensor registers are not written*/
    AX_U8   nDelayFrmNum;       /* Number of frames for register delay configuration */
    AX_U8   nDevAddr;           /* sensor device address */
    AX_U8   reserve[2];
    AX_U32  nRegAddr;           /* register address */
    AX_U32  nAddrByteNum;       /* Bit width of the register address */
    AX_U32  nData;              /* Sensor register data */
    AX_U32  nDataByteNum;       /* Bit width of sensor register data */
} AX_SNS_ISP_I2C_DATA_T;

typedef struct _AX_SNS_ISP_EXP_INFO_T_ {
    AX_U32      szExpTime[HDR_MAX_FRAME_NUM];   /* unit:us */
    AX_F32      szCurAGain[HDR_MAX_FRAME_NUM];
    AX_F32      szCurDGain[HDR_MAX_FRAME_NUM];
    AX_U32      szLineGap[HDR_MAX_FRAME_NUM];   /* index0: long frame and medium frame line gap, exposure in unit of rows */
} AX_SNS_ISP_EXP_INFO_T;

typedef struct _AX_SNS_REGS_CFG_TABLE_T_ {
    AX_BOOL                 bConfig;
    AX_SNS_CONNECT_TYPE_E   eSnsType;
    /*Number of registers requiring timing control. The member value cannot be dynamically changed*/
    AX_U32                  nRegNum;
    AX_SNS_COMMBUS_T        tComBus;
    AX_SNS_ISP_I2C_DATA_T   sztI2cData[ISP_MAX_SNS_REGISTER_NUM];
    AX_SNS_ISP_EXP_INFO_T   tSnsExpInfo;
} AX_SNS_REGS_CFG_TABLE_T;

typedef struct _AX_SENSOR_DEFAULT_PARAM_T_ {
    AX_ISP_IQ_DPC_PARAM_T       *ptDpc;
    AX_ISP_IQ_BLC_PARAM_T       *ptBlc;
    AX_ISP_IQ_DS_PARAM_T        *ptDarkshading;
    AX_ISP_IQ_FPN_PARAM_T       *ptFpn;
    AX_ISP_IQ_GBL_PARAM_T       *ptGbl;

    AX_ISP_IQ_NPU_PARAM_T       *ptNpu;
    AX_ISP_IQ_WNR_PARAM_T       *ptWnr;
    AX_ISP_IQ_EIS_PARAM_T       *ptEis;
    AX_ISP_IQ_CAC_PARAM_T       *ptCac;
    AX_ISP_IQ_LSC_PARAM_T       *ptLsc;
    AX_ISP_IQ_WB_GAIN_PARAM_T   *ptWbGain;
    AX_ISP_IQ_RLTM_PARAM_T      *ptRltm;
    AX_ISP_IQ_DEHAZE_PARAM_T    *ptDehaze;
    AX_ISP_IQ_DEMOSAIC_PARAM_T  *ptDemosaic;

    AX_ISP_IQ_CLC_PARAM_T       *ptClc;
    AX_ISP_IQ_PFR_PARAM_T       *ptPfr;
    AX_ISP_IQ_GAMMA_PARAM_T     *ptGamma;
    AX_ISP_IQ_YUV_CSC0_PARAM_T  *ptCsc0;
    AX_ISP_IQ_YUV_CSC1_PARAM_T  *ptCsc1;
    AX_ISP_IQ_SHARPEN_PARAM_T   *ptSharpen;
    AX_ISP_IQ_CSET_PARAM_T      *ptCset;
    AX_ISP_IQ_LUMA_NR_PARAM_T   *ptLumaNr;
    AX_ISP_IQ_YCPROC_PARAM_T    *ptYcproc;
    AX_ISP_IQ_CHROMA_NR_PARAM_T *ptChromaNr;
    AX_ISP_IQ_YCRT_PARAM_T      *ptYcrt;

    AX_ISP_IQ_AE_PARAM_T        *ptAeDftParam;
    AX_ISP_IQ_AWB_PARAM_T       *ptAwbDftParam;
} AX_SENSOR_DEFAULT_PARAM_T;

typedef struct _AX_SENSOR_REGISTER_FUNC_T_ {
    /* sensor ctrl */
    AX_S32(*pfn_sensor_chipid)(ISP_PIPE_ID nPipeId, int *pSnsChipId);
    AX_VOID(*pfn_sensor_init)(ISP_PIPE_ID nPipeId);
    AX_VOID(*pfn_sensor_exit)(ISP_PIPE_ID nPipeId);
    AX_S32(*pfn_sensor_streaming_ctrl)(ISP_PIPE_ID nPipeId, AX_U32 on);

    AX_S32(*pfn_sensor_set_mode)(ISP_PIPE_ID nPipeId, AX_SNS_ATTR_T *ptSnsMode);
    AX_S32(*pfn_sensor_get_mode)(ISP_PIPE_ID nPipeId, AX_SNS_ATTR_T *ptSnsMode);
    AX_S32(*pfn_sensor_set_wdr_mode)(ISP_PIPE_ID nPipeId, AX_SNS_HDR_MODE_E eHdrMode);
    AX_S32(*pfn_sensor_get_capability)(ISP_PIPE_ID nPipeId, AX_SNS_CAP_T *ptCap);
    AX_S32(*pfn_sensor_testpattern)(ISP_PIPE_ID nPipeId, AX_U32 nTestpatternEn);
    AX_S32(*pfn_sensor_mirror_flip)(ISP_PIPE_ID nPipeId, AX_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip);

    /* sensor read/write */
    AX_S32(*pfn_sensor_set_bus_info)(ISP_PIPE_ID nPipeId, AX_SNS_COMMBUS_T tSnsBusInfo);
    AX_S32(*pfn_sensor_write_register)(ISP_PIPE_ID nPipeId, AX_U32 nAddr, AX_U32 nData);
    AX_S32(*pfn_sensor_read_register)(ISP_PIPE_ID nPipeId, AX_U32 nAddr);

    /* module default parameters */
    AX_S32(*pfn_sensor_get_default_params)(ISP_PIPE_ID nPipeId, AX_SENSOR_DEFAULT_PARAM_T *ptDftParam);
    AX_S32(*pfn_sensor_get_isp_black_level)(ISP_PIPE_ID nPipeId, AX_SNS_BLACK_LEVEL_T *ptBlackLevel);

    /* ae ctrl */
    AX_S32(*pfn_sensor_set_params)(ISP_PIPE_ID nPipeId, AX_SNS_PARAMS_T *ptSnsParam);
    AX_S32(*pfn_sensor_get_params)(ISP_PIPE_ID nPipeId, AX_SNS_PARAMS_T *ptSnsParam);
    AX_S32(*pfn_sensor_get_gain_table)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_TABLE_T *ptSnsGainTbl);

    AX_S32(*pfn_sensor_set_again)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_CFG_T *ptAnalogGainTbl);
    AX_S32(*pfn_sensor_get_again)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_CFG_T *ptAnalogGainTbl);
    AX_S32(*pfn_sensor_set_dgain)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_CFG_T *ptDigitalGainTbl);
    AX_S32(*pfn_sensor_get_dgain)(ISP_PIPE_ID nPipeId, AX_SNS_AE_GAIN_CFG_T *ptDigitalGainTbl);
    AX_S32(*pfn_sensor_set_integration_time)(ISP_PIPE_ID nPipeId, AX_SNS_AE_SHUTTER_CFG_T *ptIntTimeTbl);
    AX_S32(*pfn_sensor_get_integration_time)(ISP_PIPE_ID nPipeId, AX_SNS_AE_SHUTTER_CFG_T *ptIntTimeTbl);

    AX_S32(*pfn_sensor_hcglcg_ctrl)(ISP_PIPE_ID nPipeId, AX_U32 nSnsHcgLcg, AX_F32 *pfSnsGain);
    AX_S32(*pfn_sensor_set_fps)(ISP_PIPE_ID nPipeId, AX_F32 nFps, AX_SNS_PARAMS_T *ptSnsParam);
    AX_S32(*pfn_sensor_get_slow_shutter_param)(ISP_PIPE_ID nPipeId, AX_SNS_AE_SLOW_SHUTTER_PARAM_T *ptSlowShutterParam);

    AX_S32(*pfn_sensor_get_sns_reg_info)(ISP_PIPE_ID nPipeId, AX_SNS_REGS_CFG_TABLE_T *ptSnsRegsInfo);

} AX_SENSOR_REGISTER_FUNC_T;

#endif //_AX_SENSOR_STRUCT_H_

