/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "WebServer.h"

#include <unordered_map>

#include "arraysize.h"
#include "Mpeg4Encoder.h"
#include "appweb.h"
#include "OptionHelper.h"
#include "StageOptionHelper.h"
#include "StringUtils.h"
#include "FileUtils.h"
#include "PrintHelper.h"
#include "Camera.h"
#include "Search.h"
#include "Detector.h"
#include "Md.h"
#include "Od.h"

#define WEB "WEB SERVER"

#define RESPONSE_STATUS_OK "200"
#define RESPONSE_STATUS_AUTH_FAIL "401"
#define RESPONSE_STATUS_INVALID_REQ "400"
#define DETECTION_ENABLE "true"

#define PARAM_KEY_DATA "data"
#define PARAM_KEY_META "meta"
#define PARAM_KEY_TOKEN "token"
#define PARAM_KEY_AUTH "Authorization"
#define PARAM_KEY_STATUS "status"
#define PARAM_KEY_AI_ENABLE "ai_enable"
#define PARAM_KEY_SHOWRECT "showrect"
#define PARAM_KEY_SEARCHIMG "searchimg"
#define PARAM_KEY_DETECT_MODE "detect_mode"
#define PARAM_KEY_NR "nr"

#define PARAM_KEY_LINE "drawLine"
#define PARAM_KEY_RECT "drawRect"
#define PARAM_KEY_POLYGON "drawPolygon"
#define PARAM_KEY_MOSAIC "drawMosaic"
#define PARAM_KEY_ROTATION "rotation"
#define PARAM_KEY_ENABLE "enable"
#define PARAM_KEY_LINE_WIDTH "lineWidth"
#define PARAM_KEY_LINE_COLOR "color"
#define PARAM_KEY_RECT_WIDTH "lineWidth"
#define PARAM_KEY_RECT_SOLID "solid"
#define PARAM_KEY_POLY_WIDTH "lineWidth"
#define PARAM_KEY_POLY_SOLID "solid"
#define PARAM_KEY_MOSAIC_BLOCK_SIZE "blockSize"
#define PARAM_KEY_ROTATION_DEGREE "rotation"

#define PARAM_KEY_USERNAME "username"
#define PARAM_KEY_PWD "password"
#define PARAM_KEY_PREVIEW_SOURCE "src"
#define PARAM_KEY_PREVIEW_CHANNEL "stream"
#define PARAM_KEY_PREVIEW_FACEINFO "faceinfo"

#define PARAM_KEY_APP_VERSION "appVersion"
#define PARAM_KEY_SDK_VERSION "sdkVersion"

#define UNKNOWN_CHANNEL  0


extern string        g_SDKVersion;
extern COptionHelper gOptions;
extern CPrintHelper  gPrintHelper;
extern CCamera       g_camera;
extern CIVPSStage    g_stageIVPS;
extern END_POINT_OPTIONS g_tEPOptions[MAX_VENC_CHANNEL_NUM];
extern CWebServer g_webserver;

static std::unordered_map<string, string> g_mapUserInfo;
static std::unordered_map<string, string> g_mapUser2Token;
static std::unordered_map<string, AX_U16> g_mapToken2WSData;

static MprList* g_pClients = nullptr;
static AX_U8 s_nCurrPrevChn = 0;
static struct MprEvent* g_pSendDataTimer = nullptr;


/* http event callback */
static void SendData(HttpConn *stream, AX_VOID *data)
{
    // LOG_M(WEB, "+++");
    CAXRingElement* pData = (CAXRingElement*)data;
    if (pData == nullptr) {
        return;
    }

    do {
        if (stream == nullptr || stream->connError || stream->timeout != 0 || !pData->pBuf  || pData->nSize == 0) {
            break;
        }

        ssize nRet = httpSendBlock(stream, WS_MSG_BINARY, (cchar*)pData->pBuf, pData->nSize, HTTP_BLOCK);
        if (nRet >= 0) {
            break;
        }
        switch (nRet) {
            case MPR_ERR_TIMEOUT:
                LOG_M_E(WEB, "httpSendBlock() return ERR_TIMEOUT.");
                break;
            case MPR_ERR_MEMORY:
                LOG_M_E(WEB, "httpSendBlock() return ERR_MEMORY.");
                break;
            case MPR_ERR_BAD_STATE:
                LOG_M_E(WEB, "httpSendBlock() return MPR_ERR_BAD_STATE.");
                break;
            case MPR_ERR_BAD_ARGS:
                LOG_M_E(WEB, "httpSendBlock() return MPR_ERR_BAD_ARGS.");
                break;
            case MPR_ERR_WONT_FIT:
                LOG_M_E(WEB, "httpSendBlock() return MPR_ERR_WONT_FIT.");
                break;
            default:
                LOG_M_E(WEB, "httpSendBlock failed.");
                break;
        }
    } while (false);

    if (!stream) {
        // LOG_M_E(WEB, "SendData(): stream is null");
        /* Stream destroyed. Release any custom Msg resources if required here */
    }

    if (pData) {
        pData->pParent->Free(pData);
    }
    // LOG_M(WEB, "---");
}


static bool CheckUser(HttpConn *conn, cchar* user, cchar* pwd)
{
    std::unordered_map<string, string>::iterator itFind = g_mapUserInfo.find(user);
    if (itFind != g_mapUserInfo.end()) {
        return strcmp(itFind->second.c_str(), pwd) == 0;
    }

    return false;
}

static cchar* GenToken(string user, string pwd)
{
    uint64 nTickcount = mprGetHiResTicks();
    string strTokenKey = sfmt("%s_%lld", user.c_str(), nTickcount);
    g_mapUser2Token[strTokenKey] = mprGetSHABase64(sfmt("token:%s-%s-%lld", user.c_str(), pwd.c_str(), nTickcount));

    return g_mapUser2Token[strTokenKey].c_str();
}

static MprJson* ConstructBaseResponse(cchar* pszStatus, cchar* pszToken)
{
    MprJson* pResponseBody = mprCreateJson(MPR_JSON_OBJ);
    mprWriteJson(pResponseBody, PARAM_KEY_DATA, "", MPR_JSON_OBJ);
    if (pszToken) {
        mprWriteJson(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), PARAM_KEY_TOKEN, pszToken, MPR_JSON_STRING);
    }
    mprWriteJson(pResponseBody, PARAM_KEY_META, "", MPR_JSON_OBJ);
    mprWriteJson(mprGetJsonObj(pResponseBody, PARAM_KEY_META), PARAM_KEY_STATUS, pszStatus, MPR_JSON_NUMBER);

    return pResponseBody;
}

