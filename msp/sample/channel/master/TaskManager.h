#pragma once
#include <map>
#include <string>

#include "ITask.h"


class CTaskManager {
 public:
  CTaskManager(AX_VOID) = default;
  ~CTaskManager(AX_VOID) = default;

  AX_BOOL Init(AX_VOID);
  AX_VOID DeInit(AX_VOID);

  ITask *GetTask(const string &strTask) {
    if (m_mapTasks.end() != m_mapTasks.find(strTask)) {
      return m_mapTasks[strTask];
    } else {
      return nullptr;
    }
  }

 private:
  std::map<std::string, ITask *> m_mapTasks;
};
