/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

/* ltoh: little endian to host */
/* htol: host to little endian */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ltohl(x)       (x)
#define ltohs(x)       (x)
#define htoll(x)       (x)
#define htols(x)       (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ltohl(x)     __bswap_32(x)
#define ltohs(x)     __bswap_16(x)
#define htoll(x)     __bswap_32(x)
#define htols(x)     __bswap_16(x)
#endif

#include "../../xdma/cdev_sgdma.h"
#include "dma_utils.c"
#include "video_decode_fun.c"

#define DEVICE_NAME_DEFAULT "/dev/xdma0_h2c_0"
#define SIZE_DEFAULT (32)
#define COUNT_DEFAULT (1)

/*

usage: ./tools/dma_to_device [OPTIONS]

Write via SGDMA, optionally read input from a file.

  -d (--device) device (defaults to /dev/xdma0_h2c_0)
  -a (--address) the start address on the AXI bus
  -k (--aperture) memory address aperture
  -s (--size) size of a single transfer in bytes, default 32,
  -o (--offset) page offset of transfer
  -h (--help) print usage help and exit
  -v (--verbose) verbose output

  -c   (--channel) the transfer channel, 0~7 vailed.
  -C   (--card_num) the transfer card num, 0~1 vailed.
  -f   (--file_path) the send file path.
  -t   (--format_type) the format type of send file [RAW|AUTO], default ATUO.
  -r   (--replay) replay the file count, default 1.

dma_to_device -channel [0~7] -file [file_path] -format [RAW|AUTO|] -cycle [1-1000000000]
*/

static uint64_t first_buf_addr[8]={0x810000000,0x816000000,0x81c000000,0x822000000,0x428000000,0x42e000000,0x434000000,0x43a000000};
static uint64_t buf_addr_step=0x2000000;
static uint64_t buf_ready_addr[8]={0x30170,0x30174,0x30178,0x3017c,0x30180,0x30184,0x30188,0x3018c};

static struct option const long_opts[] = {
	{"device", required_argument, NULL, 'd'},
	//{"address", required_argument, NULL, 'a'},
	//{"aperture", required_argument, NULL, 'k'},
	{"size", required_argument, NULL, 's'},
	//{"offset", required_argument, NULL, 'o'},
	{"channel", required_argument, NULL, 'c'},
	{"card_num", required_argument, NULL, 'C'},
	{"file", required_argument, NULL, 'f'},
	{"format type", required_argument, NULL, 't'},
	{"replay", required_argument, NULL, 'r'},
	//{"count", required_argument, NULL, 'c'},
	//{"data infile", required_argument, NULL, 'f'},
	//{"data outfile", required_argument, NULL, 'w'},
	{"help", no_argument, NULL, 'h'},
	{"verbose", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};

char LOG_FILE[80]="dma_to_device";
char transfer[20]="";
FILE * pFile;

char *g_dev = DEVICE_NAME_DEFAULT;
uint64_t g_addr;
uint64_t g_aperture;
uint64_t g_size;
uint64_t g_offset;
uint64_t g_count=1;
uint64_t g_wait_addr;
uint64_t g_send_count=0;
uint64_t g_channel=0;
uint8_t g_card=0;
char *infname = NULL;
char *g_format = "raw";
int verbose = 0;

int send_one_frame(unsigned char* bufffer);

static int test_dma(uint64_t addr, uint64_t aperture,
		    uint64_t size, uint64_t offset,
			unsigned char* buffer, uint64_t wait_addr);

static int reg_rw(int addr, int write, int value);

static int wait_fpga_ready(int addr);

static int set_fpga_wait(int addr);

static void usage(const char *name)
{
	int i = 0;

	fprintf(stdout, "%s\n\n", name);
	fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);
	fprintf(stdout, 
		"Write via SGDMA, optionally read input from a file.\n\n");