static cchar* GetTokenFromConn(HttpConn* conn, AX_BOOL bFromHeader)
{
    if (!conn) {
        return nullptr;
    }

    cchar* szToken = nullptr;
    if (bFromHeader) {
        szToken = (httpGetHeader(conn, PARAM_KEY_AUTH));
    } else {
        szToken = (httpGetParam(conn, PARAM_KEY_TOKEN, nullptr));
    }

    char* p = nullptr;
    while ((p = schr(szToken, ' ')) != 0) {
        *p = '+';
    }

    return szToken;
}

static AX_BOOL IsAuthorized(HttpConn* conn, AX_BOOL bGetTokenFromHeader)
{
    cchar* szToken = GetTokenFromConn(conn, bGetTokenFromHeader);
    if (0 == szToken || strlen(szToken) == 0) {
        return AX_FALSE;
    }

    for (const auto& kv : g_mapUser2Token) {
        if (strcmp(kv.second.c_str(), szToken) == 0) {
            return AX_TRUE;
        }
    }

    return AX_FALSE;
}

static AX_VOID ResponseUnauthorized(HttpConn* conn)
{
    HttpQueue* q = conn->writeq;
    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_AUTH_FAIL, 0);
    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(q, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void LoginAction(HttpConn *conn)
{
    LOG_M(WEB, "+++");

    cchar* strUser = httpGetParam(conn, PARAM_KEY_USERNAME, "unspecified");
    cchar* strPwd = httpGetParam(conn, PARAM_KEY_PWD, "unspecified");

    cchar* szToken = nullptr;
    string strStatus;
    bool bAuthRet = CheckUser(conn, strUser, strPwd);
    if (!bAuthRet) {
        strStatus = RESPONSE_STATUS_AUTH_FAIL;
    } else {
        strStatus = RESPONSE_STATUS_OK;
        szToken = GenToken(strUser, strPwd);
    }

    MprJson* pResponseBody = ConstructBaseResponse(strStatus.c_str(), szToken);

    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);

    LOG_M(WEB, "---");
}

static AX_U8 GetChannelID(AX_U32 nInnerIndex, END_POINT_TYPE eType)
{
    for (const auto& item : g_tEPOptions) {
        if (item.eEPType == eType && item.nInnerIndex == nInnerIndex) {
            return item.nChannel;
        }
    }
    return UNKNOWN_CHANNEL;
}

static void SwitchChnAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    cchar* szChannel = httpGetParam(conn, PARAM_KEY_PREVIEW_CHANNEL, "0");
    s_nCurrPrevChn = atoi(szChannel);
    AX_U8 nUniChn = GetChannelID(s_nCurrPrevChn, E_END_POINT_VENC);
    cchar* szToken = GetTokenFromConn(conn, AX_TRUE);

    LOG_M(WEB, "Switching to channel %d.", s_nCurrPrevChn);
    if (0 != szToken && strlen(szToken) > 0) {
        LOG_M(WEB, "Update token data to %d", nUniChn);
        g_mapToken2WSData[(string)szToken] = (0 | ((nUniChn & 0xFFFF) << 8));
    }

    MprJson* pResponse = ConstructBaseResponse(RESPONSE_STATUS_OK, szToken);
    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponse, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponse, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void PreviewInfoAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    LOG_M(WEB, "Request preview info.");

    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    if (strcmp(conn->rx->method, "GET") == 0) {
        CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
        AI_ATTR_T tAiAttr = pStageOption->GetAiAttr();

        AX_CHAR szData[256] = {0};
        sprintf(szData, "{%s: %s, %s: %s, %s: %s}",
                PARAM_KEY_AI_ENABLE, tAiAttr.nEnable ? "true" : "false",
                PARAM_KEY_SEARCHIMG, gOptions.IsActivedSearch()? "true" : "false",
                PARAM_KEY_DETECT_MODE, pStageOption->GetDetectModelStr().c_str());

        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "info", mprParseJson(szData));

        httpSetContentType(conn, "application/json");
        httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

        LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

        httpSetStatus(conn, 200);
        httpFinalize(conn);
        return;
    }
}

static void FaceinfoAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    AX_S32 status = CSearch::GetInstance()->GetFeatureInfo(NULL);

    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    AX_CHAR szData[256] = {0};
    sprintf(szData, "{status: %d}", status);

    mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "faceinfo", mprParseJson(szData));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void SystemAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    LOG_M(WEB, "Request system info.");

    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    if (strcmp(conn->rx->method, "GET") == 0) {
        string strDispVer(APP_BUILD_VERSION);
        mprWriteJson(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), PARAM_KEY_APP_VERSION, strDispVer.c_str(), MPR_JSON_STRING);
        mprWriteJson(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), PARAM_KEY_SDK_VERSION, g_SDKVersion.c_str(), MPR_JSON_STRING);

        httpSetContentType(conn, "application/json");
        httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

        LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

        httpSetStatus(conn, 200);
        httpFinalize(conn);
        return;
    }
}

