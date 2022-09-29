#!/bin/bash -e

SYSDIR=/sys/kernel/config/usb_gadget/
DEVDIR=$SYSDIR/$1

# These are the default values that will be used if you have not provided
# an explicit value in the environment.
: ${USB_IDVENDOR:=0x2222}
: ${USB_IDPRODUCT:=0x1111}
: ${USB_BCDDEVICE:=0x0100}
: ${USB_BCDUSB:=0x0200}
: ${USB_SERIALNUMBER:=20220805}
: ${USB_PRODUCT:="axera usb rndis gadget"}
: ${USB_MANUFACTURER:="axera-chip"}
: ${USB_MAXPOWER:=250}
: ${USB_CONFIG:=c.1}

echo "Creating USB gadget $1"

mkdir -p $DEVDIR

echo $USB_IDVENDOR > $DEVDIR/idVendor
echo $USB_IDPRODUCT > $DEVDIR/idProduct
echo $USB_BCDDEVICE > $DEVDIR/bcdDevice
echo $USB_BCDUSB > $DEVDIR/bcdUSB

mkdir -p $DEVDIR/strings/0x409
echo "$USB_SERIALNUMBER" > $DEVDIR/strings/0x409/serialnumber
echo "$USB_MANUFACTURER"        > $DEVDIR/strings/0x409/manufacturer
echo "$USB_PRODUCT"   > $DEVDIR/strings/0x409/product

mkdir -p $DEVDIR/configs/$USB_CONFIG
echo $USB_MAXPOWER > $DEVDIR/configs/$USB_CONFIG/MaxPower

for func in $USB_FUNCTIONS; do
	echo "Adding function $func to USB gadget $1"
	mkdir -p $DEVDIR/functions/$func
	ln -s $DEVDIR/functions/$func $DEVDIR/configs/$USB_CONFIG
done

udevadm settle -t 5 || :
ls /sys/class/udc/ > $DEVDIR/UDC
