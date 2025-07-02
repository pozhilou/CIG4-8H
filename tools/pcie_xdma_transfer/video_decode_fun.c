
/**
* FFMPEG视频解码流程
* 1、av_register_all()：注册所有组件。
* 2、avformat_open_input()：打开输入视频文件。
* 3、avformat_find_stream_info()：获取视频文件信息
* 4、avcodec_find_decoder()：查找解码器。
* 5、avcodec_open2()：打开解码器。
* 6、av_read_frame()：从输入文件读取一帧压缩数据。
* 7、avcodec_decode_video2()：解码一帧压缩数据。
* 8、avcodec_close()：关闭解码器。
* 9、avformat_close_input()：关闭输入视频文件。 
*/

#include <stdbool.h>
#include <stdio.h>
//#include <libavutil/pixfmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/packet.h>

enum  AVPixelFormat g_pix_fmt_decode = AV_PIX_FMT_YUV420P;
enum  AVPixelFormat g_pix_fmt = AV_PIX_FMT_YUYV422;
struct timespec start;
struct timespec end;
struct tm *p_local_tm;


int Scale_Frame(unsigned char* pSrc,unsigned char* pDst,int src_width, int src_height, \
						int dst_width, int dst_height, enum AVPixelFormat pix_src, enum AVPixelFormat pix_dst)
{
    if (src_width < 1 || src_height < 1 || pSrc == NULL || pDst == NULL)
        return -1;

    AVPicture pFrameSrc,pFrameDst;
    avpicture_fill(&pFrameSrc, pSrc, pix_src, src_width, src_height);
    avpicture_fill(&pFrameDst, pDst, pix_dst, dst_width, dst_height);
    struct SwsContext* imgCtx = NULL;
    imgCtx = sws_getContext(src_width, src_height, pix_src, dst_width, dst_height, pix_dst, SWS_BICUBLIN , 0, 0, 0);

    if (imgCtx != NULL){
        sws_scale(imgCtx, pFrameSrc.data, pFrameSrc.linesize, 0, src_height, pFrameDst.data, pFrameDst.linesize);
        if(imgCtx){
            sws_freeContext(imgCtx);
            imgCtx = NULL;
        }
		//printf("%s success\n", __func__);
        return 0;
    }
    else{
        sws_freeContext(imgCtx);
        imgCtx = NULL;
        return -1;
    }
}

void get_time_start(struct tm *p_local_tm, struct timespec start)
{
	clock_gettime(CLOCK_REALTIME, &start);
	p_local_tm = localtime(&start.tv_sec);
	printf("+++++++now datetime (%d): %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec\n", __LINE__,
			p_local_tm->tm_year+1900,
			p_local_tm->tm_mon+1,
			p_local_tm->tm_mday,
			p_local_tm->tm_hour,
			p_local_tm->tm_min,
			p_local_tm->tm_sec,
			start.tv_nsec, start.tv_sec, start.tv_nsec);
	return;
}

void get_time_end(struct tm *p_local_tm, struct timespec end)
{
	clock_gettime(CLOCK_REALTIME, &end);
	p_local_tm = localtime(&end.tv_sec);
	printf("-------now datetime (%d): %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec\n", __LINE__,
			p_local_tm->tm_year+1900,
			p_local_tm->tm_mon+1,
			p_local_tm->tm_mday,
			p_local_tm->tm_hour,
			p_local_tm->tm_min,
			p_local_tm->tm_sec,
			end.tv_nsec, end.tv_sec, end.tv_nsec);
	return;
}

AVCodecContext  *pCodecCtx = NULL; // 编码器上下文结构体，保存了视频（音频）编解码相关信息。
AVCodec         *pCodec = NULL; // AVCodec是存储编解码器信息的结构体。
enum AVPixelFormat hw_pix_fmt;
AVBufferRef *hw_device_ctx = NULL;
struct SwsContext *img_convert_ctx_yuv420p;
FILE *output_file = NULL;

