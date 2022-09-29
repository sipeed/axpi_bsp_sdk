#!/bin/sh
insmod /opt/ko/ax_osal.ko
insmod /opt/ko/ax_sys.ko
insmod /opt/ko/ax_cmm.ko cmmpool=anonymous,0,0x80000000,2047M
insmod /opt/ko/ax_npu.ko
insmod /opt/ko/ax_sdio_host.ko
insmod /opt/ko/ax_pool.ko
insmod /opt/ko/ax_proton.ko
insmod /opt/ko/ax_mipi.ko
insmod /opt/ko/ax_venc.ko
insmod /opt/ko/ax_jenc.ko
insmod /opt/ko/ax_vdec.ko
insmod /opt/ko/ax_audio.ko
insmod /opt/ko/ax_vo.ko
insmod /opt/ko/ax_ivps.ko
insmod /opt/ko/ax_gdc.ko
insmod /opt/ko/ax_hwinfo.ko
insmod /opt/ko/ax_pcal6416a.ko
insmod /opt/ko/ax_logctl.ko
insmod /opt/ko/ax_dma_hal.ko
insmod /opt/ko/ax_ddr_retention.ko
insmod /opt/ko/ax_ives.ko
insmod /opt/ko/ax_rtc.ko
bash /opt/scripts/npu_ddr_bw_limit_ctrl.sh 3.8GB
exit 0
