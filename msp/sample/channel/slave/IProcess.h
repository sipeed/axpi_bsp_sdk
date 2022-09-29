#pragma once
#include <vector>

#include "ax_channel_api.h"
#include "comdef.h"
#include "cmdline.h"

using namespace std;


class IProcess {
 public:
  virtual ~IProcess(AX_VOID) = default;
  virtual AX_VOID Init(AX_VOID* pParam) = 0;
  virtual AX_VOID DeInit(AX_VOID) = 0;
  virtual AX_BOOL FindCmd(AX_U8 nCmd, AX_U8 nSubCmd) = 0;
  virtual AX_BOOL RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) = 0;
};

class CBaseProcess : public IProcess {
 public:
  CBaseProcess(AX_VOID) = default;
  virtual ~CBaseProcess(AX_VOID) = default;

  AX_VOID Init(AX_VOID* pParam) override {}
  AX_VOID DeInit(AX_VOID) override {}
  AX_BOOL FindCmd(AX_U8 nCmd, AX_U8 nSubCmd) override {
    AX_U16 cmd = MAKE_WORD(nCmd, nSubCmd);
    for (auto & _cmd : m_vecCmd) {
      if (_cmd == cmd) {
        return AX_TRUE;
      }
    }
    return AX_FALSE;
  }

  AX_VOID RegisterCmd(AX_U8 nCmd, AX_U8 nSubCmd) {
    AX_U16 cmd = MAKE_WORD(nCmd, nSubCmd);
    for (auto & _cmd : m_vecCmd) {
      if (_cmd == cmd) {
        return;
      }
    }
    m_vecCmd.emplace_back(cmd);
  }

 private:
  vector<AX_U16> m_vecCmd;
};