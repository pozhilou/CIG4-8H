#!/bin/bash

# channel 0-7, indicate path of the tranfer file. pic38402160yuyv.yuv Video_1080_cam_6_out_0.yuv
# 文件名字要放在data/yuv下面
transfer_file_name[0]="picture19201080yuyv.yuv"
transfer_file_name[1]="picture19201080yuyv.yuv"
transfer_file_name[2]="picture19201080yuyv.yuv"
transfer_file_name[3]="picture19201080yuyv.yuv"
transfer_file_name[4]="pic38402160yuyv.yuv"
transfer_file_name[5]="pic38402160yuyv.yuv"
transfer_file_name[6]="pic38402160yuyv.yuv"
transfer_file_name[7]="pic38402160yuyv.yuv"

# channel 0-7, indicate tranfer type of the video file.(Original|H264)
transfer_file_type[0]="Original"
transfer_file_type[1]="Original"
transfer_file_type[2]="Original"
transfer_file_type[3]="Original"
transfer_file_type[4]="Original"
transfer_file_type[5]="Original"
transfer_file_type[6]="Original"
transfer_file_type[7]="Original"

# channel 0-7, indicate tranfer time of the video file, value 0 indicate no send.  
transfer_file_replay[0]=1000000
transfer_file_replay[1]=1000000
transfer_file_replay[2]=1000000
transfer_file_replay[3]=1000000
transfer_file_replay[4]=1000000
transfer_file_replay[5]=1000000
transfer_file_replay[6]=1000000
transfer_file_replay[7]=1000000

# channel 0-7 transfer type(YUV|RAW)
transfer_type[0]="YUV"
transfer_type[1]="YUV"
transfer_type[2]="YUV"
transfer_type[3]="YUV"
transfer_type[4]="YUV"
transfer_type[5]="YUV"
transfer_type[6]="YUV"
transfer_type[7]="YUV"


# channel 0-7, width of the transfer picture(1920|3840) 
transfer_resolution_width[0]=1920
transfer_resolution_width[1]=1920
transfer_resolution_width[2]=1920
transfer_resolution_width[3]=1920
transfer_resolution_width[4]=3840
transfer_resolution_width[5]=3840
transfer_resolution_width[6]=3840
transfer_resolution_width[7]=3840

# channel 0-7, hight of the transfer picture(1080|2160) 
transfer_resolution_hight[0]=1080
transfer_resolution_hight[1]=1080
transfer_resolution_hight[2]=1080
transfer_resolution_hight[3]=1080
transfer_resolution_hight[4]=2160
transfer_resolution_hight[5]=2160
transfer_resolution_hight[6]=2160
transfer_resolution_hight[7]=2160

function fpga_trig () {
    ../tools/reg_rw /dev/xdma0_user 0x30018 w 0x00000000 #disable trig
	../tools/reg_rw /dev/xdma0_user 0x30020 w 0x22222222 #trig
	../tools/reg_rw /dev/xdma0_user 0x30018 w 0x000000FF #enable trig
}

function fpga_fps () {
    #fps
	#30fps 6666666
	#20fps 10000000
	#15fps 13333333
	#10fps 20000000
    ../tools/reg_rw /dev/xdma0_user 0x30024 w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x30028 w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x3002c w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x30030 w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x30034 w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x30038 w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x3003c w 20000000
    ../tools/reg_rw /dev/xdma0_user 0x30040 w 20000000
    #ready
    sleep 1
}

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

. ./fun_app_cfg.sh
. ./fun_fpga_cfg.sh

# usage transfer_run.sh start|stop

function test_argv () {
    echo "test"
    todo
}

#单路测试注意0 1 线序问题
function execute_cfg () {
    fpga_reset ${transfer_type[@]}   #ok
    fpga_resolution ${transfer_resolution_width[@]} ${transfer_resolution_hight[@]} #ok
    fpga_fps #todo
    fpga_done ${transfer_resolution_hight[@]}
    fpga_trig
    transfer_start ${transfer_file_replay[@]} ${transfer_file_name[@]} ${transfer_file_type[@]} ${transfer_resolution_width[@]} ${transfer_resolution_hight[@]}
    echo "execute_cfg"
}

execute_cfg

exit 0







