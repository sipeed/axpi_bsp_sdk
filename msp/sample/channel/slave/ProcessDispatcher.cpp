#include <exception>
#include "ProcessDispatcher.h"

#include "Proc001.h"
#include "Proc002.h"
#include "Proc003.h"
#include "Proc004.h"


AX_BOOL CProcessDispatcher::Init(AX_VOID* pParam) {
  m_vecProcs.clear();

#define ADD_PROC(cls) \
  { \
    IProcess *pProcess = new (std::nothrow) cls(); \
    if (pProcess) { \
      pProcess->Init(pParam); \
      m_vecProcs.emplace_back(pProcess); \
    } \
  }

  ADD_PROC(CProc001)
  ADD_PROC(CProc002)
  ADD_PROC(CProc003)
  ADD_PROC(CProc004)

  return AX_TRUE;
}

AX_VOID CProcessDispatcher::DeInit(AX_VOID) {
  for (auto & proc : m_vecProcs) {
    if (proc) {
      proc->DeInit();
      delete proc;
    }
  }
  m_vecProcs.clear();
}

AX_VOID CProcessDispatcher::HandleProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize)
{
  for (auto & proc : m_vecProcs) {
    if (proc->FindCmd(nCmd, nSubCmd)) {
      proc->RunProcess(nCmd, nSubCmd, pData, nSize);
      break;
    }
  }
}