	fprintf(stdout, "  -%c (--%s) device (defaults to %s)\n",
		long_opts[i].val, long_opts[i].name, DEVICE_NAME_DEFAULT);
	//i++;
	//fprintf(stdout, "  -%c (--%s) the start address on the AXI bus\n",
	//	long_opts[i].val, long_opts[i].name);
	//i++;
	//fprintf(stdout, "  -%c (--%s) memory address aperture\n",
	//	long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout,
		"  -%c (--%s) size of a single transfer in bytes, default %d,\n",
		long_opts[i].val, long_opts[i].name, SIZE_DEFAULT);
	//i++;
	//fprintf(stdout, "  -%c (--%s) page offset of transfer\n",
	//	long_opts[i].val, long_opts[i].name);
	//i++;
	//fprintf(stdout, "  -%c (--%s) number of transfers, default %d\n",
	//	long_opts[i].val, long_opts[i].name, COUNT_DEFAULT);

	i++;
	fprintf(stdout, "  -%c (--%s) channel to send the data out.\n",
		long_opts[i].val, long_opts[i].name);

	i++;
	fprintf(stdout, "  -%c (--%s) card_num to send the data .\n",
			long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) filename to read the data from.\n",
		long_opts[i].val, long_opts[i].name);

	i++;
	fprintf(stdout, "  -%c (--%s) format of the read data (decode|raw).\n",
		long_opts[i].val, long_opts[i].name);	

	i++;
	fprintf(stdout, "  -%c (--%s) repaly time of send file.\n",
		long_opts[i].val, long_opts[i].name);		

	/* i++;
	fprintf(stdout,
		"  -%c (--%s) filename to write the data of the transfers\n",
		long_opts[i].val, long_opts[i].name); */
	i++;
	fprintf(stdout, "  -%c (--%s) print usage help and exit\n",
		long_opts[i].val, long_opts[i].name);
	i++;
	fprintf(stdout, "  -%c (--%s) verbose output\n",
		long_opts[i].val, long_opts[i].name);
	i++;

	fprintf(stdout, "\nReturn code:\n");
	fprintf(stdout, "  0: all bytes were dma'ed successfully\n");
	fprintf(stdout, "  < 0: error\n\n");
}

static void pase_arg(int argc, char *argv[])
{
	int cmd_opt;
	while ((cmd_opt =
		getopt_long(argc, argv, "vhc:C:f:d:a:k:s:o:t:r:", long_opts,
			    NULL)) != -1) {
		//printf("%s(%d) cmd_opt:%d\n", __func__, __LINE__, cmd_opt);
		switch (cmd_opt) {
		case 0:
			/* long option */
			break;
		case 'd':
			/* device node name */
			//fprintf(stdout, "'%s'\n", optarg);
			g_dev = strdup(optarg);
			break;
		case 'a':
			/* RAM address on the AXI bus in bytes */
			g_addr = getopt_integer(optarg);
			break;
		case 'k':
			/* memory aperture windows size */
			g_aperture = getopt_integer(optarg);
			break;
		case 's':
			/* size in bytes */
			g_size = getopt_integer(optarg);
			break;
		case 'o':
			g_offset = getopt_integer(optarg) & 4095;
			break;
			/* count */
		case 'c':
			g_channel = getopt_integer(optarg);
			break;
			/* count */
		case 'C':
			g_card = getopt_integer(optarg);
			break;
			/* card num 0 and 1*/
		case 'f':
			infname = strdup(optarg);
			break;
		case 't':
			g_format = strdup(optarg);
			break;
			/* print usage help and exit */
		case 'v':
			verbose = 1;
			break;
		case 'r':
			/* register dma send flag */
			g_count = getopt_integer(optarg);
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(0);
			break;
		}
	}

	g_addr = first_buf_addr[g_channel];
	g_wait_addr = buf_ready_addr[g_channel];
	
	sprintf(LOG_FILE,"dma_to_device_transfer%ld.log",g_channel);
	sprintf(transfer,"transfer%ld.log",g_channel);
}

