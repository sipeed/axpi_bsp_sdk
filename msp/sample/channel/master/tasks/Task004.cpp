#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <chrono>
#include "Task004.h"

#define MAX_BUF_SIZE (1024*1024) // 1MB

AX_BOOL CTask004::RunTask(cmdline::parser *pParser) {
  AX_BOOL bRet = AX_FALSE;
  do {
    AX_S32 nRet = 0;
    AX_U32 nRealSize = 0;
    AX_U32 nPercent = 0;

    FILE *fp = NULL;
    AX_U8 abuf[MAX_BUF_SIZE + 4/*for progress info*/] = {0};

    const string strDl = pParser->get<string>("download");
    const string strUpload = pParser->get<string>("upload");

    auto beginTime = std::chrono::high_resolution_clock::now();

    if (!strDl.empty()) {
      if (!(fp = fopen(strDl.c_str(), "wb")))
      {
        printf("Failed to create file: %s, err: %s\n", strDl.c_str(), strerror(errno));
        break;
      }

      do {
        nRealSize = 0;
        nRet = DownloadPacket(abuf, MAX_BUF_SIZE + 4, &nRealSize);
        if (0 != nRet) {
          printf("cmd[%d]: master download packet data failed, ret: 0x%x\n", CMD_010, nRet);
          break;
        }
        else {
          if (nRealSize > 0) {
            nPercent = *((AX_U32*)abuf);
            nRealSize -= 4;

            fwrite(abuf + 4, 1, nRealSize, fp);

            PrintProgress(AX_TRUE, nPercent);
          }
        }
      }
      while (nRealSize > 0);

      fclose(fp);
    }
    else if (!strUpload.empty()) {

      AX_U64 nTotalSize = 0;
      AX_U64 nSendSize = 0;

      if (!(fp = fopen(strUpload.c_str(), "rb")))
      {
        printf("Failed to open file: %s, err: %s\n", strUpload.c_str(), strerror(errno));
        break;
      }
      else {
        struct stat sfp;
        stat(strUpload.c_str(), &sfp);

        nTotalSize = sfp.st_size;
        nSendSize = 0;
      }

      do {
        nRealSize = fread(abuf + 4, 1, MAX_BUF_SIZE, fp);
        if (nRealSize > 0) {

          nSendSize += nRealSize;
          nPercent = (((float)nSendSize) / ((float)nTotalSize)) * 100;
          memcpy(abuf, &nPercent, sizeof(AX_U32));

          nRet = UploadPacket(abuf, nRealSize + 4);
          if (0 != nRet) {
            printf("cmd[%d]: master upload packet data failed, ret: 0x%x\n", CMD_009, nRet);
            break;
          }

          PrintProgress(AX_FALSE, nPercent);
        }
      }
      while (nRealSize >= MAX_BUF_SIZE);

      fclose(fp);
    }

    if (0 == nRet) {
      auto endTime = std::chrono::high_resolution_clock::now();
      if (!strUpload.empty()) {
        printf("Total elapsed:     [%d ms]\n",
               (AX_U32)(std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count() / 1000));
      }
      else {
        printf("Total elapsed:       [%d ms]\n",
               (AX_U32)(std::chrono::duration_cast<std::chrono::microseconds>(endTime - beginTime).count() / 1000));
      }

      bRet = AX_TRUE;
    }
  }
  while (0);
  return bRet;
}

AX_VOID CTask004::PrintProgress(AX_BOOL bDownload, AX_U32 nPercent)
{
#define MOVEUP(x)    printf("\033[%dA", (x))
#define MOVERIGHT(y) printf("\033[%dC", (y))

#define MAX_PRINT_LEN (64)
#define SPACE_NUM     (51)
#define BAR_X_OFFSET  (1)

#define TIP_DL_STR "Download progress: "
#define TIP_UP_STR "Upload progress: "

#define TIP_DL_LEN strlen(TIP_DL_STR)
#define TIP_UP_LEN strlen(TIP_UP_STR)

  AX_U32 nTipOffset = bDownload ? TIP_DL_LEN : TIP_UP_LEN;

  if (-1 == m_nPercent) {
    AX_CHAR szLine[MAX_PRINT_LEN] = {0};

    if (bDownload) {
      strcat(szLine, TIP_DL_STR);
    }
    else {
      strcat(szLine, TIP_UP_STR);
    }

    strcat(szLine, "\n");
    printf(szLine);
  }

  MOVEUP(1);
  MOVERIGHT(nTipOffset + BAR_X_OFFSET + 1);
  printf("[%d%%]\n", nPercent);

  m_nPercent = nPercent;
}

AX_S32 CTask004::UploadPacket(AX_U8* pData, AX_U32 nSize)
{
  return AX_Channel_Master_SendAndRecv(CMD_009, 0/*subcmd*/, pData, nSize, NULL, 0, NULL, TIMEOUT);
}

AX_S32 CTask004::DownloadPacket(AX_U8* pData, AX_U32 nSize, AX_U32* pRealSize)
{
  return AX_Channel_Master_SendAndRecv(CMD_010, 0/*subcmd*/, NULL/*optional*/, 0/*optional*/, pData, nSize, pRealSize, TIMEOUT);
}
