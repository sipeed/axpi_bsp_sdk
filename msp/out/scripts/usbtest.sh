#!/bin/sh

# preparetion, host: testusb, device: usb-sourcesink.sh
# usage: ./usbtest.sh test_transfer_type test_time
# usbtest.sh, usbtest.sh all 100, usbtest.sh bulk 100, usbtest.sh performance

# Warraning!! since four transfer and performance test use different pattern,
# so we need to reboot after testing performance/four transfer
# if we want to test four transfer/performance.


# control, bulk, int, iso, all, performance
if [ ! -n "$1" ] ;then
	test_transfet_type=all
else
    if [ $1 != "control" ] && [ $1 != "bulk" ] && [ $1 != "int" ] &&
       [ $1 != "iso" ] && [ $1 != "performance" ] && [ $1 != "all" ];then
        echo -e "don't support cmd: ${0} ${1}"
        echo -e "arg1 must be: control/bulk/int/iso/performance"
        exit 0;
    else
        test_transfet_type=$1
    fi
fi

# test time
if [ ! -n "$2" ] ;then
	test_time=1
else
    test_time=$2
fi

if [ "$1" == "performance" ] ;then
    echo -e "usbtest, test type: ${test_transfet_type}"
else
    echo -e "usbtest, test type: ${test_transfet_type}, test times: ${test_time}"
fi


# configure host usbtest param
# choose intf alt 1
echo 1 > /sys/bus/usb/drivers/usbtest/module/parameters/alt
# pattern, 0 (all zeros, do check), 1 (mod63, do check), 2 (none, don't check)
echo 0 > /sys/bus/usb/drivers/usbtest/module/parameters/pattern


echo -e "\n=====start usb test=====\n"

if [ $test_transfet_type == "control" ] || [ $test_transfet_type == "all" ]; then
	echo -e "==test usb control transfer=="

	# control ch9 subset test
	echo -e "=control ch9 subset test=\n"
	sleep 0.1
	testusb -a -t 9 -c $test_time

	# control in & out
	echo -e "\n=control in & out test=\n"
	sleep 0.1
	testusb -a -t 14 -s 1024 -v 0 -c $test_time
	# testusb -a -t 14 -s 1024 -v 1 -c 1
fi

if [ $test_transfet_type == "bulk" ] || [ $test_transfet_type == "all" ]; then
	echo -e "\n==test usb bulk transfer==\n"
	sleep 0.1
	# bulk out
	testusb -a -t 1 -s 40960 -c $test_time

	echo ""
	# bulk in
	testusb -a -t 2 -s 40960 -c $test_time
fi

if [ $test_transfet_type == "int" ] || [ $test_transfet_type == "all" ]; then
	echo -e "\n==test usb interrupt transfer==\n"
	sleep 0.1
	# int out
	testusb -a -t 25 -s 512 -c $test_time

	echo ""
	# int in
	testusb -a -t 26 -s 512 -c $test_time
fi

if [ $test_transfet_type == "iso" ] || [ $test_transfet_type == "all" ]; then
	echo -e "\n==test usb iso transfer==\n"
	sleep 0.1
	# iso out, param -c value must be >= 2
	if [ ! -n "$2" ] || [ $test_time == 1 ] ;then
		testusb -a -t 15 -s 1024 -g 1 -c 2
	else
		testusb -a -t 15 -s 1024 -g 1 -c $test_time
	fi

	echo ""
	# iso in, param -c value must be >= 2
	if [ ! -n "$2" ] || [ $test_time == 1 ] ;then
		testusb -a -t 16 -s 1024 -g 1 -c 2
	else
		testusb -a -t 16 -s 1024 -g 1 -c $test_time
	fi
fi


if [ $test_transfet_type == "performance" ]; then
	echo -e "==test usb performance==\n"
	sleep 0.1
	# write performance
	testusb -a -t 27 -s 409600 -c 100

	echo ""
	# read performance
	testusb -a -t 28 -s 409600 -c 100
fi

echo -e "\n=====usb test end=====\n"
