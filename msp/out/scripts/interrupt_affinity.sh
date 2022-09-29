#!/bin/sh

#***NOTES: dispatch irqs to specified cpus

#****************************************
#function: dispatch_irq2cpu
#param1: irq_num
#param2: cpu_val
#****************************************
function dispatch_irq2cpu(){
	if [ $# -lt 2 ];then
		echo "[error] @irq2cpu: too few parameters"
		return
	fi
	local irq=$1
	local cpu_val=$2

	echo $cpu_val > /proc/irq/$irq/smp_affinity
	return
}

#****************************************
#function: dispatch_irqs2cpu
#param1: interrupt name list
#param2: cpu_id
#****************************************
function dispatch_irqs2cpu(){
	if [ $# -lt 2 ]; then
		echo "[error] @interrupts_2_cpu: too few parameters"
		return
	fi
	local cpu_id=${@: -1}
	local index=0
	let cpu_val=2**$cpu_id

	for name in $@
	do
		index=`expr $index + 1`
		if [ $index -ge $# ];then
			break
		fi
		irqs=$(cat /proc/interrupts | grep -w $name | awk -F : '{print $1}' | sed 's/^[ \t]*//g')
		for irq in $irqs
		do
			dispatch_irq2cpu $irq $cpu_val
		done
	done
}

#dispatch interrupts to cpu3
cpu3_irqs=""eth0""
dispatch_irqs2cpu $cpu3_irqs 3

#dispatch interrupts to cpu2
cpu2_irqs=""npu" "nput" "isp" "dwc3" "4800000.usb""
dispatch_irqs2cpu $cpu2_irqs 2

#dispatch interrupts to cpu1
cpu1_irqs=""venc" "jpegenc" "jpeg_decoder" "5650000.off_scaler" "ax_decoder""
dispatch_irqs2cpu $cpu1_irqs 1

