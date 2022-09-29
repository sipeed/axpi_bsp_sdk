#!/bin/sh

#arg1: server/client mode: s or c
#arg2: test 1 or 2 emac port: 1 or 2
#arg3: dest ip addr0
#arg4: dest ip addr1
#arg5: test time

# for example
# server: ./net-perf-test.sh s
# client: ./net-perf-test.sh c 1 192.168.1.11 0 10
#         ./net-perf-test.sh c 2 192.168.1.11 192.167.1.11 10


dst_ip0=$3
dst_ip1=$4
test_time=$5
port0=11110
port1=11111
port2=11112
port3=11113


if [ $# == 0 ]; then
	echo "usage:"
	echo "on server: net-perf-test.sh s"
	echo "on client: net-perf-test.sh c test_emacs(1 or 2) dst_ip0 dst_ip1 test_times"
	exit 0
fi

echo -e "test network performance\n"

if [ $1 = "s" ]; then #server mode
	echo -e "server mode, wait client to connect...\n"
	kill -9 $(pidof iperf3)
	iperf3 -s -p ${port0} -A 1 &
	iperf3 -s -p ${port1} -A 2 &
	iperf3 -s -p ${port2} &
	iperf3 -s -p ${port3} &
else # client mode
	echo -e "client mode\n"
	if [ $# != 5 ]; then
		echo "usage: net-perf-test.sh c test_emacs(1 or 2) dst_ip0 dst_ip1 test_times"
		exit 0
	fi

	if [ $2 = "1" ]; then #test one emac
		echo "test one emac only"

		echo "test case1: tcp send"
		iperf3 -c ${dst_ip0} -t ${test_time} -p ${port0}

		echo "test case2: tcp recv"
		iperf3 -c ${dst_ip0} -t ${test_time} -R -p ${port0}

		echo "test case3: tcp dual test"
		iperf3 -c ${dst_ip0} -t ${test_time} -p ${port0} &
		iperf3 -c ${dst_ip0} -t ${test_time} -R -p ${port1}
		sleep 3

		echo "test case4: udp send"
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -p ${port0} -l 10k -A 1

		echo "test case5: udp recv"
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -R -p ${port0} -l 10k -A 1

		echo "test case6: udp dual test"
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -p ${port0} -l 10k -A 1 &
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -R -p ${port1} -l 10k -A 2
	else #test two emac
		echo "test two emacs simultaneously"

		echo "test case1: emac0 and emac1 tcp send"
		iperf3 -c ${dst_ip0} -t ${test_time} -p ${port0} &
		iperf3 -c ${dst_ip1} -t ${test_time} -p ${port1}
		sleep 3

		echo "test case2: emac0 and emac1 tcp recv"
		iperf3 -c ${dst_ip0} -t ${test_time} -R -p ${port0}	&
		iperf3 -c ${dst_ip1} -t ${test_time} -R -p ${port1}
		sleep 3

		echo "test case3: emac0 and emac1 tcp dual test"
		iperf3 -c ${dst_ip0} -t ${test_time} -p ${port0} &
		iperf3 -c ${dst_ip1} -t ${test_time} -p ${port1} &
		iperf3 -c ${dst_ip0} -t ${test_time} -R -p ${port2} &
		iperf3 -c ${dst_ip1} -t ${test_time} -R -p ${port3}
		sleep 5

		echo "test case4: emac0 and emac1 udp send"
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -p ${port0} &
		iperf3 -u -c ${dst_ip1} -t ${test_time} -b 1000M -p ${port1}
		sleep 3
		
		echo "test case5: emac0 and emac1 udp recv"
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -R -p ${port0} &
		iperf3 -u -c ${dst_ip1} -t ${test_time} -b 1000M -R -p ${port1}
		sleep 3

		echo "test case6: emac0 and emac1 udp dual test"
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -p ${port0} &
		iperf3 -u -c ${dst_ip1} -t ${test_time} -b 1000M -p ${port1} &
		iperf3 -u -c ${dst_ip0} -t ${test_time} -b 1000M -R -p ${port2} &
		iperf3 -u -c ${dst_ip1} -t ${test_time} -b 1000M -R -p ${port3}
	fi
fi

exit 0