// H265解码器名称:hevc_cuvid H264解码器名称:h264_cuvid
static int dec_init(AVCodecParameters *codec_params, AVStream *video)
{
	int ret;
	int err = 0;
	// 列举支持的硬解码
	enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
	printf("Available device types:\n");
	while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE) {
		const char *type_name = av_hwdevice_get_type_name(type);
		if (type_name) {
			printf("%s\n", type_name);
		} else {
			printf("Unknown device type\n");
		}
	}
	type = av_hwdevice_find_type_by_name("cuda");
	if (type == AV_HWDEVICE_TYPE_NONE) {
		printf("not support nvidia codec\n");
		exit(0);
	}

	pCodec = avcodec_find_decoder_by_name("h264_cuvid"); //hevc_cuvid 
	//pCodec = avcodec_find_decoder_by_name("hevc_cuvid");
	// 获取该硬解码器的像素格式。cuda对应的hw_pix_fmt是AV_PIX_FMT_CUDA
	for (int i = 0;; i++) {
		const AVCodecHWConfig *config = avcodec_get_hw_config(pCodec, i);
		if (!config) {
			printf("get config error\n");
			exit(0);
		}
		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
				config->device_type == type) {
			hw_pix_fmt = config->pix_fmt;
			break;
		}
	}
	const char *pixname = av_get_pix_fmt_name(hw_pix_fmt);
	printf("hw_pix_fmt:%s\n", pixname);
	// 初始化解码器
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (pCodecCtx == NULL) {
		printf("avcodec_alloc_context3 error\n");
		exit(0);
	}
	avcodec_parameters_to_context(pCodecCtx, video->codecpar); // 拷贝解码参数
	pCodecCtx->pix_fmt = hw_pix_fmt; // 设置硬解码格式
	// 创建一个硬件设备上下文
	if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0)) < 0) {
		printf("Failed to create specified HW device.\n");
		exit(0);
	}
	pCodecCtx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
	// 打开解码器
	ret = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (ret < 0) {
		printf("avcodec_open2 error\n");
		exit(0);
	}

	img_convert_ctx_yuv420p = sws_getContext(pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_NV12,
	pCodecCtx->width, pCodecCtx->height, g_pix_fmt_decode, SWS_BICUBIC, NULL, NULL, NULL);

	printf("dec_init ok, pix_fmt:%d\n", pCodecCtx->pix_fmt);
	return 0;
}
extern int send_one_frame(unsigned char* bufffer);
static int decode_write(AVPacket *packet)
{
    AVFrame *frame = NULL, *sw_frame = NULL;
    AVFrame *tmp_frame = NULL;
	AVFrame *pFrameYUV = NULL;
    uint8_t *buffer = NULL;
	unsigned char *dst_buffer;
	unsigned char *yuv420p_dst_buffer;
    int size, yuv420p_size, dst_size;
    int ret = 0, y_size = 0;
	unsigned char *src_buffer;

	//get_time_start(p_local_tm, start);
    ret = avcodec_send_packet(pCodecCtx, packet); //发出解码
    if (ret < 0) {
        fprintf(stderr, "Error during decoding\n");
        return ret;
    }
	else
		;//printf("avcodec_send_packet success.\n");
    while (1) {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            fprintf(stderr, "Can not alloc frame\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        ret = avcodec_receive_frame(pCodecCtx, frame); //接收解码
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            fprintf(stderr, "Error while decoding\n");
            goto fail;
        }
		//get_time_end(p_local_tm, end);
        if (frame->format == hw_pix_fmt) { // 硬解码，解码后的图像在GPU上
            // 把图像从GPU拷贝到CPU
            // av_hwframe_transfer_data 转换后的格式是NV12的格式转换
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                fprintf(stderr, "Error transferring the data to system memory\n");
                goto fail;
            }
            tmp_frame = sw_frame;
        } else { // 解码之后的图像在CPU上
            tmp_frame = frame;
        }
		//get_time_end(p_local_tm, end);
		//根据像素格式、宽、高、linesize 对其方式来计算所需的内存大小。
        size = av_image_get_buffer_size(tmp_frame->format, tmp_frame->width, tmp_frame->height, 1);
		yuv420p_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, tmp_frame->width, tmp_frame->height, 1);
		dst_size = av_image_get_buffer_size(AV_PIX_FMT_YUYV422, tmp_frame->width, tmp_frame->height, 1);
        buffer = av_malloc(size);
		yuv420p_dst_buffer = av_malloc(yuv420p_size);
		dst_buffer = av_malloc(dst_size);
		src_buffer = malloc(tmp_frame->width*tmp_frame->height*2);

		//申请AVFrame，用于yuv视频
		pFrameYUV = av_frame_alloc();
		av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, yuv420p_dst_buffer, g_pix_fmt_decode, \
									pCodecCtx->width, pCodecCtx->height, 1);

		if (!buffer) {
            fprintf(stderr, "Can not alloc buffer\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
		if (!dst_buffer) {
            fprintf(stderr, "Can not alloc buffer\n");
            ret = AVERROR(ENOMEM);
            goto fail;
        }
		//fprintf(stderr, "alloc buffer success\n");

		//把图像数据拷贝到指定缓冲区
        ret = av_image_copy_to_buffer(buffer, size,
                                      (const uint8_t *const *)tmp_frame->data,
                                      (const int *)tmp_frame->linesize, tmp_frame->format,
                                      tmp_frame->width, tmp_frame->height, 1);
        if (ret < 0) {
            fprintf(stderr, "Can not copy image to buffer\n");
            goto fail;
        }
		//get_time_end(p_local_tm, end);
        //fwrite(buffer, 1, size, output_file); //写入文件的格式是NV12
		//sws_scale(img_convert_ctx_yuv420p, (const unsigned char* const*)tmp_frame->data, tmp_frame->linesize, 0, g_pix_fmt_decode,
		//			pFrameYUV->data, pFrameYUV->linesize); // 转换图像格式
		//y_size = tmp_frame->width*tmp_frame->height;
		//memcpy(src_buffer, pFrameYUV->data[0], y_size);
		//memcpy(src_buffer+y_size, pFrameYUV->data[1], y_size/4);
		//memcpy(src_buffer+y_size+y_size/4, pFrameYUV->data[2], y_size/4);

		Scale_Frame(buffer, dst_buffer, tmp_frame->width, tmp_frame->height,
				tmp_frame->width, tmp_frame->height, AV_PIX_FMT_NV12, g_pix_fmt);
		//get_time_end(p_local_tm, end);
		send_one_frame(dst_buffer);
		
		//fwrite(dst_buffer, 1, dst_size, output_file); //写入文件的格式是 yuyv422

    fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        av_freep(&buffer);
		av_freep(&dst_buffer);
        if (ret < 0)
            return ret;
    }
}

