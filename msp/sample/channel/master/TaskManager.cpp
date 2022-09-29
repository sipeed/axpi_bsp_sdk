#include <exception>
#include "TaskManager.h"

#include "Task001.h"
#include "Task002.h"
#include "Task003.h"
#include "Task004.h"


AX_BOOL CTaskManager::Init(AX_VOID) {
  m_mapTasks.clear();

#define CREATE_TASK(name, cls)                   \
  {                                              \
    ITask *pTask = new (std::nothrow) cls(name); \
    if (pTask) {                                 \
      m_mapTasks[name] = pTask;                  \
    }                                            \
  }

  CREATE_TASK(TASK_001, CTask001)
  CREATE_TASK(TASK_002, CTask002)
  CREATE_TASK(TASK_003, CTask003)
  CREATE_TASK(TASK_004, CTask004)

  return AX_TRUE;
}

AX_VOID CTaskManager::DeInit(AX_VOID) {
  for (auto &kv : m_mapTasks) {
    if (kv.second) {
      delete kv.second;
    }
  }
  m_mapTasks.clear();
}