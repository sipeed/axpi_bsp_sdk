/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "JsonCfgParser.h"
#include "OptionHelper.h"
#include "picojson.h"

#include <fstream>
#include <sstream>
#include <string>
#include <string.h>

#define JSON "JSON"

extern COptionHelper gOptions;

CJsonCfgParser::CJsonCfgParser(){};

CJsonCfgParser::~CJsonCfgParser(){};

AX_BOOL CJsonCfgParser::Init(const string &strPath)
{
    ifstream ifs(strPath.c_str());
    picojson::value v;
    ifs >> v;

    string err = picojson::get_last_error();
    if (!err.empty()) {
        LOG_M_E(JSON, "Failed to load json config file: %s", strPath.c_str());
        return AX_FALSE;
    }

    if (!v.is<picojson::object>()) {
        LOG_M_E(JSON, "Loaded config file is not in JSON formatted JSON.");
        return AX_FALSE;
    }

    return LoadConfig(strPath);
}

AX_BOOL CJsonCfgParser::LoadConfig(const string &strPath)
{
    picojson::value obj;
    ifstream fIn(strPath.c_str());
    if (!fIn.is_open()) {
        return AX_FALSE;
    }

    string strParseRet = picojson::parse(obj, fIn);
    if (!strParseRet.empty() || !obj.is<picojson::object>()) {
        return AX_FALSE;
    }

    m_cfgRoot = obj.get<picojson::object>();

    return AX_TRUE;
}

template <typename T>
AX_BOOL CJsonCfgParser::FindValue(const picojson::value::object &obj, std::string key, T& outValue)
{
    for (picojson::value::object::const_iterator i = obj.begin();
            i != obj.end(); ++i) {
        if (i->first == key && i->second.is<T>()) {
            outValue = i->second.get<T>();
            return AX_TRUE;
        }
    }

    return AX_FALSE;
}

AX_BOOL CJsonCfgParser::GetVideoCfgByID(const AX_U32 &nID, VIDEO_CONFIG_T &stOutCfg)
{
    picojson::array &arrModules = m_cfgRoot["modules"].get<picojson::array>();
    picojson::object objMediaCodec;
    for (size_t i = 0; i < arrModules.size(); i++) {
        auto &it = arrModules[i].get<picojson::object>();
        if (it["type"].get<std::string>() == "venc") {
            objMediaCodec = it;
            break;
        }
    }

    picojson::array &codec_instance = objMediaCodec["instance"].get<picojson::array>();
    picojson::object objMediaIns;
    for (size_t i = 0; i < codec_instance.size(); i++) {
        auto &it = codec_instance[i].get<picojson::object>();
        if (it["id"].get<double>() == nID) {
            objMediaIns = it;
            break;
        }
    }

    if (objMediaIns.empty()) {
        LOG_M_E(JSON, "Video: id=%d has not been configured!!!", nID);
        return AX_FALSE;
    }

    stOutCfg.ePayloadType = objMediaCodec["encoder"].get<std::string>() == "h265" ? PT_H265 : PT_H264;
    stOutCfg.eImgFormat = AX_YUV420_SEMIPLANAR;
    stOutCfg.nBitrate = objMediaIns["bitrate"].get<double>();
    stOutCfg.nGOP = objMediaIns["gop"].get<double>();
    stOutCfg.nFrameRate = objMediaIns["fps"].get<double>();

    picojson::object rc = objMediaIns["rc"].get<picojson::object>();

    std::string strRCType = rc["type"].get<std::string>();
    if (strRCType == "CBR" || strRCType == "cbr") {
        stOutCfg.stRCInfo.eRCType = RcType::VENC_RC_CBR;
    } else if (strRCType == "VBR" || strRCType == "vbr") {
        stOutCfg.stRCInfo.eRCType = RcType::VENC_RC_VBR;
    } else if (strRCType == "FIXQP" || strRCType == "fixqp") {
        stOutCfg.stRCInfo.eRCType = RcType::VENC_RC_FIXQP;
    }

    stOutCfg.stRCInfo.nMinQp = rc["min_qp"].get<double>();
    stOutCfg.stRCInfo.nMaxQp = rc["max_qp"].get<double>();
    stOutCfg.stRCInfo.nMinIQp = rc["min_iqp"].get<double>();
    stOutCfg.stRCInfo.nMaxIQp = rc["max_iqp"].get<double>();
    stOutCfg.stRCInfo.nMinIProp = rc["min_i_prop"].get<double>();
    stOutCfg.stRCInfo.nMaxIProp = rc["max_i_prop"].get<double>();
    stOutCfg.stRCInfo.nIntraQpDelta = rc["intra_qp_delta"].get<double>();

    return AX_TRUE;
}

