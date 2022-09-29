#include "Task003.h"

extern AX_S32 gExitThread;

AX_BOOL CTask003::RunTask(cmdline::parser *pParser) {

  const AX_U32 nCount = pParser->get<AX_U32>("count");
  if (nCount > MAX_THREAD_NUM) {
    printf("thread count[%d] exceeded max number[%d]\n", nCount, MAX_THREAD_NUM);
    return AX_FALSE;
  }
  else {
    printf("task 003 thread count[%d]\n", nCount);
  }

  /* start threads */
  m_bStopThread.store(AX_FALSE);
  for (AX_U32 i = 0;i < nCount;i++) {
    m_arrThread[i] = std::thread([i, this]() { SendRecvThread(i, m_bStopThread); });
  }

  while (!gExitThread) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  /* join threads */
  m_bStopThread.store(true);
  for (AX_U32 j = 0;j < nCount;j++) {
    if (m_arrThread[j].joinable()) {
      m_arrThread[j].join();
    }
  }

  return (m_bFail ? AX_FALSE : AX_TRUE);
}

AX_VOID CTask003::SendRecvThread(AX_U32 nIndex, const std::atomic<bool>& bStopThread) {
  AX_BOOL bFail = AX_FALSE;
  while (!bStopThread.load()) {
    do {
      AX_S32 ret = 0;
      if (nIndex % 2 == 0) {
        // delay a while and it may offer more opportunity to
        // run large packet process thread
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        CASE_S_PACKET_T tData;
        tData.nSeqNum = nIndex;
        tData.nIspGain = 1224;
        tData.nAGain = 2448;
        tData.nDGain = 3672;
        tData.nIntegrationTime = 4896;
        tData.nHdrRatio = 1224;
        tData.nHcgLcg = 254;
        tData.nLuxIndex = 1024;

        /* test send data */
        ret = AX_Channel_Master_SendAndRecv(CMD_005, 0/*subcmd*/, (AX_U8*)&tData, sizeof(tData), NULL, 0, NULL, TIMEOUT);
        if (0 != ret) {
          printf("cmd[%d]: master(thread_%d) send data failed, ret: 0x%x\n", CMD_005, nIndex, ret);
          bFail = AX_TRUE;
          break;
        }

        /* test read back */
        CASE_S_PACKET_T _tData;
        ret = AX_Channel_Master_SendAndRecv(CMD_006, 0/*subcmd*/, (AX_U8*)&nIndex, sizeof(AX_U32), (AX_U8*)&_tData, sizeof(_tData), NULL, TIMEOUT);
        if (0 != ret) {
          printf("cmd[%d]: master(thread_%d) recv data failed, ret: 0x%x\n", CMD_006, nIndex, ret);
          bFail = AX_TRUE;
          break;
        }

        /* data comparision */
        auto _data_compare = [&](const CASE_S_PACKET_T& data1, const CASE_S_PACKET_T& data2) -> AX_BOOL {
          return data1.IsEqual(data2);
        };
        bFail = _data_compare(tData, _tData) ? AX_FALSE : AX_TRUE;
      }
      else {
        CASE_L_PACKET_T tData1, tData2;

        /* set init data */
        memset(tData1.aData, 0xAE, sizeof(CASE_L_PACKET_T));
        memcpy(tData1.aData, &nIndex, sizeof(AX_U32));

        /* test send packet data */
        ret = SendPacket(tData1);
        if (0 != ret) {
          printf("cmd[%d]: master(thread_%d) send packet data failed, ret: 0x%x\n", CMD_007, nIndex, ret);
          bFail = AX_TRUE;
          break;
        }

        /* test read back packet */
        memset(tData2.aData, 0x0, sizeof(CASE_L_PACKET_T));
        memcpy(tData2.aData, &nIndex, sizeof(AX_U32));
        ret = RecvPacket(tData2);
        if (0 != ret) {
          printf("cmd[%d]: master(thread_%d) recv packet data failed, ret: 0x%x\n", CMD_008, nIndex, ret);
          bFail = AX_TRUE;
          break;
        }

        /* packet data comparision */
        auto _data_compare = [&](const CASE_L_PACKET_T& data1, const CASE_L_PACKET_T& data2) -> AX_BOOL {
          return data1.IsEqual(data2);
        };
        bFail = _data_compare(tData1, tData2) ? AX_FALSE : AX_TRUE;
      }
    }
    while (0);

    if (bFail) {
      break;
    }
  }

  /* merge result status */
  {
    std::lock_guard<std::mutex> lck(m_mutxStat);
    m_bFail = (m_bFail || bFail) ? AX_TRUE : AX_FALSE;
  }
}

AX_S32 CTask003::SendPacket(const CASE_L_PACKET_T& data)
{
  return AX_Channel_Master_SendAndRecv(CMD_007, 0/*subcmd*/, (AX_U8*)&data, sizeof(CASE_L_PACKET_T), NULL, 0, NULL, TIMEOUT);
}

AX_S32 CTask003::RecvPacket(const CASE_L_PACKET_T& data)
{
  return AX_Channel_Master_SendAndRecv(CMD_008, 0/*subcmd*/, (AX_U8*)&data/*nIndex*/, sizeof(AX_U32), (AX_U8*)&data, sizeof(CASE_L_PACKET_T), NULL, TIMEOUT);
}
