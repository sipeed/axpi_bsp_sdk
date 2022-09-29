#pragma once
#include <map>
#include <string>

#include "IProcess.h"


class CProcessDispatcher {
 public:
  CProcessDispatcher(AX_VOID) = default;
  ~CProcessDispatcher(AX_VOID) = default;

  AX_BOOL Init(AX_VOID* pParam);
  AX_VOID DeInit(AX_VOID);

  AX_VOID HandleProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize);

 private:
  std::vector<IProcess*> m_vecProcs;
};
