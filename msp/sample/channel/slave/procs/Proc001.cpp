#include "Proc001.h"

AX_BOOL CProc001::RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) {

  // Note: "pData" and "size" correspond to the "pSendData" and "nSendSize" that you invoked
  // "AX_Channel_Master_SendAndRecv" with on master side, this is optional.

  switch (nCmd)
  {
  case CMD_001: // slave recv data
    {
      if (pData && nSize == sizeof(CASE_S_PACKET_T)) {
        memcpy(&m_data, pData, nSize);
      }
      else {
        printf("cmd[%d]: slave recv size mismatched, recv size: %d, expected size: %d\n",
               nCmd, nSize, sizeof(CASE_S_PACKET_T));
      }

      AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, NULL, 0);
      if (0 != ret) {
        printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
      }
    }
    break;
  case CMD_002: // slave send data
    {
      AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, (AX_U8*)&m_data, sizeof(CASE_S_PACKET_T));
      if (0 != ret) {
        printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
      }
    }
    break;
  default:
    break;
  }
  return AX_TRUE;
}