static void CameraAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    if (strcmp(conn->rx->method, "GET") == 0) {
        AX_CHAR szAttr[256] = {0};
        if (!pStageOption->GetCameraStr(szAttr, 256)) {
            LOG_M_E(WEB, "Get Camera info failed.");
            return;
        }

        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "camera_attr", mprParseJson(szAttr));
        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "framerate_opts", mprParseJson(pStageOption->GetFramerateOptStr().c_str()));
    } else {
        MprJson* jsonCamera = httpGetParams(conn);
        LOG_M(WEB, "recv:%s", mprJsonToString(jsonCamera, MPR_JSON_QUOTES));

        AX_BOOL bRestartISP = AX_FALSE;
        CAMERA_ATTR_T attr = pStageOption->GetCamera();
        /* Process sensor mode switching */
        AX_U8 nHdrMode = atoi(mprGetJson(jsonCamera, "camera_attr.sns_work_mode"));
        if (nHdrMode != attr.nHdrEnable) {
            attr.nHdrEnable = nHdrMode;
            pStageOption->SetCamera(attr);
            bRestartISP = AX_TRUE;
        }

        AX_U8 nFramerate = atoi(mprGetJson(jsonCamera, "camera_attr.framerate"));
        if (nFramerate != attr.nFramerate) {
            attr.nFramerate = nFramerate;
            pStageOption->SetCamera(attr);
            g_camera.ResetFPS(nFramerate);
        }

        if (bRestartISP) {
            CCamera::RestartNotify();
        }

        /* Process rotation */
        AX_U8 nRotation = atoi(mprGetJson(jsonCamera, "camera_attr.rotation"));
        AX_U8 nMirror = ADAPTER_BOOLSTR2INT(mprGetJson(jsonCamera, "camera_attr.mirror"));
        AX_U8 nFlip = ADAPTER_BOOLSTR2INT(mprGetJson(jsonCamera, "camera_attr.flip"));

        AX_U8 nOldRotation = attr.nRotation;
        AX_U8 nOldMirror = attr.nMirror;
        AX_U8 nOldFlip = attr.nFlip;

        if (nRotation != nOldRotation
            || nMirror != nOldMirror
            || nFlip != nOldFlip) {
            attr.nRotation = nRotation;
            attr.nMirror = nMirror;
            attr.nFlip = nFlip;
            pStageOption->SetCamera(attr);

            if (gOptions.IsEnableOSD()) {
                /* Update logo x-position */
                g_stageIVPS.RefreshOSD(1);
                g_stageIVPS.RefreshOSD(3);
            }

            CIVPSStage::AttrChangeNotify((AX_IVPS_ROTATION_E)nOldRotation, (AX_IVPS_ROTATION_E)attr.nRotation, (AX_BOOL)nMirror, (AX_BOOL)nFlip);
        }

        /* Process daynight mode exchanging */
        AX_U8 nDaynightMode = atoi(mprGetJson(jsonCamera, "camera_attr.daynight"));
        if (nDaynightMode != attr.nDaynightMode) {
            attr.nDaynightMode = nDaynightMode;
            pStageOption->SetCamera(attr);
            CCamera::DaynightChangeNotify((AX_DAYNIGHT_MODE_E)attr.nDaynightMode, bRestartISP ? AX_FALSE : AX_TRUE);
        }

        /* Process nr mode exchanging */
        AX_U8 nNrMode = 0;
        cchar* szEnable = mprGetJson(jsonCamera, "camera_attr.nr_mode");
        if (szEnable && strcmp(szEnable, "true") == 0) {
            nNrMode = 1;
        } else {
            nNrMode = 0;
        }

        if (nNrMode != attr.nNrMode) {
            attr.nNrMode = nNrMode;
            pStageOption->SetCamera(attr);
            CCamera::NrModeChangeNotify(attr.nNrMode, bRestartISP ? AX_FALSE : AX_TRUE);
        }

        /* Process EIS Switch */
        AX_U8 nEISEnable = ADAPTER_BOOLSTR2INT(mprGetJson(jsonCamera, "camera_attr.eis"));
        if (nEISEnable != attr.nEISEnable) {
            AX_BOOL bEISEnable = (1 == nEISEnable) ? AX_TRUE : AX_FALSE;
            if(CCamera::DoEISSwitch(bEISEnable)) {
                attr.nEISEnable = nEISEnable;
                pStageOption->SetCamera(attr);
            }
        }
    }

    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void AiAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    if (strcmp(conn->rx->method, "GET") == 0) {
        AX_CHAR szAiAttr[1024] = {0};
        if (!pStageOption->GetAiInfoStr(szAiAttr, 1024)) {
            LOG_M_E(WEB, "Get AI info failed.");
            return;
        }

        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "ai_attr", mprParseJson(szAiAttr));
    } else {
        MprJson* jsonAI = httpGetParams(conn);
        LOG_M(WEB, "recv:%s", mprJsonToString(jsonAI, MPR_JSON_QUOTES));

        AI_ATTR_T tNewAiAttr = pStageOption->GetAiAttr();
        tNewAiAttr.nEnable = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.ai_enable"));
        cchar* szDetectModel = mprGetJson(jsonAI, "ai_attr.detect_model");
        if (szDetectModel && (string)szDetectModel == "facehuman") {
            tNewAiAttr.eDetectModel = E_AI_DETECT_MODEL_TYPE_FACEHUMAN;
        } else if (szDetectModel && (string)szDetectModel == "hvcfp") {
            tNewAiAttr.eDetectModel = E_AI_DETECT_MODEL_TYPE_HVCFP;
        }

        tNewAiAttr.tConfig.nAiFps = atoi(mprGetJson(jsonAI, "ai_attr.detect_fps"));
        tNewAiAttr.tPushStrgy.eMode     = pStageOption->TransPushMode(mprGetJson(jsonAI, "ai_attr.push_strategy.push_mode"));
        tNewAiAttr.tPushStrgy.nInterval = atoi(mprGetJson(jsonAI, "ai_attr.push_strategy.push_interval"));
        tNewAiAttr.tPushStrgy.nCount    = atoi(mprGetJson(jsonAI, "ai_attr.push_strategy.push_count"));
        tNewAiAttr.tPushStrgy.nPushSameFrame = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.push_strategy.push_same_frame"));

        tNewAiAttr.nDetectOnly         = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.detect_only"));
        if (E_AI_DETECT_MODEL_TYPE_FACEHUMAN == tNewAiAttr.eDetectModel) {
            tNewAiAttr.tHumanFaceSetting.tFace.nEnable      = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.facehuman.face_detect.enable"));
            tNewAiAttr.tHumanFaceSetting.tFace.nDrawRect    = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.facehuman.face_detect.draw_rect"));
            tNewAiAttr.tHumanFaceSetting.tBody.nEnable      = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.facehuman.body_detect.enable"));
            tNewAiAttr.tHumanFaceSetting.tBody.nDrawRect    = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.facehuman.body_detect.draw_rect"));
            tNewAiAttr.tHumanFaceSetting.nEnableFI          = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.facehuman.face_identify.enable"));
        } else if (E_AI_DETECT_MODEL_TYPE_HVCFP == tNewAiAttr.eDetectModel) {
            tNewAiAttr.tHvcfpSetting.tFace.nEnable          = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.face_detect.enable"));
            tNewAiAttr.tHvcfpSetting.tFace.nDrawRect        = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.face_detect.draw_rect"));
            tNewAiAttr.tHvcfpSetting.tBody.nEnable          = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.body_detect.enable"));
            tNewAiAttr.tHvcfpSetting.tBody.nDrawRect        = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.body_detect.draw_rect"));
            tNewAiAttr.tHvcfpSetting.tVechicle.nEnable      = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.vechicle_detect.enable"));
            tNewAiAttr.tHvcfpSetting.tVechicle.nDrawRect    = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.vechicle_detect.draw_rect"));
            tNewAiAttr.tHvcfpSetting.tCycle.nEnable         = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.cycle_detect.enable"));
            tNewAiAttr.tHvcfpSetting.tCycle.nDrawRect       = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.cycle_detect.draw_rect"));
            tNewAiAttr.tHvcfpSetting.tPlate.nEnable         = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.plate_detect.enable"));
            tNewAiAttr.tHvcfpSetting.tPlate.nDrawRect       = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.plate_detect.draw_rect"));
            tNewAiAttr.tHvcfpSetting.nEnablePI              = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.hvcfp.plate_identify.enable"));
        }

        tNewAiAttr.tEvents.tMD.nEnable      = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.events.motion_detect.enable"));
        tNewAiAttr.tEvents.tMD.nThrsHoldY   = atoi(mprGetJson(jsonAI, "ai_attr.events.motion_detect.threshold_y"));
        tNewAiAttr.tEvents.tMD.nConfidence  = atoi(mprGetJson(jsonAI, "ai_attr.events.motion_detect.confidence"));
        tNewAiAttr.tEvents.tOD.nEnable      = ADAPTER_BOOLSTR2INT(mprGetJson(jsonAI, "ai_attr.events.occlusion_detect.enable"));
        tNewAiAttr.tEvents.tOD.nThrsHoldY   = atoi(mprGetJson(jsonAI, "ai_attr.events.occlusion_detect.threshold_y"));
        tNewAiAttr.tEvents.tOD.nConfidence  = atoi(mprGetJson(jsonAI, "ai_attr.events.occlusion_detect.confidence"));

        pStageOption->SetAiAttr(tNewAiAttr);

        CDetector::GetInstance()->UpdateConfig(tNewAiAttr);
        // AiAttrChangeNotify(tNewAiAttr);

        /* MD & OD */
        COD::GetInstance()->SetThresholdY(0, tNewAiAttr.tEvents.tOD.nThrsHoldY, tNewAiAttr.tEvents.tOD.nConfidence);
        CMD::GetInstance()->SetThresholdY(0, tNewAiAttr.tEvents.tMD.nThrsHoldY, tNewAiAttr.tEvents.tMD.nConfidence);

        gOptions.ActiveOcclusionDetect(tNewAiAttr.tEvents.tOD.nEnable ? AX_TRUE : AX_FALSE);
        gOptions.ActiveMotionDetect(tNewAiAttr.tEvents.tMD.nEnable ? AX_TRUE : AX_FALSE);

    }

    LOG_M(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void StorageAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    AX_U32  nStatus = 200;
    AX_BOOL bRecActive = AX_FALSE, bRecLoopCover = AX_FALSE;
    AX_U32  nRecMaxFileSize = 0, nRecMaxFileNum = 0;
    AX_U64  nRecTotalSpace = 0;
    AX_CHAR szRecPath[64 + 1] = {0};

    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);
    if (strcmp(conn->rx->method, "GET") == 0) {

        LOG_M(WEB, "Request storage info.");

        bRecActive      = gOptions.IsActiveMp4Record();
        bRecLoopCover   = gOptions.IsLoopCoverMp4Record();
        nRecTotalSpace  = gOptions.GetMp4TotalSpace();

        gOptions.GetMp4FileInfo(nRecMaxFileSize, nRecMaxFileNum);

        strncpy(szRecPath, gOptions.GetMp4SavedPath().c_str(), sizeof(szRecPath) - 1);

        AX_CHAR szData[256] = {0};
        sprintf(szData, "{rec_actived: %s, rec_loop_cover: %s, rec_total: %lld, rec_threshold: %d, rec_max_count: %d, rec_path: \'%s\'}",
                bRecActive ? "true" : "false",
                bRecLoopCover ? "true" : "false",
                nRecTotalSpace,
                nRecMaxFileSize,
                nRecMaxFileNum,
                szRecPath);

        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "storage", mprParseJson(szData));
    } else {
        cchar* szValue = nullptr;
        MprJson* jsonStorage = httpGetParams(conn);
        LOG_M_I(WEB, "recv:%s", mprJsonToString(jsonStorage, MPR_JSON_QUOTES));

        szValue = mprGetJson(jsonStorage, "storage.rec_actived");
        if (szValue && strcmp(szValue, "true") == 0) {
            bRecActive = AX_TRUE;
        }

        szValue = mprGetJson(jsonStorage, "storage.rec_loop_cover");
        if (szValue && strcmp(szValue, "true") == 0) {
            bRecLoopCover = AX_TRUE;
        }

        szValue = mprGetJson(jsonStorage, "storage.rec_threshold");
        if (szValue) {
            nRecMaxFileSize = atoi(szValue);
        }

        szValue = mprGetJson(jsonStorage, "storage.rec_max_count");
        if (szValue) {
            nRecMaxFileNum = atoi(szValue);
        }

        szValue = mprGetJson(jsonStorage, "storage.rec_path");
        if (szValue) {
            strncpy(szRecPath, szValue, sizeof(szRecPath) - 1);
        }

        if (gOptions.IsEnableMp4Record()) {
            if (0 == CFileUtils::CheckAndMkDir(szRecPath)) {
                AX_U32 _nRecMaxFileSize = 0, _nRecMaxFileNum = 0;
                gOptions.GetMp4FileInfo(_nRecMaxFileSize, _nRecMaxFileNum);

                if (nRecMaxFileSize != _nRecMaxFileSize
                    || nRecMaxFileNum != _nRecMaxFileNum) {

                    CMPEG4Encoder::GetInstance()->Stop();

                    gOptions.SetMp4RecordActived(bRecActive);
                    gOptions.SetLoopCoverMp4Record(bRecLoopCover);
                    gOptions.SetMp4FileInfo(nRecMaxFileSize, nRecMaxFileNum);
                    gOptions.SetMp4SavedPath(szRecPath);

                    if (bRecActive) {
                        CMPEG4Encoder::GetInstance()->Start();
                    }
                }
                else {
                    if (bRecActive != gOptions.IsActiveMp4Record()) {
                        if (bRecActive) {
                            CMPEG4Encoder::GetInstance()->Start();
                        }
                        else {
                            CMPEG4Encoder::GetInstance()->Stop();
                        }
                    }

                    gOptions.SetMp4RecordActived(bRecActive);
                    gOptions.SetLoopCoverMp4Record(bRecLoopCover);
                }
            }
            else {
                // set http error code
                // resource not found
                nStatus = 404;
            }
        }
    }

    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, nStatus);
    httpFinalize(conn);
}