int video_decode(int (*send_fun)(),char* file_name)
{
	//文件格式上下文
	AVFormatContext	*pFormatCtx;    // 封装格式上下文结构体，也是统领全局的结构体，保存了视频文件封装 格式相关信息。 
	int		i = 0, videoindex;
	AVFrame	*pFrame, *pFrameYUV;    // AVFrame是包含码流参数较多的结构体
	unsigned char *out_buffer;
	AVPacket *packet;               // AVPacket是存储压缩编码数据相关信息的结构体
	unsigned char *src_buffer;
	unsigned char *dst_buffer;
	int y_size;
	int ret, got_picture;
	int video_index = -1;

	// struct SwsContext结构体位于libswscale类库中, 该类库主要用于处理图片像素数据, 可以完成图片像素格式的转换, 图片的拉伸等工作.
	struct SwsContext *img_convert_ctx; 
	char filepath[] = "./ds.h264";//"output_0_0_0.h264";//"input.mkv";
	//FILE *fp_yuv = fopen("output.yuv", "wb+");
	output_file = fopen("output.yuv", "wb+");
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	av_register_all();    // 注册所有组件
	printf("av_register_all successful.\n");
	av_init_packet(packet);
	//avformat_network_init();   // 对网络库进行全局初始化。
	pFormatCtx = avformat_alloc_context();   // 初始化AVFormatContext结构体指针。使用avformat_free_context()释放内存。
	if (avformat_open_input(&pFormatCtx, file_name, NULL, NULL) != 0)  // 打开输入流并读取header。必须使用avformat_close_input()接口关闭。
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
	else
		printf("open input stream successful.\n");
	//读取一部分视音频数据并且获得一些相关的信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) // 读取媒体文件的包以获取流信息
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	else
		printf("find stream information successful.\n");
	
	// 找到视频流
	video_index = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	printf("nb_streams:%d, video_index:%d\n",pFormatCtx->nb_streams, video_index);
	
	// 解码器初始化
	dec_init(pFormatCtx->streams[video_index]->codecpar, pFormatCtx->streams[video_index]);

	while (av_read_frame(pFormatCtx, packet) >= 0) {
    	if (packet->stream_index == video_index) {
            AVStream *video_stream = pFormatCtx->streams[video_index];
            //printf("video pts:%lld\n", packet->pts);
			//get_time_start(p_local_tm, start);
			//decode
            decode_write(packet);
			get_time_end(p_local_tm, end);
        }
        av_packet_unref(packet);
    }
	return 0;

	//查找视频编码索引
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}

	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}

	//申请AVFrame，用于原始视频
	pFrame = av_frame_alloc();
	//申请AVFrame，用于yuv视频
	pFrameYUV = av_frame_alloc();
	//分配内存，用于图像格式转换
	src_buffer = malloc(pCodecCtx->width*pCodecCtx->height*2);//(unsigned char *)av_malloc(av_image_get_buffer_size(g_pix_fmt, pCodecCtx->width, pCodecCtx->height, 1));
	dst_buffer = malloc(pCodecCtx->width*pCodecCtx->height*2);//(unsigned char *)av_malloc(av_image_get_buffer_size(g_pix_fmt, pCodecCtx->width, pCodecCtx->height, 1));
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(g_pix_fmt_decode, pCodecCtx->width, pCodecCtx->height, 1));
	// 根据指定的图像参数和提供的数组设置参数指针和linesize大小
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer,g_pix_fmt_decode, pCodecCtx->width, pCodecCtx->height, 1);
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	//手工调试函数，输出tbn、tbc、tbr、PAR、DAR的含义
	av_dump_format(pFormatCtx, 0, file_name, 0);
	printf("-------------------------------------------------\n");

	//申请转换上下文。 sws_getContext功能：初始化 SwsContext 结构体指针  
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
	pCodecCtx->width, pCodecCtx->height, g_pix_fmt_decode, SWS_BICUBIC, NULL, NULL, NULL);

	//读取数据
	while (av_read_frame(pFormatCtx, packet) >= 0) // 读取码流中的音频若干帧或者视频一帧
	{
		clock_gettime(CLOCK_REALTIME, &start);
		p_local_tm = localtime(&start.tv_sec);
		printf("stream_index:%d videoindex:%d, %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec\n",
				packet->stream_index, videoindex,
				p_local_tm->tm_year+1900,
				p_local_tm->tm_mon+1,
				p_local_tm->tm_mday,
				p_local_tm->tm_hour,
				p_local_tm->tm_min,
				p_local_tm->tm_sec,
				start.tv_nsec, start.tv_sec, start.tv_nsec);
		if (packet->stream_index == videoindex) 
		{
			// avcodec_decode_video2 功能:解码一帧视频数据
			get_time_end(p_local_tm, start);
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) 
			{
				printf("Decode Error.\n");
				return -1;
			}
			get_time_end(p_local_tm, start);
			//printf("got picture %d\n",got_picture);

			if(got_picture >= 1000)
			{
				continue;
			}

			if (got_picture >= 1) 
			{
								//成功解码一帧
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, 
				pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize); // 转换图像格式

				//y_size = pCodecCtx->width*pCodecCtx->height;
				// fwrite 功能:把 pFrameYUV 所指向数据写入到 fp_yuv 中。
				//fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
				//fwrite(pFrameYUV->data[1], 1, y_size / 2, fp_yuv);  //U
				//fwrite(pFrameYUV->data[2], 1, y_size / 2, fp_yuv);  //V
				//printf("pix_fmt:%d\n",pCodecCtx->pix_fmt);
				y_size = pCodecCtx->width*pCodecCtx->height;

				clock_gettime(CLOCK_REALTIME, &start);
				p_local_tm = localtime(&start.tv_sec);
				//printf("Decode ++++ %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec.\n", \
				//		p_local_tm->tm_year+1900,
				//		p_local_tm->tm_mon+1,
				//		p_local_tm->tm_mday,
				//		p_local_tm->tm_hour,
				//		p_local_tm->tm_min,
				//		p_local_tm->tm_sec,
				//		start.tv_nsec, start.tv_sec, start.tv_nsec);
				memcpy(src_buffer,pFrameYUV->data[0],y_size);
				memcpy(src_buffer+y_size,pFrameYUV->data[1],y_size/4);
				memcpy(src_buffer+y_size+y_size/4,pFrameYUV->data[2],y_size/4);
				clock_gettime(CLOCK_REALTIME, &start);
				p_local_tm = localtime(&start.tv_sec);
				printf("Decode ---- %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec.\n", \
						p_local_tm->tm_year+1900,
						p_local_tm->tm_mon+1,
						p_local_tm->tm_mday,
						p_local_tm->tm_hour,
						p_local_tm->tm_min,
						p_local_tm->tm_sec,
						start.tv_nsec, start.tv_sec, start.tv_nsec);

				Scale_Frame(src_buffer,dst_buffer,pCodecCtx->width,pCodecCtx->height,
				pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt, g_pix_fmt);
				//fwrite(dst_buffer, 1, y_size*2, fp_yuv);  
				clock_gettime(CLOCK_REALTIME, &start);
				p_local_tm = localtime(&start.tv_sec);
				printf("Decode 1111 %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec.\n", \
						p_local_tm->tm_year+1900,
						p_local_tm->tm_mon+1,
						p_local_tm->tm_mday,
						p_local_tm->tm_hour,
						p_local_tm->tm_min,
						p_local_tm->tm_sec,
						start.tv_nsec, start.tv_sec, start.tv_nsec);
				send_fun(dst_buffer);
#if 0				
				//成功解码一帧
				sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize); // 转换图像格式

				y_size = pCodecCtx->width*pCodecCtx->height;
				// fwrite 功能:把 pFrameYUV 所指向数据写入到 fp_yuv 中。
				fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
				fwrite(pFrameYUV->data[1], 1, y_size / 2, fp_yuv);  //U
				fwrite(pFrameYUV->data[2], 1, y_size / 2, fp_yuv);  //V
