/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include "TimeUtil.h"
#include "OsdConfig.h"
#include <string.h>
#include <errno.h>
#include <chrono>
using namespace std::chrono;


///////////////////////////////////////////////////////////////////////
CElapsedTimer::CElapsedTimer(void)
{
    reset();
}

CElapsedTimer::~CElapsedTimer(void)
{
}

void CElapsedTimer::reset(void)
{
    m_begin = steady_clock::now();
}

AX_U32 CElapsedTimer::hh(void)const
{
    return duration_cast<hours>(steady_clock::now() - m_begin).count();
}

AX_U32 CElapsedTimer::mm(void)const
{
    return duration_cast<minutes>(steady_clock::now() - m_begin).count();
}

AX_U32 CElapsedTimer::sec(void)const
{
    return duration_cast<seconds>(steady_clock::now() - m_begin).count();
}

AX_U32 CElapsedTimer::ms(void)const
{
    return duration_cast<milliseconds>(steady_clock::now() - m_begin).count();
}

AX_U64 CElapsedTimer::us(void)const
{
    return duration_cast<microseconds>(steady_clock::now() - m_begin).count();
}

AX_U64 CElapsedTimer::ns(void)const
{
    return duration_cast<nanoseconds>(steady_clock::now() - m_begin).count();
}


///////////////////////////////////////////////////////////////////////
#ifndef USE_SIGEV_THREAD
/**
 *  Timer signal process function
 */
static void TimerSignalAction(AX_S32 nSigNo, siginfo_t *pInfo, void *pArgs)
{
    if (nSigNo == SIGUSR2) {
        CTimer *pTimer = (CTimer *)(pInfo->si_value.sival_ptr);
        if (pTimer) {
            pTimer->OnTimeOut();
        }
    }
}
#endif

CTimer::CTimer(void)
    : m_tmid(0)
    , m_bStarted(AX_FALSE)
{
    memset(&m_evp, 0, sizeof(struct sigevent));
}

CTimer::~CTimer(void)
{

}

AX_BOOL CTimer::StartTimer(AX_U32 milliseconds)
{
#ifndef USE_SIGEV_THREAD
    struct sigaction sigact = {0};
    sigemptyset(&sigact.sa_mask);
    /**
     * https://man7.org/linux/man-pages/man7/signal.7.html
     * If a blocked call to one of the following interfaces is interrupted
     * by a signal handler, then the call is automatically restarted after
     * the signal handler returns if the SA_RESTART flag was used; otherwise
     * the call fails with the error EINTR
     */
    sigact.sa_flags = (SA_SIGINFO | SA_RESTART);
    sigact.sa_sigaction = TimerSignalAction;
    sigaction(SIGUSR2, &sigact, NULL);

    m_evp.sigev_value.sival_ptr = this;
    m_evp.sigev_notify = SIGEV_SIGNAL;
    m_evp.sigev_signo = SIGUSR2;
#else
    m_evp.sigev_value.sival_ptr = this;
    m_evp.sigev_notify = SIGEV_THREAD;
    m_evp.sigev_notify_function = CTimer::sigev_notify_func;
    m_evp.sigev_notify_attributes = NULL;
#endif
    if (timer_create(CLOCK_MONOTONIC, &m_evp, &m_tmid) == -1) {
        LOG("timer_create() fail, error: %s\n", strerror(errno));
        return AX_FALSE;
    }

    struct itimerspec it;
    it.it_interval.tv_sec  = (milliseconds / 1000);
    it.it_interval.tv_nsec = (milliseconds % 1000) * 1000000;
    it.it_value = it.it_interval;
    if (timer_settime(m_tmid, 0, &it, NULL) == -1) {
        LOG("timer_settime() fail, error: %s\n", strerror(errno));
        return AX_FALSE;
    }

    m_bStarted = AX_TRUE;
    return m_bStarted;
}

void CTimer::KillTimer(void)
{
    if (m_bStarted) {
        timer_delete(m_tmid);
        m_bStarted = AX_FALSE;
    }
}

#ifdef USE_SIGEV_THREAD
void CTimer::sigev_notify_func(union sigval v)
{
    CTimer *pThis = reinterpret_cast<CTimer *>(v.sival_ptr);
    if (pThis) {
        pThis->OnTimeOut();
    }
}
#endif

void CTimer::OnTimeOut(void)
{
    LOG("TimeOut \n");
}

CTimeUtils::CTimeUtils(void)
{

}

CTimeUtils::~CTimeUtils(void)
{

}

