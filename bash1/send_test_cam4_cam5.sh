#!/bin/bash

function wait_cam5 () {
    readvalue=`../tools/reg_rw /dev/xdma1_user 0x30184 w | awk 'NR==5{print $8}'`
    while [ $readvalue = "0x00000003" ]
    do
        echo $readvalue
        readvalue=`../tools/reg_rw /dev/xdma1_user 0x30184 w | awk 'NR==5{print $8}'`
    done
    echo $readvalue
}

function wait_cam4 () {
    readvalue=`../tools/reg_rw /dev/xdma1_user 0x30180 w | awk 'NR==5{print $8}'`
    while [ $readvalue = "0x00000003" ]
    do
        echo $readvalue
        readvalue=`../tools/reg_rw /dev/xdma1_user 0x30180 w | awk 'NR==5{print $8}'`
    done
    echo $readvalue
}

for ((i=0; i<=30000000; i++)); do
    #echo $i

    wait_cam4
    echo "send cam4 buffer 0"
    #../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x428000000 -s  4147200 \
    # -f /home/ubuntu/sensing/dma_ip_drivers-master/XDMA/linux-kernel/tests/data/picture19201080.yuv -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user 0x30180 w 1 > /dev/null

    wait_cam5
    echo "send cam5 buffer 0"
    #../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x42e000000 -s  4147200 \
    # -f /home/ubuntu/sensing/dma_ip_drivers-master/XDMA/linux-kernel/tests/data/picture519201080.yuv -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user 0x30184 w 1 > /dev/null

    wait_cam4
    echo "send cam4 buffer 1"
    #../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x42a000000 -s  4147200 \
    # -f /home/ubuntu/sensing/dma_ip_drivers-master/XDMA/linux-kernel/tests/data/picture19201080.yuv -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user 0x30180 w 1 > /dev/null

    wait_cam5
    echo "send cam5 buffer 1"
    #../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x430000000 -s  4147200 \
    # -f /home/ubuntu/sensing/dma_ip_drivers-master/XDMA/linux-kernel/tests/data/picture519201080.yuv -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user 0x30184 w 1 > /dev/null
    #sleep 0.2

    wait_cam4
    echo "send cam4 buffer 2"
    #../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x42c000000 -s  4147200 \
    # -f /home/ubuntu/sensing/dma_ip_drivers-master/XDMA/linux-kernel/tests/data/picture19201080.yuv -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user 0x30180 w 1 > /dev/null

    wait_cam5
    echo "send cam5 buffer 2"
    #../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x432000000 -s  4147200 \
    # -f /home/ubuntu/sensing/dma_ip_drivers-master/XDMA/linux-kernel/tests/data/picture519201080.yuv -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user 0x30184 w 1 > /dev/null
    #sleep 0.2
done

exit 0