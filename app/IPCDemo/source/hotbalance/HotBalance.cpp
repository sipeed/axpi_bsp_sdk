/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <fcntl.h>
#include "HotBalance.h"
#include "OptionHelper.h"
#include "StageOptionHelper.h"
#include "Camera.h"
#include "VideoEncoder.h"
#include "IVPSStage.h"
#include "JsonCfgParser.h"

#define HOTBALANCE "HOTBALANCE"

#define HOTBALANCE_INVALID_HANDLE (-1)
#define HOTBALANCE_THERMAL_NODE_NAME "/sys/class/thermal/thermal_zone0/temp"

extern COptionHelper gOptions;

extern CCamera g_camera;
extern CIVPSStage g_stageIVPS;
extern vector<CVideoEncoder*> g_vecVEnc;
extern CDetectStage g_stageDetect;

extern AX_VOID IVPSConfigHotBalance(AX_BOOL bIsHotBalance, AX_U32 nSnsFramerate);

AX_VOID APP_EnterNormalMode(HOTBALANCE_BALANCE_LEVEL_E eBalanceLevel)
{
    if (HOTBALANCE_BALANCE_LEVEL_HIGH == eBalanceLevel) {
        LOG_M(HOTBALANCE, "Clk works to high mode");

        AX_SYS_CLK_SetLevel(AX_SYS_CLK_HIGH_MODE);
    }
    else {
        LOG_M(HOTBALANCE, "Clk works to mid mode");

        AX_SYS_CLK_SetLevel(AX_SYS_CLK_MID_MODE);
    }
}

AX_VOID APP_EnterHotBalanceMode(AX_VOID)
{
    AX_SYS_CLK_LEVEL_E eLevel = AX_SYS_CLK_GetLevel();

    LOG_M(HOTBALANCE, "Clk working level: %d", eLevel);

    if (AX_SYS_CLK_HIGH_MODE == eLevel) {
        AX_SYS_CLK_SetLevel(AX_SYS_CLK_HIGH_HOTBALANCE_MODE);
    }
    else {
        AX_SYS_CLK_SetLevel(AX_SYS_CLK_MID_HOTBALANCE_MODE);
    }
}

CHotBalance::CHotBalance(AX_VOID)
: m_nThermalHandle(HOTBALANCE_INVALID_HANDLE)
, m_bThreadRunning(AX_FALSE)
, m_bEscape(AX_FALSE)
, m_nCurHdrEnable(0)
, m_nNrMode(0)
{

}

CHotBalance::~CHotBalance(AX_VOID)
{

}

AX_BOOL CHotBalance::Start(HOTBALANCE_CONFIG_T tConfig)
{
    if (!tConfig.bEnable) {
        LOG_M(HOTBALANCE, "HotBalance is disabled");

        return AX_FALSE;
    }

    LOG_M(HOTBALANCE, "+++");

    m_mutex.lock();
    m_tHotBalanceConfig = tConfig;
    m_mutex.unlock();

    LOG_M(HOTBALANCE, "HotBalance monitor start: (M:%.2f,L:%.2f,Gap:%.2f,level:%d)",
                m_tHotBalanceConfig.fThersholdM, m_tHotBalanceConfig.fThersholdL,
                m_tHotBalanceConfig.fGap, m_tHotBalanceConfig.eBalanceLevel);

    if (!m_bThreadRunning) {
        m_bThreadRunning = AX_TRUE;

        std::thread t(&CHotBalance::ThreadThermalMonitor, this);
        t.detach();
    }

    APP_EnterNormalMode(m_tHotBalanceConfig.eBalanceLevel);

    LOG_M(HOTBALANCE, "---");

    return AX_TRUE;
}

AX_BOOL CHotBalance::Stop(AX_VOID)
{
    LOG_M(HOTBALANCE, "+++");

    m_bThreadRunning = AX_FALSE;

    m_mutex.lock();
    m_tHotBalanceConfig.bEnable = AX_FALSE;
    m_mutex.unlock();

    if (m_nThermalHandle != HOTBALANCE_INVALID_HANDLE) {
        close(m_nThermalHandle);

        m_nThermalHandle = HOTBALANCE_INVALID_HANDLE;
    }

    LOG_M(HOTBALANCE, "---");

    return AX_TRUE;
}

AX_BOOL CHotBalance::Update(HOTBALANCE_CONFIG_T tConfig)
{
    if (!tConfig.bEnable) {
        Stop();
    }
    else {
        if (m_bThreadRunning) {
            m_mutex.lock();
            m_tHotBalanceConfig = tConfig;
            m_mutex.unlock();
        }
        else {
            Start(tConfig);
        }
    }

    return AX_TRUE;
}

AX_BOOL CHotBalance::Stat(AX_VOID)
{
    std::lock_guard<std::mutex> lck(m_mutex);

    return m_bEscape;
}

