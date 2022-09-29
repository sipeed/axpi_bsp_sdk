#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <chrono>
#include <string>
#include <thread>

#include "cmdline.h"
#include "ax_channel_api.h"

#include "TaskManager.h"
#include "ProcessDispatcher.h"

using namespace std;

/* global define */
AX_S32 gExitThread = 0;

static AX_VOID ShowWelcome(AX_VOID) {
  printf("********************************************************\n");
  printf("* SAMPLE BUILD: %s %s *\n", __DATE__, __TIME__);
  printf("********************************************************\n");
}

static AX_VOID SigInt(AX_S32 signo) {
  printf("[SigInt] catch signal %d\n", signo);
  gExitThread = 1;
}

static AX_VOID SigStop(AX_S32 signo) {
  printf("[SigStop] catch signal %d\n", signo);
  gExitThread = 1;
}

/* forward declaration */
extern AX_VOID ChannelNotifyHandler(AX_U32 nNotifyType, AX_U8* pData, AX_U32 nSize, AX_VOID* pContext); /* for master */
extern AX_VOID ChannelMessageHandler(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize, AX_VOID* pContext); /* for slave */
extern AX_S32 ProcMaster(cmdline::parser *pParser);
extern AX_S32 ProcSlave(cmdline::parser *pParser);


AX_S32 main(AX_S32 argc, AX_CHAR *argv[]) {

  struct termios tmoOld;
  tcgetattr(0, &tmoOld);

  ShowWelcome();

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SigInt);
  signal(SIGTSTP, SigStop);

  /* parse command line params */
  cmdline::parser parser;
  parser.add<string>("type",  't', "channel type",  false, "m",             cmdline::oneof<string>("m"/*master*/, "s"/*slave*/));
  parser.add<string>("task",  'k', "task type",     false, TASK_001,        cmdline::oneof<string>(TASK_001, TASK_002, TASK_003, TASK_004));
  parser.add<string>("dev",   'd', "device name",   false, "/dev/sdio_host");
  parser.add<AX_U32>("wait",  'w', "wait interval", false, 1/*second*/,     cmdline::range(0, 100)/*unit: second*/);
  parser.add<AX_U32>("count", 'c', "thread count",  false, 2,               cmdline::range(2, 50));
  parser.add<AX_U32>("log",   'v', "log level",     false, 1/*lvl err*/,    cmdline::range(0, 6)/*AX_CHANNEL_LOG_LEVEL_E*/);

  parser.add<string>("download", 'l', "download file", false, "");
  parser.add<string>("upload",   'u', "upload file",   false, "");

  parser.add("loop",   '\0', "loop test");
  parser.add("notify", '\0', "slave notify");
  parser.parse_check(argc, argv);

  struct termios tmpSet = tmoOld;
  tmpSet.c_lflag &= ~ICANON;
  tmpSet.c_lflag &= ~ECHO;
  tmpSet.c_cc[VMIN] = 1;
  tmpSet.c_cc[VTIME] = 0;
  tcsetattr(0, TCSANOW, &tmpSet);

  /* sample works in either master or slave mode */
  AX_S32 ret = 0;
  const string strType = parser.get<string>("type");
  if (0 == strType.compare("s")) {
    ret = ProcSlave(&parser);  // slave mode
  }
  else {
    ret = ProcMaster(&parser); // master mode
  }

  tcsetattr(0, TCSANOW, &tmoOld);
  return ret;
}

/* master receives slave data notify callback */
AX_VOID ChannelNotifyHandler(AX_U32 nNotifyType, AX_U8* pData, AX_U32 nSize, AX_VOID* pContext) {
  switch (nNotifyType)
  {
  case NOTIFY_ID_001:
    {
      NOTIFY_DATA_001 tNotify;
      if (sizeof(NOTIFY_DATA_001) == nSize) {
        memcpy(&tNotify, pData, nSize);
        printf("master recv notify[%d] t.nData1[%d] t.nData2[%d]\n",
               nNotifyType, tNotify.nData1, tNotify.nData2);
      }
      else {
        printf("master recv notify data size mismatched, notify[%d] recv size[%d] expected size[%d]\n",
               nNotifyType, nSize, sizeof(NOTIFY_DATA_001));
      }
    }
    break;
  default:
    break;
  }
}

