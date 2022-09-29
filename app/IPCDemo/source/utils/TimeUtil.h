/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#ifndef _TIME_UTIL_D391A98C_A011_41B8_89CE_57790F837EB5_H_
#define _TIME_UTIL_D391A98C_A011_41B8_89CE_57790F837EB5_H_

#include "global.h"
#include <chrono>
#include <signal.h>
#include <time.h>

#define MAX_OSD_STRING_LEN (48)

enum E_OSD_TYPE
{
    E_OSD_TYPE_CHN_NAME = 0,
    E_OSD_TYPE_TIME,
    E_OSD_TYPE_MAX
};

enum E_OSD_TIME_FORMAT
{
    E_OSD_TIME_FMT_24H = 0,
    E_OSD_TIME_FMT_12H,
    E_OSD_TIME_FMT_MAX
};

typedef struct _OSD_CONFIG
{
    E_OSD_TYPE osdType;
    bool overlain;
    uint32_t osdPosX;
    uint32_t osdPosY;
    uint32_t osdFontSize; // 1-10
    E_OSD_TIME_FORMAT osdTimeStampFmt;
} OSD_CONFIG_T, *OSD_CONFIT_PTR;

///////////////////////////////////////////////////////////////////////
class CElapsedTimer final
{
public:
    CElapsedTimer(void);
    ~CElapsedTimer(void);

    void reset(void);
    AX_U32  hh(void)const;
    AX_U32  mm(void)const;
    AX_U32 sec(void)const;
    AX_U32  ms(void)const;
    AX_U64  us(void)const;
    AX_U64  ns(void)const;

private:
    std::chrono::steady_clock::time_point m_begin;
};

///////////////////////////////////////////////////////////////////////
class CTimer
{
public:
    CTimer(void);
    virtual ~CTimer(void);

    AX_BOOL StartTimer(AX_U32 milliseconds);
    void    KillTimer(void);

#ifdef USE_SIGEV_THREAD
    static void sigev_notify_func(union sigval v);
#endif

    /* TimeOut process function */
    virtual void OnTimeOut(void);

private:
    timer_t m_tmid;
    struct sigevent m_evp;
    AX_BOOL m_bStarted;
};

class CTimeUtils
{
public:
    CTimeUtils(void);
    virtual ~CTimeUtils(void);

public:
    static AX_U64  GetTickCount(void);
    static void    msSleep(AX_U32 milliseconds);
    static void    usSleep(AX_U32 microseconds);
    static void    nsSleep(AX_U32 nanoseconds);
    static AX_BOOL GenerateTimeStamp(OSD_CONFIT_PTR pTimeOSDCfg, AX_CHAR* szTimeStamp, AX_U32 nBuffLen);
    static AX_CHAR* GetCurrTimeStr(AX_CHAR* szOut);
    static wchar_t* GetCurrDateStr(wchar_t* szOut, AX_U16 nDateFmt, AX_S32& nOutCharLen);
};

#endif /* _TIME_UTIL_D391A98C_A011_41B8_89CE_57790F837EB5_H_ */