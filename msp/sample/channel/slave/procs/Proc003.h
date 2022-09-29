#pragma once
#include <map>
#include "IProcess.h"

class CProc003 : public CBaseProcess {
 public:
  CProc003(AX_VOID) {
    RegisterCmd(CMD_005, 0/*subcmd*/);
    RegisterCmd(CMD_006, 0/*subcmd*/);
    RegisterCmd(CMD_007, 0/*subcmd*/);
    RegisterCmd(CMD_008, 0/*subcmd*/);
  };
  ~CProc003(AX_VOID) {
    for (auto &kv : m_s_data) {
      if (kv.second) {
        delete kv.second;
      }
    }
    for (auto &_kv : m_l_data) {
      if (_kv.second) {
        delete _kv.second;
      }
    }
    m_s_data.clear();
    m_l_data.clear();
  };

  AX_BOOL RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) override;
 private:
  std::map<AX_U32/*index*/, CASE_S_PACKET_T*> m_s_data;
  std::map<AX_U32/*index*/, CASE_L_PACKET_T*> m_l_data;
};