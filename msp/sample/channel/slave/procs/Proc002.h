#pragma once
#include "IProcess.h"

class CProc002 : public CBaseProcess {
 public:
  CProc002(AX_VOID) {
    RegisterCmd(CMD_003, 0/*subcmd*/);
    RegisterCmd(CMD_004, 0/*subcmd*/);
  };
  virtual ~CProc002(AX_VOID) = default;

  AX_BOOL RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) override;
 private:
  CASE_L_PACKET_T m_data;
};