int main(int argc, char *argv[])
{
	
	//char *device = DEVICE_NAME_DEFAULT;
	//uint64_t address = 0;
	//uint64_t aperture = 0;
	//uint64_t size = SIZE_DEFAULT;
	uint64_t offset = 0;
	uint64_t count = COUNT_DEFAULT;
	uint64_t send_flag_addr = 0; //xdma send ready register 0 1 2 ok 3 wait
	unsigned char* buffer;
	FILE * pInFile = NULL;
	long long lSize, read_size;
	long long cur_size;
	int file_count;
	int ret;

	pase_arg(argc,argv);
	pFile = fopen (LOG_FILE,"w");

	if (verbose)
		fprintf(stdout, 
		"dev %s, addr 0x%lx, aperture 0x%lx, size 0x%lx, offset 0x%lx, "
	        "count %lu\n",
		g_dev, g_addr, g_aperture, g_size, g_offset, g_count);

	//printf("%s(%d) device:%s, g_size:%d\n", __func__, __LINE__, g_dev, g_size);
	if(0==strcmp(g_format,"H264"))
	{
		printf("decode send\n");
		video_decode(send_one_frame, infname);
	}
	else
	{
		printf("Original send\n");
		buffer = malloc(g_size);
		if (buffer != NULL)
			pInFile = fopen(infname,"r");
		else
			printf("malloc buf failed\n");
#if 0
		if(pInFile)
		{
			printf("open file ok!\n");
			fread(buffer,1,g_size,pInFile);
			for(int i=0; i<g_count;i++)
			{
				send_one_frame(buffer);
			}
		}
#else
		//get file size:
		fseek (pInFile , 0 , SEEK_END);
		lSize = ftell (pInFile);
		file_count = lSize / g_size;
		printf("open file ok, g_size:%d, this file file_count:%d\n", g_size, file_count);
		rewind(pInFile);
		for(int i = 0; i < g_count; i++)
		{
			memset(buffer, 0, g_size);
			printf("i:--->%d\n", i);
			ret = fseek(pInFile, g_size * (i % file_count), SEEK_SET);
			if(!ret)
			{
				//cur_size = ftell(pInFile); //计算往后偏移了几帧
				read_size = fread(buffer, 1, g_size, pInFile);
				//printf("read_size:%lld, cur_size:%lld\n", read_size, cur_size);
			}
			else
				printf("i:%d, fseek failed\n", i);

			send_one_frame(buffer);
		}
#endif
		fclose(pInFile);
		free(buffer); 
	}

	
	//test_dma(device, address, aperture, size, offset, count,
	//		infname, ofname, send_flag_addr);


	if (pFile!=NULL)
	{
		fputs ("fopen example",pFile);
		fclose (pFile);
	}
	return 0; 
}

int send_one_frame(unsigned char* bufffer)
{
	//printf("send_one_frame_by decode ****************************\n");
	test_dma(g_addr,g_aperture,g_size,g_offset,bufffer,g_wait_addr);
	//g_addr 每一个通道buf的首地址，代码有定义，数组索引赋值地址
	//g_aperture 固定是0
	//g_size 每个通道buf的大小 1920*1080*2
	//g_offset 好像是0，待确认
	//bufffer yuyv的数据指针
	//g_wait_addr 代码有定义，判断是否有空闲buf，=3代表空闲等待，其他值代表可写数据
}

static int test_dma(uint64_t addr, uint64_t aperture,
		    uint64_t size, uint64_t offset,unsigned char* buffer, uint64_t wait_addr)
{
	//uint64_t i;
	ssize_t rc;
	size_t bytes_done = 0;
	size_t out_offset = 0;
	//char *buffer = NULL;
	char *allocated = NULL;
	struct timespec ts_start, ts_end;
	int infile_fd = -1;
	int outfile_fd = -1;
	int fpga_fd = open(g_dev, O_RDWR);
	long total_time = 0;
	float result;
	float avg_time = 0;
	int underflow = 0;
	uint64_t addr_offset;

	if (fpga_fd < 0) {
		fprintf(stderr, "unable to open device %s, %d.\n",
			g_dev, fpga_fd);
		perror("open device");
		return -EINVAL;
	}

	//for (i = 0; i < count; i++) {
		/* write buffer to AXI MM address using SGDMA */
		printf("%s seq:#%lu\n", transfer, g_send_count);
		addr_offset = (g_send_count%3*0x2000000);
		wait_fpga_ready(wait_addr);
		rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);

		fprintf(pFile,
			"CLOCK_MONOTONIC start time %ld.%09ld sec\n",
			ts_start.tv_sec, ts_start.tv_nsec); 

		if (aperture) {
			struct xdma_aperture_ioctl io;

			io.buffer = (unsigned long)buffer;
			io.len = size;
			io.ep_addr = addr + addr_offset;
			io.aperture = aperture;
			io.done = 0UL;

			rc = ioctl(fpga_fd, IOCTL_XDMA_APERTURE_W, &io);
			if (rc < 0 || io.error) {
				fprintf(stdout,
					"#%ld: aperture W ioctl failed %ld,%d.\n",
					g_send_count, rc, io.error);
				goto out;
			}

			bytes_done = io.done;
		} else {
			rc = write_from_buffer(g_dev, fpga_fd, buffer, size,
				      	 	addr+addr_offset);
			if (rc < 0)
				goto out;

			bytes_done = rc;
		}

		rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);

		fprintf(pFile, "CLOCK_MONOTONIC end time %ld.%09ld sec\n", ts_end.tv_sec, ts_end.tv_nsec);

		if (bytes_done < size) {
			printf("#%ld: underflow %ld/%ld.\n",
				g_send_count, bytes_done, size);
			underflow = 1;
		}

		/* subtract the start time from the end time */
		timespec_sub(&ts_end, &ts_start);
		total_time += ts_end.tv_nsec;
		/* a bit less accurate but side-effects are accounted for */
		if (verbose)
		fprintf(pFile,
			"#%lu: CLOCK_MONOTONIC %ld.%09ld sec. write %ld bytes\n",
			g_send_count, ts_end.tv_sec, ts_end.tv_nsec, size); 
		set_fpga_wait(wait_addr);
	//}

	if (!underflow) {
		avg_time = (float)total_time;
		result = ((float)size)*1000/avg_time;
		if (verbose)
			printf("** Avg time device %s, total time %ld nsec, avg_time = %f, size = %lu, BW = %f \n",
			g_dev, total_time, avg_time, size, result);
		printf("addr:0x%08lx ** Average BW = %lu, %f\n", addr+addr_offset, size, result);
	}

	g_send_count++;

