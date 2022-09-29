/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef _CONFIG_PARSER_H__
#define _CONFIG_PARSER_H__

#include "Singleton.h"
#include "BaseSensor.h"
#include "VideoEncoder.h"
#include "JpgEncoder.h"
#include "HotBalance.h"

using namespace std;

class IConfigFileParser
{
public:
    virtual ~IConfigFileParser() = default;

public:
    virtual AX_BOOL Init(const string &strPath) = 0;
    virtual AX_BOOL GetVideoCfgByID(const AX_U32 &nID, VIDEO_CONFIG_T &stOutCfg) = 0;
    virtual AX_BOOL GetCameraCfg(SENSOR_CONFIG_T &stOutCfg, SENSOR_ID_E eSensorID) = 0;
    virtual AX_BOOL GetJpegCfg(const string& strType, const AX_U32 &nID, JPEG_CONFIG_T &stOutCfg) = 0;
    virtual AX_BOOL GetAiCfg(const AX_U32 &nID, AI_CONFIG_T &stOutCfg) = 0;
    virtual AX_BOOL GetHotBalanceCfg(HOTBALANCE_CONFIG_T &stOutCfg) = 0;
};

class CConfigParser: public CSingleton<CConfigParser>
{
    friend class CSingleton<CConfigParser>;

public:
    CConfigParser(AX_VOID);
    ~CConfigParser(AX_VOID);

    AX_BOOL Init();

    AX_BOOL GetVideoCfgByID(const AX_U32 &nID, VIDEO_CONFIG_T &stOutCfg);
    AX_BOOL GetCameraCfg(SENSOR_CONFIG_T &stOutCfg, SENSOR_ID_E eSensorID);
    AX_BOOL GetJpegCfg(const string& strType, const AX_U32 &nID, JPEG_CONFIG_T &stOutCfg);
    AX_BOOL GetAiCfg(const AX_U32 &nID, AI_CONFIG_T &stOutCfg);
    AX_BOOL GetHotBalanceCfg(HOTBALANCE_CONFIG_T &stOutCfg);

public:
    IConfigFileParser* m_pParser;
};

#endif // _CONFIG_PARSER_H__