static AX_S32 SnapshotCallback(AX_U32 uChn, AX_VOID *pBuf, AX_U32 nBufferSize)
{
    g_webserver.SendSnapshotData(WS_SNAPSHOT_CHANNEL, pBuf, nBufferSize);

    return 0;
}

static void SnapshotAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    LOG_M(WEB, "Request one snapshot.");

    if (CStageOptionHelper().GetInstance()->IsSnapshotOpen()) {
        LOG_M_W(WEB, "Snapshot already in process, please wait.");
        return;
    }

    CStageOptionHelper().GetInstance()->Snapshot(0, 90, SnapshotCallback);

    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));
    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void AssistInfoAction(HttpConn* conn)
{
    if (!IsAuthorized(conn, AX_TRUE)) {
        ResponseUnauthorized(conn);
        return;
    }

    CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
    MprJson* pResponseBody = ConstructBaseResponse(RESPONSE_STATUS_OK, 0);

    if (strcmp(conn->rx->method, "GET") == 0) {
        AX_U8 nUniChn = GetChannelID(s_nCurrPrevChn, E_END_POINT_VENC);

        AX_CHAR szResolution[16] = {0};
        if (!pStageOption->GetAssistResStr(nUniChn, szResolution, 16)) {
            LOG_M_E(WEB, "Get assist resolution info failed.");
            return;
        }

        AX_CHAR szBitrate[16] = {0};
        if (!pStageOption->GetAssistBitrateStr(s_nCurrPrevChn, szBitrate, 16)) {
            LOG_M_E(WEB, "Get assist bitrate info failed.");
            return;
        }

        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "assist_res", mprParseJson(szResolution));
        mprWriteJsonObj(mprGetJsonObj(pResponseBody, PARAM_KEY_DATA), "assist_bitrate", mprParseJson(szBitrate));
    }

    LOG_M_I(WEB, "resp:%s", mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetContentType(conn, "application/json");
    httpWrite(conn->writeq, mprJsonToString(pResponseBody, MPR_JSON_QUOTES));

    httpSetStatus(conn, 200);
    httpFinalize(conn);
}

