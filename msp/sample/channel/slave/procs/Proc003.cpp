#include "Proc003.h"

AX_BOOL CProc003::RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) {

  // Note: "pData" and "size" correspond to the "pSendData" and "nSendSize" that you invoked
  // "AX_Channel_Master_SendAndRecv" with on master side, this is optional.

  if (!pData || nSize < sizeof(AX_U32)) {
    printf("nil payload data or unexpected recv payload size: %d\n", nSize);
    return AX_FALSE;
  }

  /* parse (thread) index */
  AX_U32 nIndex = *((AX_U32*)pData);

  switch (nCmd)
  {
    /* process small packet recv & send */
  case CMD_005: // slave recv small packet data
    {
      if (pData && nSize == sizeof(CASE_S_PACKET_T)) {
        if (m_s_data.end() == m_s_data.find(nIndex)) {
          m_s_data[nIndex] = new CASE_S_PACKET_T;
        }
        memcpy(m_s_data[nIndex], pData, nSize);
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
  case CMD_006: // slave send small packet data
    {
      if (m_s_data.end() != m_s_data.find(nIndex)) {
        AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, (AX_U8*)m_s_data[nIndex], sizeof(CASE_S_PACKET_T));
        if (0 != ret) {
          printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
        }
      }
      else {
        printf("cmd[%d]: indicated index[%d] not found\n", nCmd, nIndex);
      }
    }
    break;

    /* process large packet recv & send */
  case CMD_007: // slave recv large packet data
    {
      if (pData && nSize == sizeof(CASE_L_PACKET_T)) {
        if (m_l_data.end() == m_l_data.find(nIndex)) {
          m_l_data[nIndex] = new CASE_L_PACKET_T;
        }
        memcpy(m_l_data[nIndex], pData, nSize);
      }
      else {
        printf("cmd[%d]: slave recv size mismatched, recv size: %d, expected size: %d\n",
               nCmd, nSize, sizeof(CASE_L_PACKET_T));
      }

      AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, NULL, 0);
      if (0 != ret) {
        printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
      }
    }
    break;
  case CMD_008: // slave send large packet data
    {
      if (m_l_data.end() != m_l_data.find(nIndex)) {
        AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, (AX_U8*)m_l_data[nIndex], sizeof(CASE_L_PACKET_T));
        if (0 != ret) {
          printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
        }
      }
      else {
        printf("cmd[%d]: indicated index[%d] not found\n", nCmd, nIndex);
      }
    }
    break;
  default:
    break;
  }
  return AX_TRUE;
}
