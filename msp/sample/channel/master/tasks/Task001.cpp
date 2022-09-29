#include "Task001.h"

AX_BOOL CTask001::RunTask(cmdline::parser *pParser) {
  do {
    AX_S32 ret = 0;

    CASE_S_PACKET_T tData;
    tData.nSeqNum = 10000;
    tData.nIspGain = 1224;
    tData.nAGain = 2448;
    tData.nDGain = 3672;
    tData.nIntegrationTime = 4896;
    tData.nHdrRatio = 1224;
    tData.nHcgLcg = 254;
    tData.nLuxIndex = 1024;

    /* test send data */
    ret = AX_Channel_Master_SendAndRecv(CMD_001, 0/*subcmd*/, (AX_U8*)&tData, sizeof(tData), NULL, 0, NULL, TIMEOUT);
    if (0 != ret) {
      printf("cmd[%d]: master send data failed, ret: 0x%x\n", CMD_001, ret);
      break;
    }

    /* test read back */
    CASE_S_PACKET_T _tData;
    ret = AX_Channel_Master_SendAndRecv(CMD_002, 0/*subcmd*/, NULL/*optional*/, 0/*optional*/, (AX_U8*)&_tData, sizeof(_tData), NULL, TIMEOUT);
    if (0 != ret) {
      printf("cmd[%d]: master recv data failed, ret: 0x%x\n", CMD_002, ret);
      break;
    }

    /* data comparision */
    auto _data_compare = [&](const CASE_S_PACKET_T& data1, const CASE_S_PACKET_T& data2) -> AX_BOOL {
      return data1.IsEqual(data2);
    };

    return _data_compare(tData, _tData);
  }
  while (0);
  return AX_FALSE;
}
