#define _CRT_SECURE_NO_WARNINGS

#include <list>
#include <fstream>
#include <string>
using namespace std;

//opencv
#include <opencv2/opencv.hpp>
#include <opencv2/legacy/compat.hpp>
using namespace cv;

//ffmpeg
extern "C"{
#include <stdlib.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}


class Picture {
public:
	double rate;
	int frame_num;
	Mat pic;
	Picture() { rate = 1; frame_num = 0; }
	Picture(double rate, int frame_num, Mat& mat) { this->rate = rate; this->pic = mat; this->frame_num = frame_num; }
	~Picture() { this->pic.release(); }
};

// internal function, skip it.
void CopyDate(AVFrame *pFrame, Mat& target_frame, int width, int height)
{
	int     nChannels;
	int     stepWidth;
	uchar*  pData;
	target_frame = Mat(cv::Size(width, height), CV_8UC3, cv::Scalar(0));
	stepWidth = target_frame.step;
	nChannels = target_frame.channels();
	pData = target_frame.data;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			pData[i*stepWidth + j*nChannels + 0] = pFrame->data[0][i*pFrame->linesize[0] + j*nChannels + 2];
			pData[i*stepWidth + j*nChannels + 1] = pFrame->data[0][i*pFrame->linesize[0] + j*nChannels + 1];
			pData[i*stepWidth + j*nChannels + 2] = pFrame->data[0][i*pFrame->linesize[0] + j*nChannels + 0];
		}
	}
}

/**
   param:
   @ pic_path: input pic location
   @ filename: input video location
   @ save_path: output file save location
   return:
   @ fail if return value < 0
   @ otherwise success
   usage: 
   int result = FindFrame(xxx,xxxx,xx)
*/
int FindFrame(String pic_path, const char *filename, String save_path)
{
	const int hist_size = 256, list_size = 5;
	float range[] = { 0, 255 };
	const float* ranges[] = { range };
	Mat hist, hist2;
	list<Picture> lst;
	list<Picture>::reverse_iterator riter_pic;
	for (int i = 0; i < list_size; ++i) {
		lst.push_back(Picture());
	}

	Mat origin_img = imread(pic_path, 0);
	calcHist(&origin_img, 1, 0, Mat(), hist, 1, &hist_size, ranges, true, false);
	normalize(hist, hist, 0, 1, NORM_MINMAX, -1, Mat());

	SwsContext *pSWSCtx;
	AVFormatContext *pFormatCtx;
	int i, videoStream;
	AVCodecContext *pCodecCtx;
	AVFrame *pFrame;
	AVFrame *pFrameRGB;
	int     numBytes, frameFinished;
	uint8_t *buffer;
	static AVPacket packet;

	av_register_all();
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) < 0)
		return -1;

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -2;

	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
		videoStream = i;
		break;
		}
	if (videoStream == -1)
		return -3;
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	AVCodec *pCodec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
		return -4;
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		return -5;
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	buffer = new uint8_t[numBytes];
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
	pSWSCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
	i = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if (packet.stream_index == videoStream)
		{
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			if (frameFinished)
			{
				int num = pFrame->coded_picture_number;
				sws_scale(pSWSCtx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
				Mat target_frame;
				CopyDate(pFrameRGB, target_frame, pCodecCtx->width, pCodecCtx->height);
				Mat gray_img(target_frame.rows, target_frame.cols, CV_8UC1);
				cvtColor(target_frame, gray_img, CV_BGR2GRAY);
				calcHist(&gray_img, 1, 0, Mat(), hist2, 1, &hist_size, ranges, true, false);
				normalize(hist2, hist2);
				double com = compareHist(hist, hist2, CV_COMP_BHATTACHARYYA);
				for (riter_pic = lst.rbegin(); riter_pic != lst.rend(); ++riter_pic) {
					if (com >= riter_pic->rate) {
						break;
					}
				}
				if (riter_pic != lst.rbegin()) {
					lst.insert(riter_pic.base(), Picture(com, num, target_frame));
					lst.resize(5);
				}
			}
		}
		av_free_packet(&packet);
	}
	String pic_loc[5];
	pic_loc[0] = save_path + "/0.jpg";
	pic_loc[1] = save_path + "/1.jpg";
	pic_loc[2] = save_path + "/2.jpg";
	pic_loc[3] = save_path + "/3.jpg";
	pic_loc[4] = save_path + "/4.jpg";
	ofstream outfile(save_path + "/result.txt", ios::out);
	int j = 0;
	for (riter_pic = lst.rbegin(); riter_pic != lst.rend(); ++riter_pic) {
		outfile << j << "," << riter_pic->rate << "," << riter_pic->frame_num << endl;
		imwrite(pic_loc[j], riter_pic->pic);
		++j;
	}
	outfile.close();
	av_free(pFrameRGB);
	av_free(pFrame);
	sws_freeContext(pSWSCtx);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

// 编译成py可调用的动态链接库时需要的
/*#include <boost/python.hpp>

BOOST_PYTHON_MODULE(pySeek)
{
	using namespace boost::python;
	def("FindFrame", FindFrame);
}*/