#!/bin/bash

size=4147200
#size=16588800
reg_wait=0x30188
reg_buf0=0x434000000
reg_buf1=0x436000000
reg_buf2=0x438000000
#mod=67
mod=440

#get_file seq_index buf_index
function get_file ()
{ 
    #0_0_99_0s_000000000ns.raw
    #FILE=data/yuv/$[$1 *3 + $2].CameraDeviceAR0820_GroupA_0_3840x2160.yuv
    FILE=data/yuv2/$[$1 *3 + $2].CameraDeviceAR0233_GroupA_3_1920x1080.yuv
    #FILE=0_0_$[$1 * 3 + $2]-video.yuv
    echo $FILE
}

function wait () {
    readvalue=`../tools/reg_rw /dev/xdma1_user $reg_wait w | awk 'NR==5{print $8}'`
    while [ $readvalue = "0x00000003" ]
    do
        echo $readvalue
        readvalue=`../tools/reg_rw /dev/xdma1_user $reg_wait w | awk 'NR==5{print $8}'`
    done
    echo $readvalue
}

for ((i=1; i<=49; i++)); do
    #echo $i
    wait
    echo "send buffer 0"
    FILE_NAME=`get_file $i%$mod 0`
    ../tools/dma_to_device -d /dev/xdma1_h2c_0  -a $reg_buf0 -s  $size -f $FILE_NAME -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user $reg_wait w 1 > /dev/null
    echo "channel 6 "$FILE_NAME
    TIME=$(date "+%s.%N")
    echo $TIME

    wait
    echo "send buffer 1"
    FILE_NAME=`get_file $i%$mod 1`
    ../tools/dma_to_device -d /dev/xdma1_h2c_0  -a $reg_buf1 -s  $size -f $FILE_NAME -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user $reg_wait w 1 > /dev/null
    echo "channel 6 "$FILE_NAME
    TIME=$(date "+%s.%N")
    echo $TIME
    #sleep 0.2

    wait
    echo "send buffer 2"
    FILE_NAME=`get_file $i%$mod 2`
    ../tools/dma_to_device -d /dev/xdma1_h2c_0  -a $reg_buf2 -s  $size -f $FILE_NAME -v > /dev/null
    ../tools/reg_rw /dev/xdma1_user $reg_wait w 1 > /dev/null
    echo "channel 6 "$FILE_NAME
    TIME=$(date "+%s.%N")
    echo $TIME
    #sleep 0.2
done

exit 0