static void WebNotifier(HttpConn *conn, int event, int arg)
{
    if (event == HTTP_EVENT_DESTROY) {
        mprRemoveItem(g_pClients, conn);
    }
}

static void WSPreviewAction(HttpConn* conn)
{
    LOG_M(WEB, "WSPreview setup.");
    if (!IsAuthorized(conn, AX_FALSE)) {
        LOG_M_E(WEB, "Unauthorized, try to login again.");
        return;
    }

    cchar* szToken = GetTokenFromConn(conn, AX_FALSE);
    if (nullptr != szToken && strlen(szToken) > 0) {

        AX_U16 nChnID = g_mapToken2WSData[string(szToken)];
        MprBuf *buf = mprCreateBuf(32, 0);
        mprAddRoot(buf);
        mprPutUint16ToBuf(buf, nChnID);
        mprRemoveRoot(buf);

        httpSetWebSocketData(conn, buf);
    }

    mprAddItem(g_pClients, conn);
    httpSetConnNotifier(conn, WebNotifier);
}

static void WSCaptureAction_0(HttpConn* conn)
{
    LOG_M(WEB, "WSCapture_0 setup.");

    if (!IsAuthorized(conn, AX_FALSE)) {
        LOG_M_E(WEB, "Unauthorized, try to login again.");
        return;
    }

    // AX_U16 LOW_BYTE: Sensor ID; HIGH_BYTE: Channel ID
    AX_U16 nChnID = (0x0/*sensor id*/ | ((JENC_CHANNEL_ID & 0x00FF) << 8));

    MprBuf *buf = mprCreateBuf(32, 0);
    mprAddRoot(buf);
    mprPutUint16ToBuf(buf, nChnID);
    mprRemoveRoot(buf);

    httpSetWebSocketData(conn, buf);

    mprAddItem(g_pClients, conn);
    httpSetConnNotifier(conn, WebNotifier);
}

static void WSCaptureAction_1(HttpConn* conn)
{
    LOG_M(WEB, "WSCapture_1 setup.");

    if (!IsAuthorized(conn, AX_FALSE)) {
        LOG_M_E(WEB, "Unauthorized, try to login again.");
        return;
    }

    // AX_U16 LOW_BYTE: Sensor ID; HIGH_BYTE: Channel ID
    AX_U16 nChnID = (0x0/*sensor id*/ | ((WS_SNAPSHOT_CHANNEL & 0x00FF) << 8));

    MprBuf *buf = mprCreateBuf(32, 0);
    mprAddRoot(buf);
    mprPutUint16ToBuf(buf, nChnID);
    mprRemoveRoot(buf);

    httpSetWebSocketData(conn, buf);
    mprAddItem(g_pClients, conn);
    httpSetConnNotifier(conn, WebNotifier);
}

static void WSEventsAction(HttpConn* conn)
{
    LOG_M(WEB, "WSEvents setup.");

    if (!IsAuthorized(conn, AX_FALSE)) {
        LOG_M_E(WEB, "Unauthorized, try to login again.");
        return;
    }

    AX_U16 nChnID = (0x0/*sensor id*/ | ((WS_EVENTS_CHANNEL & 0x00FF) << 8));

    MprBuf *buf = mprCreateBuf(32, 0);
    mprAddRoot(buf);
    mprPutUint16ToBuf(buf, nChnID);
    mprRemoveRoot(buf);

    httpSetWebSocketData(conn, buf);

    mprAddItem(g_pClients, conn);
    httpSetConnNotifier(conn, WebNotifier);
}

