#!/bin/sh

# usage
# ./SourceSink.sh usb2  ./SourceSink.sh usb3  ./SourceSink.sh usb2 disable, ./SourceSink.sh usb2 2
# note, $2 can be disable or pattern(0,1,2)

usb_dev_controller=${1}_sourcesink
export CONFIGFS_HOME=/etc/configfs
UDC=$CONFIGFS_HOME/usb_gadget/${usb_dev_controller}/UDC

# modify following to adapt different chip
USB_COUNT=1
USB1_NAME=usb2
USB1=4800000.usb



if [ $# == 0 ] || [ $# -ge 3 ]; then
	echo "usage:"
	echo "- ./SourceSink.sh usb2  ./SourceSink.sh usb3"
    echo "- ./SourceSink.sh usb2 disable ./SourceSink.sh usb2 2"
	echo "- arg2: disable or pattern(0,1,2), example: ./SourceSink.sh usb2 2"
	exit 0
fi

if [ ${USB_COUNT} == 1 ] && [ $1 != ${USB1_NAME} ]; then
    echo "don't support: ${1}"
    exit 0
fi

if [ ${USB_COUNT} == 2 ] && [ $1 != ${USB1_NAME} ] && [ $1 != ${USB2_NAME} ]; then
    echo "don't support: ${1}"
    exit 0
fi

if [ $# == 2 ]; then
	if [ $2 == "disable" ]; then
		echo > ${UDC}
		exit 0
	fi

    if [ $2 != 0 ] && [ $2 != 1 ] && [ $2 != 2 ]; then
        echo "error cmd: ${0} ${1} ${2}"
        exit 0
    fi
fi


if [ -f ${UDC} ]; then
	read line < ${UDC}
	if [ -n "${line}" ]; then
		echo "usb gadget has enabled" 	# so we don't need to enable it again
		exit 0
	fi
fi

if [ -f ${UDC} ]; then
	echo "enable usb source-sink gadget"
	if [ ${1} = ${USB1_NAME} ]; then
		echo ${USB1} > ${UDC}
	else
		echo ${USB2} > ${UDC}
	fi

	exit 0
fi


mkdir -p $CONFIGFS_HOME
if [ ! -d "${CONFIGFS_HOME}/usb_gadget/" ]; then
	mount none $CONFIGFS_HOME -t configfs
fi

mkdir -p $CONFIGFS_HOME/usb_gadget/${usb_dev_controller}
cd $CONFIGFS_HOME/usb_gadget/${usb_dev_controller}

echo 0x0525 > idVendor
echo 0xa4a0 > idProduct

mkdir -p strings/0x409
echo 20210726 > strings/0x409/serialnumber
echo "AXERA" > strings/0x409/manufacturer
echo "usb SourceSink gadget" > strings/0x409/product

mkdir -p configs/c.1

mkdir -p configs/c.1/strings/0x409
echo SourceSink > configs/c.1/strings/0x409/configuration
echo 120 > configs/c.1/MaxPower

mkdir -p functions/SourceSink.usb0

# configure device test param
if [ $# == 2 ]; then
	echo $2 > functions/SourceSink.usb0/pattern
else
	echo 0 > functions/SourceSink.usb0/pattern
fi

echo 40960 > functions/SourceSink.usb0/bulk_buflen
echo 10 > functions/SourceSink.usb0/bulk_qlen

echo 1 > functions/SourceSink.usb0/iso_qlen
echo 1 > functions/SourceSink.usb0/isoc_interval
echo 0 > functions/SourceSink.usb0/isoc_maxburst
echo 0 > functions/SourceSink.usb0/isoc_mult
# for mc40 dwc3 usb2.0, maxpacket_limit=512
if [ ${1} = "usb2" ]; then
	echo 512 > functions/SourceSink.usb0/isoc_maxpacket
else
	echo 1024 > functions/SourceSink.usb0/isoc_maxpacket
fi

ln -s functions/SourceSink.usb0 configs/c.1


echo "enable usb source-sink gadget"
if [ ${1} = ${USB1_NAME} ]; then
	echo ${USB1} > UDC
else
	echo ${USB2} > UDC
fi
