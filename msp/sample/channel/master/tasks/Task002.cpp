#include "Task002.h"

AX_BOOL CTask002::RunTask(cmdline::parser *pParser) {
  do {
    AX_S32 ret = 0;

    /* set init data */
    memset(m_tData1.aData, 0xAE, sizeof(CASE_L_PACKET_T));

    /* test send packet data */
    ret = SendPacket(m_tData1);
    if (0 != ret) {
      printf("cmd[%d]: master send packet data failed, ret: 0x%x\n", CMD_003, ret);
      break;
    }

    /* test read back packet */
    memset(m_tData2.aData, 0x0, sizeof(CASE_L_PACKET_T));
    ret = RecvPacket(m_tData2);
    if (0 != ret) {
      printf("cmd[%d]: master recv packet data failed, ret: 0x%x\n", CMD_004, ret);
      break;
    }

    /* packet data comparision */
    auto _data_compare = [&](const CASE_L_PACKET_T& data1, const CASE_L_PACKET_T& data2) -> AX_BOOL {
      return data1.IsEqual(data2);
    };

    return _data_compare(m_tData1, m_tData2);
  }
  while (0);
  return AX_FALSE;
}

AX_S32 CTask002::SendPacket(const CASE_L_PACKET_T& data)
{
  return AX_Channel_Master_SendAndRecv(CMD_003, 0/*subcmd*/, (AX_U8*)&data, sizeof(CASE_L_PACKET_T), NULL, 0, NULL, TIMEOUT);
}

AX_S32 CTask002::RecvPacket(const CASE_L_PACKET_T& data)
{
  return AX_Channel_Master_SendAndRecv(CMD_004, 0/*subcmd*/, NULL/*optional*/, 0/*optional*/, (AX_U8*)&data, sizeof(CASE_L_PACKET_T), NULL, TIMEOUT);
}
