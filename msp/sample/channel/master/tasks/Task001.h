#pragma once
#include "ITask.h"

class CTask001 : public CBaseTask {
 public:
  using CBaseTask::CBaseTask;
  virtual ~CTask001(AX_VOID) = default;

  AX_BOOL RunTask(cmdline::parser *pParser) override;
};