AX_BOOL CJsonCfgParser::GetJpegCfg(const string& strType, const AX_U32 &nID, JPEG_CONFIG_T &stOutCfg)
{
    picojson::array &arrModules = m_cfgRoot["modules"].get<picojson::array>();
    picojson::object objMediaCodec;
    for (size_t i = 0; i < arrModules.size(); i++) {
        auto &it = arrModules[i].get<picojson::object>();
        if (it["type"].get<std::string>() == strType) {
            objMediaCodec = it;
            break;
        }
    }

    if (objMediaCodec.empty()) {
        LOG_M_E(JSON, "Can not find configuration by type [%s]", strType.c_str());
        return AX_FALSE;
    }

    picojson::array &codec_instance = objMediaCodec["instance"].get<picojson::array>();
    picojson::object objMediaIns;
    for (size_t i = 0; i < codec_instance.size(); i++) {
        auto &it = codec_instance[i].get<picojson::object>();
        if (it["id"].get<double>() == nID) {
            objMediaIns = it;
            break;
        }
    }

    if (objMediaIns.empty()) {
        LOG_M_E(JSON, "Can not find configuration by type [%s], id [%d]", strType.c_str(), nID);
        return AX_FALSE;
    }

    stOutCfg.eImgFormat = AX_YUV420_SEMIPLANAR;

    // if (objMediaIns.end() != objMediaIns.find("fps")) {
    //     stOutCfg.nFrameRate = objMediaIns["fps"].get<double>();
    // }
    // if (objMediaIns.end() != objMediaIns.find("bitrate")) {
    //     stOutCfg.nBitrate = objMediaIns["bitrate"].get<double>();
    // }
    if (objMediaIns.end() != objMediaIns.find("qplevel")) {
        stOutCfg.nQpLevel = objMediaIns["qplevel"].get<double>();
    }
    // if (objMediaIns.end() != objMediaIns.find("fixqp")) {
    //     stOutCfg.nFixQP = objMediaIns["fixqp"].get<double>();
    // }

    // if (objMediaIns.end() != objMediaIns.find("rc")) {
    //     std::string strRCType = objMediaIns["rc"].get<std::string>();
    //     if (strRCType == "CBR" || strRCType == "cbr") {
    //         stOutCfg.eRCType = RcType::VENC_RC_CBR;
    //     } else if (strRCType == "VBR" || strRCType == "vbr") {
    //         stOutCfg.eRCType = RcType::VENC_RC_VBR;
    //     } else if (strRCType == "FIXQP" || strRCType == "fixqp") {
    //         stOutCfg.eRCType = RcType::VENC_RC_FIXQP;
    //     }
    // }

    // LOG_M(JSON, "[%s]: input_width: %d, input_height: %d, bitrate: %d, rc: %d, qplevel: %d, fixqp: %d",
    //             strType.c_str(),
    //             stOutCfg.nInWidth,
    //             stOutCfg.nInHeight,
    //             stOutCfg.nBitrate,
    //             stOutCfg.eRCType,
    //             stOutCfg.nQpLevel,
    //             stOutCfg.nFixQP);

    return AX_TRUE;
}