typedef struct _HTTP_ACTION_INFO {
    cchar*     name;
    HttpAction action;
} HTTP_ACTION_INFO;

const HTTP_ACTION_INFO g_httpActionInfo[] = {
    {"/action/login",            LoginAction},
    {"/action/preview/info",     PreviewInfoAction},
    {"/action/preview/stream",   SwitchChnAction},
    {"/action/preview/faceinfo", FaceinfoAction},
    {"/action/preview/snapshot", SnapshotAction},
    {"/action/preview/assist",   AssistInfoAction},
    {"/action/setting/system",   SystemAction},
    {"/action/setting/camera",   CameraAction},
    {"/action/setting/storage",  StorageAction},
    {"/action/setting/ai",       AiAction},
    {"/preview",                 WSPreviewAction},
    {"/capture_0",               WSCaptureAction_0},
    {"/capture_1",               WSCaptureAction_1},
    {"/events",                  WSEventsAction},
};

CWebServer::CWebServer(void)
    : m_bServerStarted(AX_FALSE)
    , m_pAppwebThread(nullptr)
{
    for (AX_U32 i = 0; i < MAX_WS_CONN_NUM; i++) {
        m_arrConnStatus[i] = AX_FALSE;
    }
}

CWebServer::~CWebServer(void)
{
    for (AX_U32 i = 0; i < MAX_WS_CONN_NUM; i++) {
        if (m_arrChannelData[i].pRingBuffer) {
            delete m_arrChannelData[i].pRingBuffer;
        }
    }
}

AX_VOID CWebServer::Init(const AX_VIN_CHN_ATTR_T& tAttr)
{
    g_mapUserInfo["admin"] = "admin";

    char szName[64] = {0};
    AX_F32 fRatio = gOptions.GetWebFrmSizeRatio();
    LOG_M(WEB, "FrameSize Ratio: %f", fRatio);

    for (AX_U32 index = 0; index < arraysize(m_arrChannelData); ++index) {
        WS_CHANNEL_TYPE_E eType = GetChnType(index);
        AX_U32 nElementCount = 0;
        AX_U32 nElementBuffSize = 0;
        ChannelData& channelData = m_arrChannelData[index];
        channelData.nChannel = index;

        switch (eType) {
            case E_WS_CHANNEL_TYPE_VENC: {
                nElementCount = AX_WEB_VENC_RING_BUFF_COUNT;

                AX_U8 nISPChn = CIVPSStage::GetIspChnIndex(index);
                nElementBuffSize = tAttr.tChnAttr[nISPChn].nWidthStride * tAttr.tChnAttr[nISPChn].nHeight * 3 / 2 * fRatio; // yuv_size * fRatio
                channelData.nInnerIndex = g_tEPOptions[index].nInnerIndex;
                sprintf(szName, "VENC_CH%d", channelData.nInnerIndex);
                break;
            }
            case E_WS_CHANNEL_TYPE_JENC: {
                nElementCount = AX_WEB_JENC_RING_BUFF_COUNT;
                nElementBuffSize = 300 * 1024;
                channelData.nInnerIndex = g_tEPOptions[index].nInnerIndex;
                sprintf(szName, "JENC_CH%d", channelData.nInnerIndex);
                nElementBuffSize += sizeof(JpegHead);
                break;
            }
            case E_WS_CHANNEL_TYPE_EVENTS: {
                nElementCount = AX_WEB_VENC_RING_BUFF_COUNT;
                nElementBuffSize = MAX_EVENTS_CHN_SIZE;
                channelData.nInnerIndex = WS_EVENTS_CHANNEL;
                sprintf(szName, "EVENTS_CH%d", WS_EVENTS_CHANNEL);
                break;
            }
            case E_WS_CHANNEL_TYPE_SNAPSHOT: {
                nElementCount = AX_WEB_SNAPSHOT_RING_BUFF_COUNT;
                nElementBuffSize = 300 * 1024;
                channelData.nInnerIndex = WS_SNAPSHOT_CHANNEL;
                sprintf(szName, "SNAPSHOT_CH%d", WS_SNAPSHOT_CHANNEL);
                break;
            }
            default: {
                LOG_M_E(WEB, "Not recognized channel type: %d", eType);
                break;
            }
        }

        LOG_M(WEB, "[%d]nElementBuffSize: %d, nElementCount: %d, channel type: %d",
                                    index, nElementBuffSize, nElementCount, eType);

        if (nElementCount > 0 && nElementBuffSize > 0) {
            channelData.pRingBuffer = new CAXRingBuffer(nElementBuffSize, nElementCount, szName);
        }
    }
}

AX_BOOL CWebServer::Start()
{
    LOG_M(WEB, "+++");

    m_pAppwebThread  = new thread(WebServerThreadFunc, this);

    LOG_M(WEB, "---");

    return (m_pAppwebThread) ? AX_TRUE : AX_FALSE;
}

void* CWebServer::WebServerThreadFunc(void* pThis)
{
    LOG_M(WEB, "+++");

    prctl(PR_SET_NAME, "IPC_APPWEB_Main");

    CWebServer* pWebServer = (CWebServer*)pThis;
    Mpr* pMpr = nullptr;

    do {
        if ((pMpr = mprCreate(0, NULL, MPR_USER_EVENTS_THREAD)) == 0) {
            LOG_M_E(WEB, "Cannot create runtime.");
            break;
        }

        if (httpCreate(HTTP_CLIENT_SIDE | HTTP_SERVER_SIDE) == 0) {
            LOG_M_E(WEB, "Cannot create the HTTP services.");
            break;
        }

        if (maLoadModules() < 0) {
            LOG_M_E(WEB, "Cannot load modules.");
            break;
        }
        g_pClients = mprCreateList(0, 0);
        AX_S32 flags = MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK | MPR_EVENT_STATIC_DATA;
        g_pSendDataTimer = mprCreateTimerEvent(NULL, "wsSendData", 1, (void*)CWebServer::OnSendDataTimer, pWebServer, flags);
        mprAddRoot(g_pClients);
        mprStart();

        if (maParseConfig("./config/appweb.conf") < 0) {
            LOG_M_E(WEB, "Cannot parse the config file %s.", "./config/appweb.conf");
            break;
        }

        for (AX_U32 i = 0; i < sizeof(g_httpActionInfo) / sizeof(HTTP_ACTION_INFO); i++) {
            httpDefineAction(g_httpActionInfo[i].name, g_httpActionInfo[i].action);
        }

        if (httpStartEndpoints() < 0) {
            LOG_M_E(WEB, "Cannot start the web server.");
            break;
        }

        pWebServer->m_bServerStarted = AX_TRUE;
        mprServiceEvents(-1, 0);
    } while (false);

    LOG_M(WEB, "---");
    return nullptr;
}

