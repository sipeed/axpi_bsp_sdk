#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include "IProcess.h"

class CProc004 : public CBaseProcess {
 public:
  CProc004(AX_VOID) {
    RegisterCmd(CMD_009, 0/*subcmd*/);
    RegisterCmd(CMD_010, 0/*subcmd*/);
  };
  virtual ~CProc004(AX_VOID) = default;

  AX_VOID Init(AX_VOID* pParam) override;
  AX_VOID DeInit(AX_VOID) override;
  AX_BOOL RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) override;

 private:
  FILE*   m_fp = nullptr;
  AX_U64  m_nTotalSize = 0;
  AX_U64  m_nSendSize = 0;
};