out:
	close(fpga_fd);
	if (infile_fd >= 0)
		close(infile_fd);
	if (outfile_fd >= 0)
		close(outfile_fd);
	free(allocated);

	if (rc < 0)
		return rc;
	/* treat underflow as error */
	return underflow ? -EIO : 0;
}

static int wait_fpga_ready(int addr)
{
	int read_time = 0;
	int read_value, read_value_fsync;
	struct timespec start, end;
	int wait_time;
	int count;
	//if(addr == 0x30180)
		clock_gettime(CLOCK_MONOTONIC, &start);
	while (1)
	{
		read_value = reg_rw(addr, 0, 0);
		read_value_fsync = reg_rw(0x100c4, 0, 0);
		//printf("addr:0x100c4,times:%d\n", read_value_fsync);
		read_time++;
		if(read_value == 0x3)
		{
			usleep(5000);
			//printf("addr_ready:0x%05x, this buf is 3, need wait\n", addr);
			continue;
		}
		else
		{	if(count == 3)
				continue;
			else
			{
				count++;
				break;
			}
		}
	};
	//if(addr == 0x30180)
	//{
		clock_gettime(CLOCK_MONOTONIC, &end);
		wait_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000;
		printf("wait_time: %dms\n", wait_time);
	//}
	fprintf(pFile,"%s read_time%d read_value%d\n", __func__, read_time, read_value);
}

static int set_fpga_wait(int addr)
{
	reg_rw(addr, 1, 1);
}

