
#ifndef READER_H
#define READER_H

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/bprint.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

extern "C"
{
	typedef struct FrameDetails
	{
		int width;
		int height;
		int timebase_num; ///< Time Base Numerator
		int timebase_den; ///< Time Base Denominator
	} FrameDetails;
	typedef struct Frame
	{
		double pts_seconds;
		int64_t pts;
		uint8_t *buffer[4] = {NULL};
		int size[4];

	} Frame;
};

class VideoDecoder
{
public:
	int width, height;
	AVRational time_base;

	void saveFile(uint8_t *fileBytes, int length, char *filename);
	int deleteFile(char *filename);
	void getFrameDetails(FrameDetails *frameDetails);
	int open(const char *filename, Frame *frame);
	int readFrame(Frame *frame);
	void freeFrame(Frame *frame);
	void close();

	~VideoDecoder() {}
	VideoDecoder()
	{
		av_format_ctx = nullptr;
		av_codec_ctx = nullptr;
		video_stream_index = -1;
		av_frame = nullptr;
		av_packet = nullptr;
		sws_scaler_ctx = nullptr;
	}

private:
	// Private internal state
	AVFormatContext *av_format_ctx;
	AVCodecContext *av_codec_ctx;
	int video_stream_index;
	AVFrame *av_frame;
	AVPacket *av_packet;
	SwsContext *sws_scaler_ctx;
};

extern "C"
{
	void *VideoDecoderNew();
	void VideoDecoderDelete(void *obj);
	void VideoDecoderSaveFile(void *obj, uint8_t *file_bytes, int file_length, char *filename);
	int VideoDecoderDeleteFile(void *obj, char *filename);
	int VideoDecoderOpen(void *obj, char *filename, Frame *frame);
	void VideoDecoderGetFrameDetails(void *obj, FrameDetails *frameDetails);
	int VideoDecoderReadFrame(void *obj, Frame *frame);
	void VideoDecoderClose(void *obj);
	void VideoDecoderFreeFrame(void *obj, Frame *frame);
}
#endif // READER_H
