#!/bin/bash
# 2M 4147200
# 8M 16588800
# 16634880 4915200
function transfer_start () {
    echo "transfer start"
	#sleep 1
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 0 -r ${1} -f ./data/yuv/${9}  -t ${17} -s $[${25}*${33}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 1 -r ${2} -f ./data/yuv/${10} -t ${18} -s $[${26}*${34}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 2 -r ${3} -f ./data/yuv/${11} -t ${19} -s $[${27}*${35}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 3 -r ${4} -f ./data/yuv/${12} -t ${20} -s $[${28}*${36}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 4 -r ${5} -f ./data/yuv/${13} -t ${21} -s $[${29}*${37}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 5 -r ${6} -f ./data/yuv/${14} -t ${22} -s $[${30}*${38}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 6 -r ${7} -f ./data/yuv/${15} -t ${23} -s $[${31}*${39}*2] &
    ../tools/dma_to_device_video -C 0 -d /dev/xdma0_h2c_0  -c 7 -r ${8} -f ./data/yuv/${16} -t ${24} -s $[${32}*${40}*2] &
}

function transfer_stop () {
    echo "transfer stop"
    sudo killall dma_to_device_video
}
