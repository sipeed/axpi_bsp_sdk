/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "ConfigParser.h"
#include "JsonCfgParser.h"
#include "OptionHelper.h"

extern COptionHelper gOptions;

CConfigParser::CConfigParser(AX_VOID)
{
    m_pParser = new CJsonCfgParser();
}

CConfigParser::~CConfigParser(AX_VOID)
{
    SAFE_DELETE_PTR(m_pParser);
}

AX_BOOL CConfigParser::Init()
{
    if (!m_pParser->Init(gOptions.GetJsonFile())) {
        LOG_M_E("MAIN", "Failed to parse config file: %s", gOptions.GetJsonFile().c_str());
        return AX_FALSE;
    }

    return AX_TRUE;
}

AX_BOOL CConfigParser::GetVideoCfgByID(const AX_U32 &nID, VIDEO_CONFIG_T &stOutCfg)
{
    if (m_pParser) {
        return m_pParser->GetVideoCfgByID(nID, stOutCfg);
    }

    return AX_FALSE;
}

AX_BOOL CConfigParser::GetCameraCfg(SENSOR_CONFIG_T &stOutCfg, SENSOR_ID_E eSensorID)
{
    if (m_pParser) {
        return m_pParser->GetCameraCfg(stOutCfg, eSensorID);
    }

    return AX_FALSE;
}

AX_BOOL CConfigParser::GetJpegCfg(const string& strType, const AX_U32 &nID, JPEG_CONFIG_T &stOutCfg)
{
    if (m_pParser) {
        return m_pParser->GetJpegCfg(strType, nID, stOutCfg);
    }

    return AX_FALSE;
}

AX_BOOL CConfigParser::GetAiCfg(const AX_U32 &nID, AI_CONFIG_T &stOutCfg)
{
    if (m_pParser) {
        return m_pParser->GetAiCfg(nID, stOutCfg);
    }

    return AX_FALSE;
}

AX_BOOL CConfigParser::GetHotBalanceCfg(HOTBALANCE_CONFIG_T &stOutCfg)
{
    if (m_pParser) {
        return m_pParser->GetHotBalanceCfg(stOutCfg);
    }

    return AX_FALSE;
}
