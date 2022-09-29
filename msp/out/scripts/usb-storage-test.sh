#!/bin/sh

if [ $# == 1 ]; then
	if [ $1 == "help" ] || [ $1 == "-h" ]; then
		echo "usage: ./test-usb-storage.sh test_dev test_file_size(MB) test_times"
		echo "./test-usb-storage.sh or ./test-usb-storage.sh sdb1 100 10"
		exit 0
	fi
fi


if [ $# == 0 ]; then
	test_dev=sda1
	test_file_size=1000 #1000MB
	test_times=1
else
	test_dev=$1
	test_file_size=$2 #MB
	test_times=$3
fi


test_path=usb-test


echo "mount usb-storage"
mkdir -p ${test_path}
mount /dev/$test_dev ${test_path}
sleep 0.1


echo "usb-storage performance test: dev:${test_dev}, size:${test_file_size}MB, loop times:${test_times}"

while [ $test_times -gt 0 ]
do
	if [ ! -d "${test_path}/largefile" ]; then
		rm -f ${test_path}/largefile
	fi

	echo -e "\ntest usb-storage write performance"
	cnt=$((test_file_size*10))
	time dd if=/dev/zero of=${test_path}/largefile bs=100k count=${cnt}
	sleep 0.1


	echo -e "\ntest usb-storage read performance"
	sync && echo 3 > /proc/sys/vm/drop_caches
	sleep 0.5
	time dd if=${test_path}/largefile of=/dev/null bs=10k

	test_times=$((test_times-1))
	sleep 0.1
done

umount ${test_path}
