#pragma once
#include <string>
#include <vector>

#include "ax_channel_api.h"
#include "comdef.h"
#include "cmdline.h"

using namespace std;

/* task name define */
#define TASK_001 "001"
#define TASK_002 "002"
#define TASK_003 "003"
#define TASK_004 "004"

class ITask {
 public:
  virtual ~ITask(AX_VOID) = default;
  virtual AX_BOOL RunTask(cmdline::parser *pParser) = 0;
};

class CBaseTask : public ITask {
 public:
  CBaseTask(const string &strName) { m_strTaskName = strName; }
  virtual ~CBaseTask(AX_VOID) = default;

  const string &GetTaskName(AX_VOID) { return m_strTaskName; }

 private:
  string m_strTaskName;
};