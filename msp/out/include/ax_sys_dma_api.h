/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/


#ifndef _HAL_DMA_H_
#define _HAL_DMA_H_

#include <base_types.h>
#include <ax_base_type.h>

typedef unsigned int AXI_DMA_HAND_T;
#ifndef AX_SUCCESS
#define AX_SUCCESS                          0
#endif
enum axi_dma_status {
    AX_ERR_DMA_BUSY = 0x80100011,
    AX_ERR_DMA_TIMEOUT = 0x80100027,
    AX_ERR_DMA_IO_FAIL = 0x80100081,
};

typedef enum axi_dma_xfer_mode {
    AX_DMA_TRF_SINGLE = 0,
    AX_DMA_TRF_LLI,
} axi_dma_xfer_mode_t;

typedef enum {
    AX_DMA_MEM_TO_MEM = 0,
    AX_DMA_MEM_TO_DEV,
    AX_DMA_DEV_TO_MEM,
} axi_dma_xfer_direction_t;

typedef struct {
    u64 src_addr;
    u64 dst_addr;
    u64 xfer_len;
} AXI_DMA_XFER_T;

typedef struct {
    axi_dma_xfer_mode_t xfer_mode;
    axi_dma_xfer_direction_t direction;
    u32 lli_num;
    u32 per_req_num;
} AXI_DMA_CFG_T;

#define AX_LLI_TRF_SIZE     0x1000000

#define AX_DMA_POLL_MODE        0x0
#define AX_DMA_BROKED_MODE      0x1

#define AX_MAX_DMA_CHN          0x2

#define AX_DMA_WAITING_FOREVER  (-1)

/* peripheral dma request interface */
#define AX_SSI_DMA_RX_REQ       1
#define AX_SSI_DMA_TX_REQ       0
#define AX_SSI_S_DMA_RX_REQ 10
#define AX_SSI_S_DMA_TX_REQ 11
#define AX_UART0_RX_REQ     45
#define AX_UART0_TX_REQ     46
#define AX_UART1_RX_REQ     47
#define AX_UART1_TX_REQ     48
#define AX_UART2_RX_REQ     49
#define AX_UART2_TX_REQ     50
#define AX_UART3_RX_REQ     51
#define AX_UART3_TX_REQ     52

AXI_DMA_HAND_T AX_HAL_DMA_ChnOpen();
AX_S32 AX_HAL_DMA_ChnCfg(AXI_DMA_HAND_T handle, AXI_DMA_CFG_T *dma_cfg, AXI_DMA_XFER_T *dma_xfer_cfg);
AX_S32 AX_HAL_DMA_ChnStart(AXI_DMA_HAND_T handle);
AX_S32 AX_HAL_DMA_ChnWaitDone(AXI_DMA_HAND_T handle, u32 blocked, u32 timeout);
AX_S32 AX_HAL_DMA_ChnClose(AXI_DMA_HAND_T handle);
AX_S32 AX_HAL_DMA_ChnLliNumCfg(AXI_DMA_HAND_T handle, u32 lli_num);

#endif
