#! /bin/sh
gcc dma_to_device.c -g -o dma_to_device \
-I /usr/local/ffmpeg/include -L /usr/local/ffmpeg/lib -lavcodec -lavutil -lavformat -lswscale

