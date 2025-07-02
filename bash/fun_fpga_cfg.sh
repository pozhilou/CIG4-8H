#!/bin/bash

#fpga reset

#std_out=/dev/NULL
# yuv 0 1 格式不同 yuyv uyvy
# raw10 2 3
# raw12 4 5
# raw14 6 7

format_vaule=0
function fun_fpga_type2value () {

    temp_value=0 
    if [ x"${1}" == x"YUV" ]; then
        temp_value=0 
    elif [ x"${1}" == x"RAW" ]; then
        temp_value=4
    else
        temp_value=0 
    fi
    format_vaule=`expr $temp_value + $(($format_vaule << 4))`
}

function fpga_reset () {
    # fpga_type2value
    fun_fpga_type2value $8
    fun_fpga_type2value $7
    fun_fpga_type2value $6
    fun_fpga_type2value $5
    fun_fpga_type2value $4
    fun_fpga_type2value $3
    fun_fpga_type2value $2
    fun_fpga_type2value $1
    #echo "format_value" $format_vaule
    #echo "fpga_reset"
    ../tools/reg_rw /dev/xdma0_user 0x30004 w 0xff
    sleep 1
    ../tools/reg_rw /dev/xdma0_user 0x30004 w 0x0
    ../tools/reg_rw /dev/xdma0_user 0x30008 w $format_vaule

    ../tools/reg_rw /dev/xdma0_user 0x70000 w 2
    ../tools/reg_rw /dev/xdma0_user 0x80000 w 2
    ../tools/reg_rw /dev/xdma0_user 0x90000 w 2
    ../tools/reg_rw /dev/xdma0_user 0xa0000 w 2
    ../tools/reg_rw /dev/xdma0_user 0xb0000 w 2
    ../tools/reg_rw /dev/xdma0_user 0xc0000 w 2
    ../tools/reg_rw /dev/xdma0_user 0xd0000 w 2
    ../tools/reg_rw /dev/xdma0_user 0xe0000 w 2
    #echo "fpga_reset ok" 
}

function fpga_done () {
    #echo "fpga_done"
    ../tools/reg_rw /dev/xdma0_user 0x70000 w
    ../tools/reg_rw /dev/xdma0_user 0x80000 w
    ../tools/reg_rw /dev/xdma0_user 0x90000 w
    ../tools/reg_rw /dev/xdma0_user 0xa0000 w
    ../tools/reg_rw /dev/xdma0_user 0xb0000 w
    ../tools/reg_rw /dev/xdma0_user 0xc0000 w
    ../tools/reg_rw /dev/xdma0_user 0xd0000 w
    ../tools/reg_rw /dev/xdma0_user 0xe0000 w

    #heigt
	../tools/reg_rw /dev/xdma0_user 0x70040 w ${1}
	../tools/reg_rw /dev/xdma0_user 0x80040 w ${2}
	../tools/reg_rw /dev/xdma0_user 0x90040 w ${3}
	../tools/reg_rw /dev/xdma0_user 0xa0040 w ${4}
	../tools/reg_rw /dev/xdma0_user 0xb0040 w ${5}
	../tools/reg_rw /dev/xdma0_user 0xc0040 w ${6}
	../tools/reg_rw /dev/xdma0_user 0xd0040 w ${7}
	../tools/reg_rw /dev/xdma0_user 0xe0040 w ${8}

    ../tools/reg_rw /dev/xdma0_user 0x70000 w 1
    ../tools/reg_rw /dev/xdma0_user 0x80000 w 1
    ../tools/reg_rw /dev/xdma0_user 0x90000 w 1
    ../tools/reg_rw /dev/xdma0_user 0xa0000 w 1
    ../tools/reg_rw /dev/xdma0_user 0xb0000 w 1
    ../tools/reg_rw /dev/xdma0_user 0xc0000 w 1
    ../tools/reg_rw /dev/xdma0_user 0xd0000 w 1
    ../tools/reg_rw /dev/xdma0_user 0xe0000 w 1

    ../tools/reg_rw /dev/xdma0_user 0x3000c w 0xff

	../tools/reg_rw /dev/xdma0_user 0x30044 w 100
	../tools/reg_rw /dev/xdma0_user 0x30048 w 100
	../tools/reg_rw /dev/xdma0_user 0x3004c w 100
	../tools/reg_rw /dev/xdma0_user 0x30050 w 100
	../tools/reg_rw /dev/xdma0_user 0x30054 w 100
	../tools/reg_rw /dev/xdma0_user 0x30058 w 100
	../tools/reg_rw /dev/xdma0_user 0x3005c w 100
	../tools/reg_rw /dev/xdma0_user 0x30060 w 100

	# delay write 
    ../tools/reg_rw /dev/xdma0_user 0x30020 w 0x00000000
    #echo "fpga_done ok"
}

function fpga_resolution () {
    #sleep 1
    #echo "fpga_resolution"
    ../tools/reg_rw /dev/xdma0_user 0x30100 w ${1}
    ../tools/reg_rw /dev/xdma0_user 0x30104 w ${9}
    ../tools/reg_rw /dev/xdma0_user 0x30108 w ${2}
    ../tools/reg_rw /dev/xdma0_user 0x3010c w ${10}
    ../tools/reg_rw /dev/xdma0_user 0x30110 w ${3}
    ../tools/reg_rw /dev/xdma0_user 0x30114 w ${11}
    ../tools/reg_rw /dev/xdma0_user 0x30118 w ${4}
    ../tools/reg_rw /dev/xdma0_user 0x3011c w ${12}
    ../tools/reg_rw /dev/xdma0_user 0x30120 w ${5}
    ../tools/reg_rw /dev/xdma0_user 0x30124 w ${13}
    ../tools/reg_rw /dev/xdma0_user 0x30128 w ${6}
    ../tools/reg_rw /dev/xdma0_user 0x3012c w ${14}
    ../tools/reg_rw /dev/xdma0_user 0x30130 w ${7}
    ../tools/reg_rw /dev/xdma0_user 0x30134 w ${15}
    ../tools/reg_rw /dev/xdma0_user 0x30138 w ${8}
    ../tools/reg_rw /dev/xdma0_user 0x3013c w ${16}
    #echo "fpga_resolution ok"

    #echo "fpga_fps" 600M  2M 1100 960,8M 1100 1920
    ../tools/reg_rw /dev/xdma0_user 0x30190 w 1100
    ../tools/reg_rw /dev/xdma0_user 0x30194 w 1100
    ../tools/reg_rw /dev/xdma0_user 0x30198 w 1100
    ../tools/reg_rw /dev/xdma0_user 0x3019c w 1100
    ../tools/reg_rw /dev/xdma0_user 0x301A0 w 1100
    ../tools/reg_rw /dev/xdma0_user 0x301A4 w 1100
    ../tools/reg_rw /dev/xdma0_user 0x301A8 w 1100
    ../tools/reg_rw /dev/xdma0_user 0x301AC w 1100

    #w/2
    ../tools/reg_rw /dev/xdma0_user 0x301b0 w $[${1}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301b4 w $[${2}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301b8 w $[${3}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301bc w $[${4}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301c0 w $[${5}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301c4 w $[${6}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301c8 w $[${7}/2]
    ../tools/reg_rw /dev/xdma0_user 0x301cc w $[${8}/2]
}