AX_S32 ProcMaster(cmdline::parser *pParser) {

  const string strTask = pParser->get<string>("task");
  const string strDev = pParser->get<string>("dev");
  const string strDl = pParser->get<string>("download");
  const string strUpload = pParser->get<string>("upload");
  const AX_BOOL bLoop = pParser->exist("loop") ? AX_TRUE : AX_FALSE;
  const AX_U32 nWait = pParser->get<AX_U32>("wait");
  const AX_U32 nDbgLv = pParser->get<AX_U32>("log");

  string _strTask;
  if (!strDl.empty() || !strUpload.empty()) {
    _strTask = TASK_004;
  }
  else {
    _strTask = strTask;
  }

  printf("master cmdline params: task[%s] dev[%s] bLoop[%d] wait[%d] dbg level[%d]\n",
         _strTask.c_str(), strDev.c_str(), bLoop, nWait, nDbgLv);

  /* task manager init */
  CTaskManager tasks;
  tasks.Init();

  AX_CHANNEL_STARTUP_INFO tStartInfo;
  memset(&tStartInfo, 0, sizeof(tStartInfo));
  strcpy(tStartInfo.szDevPath, strDev.c_str());
  tStartInfo.nDbgLv = nDbgLv;

  /* api init */
  AX_S32 ret = AX_Channel_Master_Init(&tStartInfo, ChannelNotifyHandler, nullptr/*context, optional*/);
  if (0 != ret) {
    printf("master ax channel init failed, ret = 0x%x\n", ret);
    return ret;
  }

  AX_U32 nLoopCnt = 0;
  do {
    if (bLoop) {
      printf("\n\n------------ loop to start %d ------------\n\n", ++nLoopCnt);
    }

    /* exec indicated task */
    ITask *pTask = tasks.GetTask(_strTask);
    if (!pTask) {
      printf("%s not support!", _strTask.c_str());
      ret = 1;
      goto __EXIT;
    }

    if (!pTask->RunTask(pParser)) {
      ret = 1;
      break;
    }

    /* wait interval */
    if (nWait > 0) {
      sleep(nWait);
    }

  } while (bLoop && !gExitThread);

  if (bLoop) {
    printf("\n\n------------ loop total: %d ------------\n\n", nLoopCnt);
  }

__EXIT:
  AX_S32 channelRet = 0;
  if (0 == ret) {
    channelRet = 0;
    printf("sample master run success\n");
  } else {
    channelRet = 1;
    printf("sample master run fail, ret = 0x%x\n", ret);
  }

  /* api de-init */
  AX_Channel_Master_DeInit();

  /* task manager de-init */
  tasks.DeInit();

  return channelRet;
}

/* slave message pump */
AX_VOID ChannelMessageHandler(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize, AX_VOID* pContext) {
  /*
  normally, here use "switch-case" to handle incoming messages from master,
  pseudo code is as follows:

  switch (nCmd)
  {
  case MESSAGE_0:
    step 1. Handle incoming message data and do your own works, but it should not take too long or
            it will cause sdio read timeout on master side. (< 5s, for ioctl read timeout)
    ...
    step 2. You have to response back to master side owing to sdio ioctl still blocks their, code like below:
            AX_Channel_Slave_WriteData(nCmd, nSubCmd, "response data addr", "response data size");
    break;
  case MESSAGE_1:
    as above...
    break;
  case MESSAGE_2:
    as above...
    break;
  ...
  default:
    break;
  }
  */
  CProcessDispatcher* pProcDispatcher = (CProcessDispatcher*)pContext;
  if (pProcDispatcher) {
    pProcDispatcher->HandleProcess(nCmd, nSubCmd, pData, nSize);
  }
}

AX_S32 ProcSlave(cmdline::parser *pParser) {

  const string strDev = pParser->get<string>("dev");
  const AX_BOOL bNotify = pParser->exist("notify") ? AX_TRUE : AX_FALSE;
  const AX_U32 nDbgLv = pParser->get<AX_U32>("log");

  printf("slave cmdline params: dev[%s] bNotify[%d] dbg level[%d]\n", strDev.c_str(), bNotify, nDbgLv);

  CProcessDispatcher _dispatcher;
  _dispatcher.Init((AX_VOID*)pParser);

  AX_U32 nNotifyType = NOTIFY_ID_001;
  NOTIFY_DATA_001 tNotify;

  AX_CHANNEL_STARTUP_INFO tStartInfo;
  memset(&tStartInfo, 0, sizeof(tStartInfo));
  strcpy(tStartInfo.szDevPath, strDev.c_str());
  tStartInfo.nDbgLv = nDbgLv;

  /* api init */
  AX_S32 ret = AX_Channel_Slave_Init(&tStartInfo, ChannelMessageHandler, (AX_VOID*)&_dispatcher);
  if (0 != ret) {
    printf("slave ax channel init failed, ret = 0x%x\n", ret);
    return ret;
  }

  while (!gExitThread) {
    sleep(1);

    /* notify data to master, optional */
    if (bNotify) {
      AX_Channel_Slave_Notify(nNotifyType, (AX_U8*)&tNotify, sizeof(tNotify));
      tNotify.nData1 += 1;
      tNotify.nData2 += 2;
    }
  }

__EXIT:
  AX_S32 channelRet = 0;
  if (0 == ret) {
    channelRet = 0;
    printf("sample slave run success\n");
  } else {
    channelRet = 1;
    printf("sample slave run fail, ret = 0x%x\n", ret);
  }

  /* api de-init */
  AX_Channel_Slave_DeInit();

  _dispatcher.DeInit();

  return channelRet;
}