void CWebServer::OnSendDataTimer(void* pThis)
{
    CWebServer* pWebServer = (CWebServer*)pThis;
    AX_U8 nChannel = 0;
    AX_BOOL arrConnStatus[MAX_WS_CONN_NUM] = {AX_FALSE};
    AX_BOOL arrDataStatus[MAX_WS_CONN_NUM] = {AX_FALSE};
    HttpConn* client = nullptr;

    gPrintHelper.Remove(E_PH_MOD_WEB_CONN, 0);
    for (AX_S32 next = 0; (client = (HttpConn *)mprGetNextItem(g_pClients, &next))!= 0;) {
        if (WS_STATE_OPEN != httpGetWebSocketState(client)) {
            continue;
        }
         MprBuf* pWSData = (MprBuf*)httpGetWebSocketData(client);
         if (!pWSData) {
             continue;
         } else {
             AX_U16 nWSData = (AX_U16)mprGetUint16FromBuf(pWSData);
             mprAdjustBufStart(pWSData, -2);  // restore the start pointer to offfset 0
             nChannel = nWSData >> 8;
         }

        if (nChannel >= MAX_WS_CONN_NUM) {
            continue;
        }
        gPrintHelper.Add(E_PH_MOD_WEB_CONN, 0, 0);
        {
            std::lock_guard<std::mutex> guard(pWebServer->m_mutex);
            pWebServer->m_arrConnStatus[nChannel] = AX_TRUE;
        }

        arrConnStatus[nChannel] = AX_TRUE;

        if (!pWebServer->m_arrChannelData[nChannel].pRingBuffer) {
            /* Ringbuff is null */
            continue;
        }

        CAXRingElement *pData = pWebServer->m_arrChannelData[nChannel].pRingBuffer->Get();
        if (!pData) {
            /* Ringbuff is empty */
            continue;
        }

        arrDataStatus[nChannel] = AX_TRUE; // got data
        AX_U32 limit = client->rx->route->limits->webSocketsFrameSize;
        if (pData->nSize >= (AX_U32)limit) {
            LOG_M_E(WEB, "Websocket data size(%d) exceeding max frame size(%d).", pData->nSize, limit);
        }

        if (httpCreateEvent(client->seqno, (HttpEventProc)SendData, pData) < 0) {
            pData->pParent->Free(pData);
        }
    }
    {
        std::lock_guard<std::mutex> guard(pWebServer->m_mutex);
        for (AX_U32 i = 0; i < MAX_WS_CONN_NUM; i++) {
            if (!arrConnStatus[i]) {
                pWebServer->m_arrConnStatus[i] = AX_FALSE;
            }
        }
    }
    for (AX_U32 i = 0; i < MAX_WS_CONN_NUM; i++) {
        if (arrDataStatus[i] && pWebServer->m_arrChannelData[i].pRingBuffer) {
            pWebServer->m_arrChannelData[i].pRingBuffer->Pop(AX_FALSE);
        }
    }
}

AX_BOOL CWebServer::Stop()
{
    LOG_M(WEB, "+++");
    memset(&m_arrConnStatus[0], 0, sizeof(AX_BOOL) * MAX_WS_CONN_NUM);

    if (m_pAppwebThread && m_bServerStarted) {
        mprShutdown(MPR_EXIT_NORMAL, 0, MPR_EXIT_TIMEOUT);

        m_bServerStarted = AX_FALSE;
        m_pAppwebThread->join();
        delete m_pAppwebThread;
        m_pAppwebThread = nullptr;
    }

    LOG_M(WEB, "---");
    return AX_TRUE;
}

AX_VOID CWebServer::StopAction()
{
    HttpStage* stage = nullptr;
    if ((stage = httpLookupStage("actionHandler")) != 0) {
        for (const auto& info : g_httpActionInfo) {
            mprRemoveKey((MprHash*)stage->stageData, (cvoid*)info.name);
        }
    }
}

AX_VOID CWebServer::SendPreviewData(AX_U8 nStreamID, AX_VOID* data, AX_U32 size, AX_U64 nPts/*=0*/, AX_BOOL bIFrame/*=AX_FALSE*/)
{
    if (!m_bServerStarted || IsJencChannel(nStreamID)) {
        return;
    }

    {
        /* Waiting for reading thread to refresh the websock conn status */
        std::lock_guard<std::mutex> guard(m_mutex);
        AX_BOOL bConnect = m_arrConnStatus[nStreamID];
        if (!bConnect) {
            return;
        }
    }

    AX_U8 nChnnelID = m_arrChannelData[nStreamID].nInnerIndex;
    CAXRingElement ele((AX_U8*)data, size, nChnnelID, nPts, bIFrame);
    m_arrChannelData[nStreamID].pRingBuffer->Put(ele);
}


