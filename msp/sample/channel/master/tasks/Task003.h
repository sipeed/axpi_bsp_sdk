#pragma once
#include <atomic>
#include <mutex>
#include <thread>
#include "ITask.h"

#define MAX_THREAD_NUM (50)

class CTask003 : public CBaseTask {
 public:
  using CBaseTask::CBaseTask;
  virtual ~CTask003(AX_VOID) = default;

  AX_BOOL RunTask(cmdline::parser *pParser) override;

 protected:
  AX_VOID SendRecvThread(AX_U32 nIndex, const std::atomic<bool>& bStopThread);

 private:
  AX_S32 SendPacket(const CASE_L_PACKET_T& data);
  AX_S32 RecvPacket(const CASE_L_PACKET_T& data);

 private:
  /* thread variables define */
  std::thread       m_arrThread[MAX_THREAD_NUM];
  std::atomic<bool> m_bStopThread;

  std::mutex m_mutxStat;
  AX_BOOL m_bFail = AX_FALSE;
};