static int reg_rw(int addr, int write, int value)
{
	int fd;
	int err = 0;
	void *map;
	uint32_t read_result, writeval;
	off_t target = addr;
	off_t pgsz, target_aligned, offset;
	/* access width */
	char access_width = 'w';
	char device[20] = {"0"};

	/* not enough arguments given? */
	// if (argc < 3) {
	// 	fprintf(stderr,
	// 		"\nUsage:\t%s <device> <address> [[type] data]\n"
	// 		"\tdevice  : character device to access\n"
	// 		"\taddress : memory address to access\n"
	// 		"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n"
	// 		"\tdata    : data to be written for a write\n\n",
	// 		argv[0]);
	// 	exit(1);
	// }

	//device = strdup(argv[1]);
	//target = strtoul(argv[2], 0, 0);
	/* check for target page alignment */
	pgsz = sysconf(_SC_PAGESIZE);
	offset = target & (pgsz - 1);
	target_aligned = target & (~(pgsz - 1));

	//printf("device: %s, address: 0x%lx (0x%lx+0x%lx)\n",
	// 	device, target, target_aligned, offset);

	// /* data given? */
	// if (argc >= 4)
	// 	access_width = tolower(argv[3][0]);
	// printf("access width: ");
	// if (access_width == 'b')
	// 	printf("byte (8-bits)\n");
	// else if (access_width == 'h')
	// 	printf("half word (16-bits)\n");
	// else if (access_width == 'w')
	// 	printf("word (32-bits)\n");
	// else {
	// 	printf("default to word (32-bits)\n");
	// 	access_width = 'w';
	// }
	sprintf(device,"/dev/xdma%d_user", g_card);
	//printf("%s(%d) device:%s\n", __func__, __LINE__, device);
	if ((fd = open(device, O_RDWR | O_SYNC)) == -1) {
		printf("character device %s opened failed: %s.\n",
			device, strerror(errno));
		return -errno;
	}
	//printf("character device %s opened.\n", device);

	map = mmap(NULL, offset + 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
		       	target_aligned);
	if (map == (void *)-1) {
		printf("Memory 0x%lx mapped failed: %s.\n",
			target, strerror(errno));
		err = 1;
		goto close;
	}
	//printf("Memory 0x%lx mapped at address %p.\n", target_aligned, map);

	map += offset;
	/* read only */
	if (!write) {
		switch (access_width) {
		case 'b':
			read_result = *((uint8_t *) map);
			printf
			    ("Read 8-bits value at address 0x%lx (%p): 0x%02x\n",
			     target, map, (unsigned int)read_result);
			break;
		case 'h':
			read_result = *((uint16_t *) map);
			/* swap 16-bit endianess if host is not little-endian */
			read_result = ltohs(read_result);
			printf
			    ("Read 16-bit value at address 0x%lx (%p): 0x%04x\n",
			     target, map, (unsigned int)read_result);
			break;
		case 'w':
			read_result = *((uint32_t *) map);
			/* swap 32-bit endianess if host is not little-endian */
			read_result = ltohl(read_result);
			//printf
			//    ("Read 32-bit value at address 0x%lx (%p): 0x%08x\n",
			//     target, map, (unsigned int)read_result);
			err = read_result;	 
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n",
				access_width);
			err = 1;
			goto unmap;
		}
	}

	/* data value given, i.e. writing? */
	if (write) {
		writeval = value;//strtoul(value, 0, 0);
		switch (access_width) {
		case 'b':
			printf("Write 8-bits value 0x%02x to 0x%lx (0x%p)\n",
			       (unsigned int)writeval, target, map);
			*((uint8_t *) map) = writeval;
			break;
		case 'h':
			printf("Write 16-bits value 0x%04x to 0x%lx (0x%p)\n",
			       (unsigned int)writeval, target, map);
			/* swap 16-bit endianess if host is not little-endian */
			writeval = htols(writeval);
			*((uint16_t *) map) = writeval;
			break;
		case 'w':
			//printf("Write 32-bits value 0x%08x to 0x%lx (0x%p)\n",
			//       (unsigned int)writeval, target, map);
			/* swap 32-bit endianess if host is not little-endian */
			writeval = htoll(writeval);
			*((uint32_t *) map) = writeval;
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n",
				access_width);
			err = 1;
			goto unmap;
		}
	}
unmap:
	map -= offset;
	if (munmap(map, offset + 4) == -1) {
		printf("Memory 0x%lx mapped failed: %s.\n",
			target, strerror(errno));
	}
close:
	close(fd);

	return err;
}

// 按照顺序依次调用
int fpga_reset(int card_num)
{
	g_card = card_num;
	reg_rw(0x30004, 1, 0xFF);

	usleep(1000*1000);//1s

	reg_rw(0x30004, 1, 0x00);
	reg_rw(0x30008, 1, 0x00);
	reg_rw(0x70000, 1, 0x02);
	reg_rw(0x80000, 1, 0x02);
	reg_rw(0x90000, 1, 0x02);
	reg_rw(0xA0000, 1, 0x02);
	reg_rw(0xB0000, 1, 0x02);
	reg_rw(0xC0000, 1, 0x02);
	reg_rw(0xD0000, 1, 0x02);
	reg_rw(0xE0000, 1, 0x02);
}

