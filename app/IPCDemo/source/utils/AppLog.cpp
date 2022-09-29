#include <stdarg.h>
#include <mutex>
#include <stdio.h>
#include <sys/time.h>
#include "AppLog.h"

#define MAX_FILE_NUM  (5)
#define MAX_FILE_SIZE (1 * 1024 * 1024)

static APP_LOG_LEVEL  gLogLevel = E_LOG_LV_ERROR;
static APP_LOG_TARGET gLogTarget = E_LOG_TARGET_FILE;

FILE* gLogFilePtr = nullptr;
string gArrLogFileName[MAX_FILE_NUM] = {""};
std::mutex gMutex;

AX_BOOL APP_LogOpen(string strAppName)
{
    if (E_LOG_TARGET_FILE == gLogTarget || E_LOG_TARGET_BOTH == gLogTarget) {
        APP_LogInit(strAppName);

        if ((gLogFilePtr = fopen(gArrLogFileName[0].c_str(), "w+")) == NULL) {
            printf("ERROR: Failed to open app log file: %s!\n", gArrLogFileName[0].c_str());
            return AX_FALSE;
        }
    }

    return AX_TRUE;
}

AX_VOID APP_LogClose()
{
    if (gLogFilePtr) {
        fflush(gLogFilePtr);
        fclose(gLogFilePtr);
        gLogFilePtr = nullptr;
    }
}

AX_BOOL APP_LogInit(string strAppName)
{
    char szPath[256] = {0};
    for (AX_U32 i = 0; i < MAX_FILE_NUM; i++) {
        if (0 == i) {
            sprintf(szPath, "/var/log/%s.log", strAppName.c_str());
        } else {
            sprintf(szPath, "/var/log/%s_%d.log", strAppName.c_str(), i);
        }
        gArrLogFileName[i] = szPath;
    }

    return AX_TRUE;
}

AX_BOOL APP_LogWrite(AX_S8* szLog, AX_U32 nSize, AX_U32 nLevel)
{
    std::lock_guard<std::mutex> lck(gMutex);
    if (nullptr == gLogFilePtr) {
        return AX_FALSE;
    }

    AX_S32 nFileLen = ftell(gLogFilePtr);
    if (-1 == nFileLen) {
        return AX_FALSE;
    }

    if (nFileLen + nSize > MAX_FILE_SIZE) {
        if (!APP_SwitchFile()) {
            return AX_FALSE;
        }
    }

    AX_U32 nWriteSize = fwrite(szLog, nSize, 1, gLogFilePtr);
    if (nLevel <= E_LOG_LV_ERROR) {
        fflush(gLogFilePtr);
    }

    return nWriteSize == 1 ? AX_TRUE : AX_FALSE;
}

AX_VOID APP_Log(AX_U32 eLv, const char *fmt, ...)
{
    if (eLv <= gLogLevel) {
        va_list args;
        char szLog[1024] = {0};
        char output[2048] = {0};

        va_start(args, fmt);

        AX_U32 len = vsnprintf(szLog, sizeof(szLog), (char *)fmt, args);
        if (len < (AX_U32)sizeof(szLog)) {
            szLog[len] = '\0';
        } else {
            szLog[sizeof(szLog) - 1] = '\0';
        }

        struct timeval current;
        gettimeofday(&current, NULL);
        AX_U32 nWriteNum = snprintf(output, sizeof(output) - 1, "[%lu.%06lu] %s", current.tv_sec, current.tv_usec, szLog);

        if (nullptr != gLogFilePtr && (E_LOG_TARGET_FILE == gLogTarget || E_LOG_TARGET_BOTH == gLogTarget)) {
            AX_BOOL bRet = APP_LogWrite((AX_S8 *)output, nWriteNum, eLv);
            if (AX_TRUE != bRet) {
                printf("ERROR: Write app log file failed: %s\n", gArrLogFileName[0].c_str());
            }
        }

        if (E_LOG_TARGET_STDERR == gLogTarget || E_LOG_TARGET_BOTH == gLogTarget) {
            printf(output);
        }

        va_end(args);
    }
}

AX_VOID APP_SetLogLevel(APP_LOG_LEVEL eLv)
{
    if (eLv >= E_LOG_LV_NONE && eLv < E_LOG_LV_MAX) {
        gLogLevel = eLv;
    }
}

APP_LOG_LEVEL APP_GetLogLevel(AX_VOID)
{
    return gLogLevel;
}

AX_VOID APP_SetLogTarget(APP_LOG_TARGET eTarget)
{
    if (eTarget >= E_LOG_TARGET_NONE && eTarget < E_LOG_TARGET_MAX) {
        gLogTarget = eTarget;
    }
}

AX_BOOL APP_SwitchFile()
{
    APP_LogClose();

    for (AX_U32 i = MAX_FILE_NUM - 1; i > 0; i--) {
        if (MAX_FILE_NUM - 1 == i && 0 == access(gArrLogFileName[i].c_str(), F_OK)) {
            if (0 != remove(gArrLogFileName[i].c_str())) {
                printf("ERROR: Failed to remove app log file: %s!\n", gArrLogFileName[i].c_str());
                return AX_FALSE;
            }
        }

        if (0 == access(gArrLogFileName[i - 1].c_str(), F_OK)) {
            if (0 != rename(gArrLogFileName[i - 1].c_str(), gArrLogFileName[i].c_str())) {
                printf("ERROR: Failed to rename app log file: %s!\n", gArrLogFileName[i - 1].c_str());
                return AX_FALSE;
            }
        }
    }

    if ((gLogFilePtr = fopen(gArrLogFileName[0].c_str(), "w+")) == NULL) {
        printf("ERROR: Failed to open app log file: %s!\n", gArrLogFileName[0].c_str());
        return AX_FALSE;
    }

    return AX_TRUE;
}