AX_BOOL CTimeUtils::GenerateTimeStamp(OSD_CONFIT_PTR pTimeOSDCfg, AX_CHAR* szTimeStamp, AX_U32 nBuffLen)
{
    time_t t;
    struct tm tm;
    int pm = 0;
    char time_str[MAX_OSD_STRING_LEN] = {0};
    char date_str[MAX_OSD_STRING_LEN] = {0};

    if (pTimeOSDCfg->overlain == false || pTimeOSDCfg->osdType != E_OSD_TYPE_TIME)
        return AX_FALSE;

    t = time(NULL);
    localtime_r(&t, &tm);

    if (pTimeOSDCfg->osdTimeStampFmt == E_OSD_TIME_FMT_24H) {
        snprintf(time_str, MAX_OSD_STRING_LEN, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else if (pTimeOSDCfg->osdTimeStampFmt == E_OSD_TIME_FMT_12H) {
        if (tm.tm_hour >= 12) {
            pm = 1;
        }

        if (pm == 1) {
            snprintf(time_str, MAX_OSD_STRING_LEN, "%02d:%02d:%02d PM", tm.tm_hour - 12, tm.tm_min, tm.tm_sec);
        } else {
            snprintf(time_str, MAX_OSD_STRING_LEN, "%02d:%02d:%02d AM", tm.tm_hour, tm.tm_min, tm.tm_sec);
        }
    }

    snprintf(date_str, MAX_OSD_STRING_LEN, "%04d-%02d-%02d",tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday);
    snprintf((char *)szTimeStamp, nBuffLen, "%s %s", date_str, time_str);

    return AX_TRUE;
}

AX_CHAR* CTimeUtils::GetCurrTimeStr(AX_CHAR* szOut)
{
    auto tp = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(tp);

    struct tm tm;
    localtime_r(&t, &tm);

    snprintf((char *)szOut, 10, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    return szOut;
}

AX_U64 CTimeUtils::GetTickCount(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

wchar_t *CTimeUtils::GetCurrDateStr(wchar_t *szOut, AX_U16 nDateFmt, AX_S32 &nOutCharLen) {
    time_t t;
    struct tm tm;

    t = time(NULL);
    localtime_r(&t, &tm);

    AX_U32 nDateLen = 64;
    OSD_DATE_FORMAT_E eDateFmt = (OSD_DATE_FORMAT_E)nDateFmt;
    switch (eDateFmt) {
        case OSD_DATE_FORMAT_YYMMDD1: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            break;
        }
        case OSD_DATE_FORMAT_MMDDYY1: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d-%02d-%04d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);
            break;
        }
        case OSD_DATE_FORMAT_DDMMYY1: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
            break;
        }
        case OSD_DATE_FORMAT_YYMMDD2: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%04d年%02d月%02d日", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            break;
        }
        case OSD_DATE_FORMAT_MMDDYY2: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d月%02d日%04d年", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);
            break;
        }
        case OSD_DATE_FORMAT_DDMMYY2: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d日%02d月%04d年", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
            break;
        }
        case OSD_DATE_FORMAT_YYMMDD3: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%04d/%02d/%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            break;
        }
        case OSD_DATE_FORMAT_MMDDYY3: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d/%02d/%04d", tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900);
            break;
        }
        case OSD_DATE_FORMAT_DDMMYY3: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
            break;
        }
        case OSD_DATE_FORMAT_YYMMDDWW1: {
            const wchar_t *wday[] = {L"星期天", L"星期一", L"星期二", L"星期三", L"星期四", L"星期五", L"星期六"};
            nOutCharLen = swprintf(szOut, nDateLen, L"%04d-%02d-%02d %ls", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, wday[tm.tm_wday]);
            break;
        }
        case OSD_DATE_FORMAT_HHmmSS: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
            break;
        }
        case OSD_DATE_FORMAT_YYMMDDHHmmSS: {
            nOutCharLen = swprintf(szOut, nDateLen, L"%04d-%02d-%02d  %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            break;
        }
        case OSD_DATE_FORMAT_YYMMDDHHmmSSWW: {
            const wchar_t *wday[] = {L"星期天", L"星期一", L"星期二", L"星期三", L"星期四", L"星期五", L"星期六"};
            nOutCharLen = swprintf(szOut, nDateLen, L"%04d-%02d-%02d  %02d:%02d:%02d  %ls", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, wday[tm.tm_wday]);
            break;
        }
        default: {
            nOutCharLen = 0;
            LOG_M_E("TIME", "Not supported date format: %d.", eDateFmt);
            return nullptr;
        }
    }

    return szOut;
}

void CTimeUtils::msSleep(AX_U32 milliseconds)
{
    struct timespec ts = {
        (time_t)(milliseconds / 1000),
        (long)((milliseconds % 1000) * 1000000)
        };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

void CTimeUtils::usSleep(AX_U32 microseconds)
{
    struct timespec ts = {
        (time_t)(microseconds / 1000000),
        (long)((microseconds % 1000000) * 1000)
        };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

void CTimeUtils::nsSleep(AX_U32 nanoseconds)
{
    struct timespec ts = {
        (time_t)(nanoseconds / 1000000000),
        (long)(nanoseconds % 1000000000)
        };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}