AX_S32 CHotBalance::ThreadThermalMonitor(AX_VOID)
{
    prctl(PR_SET_NAME, "APP_ThermalMonitor");

    while (m_bThreadRunning) {
        if (m_nThermalHandle == HOTBALANCE_INVALID_HANDLE) {
            m_nThermalHandle = open(HOTBALANCE_THERMAL_NODE_NAME, O_RDONLY);
        }

        if (m_nThermalHandle != HOTBALANCE_INVALID_HANDLE) {
            AX_CHAR strThermal[50] = {0};

            lseek(m_nThermalHandle, 0, SEEK_SET);

            if (read(m_nThermalHandle, strThermal, 50) > 0) {
                AX_S32 nThermal = atoi(strThermal);
                AX_F32 fThermal = (AX_F32)nThermal/1000.0;

                LOG_M_I(HOTBALANCE, "Thermal: %.3f", fThermal);

                ProcessThermal(fThermal);
            }
            else {
                LOG_M(HOTBALANCE, "read %s fail", HOTBALANCE_THERMAL_NODE_NAME);

                close(m_nThermalHandle);

                m_nThermalHandle = HOTBALANCE_INVALID_HANDLE;
            }
        }
        else {
            LOG_M(HOTBALANCE, "open %s fail", HOTBALANCE_THERMAL_NODE_NAME);
        }

        CTimeUtils::msSleep(2000);
    }

    return 0;
}

AX_BOOL CHotBalance::ProcessThermal(AX_F32 fThermal)
{
    m_mutex.lock();
    HOTBALANCE_CONFIG_T *pConfig = &m_tHotBalanceConfig;
    m_mutex.unlock();

    if (pConfig->bEnable) {
        if (fThermal >= pConfig->fThersholdM + pConfig->fGap) {
            // Escape
            if (!m_bEscape) {
                LOG_M(HOTBALANCE, "Thermal: %.3f", fThermal);

                m_bEscape = AX_TRUE;

                Escape();
            }
        }
        else if (fThermal <= pConfig->fThersholdL - pConfig->fGap) {
            // Recovery
            if (m_bEscape) {
                LOG_M(HOTBALANCE, "Thermal: %.3f", fThermal);

                m_bEscape = AX_FALSE;

                Recovery();
            }
        }
    }

    return AX_TRUE;
}

AX_BOOL CHotBalance::Escape(AX_VOID)
{
    LOG_M(HOTBALANCE, "Escape ...");

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    CAMERA_ATTR_T tCameraAttr = pStageOption->GetCamera();
    AX_F32 fSnsFps = 0;

    if (m_tHotBalanceConfig.tCameraConf[AX_DEV_SOURCE_SNS_ID].bValid) {
        fSnsFps = m_tHotBalanceConfig.tCameraConf[AX_DEV_SOURCE_SNS_ID].fSnsFps;
    }
    else {
        fSnsFps = (AX_F32)tCameraAttr.nFramerate;
    }

    if (m_tHotBalanceConfig.tCameraConf[AX_DEV_SOURCE_SNS_ID].bValid) {
        m_nCurHdrEnable = tCameraAttr.nHdrEnable;

        AX_BOOL bRestartISP = AX_FALSE;
        if (m_tHotBalanceConfig.tCameraConf[AX_DEV_SOURCE_SNS_ID].bSdrOnly) {
            if (tCameraAttr.nHdrEnable > 0) {
                LOG_M(HOTBALANCE, "Change to SDR mode ...");

                tCameraAttr.nHdrEnable = 0;

                bRestartISP = AX_TRUE;
            }
        }

        m_nNrMode = tCameraAttr.nNrMode;

        tCameraAttr.nNrMode = 0; //Dummy
        pStageOption->SetCamera(tCameraAttr);

        if (bRestartISP) {
            g_camera.Restart();
        }

        if (tCameraAttr.nHdrEnable == 0) {
            LOG_M(HOTBALANCE, "Change SDR Nr mode ...");
            g_camera.SwitchSdrNrModeForHotbalance(tCameraAttr.nNrMode);
        }
    }

    if (gOptions.IsActivedDetect() && gOptions.IsActivedDetectFromWeb()) {
        if (m_tHotBalanceConfig.tAiConf[0].bValid) {
            LOG_M(HOTBALANCE, "Change AI framerate ...");

            AI_ATTR_T tAiAttr = pStageOption->GetAiAttr();
            tAiAttr.tConfig.nDetectFps = (AX_U32)m_tHotBalanceConfig.tAiConf[0].fDetectFps;
            tAiAttr.tConfig.nAiFps = (AX_U32)m_tHotBalanceConfig.tAiConf[0].fAiFps;
            tAiAttr.tConfig.nIvesFps = (AX_U32)m_tHotBalanceConfig.tAiConf[0].fIvesFps;
            pStageOption->SetAiAttr(tAiAttr);

            DETECT_STAGE_INFO_T tInfo;
            tInfo.nFrmWidth  = tAiAttr.tConfig.nWidth;
            tInfo.nFrmHeight = tAiAttr.tConfig.nHeight;
            tInfo.nVinFps    = (AX_U16)fSnsFps;
            tInfo.nFrmFps    = tAiAttr.tConfig.nDetectFps;
            g_stageDetect.ConfigStageInfo(tInfo);
        }
    }

    if (m_tHotBalanceConfig.tCameraConf[AX_DEV_SOURCE_SNS_ID].bValid) {
        LOG_M(HOTBALANCE, "Change sensor framerate ...");
        g_camera.UpdateFramerate(fSnsFps);

        LOG_M(HOTBALANCE, "Change Ae param ...");
        g_camera.UpdateAeForHotbalance(AX_TRUE);

        LOG_M(HOTBALANCE, "Change IVPS framerate ...");
        g_stageIVPS.UpdateFramerate(fSnsFps);
    }

    for (AX_U32 i = 0; i < g_vecVEnc.size(); i++) {
        if (m_tHotBalanceConfig.tVencConf[i].bValid) {
            LOG_M(HOTBALANCE, "Change venc%d framerate ...", i);

            AX_F32 fVencFps = m_tHotBalanceConfig.tVencConf[i].fVencFps;

            g_vecVEnc[i]->UpdateFramerate(fSnsFps, fVencFps);
        }
    }

    APP_EnterHotBalanceMode();

    return AX_TRUE;
}

