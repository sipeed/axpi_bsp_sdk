/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _STAGE_OPTION_HELPER_H__
#define _STAGE_OPTION_HELPER_H__
#include <map>
#include <mutex>
#include "global.h"
#include "Singleton.h"
#include "VideoEncoder.h"
#include "StageOptionHelper.h"
#include "HotBalance.h"

typedef enum
{
    E_AI_DETECT_MODEL_TYPE_FACEHUMAN = 0,
    E_AI_DETECT_MODEL_TYPE_HVCFP,
    E_AI_DETECT_MODEL_TYPE_MAX
} AI_DETECT_MODEL_TYPE_E;

typedef enum
{
    E_AI_DETECT_PUSH_MODE_TYPE_FAST = 1,
    E_AI_DETECT_PUSH_MODE_TYPE_INTERVAL,
    E_AI_DETECT_PUSH_MODE_TYPE_BEST,
    E_AI_DETECT_PUSH_MODE_TYPE_MAX
} AI_DETECT_PUSH_MODE_TYPE_E;

typedef struct _CAMERA_ATTR_T
{
    AX_U8 nHdrEnable;
    AX_U8 nRotation;
    AX_U8 nMirror;
    AX_U8 nFlip;
    AX_U8 nFramerate;
    AX_U8 nDaynightMode;
    AX_U8 nNrMode;
    AX_U8 nEISSupport;
    AX_U8 nEISEnable;

    _CAMERA_ATTR_T() {
        nHdrEnable = 0;
        nRotation = 0;  /* 0:0°; 1:90°; 2:180°; 3:270° */
        nMirror = 0; /* 0:normal; 1:mirror */
        nFlip = 0; /* 0:normal; 1:flip */
        nFramerate = 25;
        nDaynightMode = 0; /* 0:Day mode; 1:Night Mode */
        nNrMode = 1; /* 0:Dummy mode; 1:Nr Mode */
        nEISSupport = 0;
        nEISEnable = 0;
    }

} CAMERA_ATTR_T, *CAMERA_ATTR_PTR;

typedef struct _AI_MODEL_OPTION_T
{
    AX_U16 nEnable;
    AX_U16 nDrawRect;

    _AI_MODEL_OPTION_T() {
        memset(this, 0, sizeof(_AI_MODEL_OPTION_T));
    }

} AI_MODEL_OPTION_T, *AI_MODEL_OPTION_PTR;

typedef struct _AI_MODEL_FALCEHUMAN_ATTR_T
{
    AI_MODEL_OPTION_T tFace;
    AI_MODEL_OPTION_T tBody;
    AX_U16 nEnableFI; /* face identify */

    _AI_MODEL_FALCEHUMAN_ATTR_T() {
        memset(this, 0, sizeof(_AI_MODEL_OPTION_T));
    }

} AI_MODEL_FALCEHUMAN_ATTR_T, *AI_MODEL_FALCEHUMAN_ATTR_PTR;

typedef struct _AI_MODEL_HVCFP_ATTR_T
{
    AI_MODEL_OPTION_T tFace;
    AI_MODEL_OPTION_T tBody;
    AI_MODEL_OPTION_T tVechicle;
    AI_MODEL_OPTION_T tCycle;
    AI_MODEL_OPTION_T tPlate;
    AX_U16 nEnablePI; /* plate identify */

    _AI_MODEL_HVCFP_ATTR_T() {
        memset(this, 0, sizeof(_AI_MODEL_HVCFP_ATTR_T));
    }

} AI_MODEL_HVCFP_ATTR_T, *AI_MODEL_HVCFP_ATTR_PTR;

typedef struct _AI_PUSH_STRGY_T
{
    AI_DETECT_PUSH_MODE_TYPE_E eMode;
    AX_U16 nInterval;
    AX_U16 nCount;
    AX_U8 nPushSameFrame;

    _AI_PUSH_STRGY_T() {
        memset(this, 0, sizeof(_AI_PUSH_STRGY_T));
    }

} AI_PUSH_STRGY_T, *AI_PUSH_STRGY_PTR;

typedef struct _AI_EVENTS_DETECT_ATTR_T
{
    AX_U16 nEnable;
    AX_U16 nThrsHoldY;
    AX_U16 nConfidence;

    _AI_EVENTS_DETECT_ATTR_T() {
        memset(this, 0, sizeof(_AI_EVENTS_DETECT_ATTR_T));
    }

} AI_EVENTS_DETECT_ATTR_T, *AI_EVENTS_DETECT_ATTR_PTR;

typedef struct _AI_EVENTS_OPTION_T
{
    AI_EVENTS_DETECT_ATTR_T tMD;
    AI_EVENTS_DETECT_ATTR_T tOD;

    _AI_EVENTS_OPTION_T() {
        memset(this, 0, sizeof(_AI_EVENTS_OPTION_T));
    }

} AI_EVENTS_OPTION_T, *AI_EVENTS_OPTION_PTR;

