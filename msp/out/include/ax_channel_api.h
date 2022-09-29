#ifndef _AX_CHANNEL_API_H_
#define _AX_CHANNEL_API_H_

#include "ax_base_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* channel log level */
typedef enum {
  AX_CHANNEL_LOG_LEVEL_NONE    = 0,
  AX_CHANNEL_LOG_LEVEL_ERROR   = 1,
  AX_CHANNEL_LOG_LEVEL_WARN    = 2,
  AX_CHANNEL_LOG_LEVEL_INFO    = 3,
  AX_CHANNEL_LOG_LEVEL_DEBUG   = 4,
  AX_CHANNEL_LOG_LEVEL_VERBOSE = 5,
  AX_CHANNEL_LOG_LEVEL_DATA    = 6,
  AX_CHANNEL_LOG_LEVEL_BUTT
} AX_CHANNEL_LOG_LEVEL_E;

/* startup info */
#define AX_CH_MAX_PATH     (256)
typedef struct _ChannelStartupInfo {
  AX_CHAR szDevPath[AX_CH_MAX_PATH]; /* device path, e.g. /dev/xxxx */
  AX_U32  nDbgLv;                    /* see "AX_CHANNEL_LOG_LEVEL_E" */
} AX_CHANNEL_STARTUP_INFO;

/*
  [notify data callback for master]

  - nNotifyType: User defined enumeration type.
  - pData:       Notify data buffer.
  - nSize:       Notify data buffer size.
  - pContext:    User context data.
*/
typedef AX_VOID (*AX_ChannelNotifyCB)(AX_U32 nNotifyType, AX_U8* pData, AX_U32 nSize, AX_VOID* pContext);

/*
  [incoming message data callback for slave]

  - nCmd:     User defined command id, [1, 255].
  - nSubCmd:  User defined subcommand id, [0, 255].
  - pData:    Payload buffer from master, correspond to param "pSendData" in "AX_Channel_Master_SendAndRecv"
  - nSize:    Payload buffer size, correspond to param "nSendSize" in "AX_Channel_Master_SendAndRecv"
  - pContext: User context data.
*/
typedef AX_VOID (*AX_ChannelMessageCB)(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize, AX_VOID* pContext);


//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Master_Init
///
/// @brief Initialize (sdio) master.
///
/// @param pInfo     Specifies the pointer of AX_CHANNEL_STARTUP_INFO structure.
/// @param pCallback Pointer to a callback function to do with slave notify data report.
/// @param pContext  Pointer to a user context data.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Master_Init(const AX_CHANNEL_STARTUP_INFO *pInfo, AX_ChannelNotifyCB pCallback, AX_VOID* pContext);

//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Master_DeInit
///
/// @brief Deinitialize (sdio) master.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Master_DeInit();

//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Master_SendAndRecv
///
/// @brief Master send data to slave and receive response data, response data is optional.
///
/// @param nCmd      User defined command id, [1, 255]. 0 is for internal usage.
/// @param nSubCmd   User defined subcommand id, [0, 255].
/// @param pSendData Send data buffer.
/// @param nSendSize Send data buffer size.
/// @param pRecvData Receive data buffer.
/// @param nRecvSize Receive data buffer size.
/// @param pReceived Actually received data size.
/// @param nTimeOut  communication timeout(reserved).
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Master_SendAndRecv(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pSendData, AX_U32 nSendSize, AX_U8* pRecvData, AX_U32 nRecvSize, AX_U32* pReceived, AX_S32 nTimeOut/*ms*/);


//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Slave_Init
///
/// @brief Initialize (sdio) slave.
///
/// @param pInfo     Specifies the pointer of AX_CHANNEL_STARTUP_INFO structure.
/// @param pCallback Pointer to a callback function to do with incoming message from master.
/// @param pContext  Pointer to a user context data.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Slave_Init(const AX_CHANNEL_STARTUP_INFO *pInfo, AX_ChannelMessageCB pCallback, AX_VOID* pContext);

//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Slave_DeInit
///
/// @brief Deinitialize (sdio) slave.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Slave_DeInit();

//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Slave_WriteData
///
/// @brief Slave write response data to master, response data is optional.
///        This only can be invoked during "AX_ChannelMessageCB".
///
/// @param nCmd       User defined command id, [1, 255]. 0 is for internal usage.
/// @param nSubCmd    User defined subcommand id, [0, 255].
/// @param pWriteData Response data buffer.
/// @param nWriteSize Response data buffer size.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Slave_WriteData(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pWriteData, AX_U32 nWriteSize);

//////////////////////////////////////////////////////////////////////////////////////
/// AX_Channel_Slave_Notify
///
/// @brief Slave notify report data to master.
///
/// @param nNotifyType User defined enumeration type.
/// @param pNotifyData Notify data buffer.
/// @param nNotifySize Notify data buffer size.
///
/// @return 0 if success, otherwise failure
//////////////////////////////////////////////////////////////////////////////////////
AX_S32 AX_Channel_Slave_Notify(AX_U32 nNotifyType, AX_U8* pNotifyData, AX_U32 nNotifySize);

#ifdef __cplusplus
}
#endif

#endif /* _AX_CHANNEL_API_H_ */
