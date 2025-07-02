CC ?= gcc

INCS=-I /usr/local/ffmpeg/include  -I /usr/local/include #头文件目录
STATCI_LIBS=-L /usr/local/ffmpeg/lib -lavcodec -lavutil -lavformat -lswscale  #链接libtesta.a
#SHARED_LIBS=-L. -Wl,-Bdynamic -ltestso #链接libtestso.so
#$(CC) dma_to_device.c -g -o dma_to_device -I /usr/local/include -L /usr/local/lib -lavcodec -lavutil -lavformat -lswscale

all: dma_to_device_video

dma_to_device_video: dma_to_device.o
	$(CC) -o $@ $< $(INCS) $(STATCI_LIBS)

%.o: %.c
	$(CC) -c -std=c99 -o $@ $< -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_LARGE_FILE_SOURCE

clean:
	rm -rf dma_to_device_video *.o ../dma_to_device_video

install:
	cp dma_to_device_video  ../
