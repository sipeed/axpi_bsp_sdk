#include <sys/stat.h>
#include "Proc004.h"

#define MAX_BUF_SIZE (1024*1024) // 1MB

AX_VOID CProc004::Init(AX_VOID* pParam) {
  cmdline::parser *pParser = (cmdline::parser *)pParam;

  const string strDl = pParser->get<string>("download");
  const string strUpload = pParser->get<string>("upload");

  if (!strDl.empty()) {
    if (!(m_fp = fopen(strDl.c_str(), "rb")))
    {
      printf("Failed to open file: %s, err: %s\n", strDl.c_str(), strerror(errno));
    }
    else {
      struct stat sfp;
      stat(strDl.c_str(), &sfp);

      m_nTotalSize = sfp.st_size;
      m_nSendSize = 0;
    }
  }
  else if (!strUpload.empty()) {
    if (!(m_fp = fopen(strUpload.c_str(), "wb")))
    {
      printf("Failed to create file: %s, err: %s\n", strUpload.c_str(), strerror(errno));
    }
  }
}

AX_VOID CProc004::DeInit(AX_VOID) {
  if (m_fp) {
    fclose(m_fp);
  }
}

AX_BOOL CProc004::RunProcess(AX_U8 nCmd, AX_U8 nSubCmd, AX_U8* pData, AX_U32 nSize) {

  // Note: "pData" and "size" correspond to the "pSendData" and "nSendSize" that you invoked
  // "AX_Channel_Master_SendAndRecv" with on master side, this is optional.

  switch (nCmd)
  {
  case CMD_009: // slave recv data
    {
      if (m_fp && pData && nSize > 0) {
        fwrite(pData + 4, 1, nSize - 4, m_fp);

        AX_U32 nPercent = *((AX_U32*)pData);
        if (nPercent >= 100) {
          fseek(m_fp, 0, SEEK_SET);
        }
      }

      AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, NULL, 0);
      if (0 != ret) {
        printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
      }
    }
    break;
  case CMD_010: // slave send data
    {
      AX_U8* pBufData = nullptr;
      AX_U32 nBufSize = 0;

      AX_U32 nRealSize = 0;
      AX_U32 nPercent = 0;
      AX_U8  abuf[MAX_BUF_SIZE + 4/*for progress info*/] = {0};

      if (m_fp) {
        nRealSize = fread(abuf + 4, 1, MAX_BUF_SIZE, m_fp);
        if (nRealSize > 0) {
          m_nSendSize += nRealSize;

          nPercent = (((float)m_nSendSize) / ((float)m_nTotalSize)) * 100;
          memcpy(abuf, &nPercent, sizeof(AX_U32));

          pBufData = abuf;
          nBufSize = nRealSize + 4;
        }
        else {
          m_nSendSize = 0;
          fseek(m_fp, 0, SEEK_SET);
        }
      }

      AX_S32 ret = AX_Channel_Slave_WriteData(nCmd, nSubCmd, pBufData, nBufSize);
      if (0 != ret) {
        printf("cmd[%d]: slave write back data failed, ret: 0x%x\n", nCmd, ret);
      }
    }
    break;
  default:
    break;
  }
  return AX_TRUE;
}
