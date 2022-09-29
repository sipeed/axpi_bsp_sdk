/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include "MediaFrame.h"
#include "VideoEncoder.h"
#include "BaseSensor.h"
#include "IVPSStage.h"

#include <list>

using namespace std;

typedef struct _YUV_THREAD_PARAM
{
    AX_BOOL bValid;
    AX_U8 nPipeID;
    AX_U8 nIspChn;
    AX_BOOL bThreadRunning;

    _YUV_THREAD_PARAM() {
        bValid = AX_FALSE;
        nPipeID = 0;
        nIspChn = 0;
        bThreadRunning = AX_FALSE;
    }
} YUV_THREAD_PARAM_T, *YUV_THREAD_PARAM_PTR;

class CCamera : public IFrameRelease
{
public:
    CCamera();
    virtual ~CCamera();

public:
    virtual AX_VOID MediaFrameRelease(CMediaFrame *pMediaFrame);
    virtual AX_BOOL Init(AX_POOL_FLOORPLAN_T *stVbConf, AX_U8& nCount, AX_U8 nSensorID = E_SENSOR_ID_0, AX_U8 nDeviceID = 0, AX_U8 nPipeID = 0, AX_BOOL bUpdateAttrOnReset = AX_FALSE);
    virtual AX_BOOL Open();
    virtual AX_BOOL Close();

    AX_BOOL Start();
    AX_VOID Stop();
    AX_VOID Restart(AX_VOID);

    AX_VOID BindIvpsStage(CIVPSStage* pStage);
    AX_SNS_ATTR_T     GetSnsAttr();
    AX_VIN_CHN_ATTR_T GetChnAttr();

    static AX_VOID RestartNotify();
    static AX_VOID DaynightChangeNotify(AX_DAYNIGHT_MODE_E eDaynightMode, AX_BOOL bImmediately);
    static AX_VOID NrModeChangeNotify(AX_U8 nNrMode, AX_BOOL bImmediately);

    static AX_VOID ChangeNrMode(AX_U8 nNrMode);
    static AX_VOID ChangeDaynightMode(AX_DAYNIGHT_MODE_E eDaynightMode);

    static AX_BOOL DoEISSwitch(AX_BOOL bIsEISOn);

    AX_VOID UpdateFramerate(AX_F32 fFramerate);
    AX_VOID SwitchSdrNrModeForHotbalance(AX_U8 nNrMode);
    AX_VOID UpdateAeForHotbalance(AX_BOOL bHotBalance);
    AX_VOID ResetFPS(AX_U32 nFPS);

private:
    AX_VOID YuvGetThreadFunc(YUV_THREAD_PARAM_PTR pThreadParam);
    AX_VOID ItpLoopThreadFunc();
    AX_VOID ClearQFrame();

public :
    SENSOR_ID_E m_eSnsID;
    AX_U8       m_nDevID;
    AX_U8       m_nPipeID;
    AX_BOOL     m_bRtpThreadRunning;
    AX_BOOL     m_bRestarting;

    static AX_BOOL s_bISPRestartRequired;
    static AX_BOOL s_bResetDaynightModeRequired;
    static AX_BOOL s_bResetNrModeRequired;

private:
    YUV_THREAD_PARAM_T m_tYUVThreadParam[MAX_ISP_CHANNEL_NUM];
    thread* m_pYuvGetThread[MAX_ISP_CHANNEL_NUM];
    thread* m_pRtpThread;
    list<CMediaFrame *>  m_qFrame[MAX_ISP_CHANNEL_NUM];
    mutex   m_mtxFrame[MAX_ISP_CHANNEL_NUM];

    CBaseSensor*        m_pSensorInstance;
    CIVPSStage*         m_pIvpsStage;
};

#endif // camera_H
