#pragma once
#include <stdio.h>
#include <string.h>
#include "ax_base_type.h"

/* cmd define */
typedef enum {
  CMD_001 = 128,  /* master send small packet */
  CMD_002,        /* master read small packet */
  CMD_003,        /* master send large packet */
  CMD_004,        /* master read large packet */
  CMD_005,        /* master send small packet (for multi-thread test) */
  CMD_006,        /* master read small packet (for multi-thread test) */
  CMD_007,        /* master send large packet (for multi-thread test) */
  CMD_008,        /* master read large packet (for multi-thread test) */
  CMD_009,        /* master upload file to slave */
  CMD_010,        /* master download file from slave */
  CMD_MAX
} USER_CMD_E;

/* notify define */
typedef enum {
  NOTIFY_ID_001 = 4096,
  // pending
  NOTIFY_ID_MAX
} USER_NOTIFY_E;

/* default timeout(ms) */
#define TIMEOUT (5000)

/* cmd => low 8 bit, subcmd => high 8 bit */
#define MAKE_WORD(l, h) (l | ((h & 0xFFFF) << 8))

/* field comparator */
#define CMP_FIELD(cls, p, t, member) { \
  if (p->member != t.member) { \
    printf(""#cls" member: ["#member"] not equal, v1[%d] v2[%d]\n", p->member, t.member); \
    return AX_FALSE; \
  } \
}

/* small packet(< 512 bytes) */
typedef struct _CASE_S_PACKET_T {
  AX_U32 nSeqNum;
  AX_U32 nIspGain;
  AX_U32 nAGain;
  AX_U32 nDGain;
  AX_U32 nIntegrationTime;
  AX_U32 nHdrRatio;
  AX_U8  nHcgLcg;
  AX_U32 nLuxIndex;

  _CASE_S_PACKET_T() {
    Init();
  }

  AX_VOID Init() {
    memset(this, 0x0, sizeof(_CASE_S_PACKET_T));
  }

  AX_BOOL IsEqual(const _CASE_S_PACKET_T & data) const {
    CMP_FIELD(CASE_S_PACKET_T, this, data, nSeqNum)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nIspGain)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nAGain)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nDGain)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nIntegrationTime)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nHdrRatio)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nHcgLcg)
    CMP_FIELD(CASE_S_PACKET_T, this, data, nLuxIndex)
    return AX_TRUE;
  }

} CASE_S_PACKET_T;

/* large packet(> 64 kb) */
#define PACKET_SIZE (65536 * 2 + 256)
typedef struct _CASE_L_PACKET_T {
  AX_U8 aData[PACKET_SIZE];

  _CASE_L_PACKET_T() {
    Init();
  }

  AX_VOID Init() {
    memset(aData, 0x0, sizeof(_CASE_L_PACKET_T));
  }

  AX_BOOL IsEqual(const _CASE_L_PACKET_T & data) const {
    for (AX_U32 i = 0;i < PACKET_SIZE;i++) {
      if (aData[i] != data.aData[i]) {
        printf("CASE_L_PACKET_T packet data index[%d] mismatched\n", i);
        return AX_FALSE;
      }
    }
    return AX_TRUE;
  }

} CASE_L_PACKET_T;

/* notify data */
typedef struct _NOTIFY_DATA_001 {
  AX_U32 nData1;
  AX_U32 nData2;

  _NOTIFY_DATA_001() {
    memset(this, 0, sizeof(_NOTIFY_DATA_001));
  }

} NOTIFY_DATA_001;