typedef struct _AI_ATTR_T
{
    AX_U16                  nEnable;
    AX_U16                  nDetectOnly;
    AI_CONFIG_T             tConfig;
    AI_DETECT_MODEL_TYPE_E  eDetectModel;
    AI_PUSH_STRGY_T         tPushStrgy;
    AI_EVENTS_OPTION_T      tEvents;
    union {
        AI_MODEL_FALCEHUMAN_ATTR_T tHumanFaceSetting;
        AI_MODEL_HVCFP_ATTR_T      tHvcfpSetting;
    };

    _AI_ATTR_T() {
        memset(this, 0, sizeof(_AI_ATTR_T));

        tConfig.nWidth = DETECT_DEFAULT_WIDTH;
        tConfig.nHeight = DETECT_DEFAULT_HEIGHT;
        tConfig.nDetectFps = DETECT_DEFAULT_FRAMERATE_CTRL;
        tConfig.nAiFps = DETECT_DEFAULT_AI_FRAMERATE_CTRL;
        tConfig.nIvesFps = DETECT_DEFAULT_IVES_FRAMERATE_CTRL;
    }
} AI_ATTR_T, *AI_ATTR_PTR;

typedef struct _STATISTICS_INFO_T
{
    AX_U64 nStartTick;
    AX_U32 nVencOutBytes;
    AX_F32 fBitrate;

    _STATISTICS_INFO_T() {
        memset(this, 0, sizeof(_STATISTICS_INFO_T));
    }
} STATISTICS_INFO_T, *STATISTICS_INFO_PTR;

typedef struct _HOTBALANCE_ATTR_T
{
    HOTBALANCE_CONFIG_T tConfig;
} HOTBALANCE_ATTR_T, *HOTBALANCE_ATTR_PTR;

typedef AX_S32 (*SNAPSHOT_CALLBACK_FUNC)(AX_U32 uChn, AX_VOID *pBuf, AX_U32 nBufferSize);

class CStageOptionHelper: public CSingleton<CStageOptionHelper>
{
    friend class CSingleton<CStageOptionHelper>;

public:
    CStageOptionHelper(AX_VOID);
    ~CStageOptionHelper(AX_VOID);

    CAMERA_ATTR_T   GetCamera();
    AX_VOID         SetCamera(const CAMERA_ATTR_T& tCamera);
    AX_BOOL         GetCameraStr(AX_CHAR* pOutBuf, AX_U32 nSize);

    VIDEO_ATTR_T    GetVideo(AX_U32 nChan);
    AX_VOID         SetVideo(AX_U32 nChan, const VIDEO_ATTR_T& tVideo);
    std::string     GetFramerateOptStr();

    AI_ATTR_T       GetAiAttr();
    AX_VOID         SetAiAttr(const AI_ATTR_T& tAttr);
    AX_BOOL         GetAiInfoStr(AX_CHAR* pOutBuf, AX_U32 nSize);
    std::string     GetDetectModelStr();
    AX_BOOL         GetPushStrgyStr(AX_CHAR* pOutBuf, AX_U32 nSize);
    std::string     GetPushModeStr();
    AX_BOOL         GetDetectModelAttrStr(AX_CHAR* pOutBuf, AX_U32 nSize);
    AX_BOOL         GetEventsStr(AX_CHAR* pOutBuf, AX_U32 nSize);
    AI_DETECT_PUSH_MODE_TYPE_E TransPushMode(const AX_CHAR* pszPushMode);

    AX_BOOL         InitAiAttr();
    HOTBALANCE_ATTR_T GetHotBalanceAttr();
    AX_VOID         SetHotBalanceAttr(const HOTBALANCE_ATTR_T& tAttr);
    AX_BOOL         InitHotBalanceAttr();

    AX_S32          Snapshot(AX_U32 nChn, AX_U32 nQpLevel, SNAPSHOT_CALLBACK_FUNC callback);
    AX_BOOL         IsSnapshotOpen();

    AX_BOOL         StatVencOutBytes(AX_U32 nVencInner, AX_U32 nBytes);
    AX_BOOL         GetAssistBitrateStr(AX_U32 nVencInner, AX_CHAR* pOutBuf, AX_U32 nSize);
    AX_BOOL         GetAssistResStr(AX_U32 nUniChn, AX_CHAR* pOutBuf, AX_U32 nSize);

private:
    AX_BOOL Init();

private:
    std::mutex      m_mtxOption;
    std::mutex      m_mtxAi;
    std::mutex      m_mtxHotBalance;
    std::mutex      m_mtxSnapshot;
    CAMERA_ATTR_T   m_tCamera;
    AI_ATTR_T       m_tAiAttr;
    HOTBALANCE_ATTR_T m_tHotBalanceAttr;
    AX_BOOL         m_bSnapshotOpen;

    std::map<AX_U32, STATISTICS_INFO_T> m_mapStatInfo;

    // channle id : VIDEO_ATTR
    std::map<AX_U32, VIDEO_ATTR_T> m_mapVideo;
};

#endif /* _STAGE_OPTION_HELPER_H__ */