AX_VOID CWebServer::SendCaptureData(AX_U8 nStreamID, AX_VOID* data, AX_U32 size, AX_U64 nPts/*=0*/, AX_BOOL bIFrame/*=AX_TRUE*/,
                                    JpegDataInfo* pJpegInfo /*= nullptr*/)
{
    if (!m_bServerStarted || !IsJencChannel(nStreamID)) {
        return;
    }

    {
        /* Waiting for reading thread to refresh the websock conn status */
        std::lock_guard<std::mutex> guard(m_mutex);
        AX_BOOL bConnect = m_arrConnStatus[nStreamID];
        if (!bConnect) {
           return;
        }
    }

    AX_U8 nChnnelID = m_arrChannelData[nStreamID].nInnerIndex;
    if (nullptr != pJpegInfo) {
        CStageOptionHelper *pStageOption = CStageOptionHelper().GetInstance();
        AI_ATTR_T tAiAttr = pStageOption->GetAiAttr();

        AX_CHAR szJsonData[256] = {0};
        if (tAiAttr.nEnable) {
            /* construct json data info */
            if (E_AI_DETECT_MODEL_TYPE_FACEHUMAN == tAiAttr.eDetectModel) {
                sprintf(szJsonData, "{\"type\": %d, \"attribute\": {\"face\": {\"gender\": %d, \"age\": %d, \"mask\": \"%s\", \"info\": \"%s\"}}}",
                    pJpegInfo->eType,
                    pJpegInfo->tFaceInfo.nGender,
                    pJpegInfo->tFaceInfo.nAge,
                    pJpegInfo->tFaceInfo.szMask,
                    pJpegInfo->tFaceInfo.szInfo);
            } else if (E_AI_DETECT_MODEL_TYPE_HVCFP == tAiAttr.eDetectModel) {
                sprintf(szJsonData, "{\"type\": %d, \"attribute\": {\"plate\": {\"num\": \"%s\", \"color\": \"%s\"}}}",
                    pJpegInfo->eType,
                    pJpegInfo->tPlateInfo.szNum,
                    pJpegInfo->tPlateInfo.szColor);
            }
        }

        AX_U32 nJsnLen = strlen(szJsonData);

        /* construct jpeg head */
        JpegHead tJpegHead;
        tJpegHead.nJsonLen  = nJsnLen > 0 ? nJsnLen + 1 : 0;
        tJpegHead.nTotalLen = 4/*magic*/ + 4/*total len*/ + 4/*tag*/ + 4/*json len*/ + tJpegHead.nJsonLen;

        strcpy(tJpegHead.szJsonData, szJsonData);

        CAXRingElement _ele((AX_U8*)data, size, nChnnelID, nPts, bIFrame, (AX_U8*)&tJpegHead, tJpegHead.nTotalLen);
        m_arrChannelData[nStreamID].pRingBuffer->Put(_ele);
    }
    else {
        CAXRingElement ele((AX_U8*)data, size, nChnnelID, nPts, bIFrame);
        m_arrChannelData[nStreamID].pRingBuffer->Put(ele);
    }
}

AX_VOID CWebServer::SendSnapshotData(AX_U8 nStreamID, AX_VOID* data, AX_U32 size)
{
    if (!m_bServerStarted || nStreamID != WS_SNAPSHOT_CHANNEL) {
        return;
    }

    AX_U8 nChannel = 0;
    HttpConn* client = nullptr;
    for (AX_S32 next = 0; (client = (HttpConn *)mprGetNextItem(g_pClients, &next))!= 0;) {
        if (WS_STATE_OPEN != httpGetWebSocketState(client)) {
            continue;
        }

        MprBuf* pWSData = (MprBuf*)httpGetWebSocketData(client);
        if (!pWSData) {
            continue;
        } else {
            AX_U16 nWSData = (AX_U16)mprGetUint16FromBuf(pWSData);
            mprAdjustBufStart(pWSData, -2);  // restore the start pointer to offfset 0
            nChannel = nWSData >> 8;
        }

        if (nChannel != WS_SNAPSHOT_CHANNEL) {
            continue;
        }

        AX_U32 limit = client->rx->route->limits->webSocketsFrameSize;
        if (size >= (AX_U32)limit) {
            LOG_M_E(WEB, "Websocket data size(%d) exceeding max frame size(%d).", size, limit);
        }

        ssize nRet = httpSendBlock(client, WS_MSG_BINARY, (cchar*)data, size, HTTP_BLOCK);
        if (nRet >= 0) {
            break;
        }
    }
}

AX_VOID CWebServer::SendEventsData(AI_EVENTS_DATA_T* data)
{
    if (!m_bServerStarted) {
        return;
    }

    {
        /* Waiting for reading thread to refresh the websock conn status */
        std::lock_guard<std::mutex> guard(m_mutex);
        AX_BOOL bConnect = m_arrConnStatus[WS_EVENTS_CHANNEL];
        if (!bConnect) {
            return;
        }
    }

    AX_U8 nChnnelID = m_arrChannelData[WS_EVENTS_CHANNEL].nInnerIndex;
    string strEventsJson = FormatEventsJson(data);

    CAXRingElement ele((AX_U8*)strEventsJson.c_str(), strEventsJson.length(), nChnnelID);
    m_arrChannelData[WS_EVENTS_CHANNEL].pRingBuffer->Put(ele);
}

AX_BOOL CWebServer::IsJencChannel(AX_U8 nStreamID)
{
    WS_CHANNEL_TYPE_E eType = GetChnType(nStreamID);

    return (E_WS_CHANNEL_TYPE_JENC == eType ? AX_TRUE : AX_FALSE);
}

WS_CHANNEL_TYPE_E CWebServer::GetChnType(AX_U8 nStreamID)
{
    if (WS_EVENTS_CHANNEL == nStreamID) {
        return E_WS_CHANNEL_TYPE_EVENTS;
    } else if (WS_SNAPSHOT_CHANNEL == nStreamID) {
        return E_WS_CHANNEL_TYPE_SNAPSHOT;
    }  else if (E_END_POINT_VENC == g_tEPOptions[nStreamID].eEPType) {
        return E_WS_CHANNEL_TYPE_VENC;
    } else if (E_END_POINT_JENC == g_tEPOptions[nStreamID].eEPType) {
        return E_WS_CHANNEL_TYPE_JENC;
    } else if (E_END_POINT_DET == g_tEPOptions[nStreamID].eEPType) {
        return E_WS_CHANNEL_TYPE_JENC;
    }

    return E_WS_CHANNEL_TYPE_MAX;
}

std::string CWebServer::FormatEventsJson(AI_EVENTS_DATA_T* pEvent)
{
    AX_CHAR szEventJson[MAX_EVENTS_CHN_SIZE] = {0};
    AX_CHAR szDate[64] = {0};
    CTimeUtils::GetCurrTimeStr(szDate);
    if (E_AI_EVENTS_TYPE_MD == pEvent->eType) {
        snprintf(szEventJson, MAX_EVENTS_CHN_SIZE, "{\"events\": [{\"type\": %d, \"date\": \"%s\", \"info\": \"%s\"}]}", pEvent->eType, szDate, pEvent->tMD.szDisplay);
    } else {
        snprintf(szEventJson, MAX_EVENTS_CHN_SIZE, "{\"events\": [{\"type\": %d, \"date\": \"%s\", \"info\": \"%s\"}]}", pEvent->eType, szDate, pEvent->tOD.szDisplay);
    }

    std::string strEvent;
    strEvent = szEventJson;

    return strEvent;
}