int fpga_resolution(int card_num, int channel, int width, int height)
{
	g_card = card_num;
	if(channel == 0)
	{
		reg_rw(0x30100, 1, width);
		reg_rw(0x30104, 1, height);
		reg_rw(0x301b0, 1, width/2);
		reg_rw(0x70040, 1, height);
	} else if(channel == 1)
	{
		reg_rw(0x30108, 1, width);
		reg_rw(0x3010C, 1, height);
		reg_rw(0x301b4, 1, width/2);
		reg_rw(0x80040, 1, height);
	} else if(channel == 2)
	{
		reg_rw(0x30110, 1, width);
		reg_rw(0x30114, 1, height);
		reg_rw(0x301b8, 1, width/2);
		reg_rw(0x90040, 1, height);
	} else if(channel == 3)
	{
		reg_rw(0x30118, 1, width);
		reg_rw(0x3011c, 1, height);
		reg_rw(0x301bc, 1, width/2);
		reg_rw(0xa0040, 1, height);
	} else if(channel == 4)
	{
		reg_rw(0x30120, 1, width);
		reg_rw(0x30124, 1, height);
		reg_rw(0x301c0, 1, width/2);
		reg_rw(0xb0040, 1, height);
	} else if(channel == 5)
	{
		reg_rw(0x30128, 1, width);
		reg_rw(0x3012c, 1, height);
		reg_rw(0x301c4, 1, width/2);
		reg_rw(0xc0040, 1, height);

	} else if(channel == 6)
	{
		reg_rw(0x30130, 1, width);
		reg_rw(0x30134, 1, height);
		reg_rw(0x301c8, 1, width/2);
		reg_rw(0xd0040, 1, height);
	} else if(channel == 7)
	{
		reg_rw(0x30138, 1, width);
		reg_rw(0x3013c, 1, height);
		reg_rw(0x301cc, 1, width/2);
		reg_rw(0xe0040, 1, height);
	}
}

int fpga_fps(int card_num, int fps)
{
	int value;
	if (fps == 30)
		value = 6666666;
	else if(fps == 20)
		value = 10000000;
	else if(fps == 10)
		value = 20000000;

	g_card = card_num;
	//set fps
	reg_rw(0x30190, 1, 1100);
	reg_rw(0x30194, 1, 1100);
	reg_rw(0x30198, 1, 1100);
	reg_rw(0x3019c, 1, 1100);
	reg_rw(0x301a0, 1, 1100);
	reg_rw(0x301a4, 1, 1100);
	reg_rw(0x301a8, 1, 1100);
	reg_rw(0x301ac, 1, 1100);

	reg_rw(0x30024, 1, value);
	reg_rw(0x30028, 1, value);
	reg_rw(0x3002c, 1, value);
	reg_rw(0x30030, 1, value);
	reg_rw(0x30034, 1, value);
	reg_rw(0x30038, 1, value);
	reg_rw(0x3003c, 1, value);
	reg_rw(0x30040, 1, value);
	usleep(1000*1000);
}

int fpga_done(int card_num)
{
	g_card = card_num;
	reg_rw(0x70000, 0, 0);
	reg_rw(0x80000, 0, 0);
	reg_rw(0x90000, 0, 0);
	reg_rw(0xa0000, 0, 0);
	reg_rw(0xb0000, 0, 0);
	reg_rw(0xc0000, 0, 0);
	reg_rw(0xd0000, 0, 0);
	reg_rw(0xe0000, 0, 0);

	reg_rw(0x70000, 1, 1);
	reg_rw(0x80000, 1, 1);
	reg_rw(0x90000, 1, 1);
	reg_rw(0xa0000, 1, 1);
	reg_rw(0xb0000, 1, 1);
	reg_rw(0xc0000, 1, 1);
	reg_rw(0xd0000, 1, 1);
	reg_rw(0xe0000, 1, 1);

	reg_rw(0x3000c, 1, 0xff);

	//延迟发送 us
	reg_rw(0x30044, 1, 100);
	reg_rw(0x30048, 1, 100);
	reg_rw(0x3004c, 1, 100);
	reg_rw(0x30050, 1, 100);
	reg_rw(0x30054, 1, 100);
	reg_rw(0x30058, 1, 100);
	reg_rw(0x3005c, 1, 100);
	reg_rw(0x30060, 1, 100);

	reg_rw(0x30020, 1, 0);
}

int fpga_trig(int card_num, int channel)
{
	//2 代表内触发, 1代表外触发
	g_card = card_num;
	reg_rw(0x30018, 1, 0); //disable trig
	reg_rw(0x30020, 1, 0x22222222); //每一位代表一个通道
	reg_rw(0x30018, 1, 0xff); //enable trig 每一位代表一个通道,上升沿触发
}
