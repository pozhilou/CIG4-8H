#!/bin/bash

size=4147200
#size=16588800
reg_wait=0x30170
reg_buf0=0x810000000
reg_buf1=0x812000000
reg_buf2=0x814000000

reg_wait_ch1=0x30174
reg_buf0_ch1=0x816000000
reg_buf1_ch1=0x818000000
reg_buf2_ch1=0x81a000000

reg_wait_ch2=0x30178
reg_buf0_ch2=0x81c000000
reg_buf1_ch2=0x81e000000
reg_buf2_ch2=0x820000000

reg_wait_ch3=0x3017c
reg_buf0_ch3=0x822000000
reg_buf1_ch3=0x824000000
reg_buf2_ch3=0x826000000

reg_wait_ch4=0x30180
reg_buf0_ch4=0x428000000
reg_buf1_ch4=0x42a000000
reg_buf2_ch4=0x42c000000

reg_wait_ch5=0x30184
reg_buf0_ch5=0x42e000000
reg_buf1_ch5=0x430000000
reg_buf2_ch5=0x432000000

reg_wait_ch6=0x30188
reg_buf0_ch6=0x434000000
reg_buf1_ch6=0x436000000
reg_buf2_ch6=0x438000000

reg_wait_ch7=0x3018c
reg_buf0_ch7=0x43a000000
reg_buf1_ch7=0x43c000000
reg_buf2_ch7=0x43e000000

mod=4
tool_dma="../tools/dma_to_device"
tool_reg="../tools/reg_rw"

#get_file seq_index buf_index
function get_file ()
{
    #FILE=data/yuv/AR0820_cam_0_out_$[$1 *3 + $2].yuv
	FILE=data/yuv2/$[$1 *3 + $2].CameraDeviceAR0233_GroupA_3_1920x1080.yuv
    echo $FILE
}

function wait () {
    readvalue=`$tool_reg /dev/xdma1_user $1 w | awk 'NR==5{print $8}'`
    while [ $readvalue = "0x00000003" ]
    do
        echo $readvalue
        readvalue=`$tool_reg /dev/xdma1_user $1 w | awk 'NR==5{print $8}'`
    done
    echo $readvalue
}

function send () {
    echo "send" ${1} ${2} ${3} ${4}
    FILE_NAME=`get_file ${1}%$mod ${4}`
    echo $FILE_NAME
	$tool_dma -d /dev/xdma1_h2c_0  -a ${2} -s  $size -f $FILE_NAME -v > /dev/null
    $tool_reg /dev/xdma1_user ${3} w 1 > /dev/null
}

function send_test () {
    FILE_NAME=`get_file ${1}%$mod 1`
    echo ${2} $FILE_NAME $3
}

for ((i=0; i<1; i++)); do
    wait $reg_wait
    send $i $reg_buf0 $reg_wait 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch1
    send $i $reg_buf0_ch1 $reg_wait_ch1 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch2
    send $i $reg_buf0_ch2 $reg_wait_ch2 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch3
    send $i $reg_buf0_ch3 $reg_wait_ch3 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch4
    send $i $reg_buf0_ch4 $reg_wait_ch4 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch5
    send $i $reg_buf0_ch5 $reg_wait_ch5 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch6
    send $i $reg_buf0_ch6 $reg_wait_ch6 0
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch7
    send $i $reg_buf0_ch7 $reg_wait_ch7 0
    TIME=$(date "+%s.%N")
    echo $TIME

    $tool_reg /dev/xdma1_user 0x30020 w 0x22222222

    wait $reg_wait
    send $i $reg_buf1 $reg_wait 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch1
    send $i $reg_buf1_ch1 $reg_wait_ch1 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch2
    send $i $reg_buf1_ch2 $reg_wait_ch2 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch3
    send $i $reg_buf1_ch3 $reg_wait_ch3 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch4
    send $i $reg_buf1_ch4 $reg_wait_ch4 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch5
    send $i $reg_buf1_ch5 $reg_wait_ch5 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch6
    send $i $reg_buf1_ch6 $reg_wait_ch6 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch7
    send $i $reg_buf1_ch7 $reg_wait_ch7 1
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait
    send $i $reg_buf2 $reg_wait 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch1
    send $i $reg_buf2_ch1 $reg_wait_ch1 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch2
    send $i $reg_buf2_ch2 $reg_wait_ch2 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch3
    send $i $reg_buf2_ch3 $reg_wait_ch3 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch4
    send $i $reg_buf2_ch4 $reg_wait_ch4 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch5
    send $i $reg_buf2_ch5 $reg_wait_ch5 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch6
    send $i $reg_buf2_ch6 $reg_wait_ch6 2
    TIME=$(date "+%s.%N")
    echo $TIME

    wait $reg_wait_ch7
    send $i $reg_buf2_ch7 $reg_wait_ch7 2
    TIME=$(date "+%s.%N")
    echo $TIME
done

./send_test_cam0.sh &
./send_test_cam1.sh &
./send_test_cam2.sh &
./send_test_cam3.sh &

./send_test_cam4.sh &
./send_test_cam5.sh &
./send_test_cam6.sh &
./send_test_cam7.sh &

#for ((i=1; i<=30000; i++)); do
#    wait $reg_wait 
#    send $i $reg_buf0 $reg_wait 1
#
#    wait $reg_wait_ch1 
#    send $i $reg_buf0_ch1 $reg_wait_ch1 1
#
#    wait $reg_wait
#    send $i $reg_buf1 $reg_wait 2
#
#    wait $reg_wait_ch1
#    send $i $reg_buf1_ch1 $reg_wait_ch1 2
#
#    wait $reg_wait
#    send $i $reg_buf2 $reg_wait 3
#
#    wait $reg_wait_ch1
#    send $i $reg_buf2_ch1 $reg_wait_ch1 3
#done

exit 0
