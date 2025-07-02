#!/bin/bash

transfer_resolution 0 1920  1080
camera_resolution 1 1920  1080
camera_resolution 2 1920  1080
camera_resolution 3 1920  1080
camera_resolution 4 1920  1080
camera_resolution 5 1920  1080
camera_resolution 6 1920  1080
camera_resolution 7 1920  1080

#signle frame cycle
#frames group
#h.264 decode send

# must option  -c(channel) -f(filename)  -t(format_type) -s size
# alter option -r (replay count  default 1) -d(device default /dev/xdma0_h2c_0)
# 

#800m
../tools/dma_to_device -d /dev/xdma1_h2c_0  -c 0 -s  16588800 -f ./data/video800m.h264  -t decode &
../tools/dma_to_device -d /dev/xdma1_h2c_1  -c 1 -s  16588800 -f ./data/video800m.h264  -t decode &
../tools/dma_to_device -d /dev/xdma1_h2c_2  -c 2 -s  16588800 -f ./data/video800m.h264  -t decode &
../tools/dma_to_device -d /dev/xdma1_h2c_3  -c 3 -s  16588800 -f ./data/video800m.h264  -t decode &

../tools/dma_to_device_video -d /dev/xdma1_h2c_0  -c 0 -s  4147200 -f ./data/video.h264  -t H264 &
../tools/dma_to_device_video -d /dev/xdma1_h2c_1  -c 1 -s  4147200 -f ./data/video.h264  -t H264 &
../tools/dma_to_device_video -d /dev/xdma1_h2c_2  -c 2 -s  4147200 -f ./data/video.h264  -t H264 &
../tools/dma_to_device_video -d /dev/xdma1_h2c_3  -c 3 -s  4147200 -f ./data/video.h264  -t H264 &

../tools/dma_to_device_video -d /dev/xdma1_h2c_0  -c 0 -s  16588800 -f ./data/pic38402160yuyv.yuv -t raw -r 100 &
../tools/dma_to_device -d /dev/xdma1_h2c_1  -c 1 -s  16588800 -f ./data/pic38402160yuyv.yuv -t raw -r 100 &
../tools/dma_to_device -d /dev/xdma1_h2c_2  -c 2 -s  16588800 -f ./data/pic38402160yuyv.yuv -t raw -r 100 &
../tools/dma_to_device -d /dev/xdma1_h2c_3  -c 3 -s  16588800 -f ./data/pic38402160yuyv.yuv -t raw -r 100 &

#../tools/dma_to_device -d /dev/xdma1_h2c_2  -a 0x428000000 -r 0x30180 -s  16588800 -f ./data/video800m.h264  -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_2  -a 0x42e000000 -r 0x30184 -s  16588800 -f ./data/video800m.h264  -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_3  -a 0x434000000 -r 0x30188 -s  16588800 -f ./data/video800m.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x43a000000 -r 0x3018c -s  16588800 -f ./data/video800m.h264 -v -c 1000000 &

#200m
#../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x810000000 -r 0x30170 -s  4147200 -f ./data/video.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_1  -a 0x816000000 -r 0x30174 -s  4147200 -f ./data/video.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_1  -a 0x81c000000 -r 0x30178 -s  4147200 -f ./data/video.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_3  -a 0x822000000 -r 0x3017c -s  4147200 -f ./data/video.h264 -v -c 1000000 &

#../tools/dma_to_device -d /dev/xdma1_h2c_0  -a 0x428000000 -r 0x30180 -s  4147200 -f ./data/video.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_1  -a 0x42e000000 -r 0x30184 -s  4147200 -f ./data/video.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_2  -a 0x434000000 -r 0x30188 -s  4147200 -f ./data/video.h264 -v -c 1000000 &
#../tools/dma_to_device -d /dev/xdma1_h2c_3  -a 0x43a000000 -r 0x3018c -s  4147200 -f ./data/video.h264 -v -c 1000000 &

exit 0