AX_BOOL CJsonCfgParser::GetCameraCfg(SENSOR_CONFIG_T &stOutCfg, SENSOR_ID_E eSensorID)
{
    picojson::array &arrModules = m_cfgRoot["modules"].get<picojson::array>();
    picojson::object objCamera;

    for (size_t i = 0; i < arrModules.size(); i++) {
        auto &it = arrModules[i].get<picojson::object>();
        if (it["type"].get<std::string>() == "camera") {
            objCamera = it;
            break;
        }
    }

    picojson::array &arrCameraIns = objCamera["instance"].get<picojson::array>();
    picojson::object objSensor;
    bool bCameraFound = false;
    for (size_t i = 0; i < arrCameraIns.size(); i++) {
        auto &it = arrCameraIns[i].get<picojson::object>();
        if (((SENSOR_ID_E)it["id"].get<double>() == eSensorID)) {
            objSensor = it;
            bCameraFound = true;
            break;
        }
    }

    if (!bCameraFound) {
        LOG_M_E(JSON, "Camera configuration not found!");
        return AX_FALSE;
    }

    stOutCfg.eSensorType = (SNS_TYPE_E)objSensor["sns_type"].get<double>();
    stOutCfg.eSensorMode = (AX_SNS_HDR_MODE_E)objSensor["sns_mode"].get<double>();
    stOutCfg.nFrameRate = objSensor["frame_rate"].get<double>();
    stOutCfg.eRunMode = objSensor["run_mode"].get<double>() == 0 ? AX_ISP_PIPELINE_NORMAL : AX_ISP_PIPELINE_NONE_NPU;
    stOutCfg.bTuning = objSensor["tuning_ctrl"].get<double>() == 0 ? AX_FALSE : AX_TRUE;
    stOutCfg.nTuningPort = objSensor["tuning_port"].get<double>();
    if (objSensor.end() != objSensor.find("normal_mode_bin")) {
        strncpy(stOutCfg.aNormalModeBin, objSensor["normal_mode_bin"].get<std::string>().c_str(), SENSOR_BIN_PATH_LEN - 1);
    }
    if (objSensor.end() != objSensor.find("hotbalance_mode_bin")) {
        strncpy(stOutCfg.aHotbalanceModeBin, objSensor["hotbalance_mode_bin"].get<std::string>().c_str(), SENSOR_BIN_PATH_LEN - 1);
    }

    // EIS config
    if (objSensor.end() != objSensor.find("eis_enable")) {
        stOutCfg.bEnableEIS = objSensor["eis_enable"].get<double>() == 1 ? AX_TRUE : AX_FALSE;
    }
    if (objSensor.end() != objSensor.find("eis_sdr_bin")) {
        strncpy(stOutCfg.aEISSdrBin, objSensor["eis_sdr_bin"].get<std::string>().c_str(), SENSOR_BIN_PATH_LEN - 1);
    }
    if (objSensor.end() != objSensor.find("eis_hdr_bin")) {
        strncpy(stOutCfg.aEISHdrBin, objSensor["eis_hdr_bin"].get<std::string>().c_str(), SENSOR_BIN_PATH_LEN - 1);
    }

    LOG_M(JSON, "[%d] sns type:%d, sns mode:%d", eSensorID, stOutCfg.eSensorType, stOutCfg.eSensorMode);

    return AX_TRUE;
}

AX_BOOL CJsonCfgParser::GetAiCfg(const AX_U32 &nID, AI_CONFIG_T &stOutCfg)
{
    picojson::array &arrModules = m_cfgRoot["modules"].get<picojson::array>();
    picojson::object objMediaCodec;
    for (size_t i = 0; i < arrModules.size(); i++) {
        auto &it = arrModules[i].get<picojson::object>();
        if (it["type"].get<std::string>() == "ai") {
            objMediaCodec = it;
            break;
        }
    }

    if (objMediaCodec.empty()) {
        LOG_M(JSON, "module(ai) has not been configured, will use default");
        return AX_FALSE;
    }

    picojson::array &codec_instance = objMediaCodec["instance"].get<picojson::array>();
    picojson::object objMediaIns;
    for (size_t i = 0; i < codec_instance.size(); i++) {
        auto &it = codec_instance[i].get<picojson::object>();
        if (it["id"].get<double>() == nID) {
            objMediaIns = it;
            break;
        }
    }

    if (objMediaIns.empty()) {
        LOG_M(JSON, "AI: id=%d has not been configured, will use default", nID);
        return AX_FALSE;
    }

    stOutCfg.nWidth = objMediaIns["width"].get<double>();
    stOutCfg.nHeight = objMediaIns["height"].get<double>();
    stOutCfg.nDetectFps = objMediaIns["detect_fps"].get<double>();
    stOutCfg.nAiFps = objMediaIns["ai_fps"].get<double>();
    stOutCfg.nIvesFps = objMediaIns["ives_fps"].get<double>();

    return AX_TRUE;
}

