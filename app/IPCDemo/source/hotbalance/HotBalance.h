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

#include <thread>
#include <mutex>
#include "global.h"
#include "Singleton.h"
#include "BaseSensor.h"

#define HOTBALANCE_SNS_FPS_DEFAULT 12
#define HOTBALANCE_VENC_FPS_DEFAULT 12
#define HOTBALANCE_THERSHOLD_MEDIAN_DEFAULT 105
#define HOTBALANCE_THERSHOLD_LOW_DEFAULT 90
#define HOTBALANCE_THERSHOLD_GAP_DEFAULT 0

typedef enum {
    HOTBALANCE_BALANCE_LEVEL_HIGH,
    HOTBALANCE_BALANCE_LEVEL_MID,
    HOTBALANCE_BALANCE_LEVEL_BUTT
} HOTBALANCE_BALANCE_LEVEL_E;

#define HOTBALANCE_BALANCE_LEVEL_DEFAULT HOTBALANCE_BALANCE_LEVEL_MID

typedef struct _HOTBALANCE_CAMERA_CONFIG_T {
    AX_BOOL bValid;
    AX_BOOL bSdrOnly;
    AX_F32 fSnsFps;
} HOTBALANCE_CAMERA_CONFIG_T;

typedef struct _HOTBALANCE_VENC_CONFIG_T {
    AX_BOOL bValid;
    AX_F32 fVencFps;
} HOTBALANCE_VENC_CONFIG_T;

typedef struct _HOTBALANCE_AI_CONFIG_T {
    AX_BOOL bValid;
    AX_F32 fDetectFps;
    AX_F32 fAiFps;
    AX_F32 fIvesFps;
} HOTBALANCE_AI_CONFIG_T;

typedef struct _HOTBALANCE_CONFIG_T {
    AX_BOOL bEnable;
    AX_F32 fThersholdM;
    AX_F32 fThersholdL;
    AX_F32 fGap;
    HOTBALANCE_BALANCE_LEVEL_E eBalanceLevel;
    HOTBALANCE_CAMERA_CONFIG_T tCameraConf[E_SENSOR_ID_MAX];
    HOTBALANCE_VENC_CONFIG_T tVencConf[MAX_VENC_CHANNEL_NUM];
    HOTBALANCE_AI_CONFIG_T tAiConf[MAX_AI_CHANNEL_NUM];

    _HOTBALANCE_CONFIG_T() {
        bEnable = AX_FALSE;
        fThersholdM = HOTBALANCE_THERSHOLD_MEDIAN_DEFAULT;
        fThersholdL = HOTBALANCE_THERSHOLD_LOW_DEFAULT;
        fGap = HOTBALANCE_THERSHOLD_GAP_DEFAULT;
        eBalanceLevel = HOTBALANCE_BALANCE_LEVEL_DEFAULT;

        for (AX_U8 i = 0; i < E_SENSOR_ID_MAX; i ++) {
            tCameraConf[i].bValid = AX_FALSE;
            tCameraConf[i].bSdrOnly = AX_TRUE;
            tCameraConf[i].fSnsFps = HOTBALANCE_SNS_FPS_DEFAULT;
        }

        for (AX_U8 i = 0; i < MAX_VENC_CHANNEL_NUM; i ++) {
            tVencConf[i].bValid = AX_FALSE;
            tVencConf[i].fVencFps = HOTBALANCE_VENC_FPS_DEFAULT;
        }

        for (AX_U8 i = 0; i < MAX_AI_CHANNEL_NUM; i ++) {
            tAiConf[i].bValid = AX_FALSE;
            tAiConf[i].fDetectFps = DETECT_DEFAULT_HOTBALANCE_FRAMERATE_CTRL;
            tAiConf[i].fAiFps = DETECT_DEFAULT_HOTBALANCE_AI_FRAMERATE_CTRL;
            tAiConf[i].fIvesFps = DETECT_DEFAULT_HOTBALANCE_IVES_FRAMERATE_CTRL;
        }
    }
} HOTBALANCE_CONFIG_T;

class CHotBalance : public CSingleton<CHotBalance> {
    friend class CSingleton<CHotBalance>;

private:
    CHotBalance(AX_VOID);
    virtual ~CHotBalance(AX_VOID);

    AX_BOOL Init(AX_VOID) override {
        return AX_TRUE;
    }

    AX_S32 ThreadThermalMonitor(AX_VOID);
    AX_BOOL ProcessThermal(AX_F32 fThermal);
    AX_BOOL Escape(AX_VOID);
    AX_BOOL Recovery(AX_VOID);

public:
    AX_BOOL Start(HOTBALANCE_CONFIG_T tConfig);
    AX_BOOL Stop(AX_VOID);
    AX_BOOL Update(HOTBALANCE_CONFIG_T tConfig);
    AX_BOOL Stat(AX_VOID);

private:
    AX_S32 m_nThermalHandle;
    AX_BOOL m_bThreadRunning;
    AX_BOOL m_bEscape;
    AX_U8 m_nCurHdrEnable;
    AX_U8 m_nNrMode;
    HOTBALANCE_CONFIG_T m_tHotBalanceConfig;
    std::mutex m_mutex;
};

