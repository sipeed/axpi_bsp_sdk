#ifndef _APP_LOG_71158D90_7332_48F2_9200_2239312AA03D_H_
#define _APP_LOG_71158D90_7332_48F2_9200_2239312AA03D_H_

#include "global.h"
#include "string"

using namespace std;

typedef enum
{
    E_LOG_LV_NONE       = 0,
    E_LOG_LV_CRITAL     = 1,
    E_LOG_LV_ERROR      = 2,
    E_LOG_LV_INFO       = 3,
    E_LOG_LV_DEBUG      = 4,
    E_LOG_LV_MAX        = 5,
} APP_LOG_LEVEL;

typedef enum
{
    E_LOG_TARGET_NONE   = 0,
    E_LOG_TARGET_STDERR = 1,
    E_LOG_TARGET_FILE   = 2,
    E_LOG_TARGET_BOTH   = 3,
    E_LOG_TARGET_MAX    = 4
} APP_LOG_TARGET;

AX_BOOL APP_LogInit(std::string strAppName);
AX_BOOL APP_LogWrite(AX_S8* szLog, AX_U32 nSize, AX_U32 nLevel);
AX_VOID APP_Log(AX_U32 eLv, const char *fmt, ...);
AX_BOOL APP_SwitchFile();
AX_VOID APP_SetLogLevel(APP_LOG_LEVEL eLv);
AX_VOID APP_SetLogTarget(APP_LOG_TARGET eTarget);

AX_BOOL APP_LogOpen(std::string strAppName);
AX_VOID APP_LogClose();


#define PRINT               APP_Log
#define LOG                 LOG_W
#define LOG_M               LOG_M_W
#define LOG_W(fmt, ...)     APP_Log(E_LOG_LV_CRITAL, "\e[33;1m" "[%25s] " fmt "\033[0m\n", __func__, ##__VA_ARGS__)
#define LOG_E(fmt, ...)     APP_Log(E_LOG_LV_ERROR,  "\e[31;1m" "[%25s] " fmt "\033[0m\n", __func__, ##__VA_ARGS__)
#define LOG_I(fmt, ...)     APP_Log(E_LOG_LV_INFO,   "\e[35;1m" "[%25s] " fmt "\033[0m\n", __func__, ##__VA_ARGS__)
#define LOG_D(fmt, ...)     APP_Log(E_LOG_LV_DEBUG,  "\e[30;1m" "[%25s] " fmt "\033[0m\n", __func__, ##__VA_ARGS__)

#define LOG_M_W(m, fmt, ...)   APP_Log(E_LOG_LV_CRITAL, "\e[33;1m" "[%10s][%25s] " fmt "\033[0m\n", m, __func__, ##__VA_ARGS__)
#define LOG_M_E(m, fmt, ...)   APP_Log(E_LOG_LV_ERROR,  "\e[31;1m" "[%10s][%25s] " fmt "\033[0m\n", m, __func__, ##__VA_ARGS__)
#define LOG_M_I(m, fmt, ...)   APP_Log(E_LOG_LV_INFO,   "\e[35;1m" "[%10s][%25s] " fmt "\033[0m\n", m, __func__, ##__VA_ARGS__)
#define LOG_M_D(m, fmt, ...)   APP_Log(E_LOG_LV_DEBUG,  "\e[30;1m" "[%10s][%25s] " fmt "\033[0m\n", m, __func__, ##__VA_ARGS__)

#endif /* _APP_LOG_71158D90_7332_48F2_9200_2239312AA03D_H_ */