AX_BOOL CHotBalance::Recovery(AX_VOID)
{
    LOG_M(HOTBALANCE, "Recovery ...");

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    CAMERA_ATTR_T tCameraAttr = pStageOption->GetCamera();

    AX_BOOL bRestartISP = AX_FALSE;
    if (tCameraAttr.nHdrEnable != m_nCurHdrEnable) {
        LOG_M(HOTBALANCE, "Recovery %s mode ...", (m_nCurHdrEnable == 0) ? "SDR" : "HDR");

        tCameraAttr.nHdrEnable = m_nCurHdrEnable;

        bRestartISP = AX_TRUE;
    }

    tCameraAttr.nNrMode = m_nNrMode; //Nr mode
    pStageOption->SetCamera(tCameraAttr);

    if (bRestartISP) {
        g_camera.Restart();
    }

    if (tCameraAttr.nHdrEnable == 0) {
        LOG_M(HOTBALANCE, "Change SDR Nr mode ...");
        g_camera.SwitchSdrNrModeForHotbalance(tCameraAttr.nNrMode);
    }

    if (gOptions.IsActivedDetect() && gOptions.IsActivedDetectFromWeb()) {
        LOG_M(HOTBALANCE, "Recovery AI framerate ...");
        AI_ATTR_T tAiAttr = pStageOption->GetAiAttr();

        AI_CONFIG_T tAiConfig;
        memset(&tAiConfig, 0x00, sizeof(tAiConfig));
        if (CConfigParser().GetInstance()->GetAiCfg(0, tAiConfig)) {
            tAiAttr.tConfig = tAiConfig;
        }
        else {
            tAiAttr.tConfig.nDetectFps = DETECT_DEFAULT_FRAMERATE_CTRL;
            tAiAttr.tConfig.nAiFps = DETECT_DEFAULT_AI_FRAMERATE_CTRL;
            tAiAttr.tConfig.nIvesFps = DETECT_DEFAULT_IVES_FRAMERATE_CTRL;
            tAiAttr.tConfig.nWidth = DETECT_DEFAULT_WIDTH;
            tAiAttr.tConfig.nHeight = DETECT_DEFAULT_HEIGHT;
        }
        pStageOption->SetAiAttr(tAiAttr);

        DETECT_STAGE_INFO_T tInfo;
        tInfo.nFrmWidth  = tAiAttr.tConfig.nWidth;
        tInfo.nFrmHeight = tAiAttr.tConfig.nHeight;
        tInfo.nVinFps    = tCameraAttr.nFramerate;
        tInfo.nFrmFps    = tAiAttr.tConfig.nDetectFps;
        g_stageDetect.ConfigStageInfo(tInfo);
    }

    LOG_M(HOTBALANCE, "Recovery sensor framerate ...");
    g_camera.UpdateFramerate(tCameraAttr.nFramerate);

    LOG_M(HOTBALANCE, "Recovery Ae param ...");
    g_camera.UpdateAeForHotbalance(AX_FALSE);

    LOG_M(HOTBALANCE, "Recovery IVPS framerate ...");
    g_stageIVPS.UpdateFramerate(tCameraAttr.nFramerate);

    LOG_M(HOTBALANCE, "Recovery venc framerate ...");
    for (AX_U32 i = 0; i < g_vecVEnc.size(); i++) {
        VIDEO_CONFIG_T tVEncConfig;
        AX_U32 nDstFramerate = (AX_U32)tCameraAttr.nFramerate;
        if (CConfigParser().GetInstance()->GetVideoCfgByID(i, tVEncConfig)) {
            nDstFramerate = tVEncConfig.nFrameRate;
        }

        g_vecVEnc[i]->UpdateFramerate(tCameraAttr.nFramerate, nDstFramerate);
    }

    APP_EnterNormalMode(m_tHotBalanceConfig.eBalanceLevel);

    return AX_TRUE;
}
