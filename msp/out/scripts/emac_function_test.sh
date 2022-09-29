#!/bin/sh

port_num=10
port=11000

test_type=$1

if [ $test_type == "udp_send" ]; then
    while [ $port_num -gt 0 ]
    do
        iperf3 -u -c 10.126.11.254 -b 10M -k 5000 -p ${port}
        if [ $? -eq 0 ]; then
            break
        fi

        port=$((port+1))
        port_num=$((port_num-1))
        sleep 0.1
    done
fi


port_num=10
port=11000

if [ $test_type == "udp_recv" ]; then
    while [ $port_num -gt 0 ]
    do
        iperf3 -u -c 10.126.11.254 -b 10M -t 5 -p ${port} -R
        if [ $? -eq 0 ]; then
            break
        fi

        port=$((port+1))
        port_num=$((port_num-1))
        sleep 0.1
    done
fi


port_num=10
port=11000

if [ $test_type == "tcp_send" ]; then
    while [ $port_num -gt 0 ]
    do
        iperf3 -c 10.126.11.254 -l 2000 -b 10m -n 5m -p ${port}
        if [ $? -eq 0 ]; then
            break
        fi

        port=$((port+1))
        port_num=$((port_num-1))
        sleep 0.1
    done
fi


port_num=10
port=11000

if [ $test_type == "tcp_recv" ]; then
    while [ $port_num -gt 0 ]
    do
        iperf3 -c 10.126.11.254 -l 2000 -b 10m -t 5 -p ${port} -R
        if [ $? -eq 0 ]; then
            break
        fi

        port=$((port+1))
        port_num=$((port_num-1))
        sleep 0.1
    done
fi