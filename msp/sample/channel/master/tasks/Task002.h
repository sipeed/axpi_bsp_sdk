#pragma once
#include "ITask.h"

class CTask002 : public CBaseTask {
 public:
  using CBaseTask::CBaseTask;
  virtual ~CTask002(AX_VOID) = default;

  AX_BOOL RunTask(cmdline::parser *pParser) override;
 private:
  AX_S32 SendPacket(const CASE_L_PACKET_T& data);
  AX_S32 RecvPacket(const CASE_L_PACKET_T& data);
 private:
  CASE_L_PACKET_T m_tData1, m_tData2;
};