#endif			
				clock_gettime(CLOCK_REALTIME, &start);
				p_local_tm = localtime(&start.tv_sec);	
				printf("Succeed to decode 1 frame, %04d-%02d-%02d %02d:%02d:%02d.%09ld, %ld.%09ld sec!\n", 
						p_local_tm->tm_year+1900,
						p_local_tm->tm_mon+1,
						p_local_tm->tm_mday,
						p_local_tm->tm_hour,
						p_local_tm->tm_min,
						p_local_tm->tm_sec,
						start.tv_nsec, start.tv_sec, start.tv_nsec);
			}
			else
			{
				//未解码到一帧，可能时结尾B帧或延迟帧，在后面做flush decoder处理
			}
		}
		av_free_packet(packet); // free
	}

	//flush decoder
	//FIX: Flush Frames remained in Codec
 	// while (true) 
	// {
	// 	if (!(pCodec->capabilities & AV_CODEC_CAP_DELAY))
	// 		return 0;
	// 	// avcodec_decode_video2 功能:解码一帧视频数据
	// 	ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
	// 	if (ret < 0)
	// 	{
	// 		break;
	// 	}/*  */
	// 	if (!got_picture)
	// 	{
	// 		break;
	// 	}

	// 	sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
	// 		pFrameYUV->data, pFrameYUV->linesize); // 转换图像格式

	// 	int y_size = pCodecCtx->width*pCodecCtx->height;
	// 	// fwrite 功能:把 pFrameYUV 所指向数据写入到 fp_yuv 中。
	// 	fwrite(pFrameYUV->data[0], 1, y_size, fp_yuv);    //Y 
	// 	fwrite(pFrameYUV->data[1], 1, y_size / 4, fp_yuv);  //U
	// 	fwrite(pFrameYUV->data[2], 1, y_size / 4, fp_yuv);  //V
	// 	printf("Flush Decoder: Succeed to decode 1 frame!\n");
	// } 

	sws_freeContext(img_convert_ctx);
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	fclose(output_file);

    return 0;
}
