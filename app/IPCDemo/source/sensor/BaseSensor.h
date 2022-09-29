/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#pragma once

#include "global.h"

typedef enum _SENSOR_ID_E
{
    E_SENSOR_ID_0 = 0,
    E_SENSOR_ID_1 = 1,
    E_SENSOR_ID_MAX
} SENSOR_ID_E;

typedef enum {
    E_SNS_TYPE_OS04A10 = 0,
    E_SNS_TYPE_IMX334,
    E_SNS_TYPE_GC4653,
    E_SNS_TYPE_OS08A20,
    E_SNS_TYPE_SC1345,
    E_SNS_TYPE_SC530AI,
    E_SNS_TYPE_MAX,
} SNS_TYPE_E;

enum COLOR_FORMAT_E
{
    E_COLOR_FMT_YUV422I_UYVY = 0,
    E_COLOR_FMT_YUV422I_YUYV,
    E_COLOR_FMT_YUV420SP_NV12,
    E_COLOR_FMT_YUV420SP_NV21,
    E_COLOR_FMT_YUV422P,
    E_COLOR_FMT_YUV444I,
    E_COLOR_FMT_YUV444P,
};

typedef struct _CAMERA_CHAN_CFG_T
{
    AX_BOOL bEnable;
    AX_U32 nWidth;
    AX_U32 nHeight;

    _CAMERA_CHAN_CFG_T() {
        this->bEnable = AX_FALSE;
        this->nWidth  = 0;
        this->nHeight = 0;
    }

    _CAMERA_CHAN_CFG_T(AX_U32 nWidth, AX_U32 nHeight) {
        this->nWidth = nWidth;
        this->nHeight = nHeight;
    }

} CAMERA_CHAN_CFG_T, *CAMERA_CHAN_CFG_PTR;

typedef struct _SENSOR_INFO_T
{
    AX_RUN_MODE_E       eRunMode;
    SNS_TYPE_E          eSensorType;
    AX_SNS_HDR_MODE_E   eSensorMode;
    AX_U32              nFrameRate;
    AX_BOOL             bSupportTuning;
    AX_U32              nTuningPort;

    _SENSOR_INFO_T() {
        memset(this, 0, sizeof(_SENSOR_INFO_T));
    }

} SENSOR_INFO_T;

#define SENSOR_BIN_PATH_LEN    50

typedef struct _SENSOR_CONFIG_T
{
    AX_U32              nFrameRate;
    AX_RUN_MODE_E       eRunMode;
    SNS_TYPE_E          eSensorType;
    AX_SNS_HDR_MODE_E   eSensorMode;
    AX_CHAR             aNormalModeBin[SENSOR_BIN_PATH_LEN];
    AX_CHAR             aHotbalanceModeBin[SENSOR_BIN_PATH_LEN];
    AX_CHAR             aEISSdrBin[SENSOR_BIN_PATH_LEN];
    AX_CHAR             aEISHdrBin[SENSOR_BIN_PATH_LEN];
    AX_BOOL             bTuning;
    AX_U32              nTuningPort;
    AX_BOOL             bEnableEIS;
    CAMERA_CHAN_CFG_T   arrChannels[MAX_ISP_CHANNEL_NUM];

    _SENSOR_CONFIG_T() {
        memset(this, 0, sizeof(_SENSOR_CONFIG_T));

        nFrameRate      = 25;
        eRunMode        = AX_ISP_PIPELINE_NORMAL;
        eSensorType     = E_SNS_TYPE_MAX;
        eSensorMode     = AX_SNS_LINEAR_MODE;
        bTuning         = AX_TRUE;
        nTuningPort     = 8082;
        bEnableEIS      = AX_FALSE;
    }

} SENSOR_CONFIG_T, *SENSOR_CONFIG_PTR;

typedef struct _SENSOR_CLK_T {
    AX_U8             nSnsClkIdx;
    AX_SNS_CLK_RATE_E eSnsClkRate;

    _SENSOR_CLK_T() {
        memset(this, 0, sizeof(_SENSOR_CLK_T));
    }

} SENSOR_CLK_T, *SENSOR_CLK_PTR;


