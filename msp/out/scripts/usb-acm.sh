#!/bin/sh

# usage
# ./usb-acm.sh usb2 or ./usb-acm.sh usb3
# ./usb-acm.sh usb2 disable or ./usb-acm.sh usb3 disable

usb_dev_controller=${1}_acm
export CONFIGFS_HOME=/etc/configfs
UDC=$CONFIGFS_HOME/usb_gadget/${usb_dev_controller}/UDC

# modify following to adapt different chip
USB_COUNT=1
USB1_NAME=usb2
USB1=4800000.usb


if [ $# == 0 ] || [ $# -ge 3 ]; then
	echo "usage:"
	echo "./usb-acm.sh usb2 or ./usb-acm.sh usb3"
	echo "./usb-acm.sh usb2 disable or ./usb-acm.sh usb3 disable"
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

    echo "error cmd: ${0} ${1} ${2}"
    exit 0
fi


if [ -f ${UDC} ]; then
	read line < ${UDC}
	if [ -n "${line}" ]; then
		echo "usb ACM has enabled" 	# so we don't need to enable it again
		exit 0
	fi
fi

if [ -f ${UDC} ]; then
	echo "enable usb ACM gadget"
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

echo 0x2222 > idVendor
echo 0x1111 > idProduct

mkdir -p strings/0x409
echo 20200210 > strings/0x409/serialnumber
echo "axera-chip" > strings/0x409/manufacturer
echo "axera usb acm serial gadget" > strings/0x409/product

mkdir -p configs/c.1

mkdir -p configs/c.1/strings/0x409
echo acm > configs/c.1/strings/0x409/configuration
echo 120 > configs/c.1/MaxPower

mkdir -p functions/acm.usb0

ln -s functions/acm.usb0 configs/c.1


echo "enable usb ACM gadget"
if [ ${1} = ${USB1_NAME} ]; then
	echo ${USB1} > UDC
else
	echo ${USB2} > UDC
fi
