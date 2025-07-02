#!/bin/bash

size=4147200
#size=16588800
reg_wait=0x30170
reg_buf0=0x810000000
reg_buf1=0x812000000
reg_buf2=0x814000000
#mod=67
mod=4
tool_dma="../tools/dma_to_device"
tool_reg="../tools/reg_rw"

#get_file seq_index buf_index
function get_file ()
{ 
    #0_0_99_0s_000000000ns.raw
    #FILE=data/yuv/AR0820_cam_0_out_$[$1 *3 + $2].yuv
    FILE=data/yuv2/$[$1 *3 + $2].CameraDeviceAR0233_GroupA_3_1920x1080.yuv
    #FILE=0_0_$[$1 * 3 + $2]-video.yuv
    echo $FILE
}

function wait () {
    readvalue=`$tool_reg /dev/xdma1_user $reg_wait w | awk 'NR==5{print $8}'`
    while [ $readvalue = "0x00000003" ]
    do
        echo $readvalue
        readvalue=`$tool_reg /dev/xdma1_user $reg_wait w | awk 'NR==5{print $8}'`
    done
    echo $readvalue
}

for ((i=1; i<=10000; i++)); do
    #echo $i
    wait
    echo "send buffer 0"
    FILE_NAME=`get_file $i%$mod 1`
    $tool_dma -d /dev/xdma1_h2c_0  -a $reg_buf0 -s  $size -f $FILE_NAME -v > /dev/null
    $tool_reg /dev/xdma1_user $reg_wait w 1 > /dev/null
    echo "channel 0 "$FILE_NAME
    TIME=$(date "+%s.%N")
    echo $TIME

    wait
    echo "send buffer 1"
    FILE_NAME=`get_file $i%$mod 2`
    $tool_dma -d /dev/xdma1_h2c_0  -a $reg_buf1 -s  $size -f $FILE_NAME -v > /dev/null
    $tool_reg /dev/xdma1_user $reg_wait w 1 > /dev/null
    echo "channel 0 "$FILE_NAME
    TIME=$(date "+%s.%N")
    echo $TIME
    #sleep 0.2

    wait
    echo "send buffer 2"
    FILE_NAME=`get_file $i%$mod 3`
    $tool_dma -d /dev/xdma1_h2c_0  -a $reg_buf2 -s  $size -f $FILE_NAME -v > /dev/null
    $tool_reg /dev/xdma1_user $reg_wait w 1 > /dev/null
    echo "channel 0 "$FILE_NAME
    TIME=$(date "+%s.%N")
    echo $TIME
    #sleep 0.2
done

exit 0