using namespace std;
class CBaseSensor
{
public:
    CBaseSensor(SENSOR_CONFIG_T tSensorConfig);
    virtual ~CBaseSensor(AX_VOID);

public:
    static AX_NPU_SDK_EX_ATTR_T GetNpuAttr(SNS_TYPE_E eSnsType, AX_SNS_HDR_MODE_E eSnsMode);
    static CBaseSensor* NewInstance(AX_U8 nSensorIndex);
    static AX_BOOL BuffPoolInit(AX_POOL_FLOORPLAN_T *stVbConf, AX_U8 nCount);
    static AX_BOOL BuffPoolExit();
    static AX_S8 GetI2cDevNode(AX_U8 nPipe);

    virtual AX_BOOL Init(AX_U8 nSensorID, AX_U8 nDevID, AX_U8 nPipeID, AX_POOL_FLOORPLAN_T *stVbConf, AX_U8& nCount);
    virtual AX_BOOL Open();
    virtual AX_BOOL Close();

    AX_BOOL GetSnsAttr(AX_SNS_ATTR_T& attr);
    AX_BOOL GetChnAttr(AX_VIN_CHN_ATTR_T& attr);
    AX_VOID UpdateSnsMode(AX_SNS_HDR_MODE_E eMode);
    AX_VOID UpdateFramerate(AX_F32 fFramerate);
    AX_BOOL EnableEIS(AX_BOOL bEnableEIS);
    AX_VOID InitSensor(AX_U8 nPipe);
    AX_VOID ExitSensor(AX_U8 nPipe);

protected:
    virtual AX_VOID InitSnsAttr()  = 0;
    virtual AX_VOID InitDevAttr()  = 0;
    virtual AX_VOID InitPipeAttr() = 0;
    virtual AX_VOID InitMipiRxAttr() = 0;
    virtual AX_VOID InitChnAttr() = 0;
    virtual AX_S32  RegisterSensor(AX_U8 nPipe) = 0;
    virtual AX_S32  UnRegisterSensor(AX_U8 nPipe) = 0;
    virtual AX_S32  RegisterAeAlgLib(AX_U8 nPipe) = 0;
    virtual AX_S32  UnRegisterAeAlgLib(AX_U8 nPipe) = 0;
    virtual AX_S32  RegisterAwbAlgLib(AX_U8 nPipe) = 0;
    virtual AX_S32  UnRegisterAwbAlgLib(AX_U8 nPipe) = 0;
    virtual AX_S32  SetFps(AX_U8 nPipe, AX_F32 fFrameRate) = 0;
    virtual AX_BOOL HdrModeSupport(AX_U8 nPipe) = 0;

    virtual AX_VOID InitNPU();

private:
    AX_BOOL InitISP();
    AX_VOID FillValues(SENSOR_CONFIG_T tSensorConfig);
    AX_VOID CalcBufSize(AX_U8 nPipe, AX_POOL_FLOORPLAN_T *stVbConf, AX_U8& nCount, AX_U32 nPreISPBlkCnt = 10, AX_U32 nNpuBlkCnt = 5);

protected:
    AX_U8               m_nSensorID;
    AX_U8               m_nDeviceID;
    AX_U8               m_nPipeID;

    AX_VOID*            m_pSnsLib;
    AX_SENSOR_REGISTER_FUNC_T * m_pSnsObj;

    SENSOR_INFO_T       m_tSnsInfo;
    SENSOR_CLK_T        m_tSnsClkAttr;
    AX_SNS_ATTR_T       m_tSnsAttr;
    AX_DEV_ATTR_T       m_tDevAttr;
    AX_PIPE_ATTR_T      m_tPipeAttr;
    AX_MIPI_RX_ATTR_S   m_tMipiRxAttr;
    AX_VIN_CHN_ATTR_T   m_tChnAttr;

    AX_IMG_FORMAT_E     m_eImgFormatSDR;
    AX_IMG_FORMAT_E     m_eImgFormatHDR;
};