AX_BOOL CJsonCfgParser::GetHotBalanceCfg(HOTBALANCE_CONFIG_T &stOutCfg)
{
    picojson::array &arrModules = m_cfgRoot["modules"].get<picojson::array>();
    picojson::object objHotbalance;
    for (size_t i = 0; i < arrModules.size(); i++) {
        auto &it = arrModules[i].get<picojson::object>();
        if (it["type"].get<std::string>() == "hotbalance") {
            objHotbalance = it;
            break;
        }
    }

    if (objHotbalance.empty()) {
        LOG_M(JSON, "module(HotBalance) not configured, will use default");
        return AX_FALSE;
    }

    if (objHotbalance.end() == objHotbalance.find("instance")) {
        LOG_M(JSON, "instance(HotBalance) not configured, will use default");
        return AX_FALSE;
    }

    picojson::object ObjInstance = objHotbalance["instance"].get<picojson::object>();
    if (ObjInstance.empty()) {
        LOG_M(JSON, "instance empty, will use default");
        return AX_FALSE;
    }

    // common setting
    stOutCfg.bEnable = (AX_BOOL)ObjInstance["enable"].get<bool>();
    stOutCfg.eBalanceLevel = (HOTBALANCE_BALANCE_LEVEL_E)ObjInstance["balance_level"].get<double>();
    stOutCfg.fThersholdM = (AX_F32)ObjInstance["median"].get<double>();
    stOutCfg.fThersholdL = (AX_F32)ObjInstance["low"].get<double>();
    stOutCfg.fGap = (AX_F32)ObjInstance["gap"].get<double>();

    // camera
    if (ObjInstance.end() != ObjInstance.find("camera")) {
        picojson::array &camera_instance = ObjInstance["camera"].get<picojson::array>();
        for (size_t i = 0; i < camera_instance.size(); i++) {
            auto &it = camera_instance[i].get<picojson::object>();
            if (it["id"].get<double>() == i && i < E_SENSOR_ID_MAX) {
                stOutCfg.tCameraConf[i].bValid = AX_TRUE;
                stOutCfg.tCameraConf[i].bSdrOnly = (AX_BOOL)it["sdr_only"].get<bool>();
                stOutCfg.tCameraConf[i].fSnsFps = (AX_F32)it["sns_fps"].get<double>();
            }
        }
    }
    else {
        LOG_M(JSON, "camera not configure");
    }

    // venc
    if (ObjInstance.end() != ObjInstance.find("venc")) {
        picojson::array &venc_instance = ObjInstance["venc"].get<picojson::array>();
        for (size_t i = 0; i < venc_instance.size(); i++) {
            auto &it = venc_instance[i].get<picojson::object>();
            if (it["id"].get<double>() == i && i < MAX_VENC_CHANNEL_NUM) {
                stOutCfg.tVencConf[i].bValid = AX_TRUE;
                stOutCfg.tVencConf[i].fVencFps = (AX_F32)it["venc_fps"].get<double>();
            }
        }
    }
    else {
        LOG_M(JSON, "venc not configured");
    }

    // ai
    if (ObjInstance.end() != ObjInstance.find("ai")) {
        picojson::array &ai_instance = ObjInstance["ai"].get<picojson::array>();
        for (size_t i = 0; i < ai_instance.size(); i++) {
            auto &it = ai_instance[i].get<picojson::object>();
            if (it["id"].get<double>() == i && i < MAX_AI_CHANNEL_NUM) {
                stOutCfg.tAiConf[i].bValid = AX_TRUE;
                stOutCfg.tAiConf[i].fDetectFps = (AX_F32)it["detect_fps"].get<double>();
                stOutCfg.tAiConf[i].fAiFps = (AX_F32)it["ai_fps"].get<double>();
                stOutCfg.tAiConf[i].fIvesFps = (AX_F32)it["ives_fps"].get<double>();
            }
        }
    }
    else {
        LOG_M(JSON, "ai not configured");
    }

    return AX_TRUE;
}
