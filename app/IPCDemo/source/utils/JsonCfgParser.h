/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef JSONCFGPARSER_H
#define JSONCFGPARSER_H

#include "global.h"
#include "picojson.h"
#include "BaseSensor.h"
#include "JpgEncoder.h"
#include "VideoEncoder.h"
#include "ConfigParser.h"

using namespace std;

class CJsonCfgParser: public IConfigFileParser
{
public:
    CJsonCfgParser();
    virtual ~CJsonCfgParser();

public:
    AX_BOOL Init(const string &strPath);
    picojson::object GetCfgRoot() { return m_cfgRoot; };
    AX_BOOL GetVideoCfgByID(const AX_U32 &nID, VIDEO_CONFIG_T &stOutCfg);
    AX_BOOL GetCameraCfg(SENSOR_CONFIG_T &stOutCfg, SENSOR_ID_E eSensorID);
    AX_BOOL GetJpegCfg(const string& strType, const AX_U32 &nID, JPEG_CONFIG_T &stOutCfg);
    AX_BOOL GetAiCfg(const AX_U32 &nID, AI_CONFIG_T &stOutCfg);
    AX_BOOL GetHotBalanceCfg(HOTBALANCE_CONFIG_T &stOutCfg);

    template <typename T>
    static AX_BOOL FindValue(const picojson::value::object &obj, std::string key, T& outValue);

protected:
    AX_BOOL LoadConfig(const string &strPath);

private:
    picojson::object m_cfgRoot;
};

#endif // JSONCFGPARSER_H
