#pragma once
#include "ITask.h"

class CTask004 : public CBaseTask {
 public:
  using CBaseTask::CBaseTask;
  virtual ~CTask004(AX_VOID) = default;

  AX_BOOL RunTask(cmdline::parser *pParser) override;
 private:
  AX_S32 UploadPacket(AX_U8* pData, AX_U32 nSize);
  AX_S32 DownloadPacket(AX_U8* pData, AX_U32 nSize, AX_U32* pRealSize);
  AX_VOID PrintProgress(AX_BOOL bDownload, AX_U32 nPercent);

 private:
  AX_S32 m_nPercent = -1;
};