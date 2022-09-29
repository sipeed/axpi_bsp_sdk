#pragma once
#include "IProcess.h"

class CProc001 : public CBaseProcess {
 public:
  CProc001(AX_VOID) {
    RegisterCmd(CMD_001, 0/*subcmd*/);
    RegisterCmd(CMD_002, 0/*subcmd*/);
  };
  virtual ~CProc001(AX_VOID) = default;

  AX_BOOL RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) override;
 private:
  CASE_S_PACKET_T m_data;
};