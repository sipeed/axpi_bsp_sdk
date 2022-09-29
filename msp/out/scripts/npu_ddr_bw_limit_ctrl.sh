#!/bin/sh
# usage:
#   source ./npu_ddr_bw_limit_ctrl.sh disable
#   source ./npu_ddr_bw_limit_ctrl.sh bw_param
#     bw_param : 3.8GB as default config
#              : other config like [2.1GB, 2.8GB, 3.3GB, 3.8GB, 4.4GB, 5.5GB, 6.2GB, 6.9GB, 7.6GB, 8.3GB, 9.0GB, 9.9GB]
    # AX_NPU_DDR_BW_LIMIT_DISABLE = 0,
    # AX_NPU_DDR_BW_LIMIT_2P1GB = 0x4,    // DRR BW 2.22 GB (from perf monitor)
    # AX_NPU_DDR_BW_LIMIT_2P8GB = 0x6,    // DRR BW 2.77 GB (from perf monitor)
    # AX_NPU_DDR_BW_LIMIT_3P5GB = 0x8,    // DRR BW 3.33 GB (from perf monitor)
    # AX_NPU_DDR_BW_LIMIT_4P1GB = 0xA,    // DRR BW 3.88 GB (from perf monitor)
    # AX_NPU_DDR_BW_LIMIT_4P8GB = 0xC,    // DRR BW 4.44 GB (from perf monitor)
    # AX_NPU_DDR_BW_LIMIT_5P5GB = 0xE,    // the below options are reserved for debug
    # AX_NPU_DDR_BW_LIMIT_6P2GB = 0x10,
    # AX_NPU_DDR_BW_LIMIT_6P9GB = 0x12,
    # AX_NPU_DDR_BW_LIMIT_7P6GB = 0x14,
    # AX_NPU_DDR_BW_LIMIT_8P3GB = 0x16,
    # AX_NPU_DDR_BW_LIMIT_9P0GB = 0x18,
    # AX_NPU_DDR_BW_LIMIT_9P9GB = 0x1A,
    # AX_NPU_DDR_BW_LIMIT_MAX

if [ $# == 0 ] || [ $# -ge 2 ]; then
    echo "----usage:----"
    echo "./npu_ddr_bw_limit_ctrl.sh param0_bw"
    echo "      param0_bw :  one of these options {disable, 2.1GB, 2.8GB, 3.3GB, 3.8GB, 4.4GB}"
    exit 0
fi

if [ "${1}" != "disable" ] && [ "${1}" != "2.1GB" ] && [ "${1}" != "2.8GB" ] &&
   [ "${1}" != "3.3GB" ] && [ "${1}" != "3.8GB" ] && [ "${1}" != "4.4GB" ] &&
   [ "${1}" != "5.5GB" ] && [ "${1}" != "6.2GB" ] && [ "${1}" != "6.9GB" ] &&
   [ "${1}" != "7.6GB" ] && [ "${1}" != "8.3GB" ] && [ "${1}" != "9.0GB" ] && [ "${1}" != "9.9GB" ]; then
    echo "invalid BW param ,please select one of {disable, 2.1GB, 2.8GB, 3.3GB, 3.8GB, 4.4GB}"
    exit 0
fi

npu_ddr_bw_limit_opt=${1}

if [ "$npu_ddr_bw_limit_opt" == "disable" ]; then
    devmem 0x3806038 32 0x840
    devmem 0x380603c 32 0x2000
    devmem 0x3806040 32 0x400
else
    devmem 0x3806038 32 0x842
    devmem 0x380603c 32 0x20
    if [ "$npu_ddr_bw_limit_opt" == "2.1GB" ]; then
        devmem 0x3806040 32 0x4
    elif [ "$npu_ddr_bw_limit_opt" == "2.8GB" ]; then
        devmem 0x3806040 32 0x6
    elif [ "$npu_ddr_bw_limit_opt" == "3.3GB" ]; then
        devmem 0x3806040 32 0x8
    elif [ "$npu_ddr_bw_limit_opt" == "3.8GB" ]; then
        devmem 0x3806040 32 0xA
    elif [ "$npu_ddr_bw_limit_opt" == "4.4GB" ]; then
        devmem 0x3806040 32 0xC
    elif [ "$npu_ddr_bw_limit_opt" == "5.5GB" ]; then
        devmem 0x3806040 32 0xE
    elif [ "$npu_ddr_bw_limit_opt" == "6.2GB" ]; then
        devmem 0x3806040 32 0x10
    elif [ "$npu_ddr_bw_limit_opt" == "6.9GB" ]; then
        devmem 0x3806040 32 0x12
    elif [ "$npu_ddr_bw_limit_opt" == "7.6GB" ]; then
        devmem 0x3806040 32 0x14
    elif [ "$npu_ddr_bw_limit_opt" == "8.3GB" ]; then
        devmem 0x3806040 32 0x16
    elif [ "$npu_ddr_bw_limit_opt" == "9.0GB" ]; then
        devmem 0x3806040 32 0x18
    elif [ "$npu_ddr_bw_limit_opt" == "9.9GB" ]; then
        devmem 0x3806040 32 0x1A
    else
        echo "invalid BW param!!!"
        # exit 0
    fi

    # echo "set npu ddr limit ctrl value: ${1}"
    # dev_mem 0x3806038
    # dev_mem 0x380603c
    # dev_mem 0x3806040

fi
