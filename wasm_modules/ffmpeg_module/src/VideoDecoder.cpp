#include "VideoDecoder.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

// av_err2str returns a temporary array. This doesn't work in gcc.
// This function can be used as a replacement for av_err2str.
static const char *av_make_error(int errnum)
{
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

static AVPixelFormat correct_for_deprecated_pixel_format(AVPixelFormat pix_fmt)
{
    // Fix swscaler deprecated pixel format warning
    // (YUVJ has been deprecated, change pixel format to regular YUV)
    switch (pix_fmt)
    {
    case AV_PIX_FMT_YUVJ420P:
        return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P:
        return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ444P:
        return AV_PIX_FMT_YUV444P;
    case AV_PIX_FMT_YUVJ440P:
        return AV_PIX_FMT_YUV440P;
    default:
        return pix_fmt;
    }
}
static AVFrame *allocPicture(enum AVPixelFormat pix_fmt, int width, int height)
{
    // Allocate a frame
    AVFrame *frame = av_frame_alloc();

    if (frame == NULL)
    {
        fprintf(stderr, "avcodec_alloc_frame failed");
    }

    if (av_image_alloc(frame->data, frame->linesize, width, height, pix_fmt, 1) < 0)
    {
        fprintf(stderr, "av_image_alloc failed");
    }

    frame->width = width;
    frame->height = height;
    frame->format = pix_fmt;

    return frame;
}
void VideoDecoder::saveFile(uint8_t *fileBytes, int length, char *filename)
{
    std::ofstream fp;
    std::string fileNameStr(filename);
    fp.open(fileNameStr, std::ios::out | std::ios::binary);
    fp.write((char *)fileBytes, length);
}
int VideoDecoder::deleteFile(char *filename)
{
    int result = remove(filename);
    return result;
}
void VideoDecoder::getFrameDetails(FrameDetails *frameDetails)
{
    frameDetails->width = width;
    frameDetails->height = height;
    frameDetails->timebase_num = time_base.num;
    frameDetails->timebase_den = time_base.den;
}
int VideoDecoder::open(const char *filename, Frame *frame)
{
    // Open the file using libavformat
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx)
    {
        printf("Couldn't created AVFormatContext\n");
        return -1;
    }

    if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) != 0)
    {
        printf("Couldn't open video file\n");
        return -1;
    }

    // Find the first valid video stream inside the file
    video_stream_index = -1;
    AVCodecParameters *av_codec_params = nullptr;
    const AVCodec *av_codec = nullptr;
    for (int i = 0; i < av_format_ctx->nb_streams; ++i)
    {
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);
        if (!av_codec)
        {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            width = av_codec_params->width;
            height = av_codec_params->height;
            time_base = av_format_ctx->streams[i]->time_base;
            break;
        }
    }
    if (video_stream_index == -1)
    {
        printf("Couldn't find valid video stream inside file\n");
        return -1;
    }
    // Set up a codec context for the decoder
    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx)
    {
        printf("Couldn't create AVCodecContext\n");
        return -1;
    }
    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0)
    {
        printf("Couldn't initialize AVCodecContext\n");
        return -1;
    }
    av_codec_ctx->thread_count = 0;

    if (av_codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
        av_codec_ctx->thread_type = FF_THREAD_FRAME;
    else if (av_codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
        av_codec_ctx->thread_type = FF_THREAD_SLICE;
    else
        av_codec_ctx->thread_count = 1; // don't use multithreading

    if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0)
    {
        printf("Couldn't open codec\n");
        return -1;
    }

    av_frame = av_frame_alloc();
    if (!av_frame)
    {
        printf("Couldn't allocate AVFrame\n");
        return -1;
    }
    if (av_image_alloc(frame->buffer, frame->size,
                       width, height, AV_PIX_FMT_YUV420P, 1) < 0)
    {
        printf("Couldn't allocate Frame YUV\n");
        return -1;
    }

    av_packet = av_packet_alloc();
    if (!av_packet)
    {
        printf("Couldn't allocate AVPacket\n");
        return -1;
    }

    return 1;
}
void VideoDecoder::freeFrame(Frame *frame)
{
    av_freep(&frame->buffer);
}

int VideoDecoder::readFrame(Frame *frame)
{
    int response;
    while (av_read_frame(av_format_ctx, av_packet) >= 0)
    {
        if (av_packet->stream_index != video_stream_index)
        {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0)
        {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return -1;
        }

        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        // char frame_filename[1024];
        // snprintf(frame_filename, sizeof(frame_filename), "%s-%d.yuv", "frame", av_codec_ctx->frame_number);
        // save_av_frame(av_frame, frame_filename);

        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
        {
            av_packet_unref(av_packet);
            continue;
        }
        else if (response < 0)
        {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return -1;
        }

        av_packet_unref(av_packet);
        break;
    }

    int64_t pts = av_frame->pts;
    frame->pts = pts;
    frame->pts_seconds = pts * (double)time_base.num / time_base.den;
    auto source_pix_fmt = correct_for_deprecated_pixel_format(av_codec_ctx->pix_fmt);
    bool is_yuv420p = source_pix_fmt == AV_PIX_FMT_YUV420P;
    if (is_yuv420p)
    {
        av_image_copy(frame->buffer, frame->size,
                      (const uint8_t **)(av_frame->data), av_frame->linesize,
                      AV_PIX_FMT_YUV420P, width, height);
        return 1;
    }

    // Set up sws scaler
    if (!sws_scaler_ctx)
    {

        sws_scaler_ctx = sws_getContext(width, height, source_pix_fmt,
                                        width, height, AV_PIX_FMT_YUV420P,
                                        SWS_BILINEAR, NULL, NULL, NULL);
    }
    if (!sws_scaler_ctx)
    {
        printf("Couldn't initialize sw scaler\n");
        return -1;
    }
    sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, frame->buffer, frame->size);

    return 1;
}
void VideoDecoder::close()
{
    if (av_codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        sws_freeContext(sws_scaler_ctx);
    }
    avformat_close_input(&av_format_ctx);
    avformat_free_context(av_format_ctx);
    av_frame_free(&av_frame);
    av_packet_free(&av_packet);
    avcodec_free_context(&av_codec_ctx);
}

// extern "C"
// {
//     void *VideoDecoderNew() { return new VideoDecoder(); }
//     void VideoDecoderDelete(void *obj) { delete (VideoDecoder *)obj; }
//     void VideoDecoderSaveFile(void *obj, uint8_t *file_bytes, int file_length, char *filename)
//     {
//         ((VideoDecoder *)obj)->saveFile(file_bytes, file_length, filename);
//     }
//     int VideoDecoderDeleteFile(void *obj, char *filename)
//     {
//         return ((VideoDecoder *)obj)->deleteFile(filename);
//     }
//     int VideoDecoderOpen(void *obj, char *filename, Frame *frame)
//     {
//         return ((VideoDecoder *)obj)->open(filename, frame);
//     }
//     void VideoDecoderGetFrameDetails(void *obj, FrameDetails *frameDetails)
//     {
//         ((VideoDecoder *)obj)->getFrameDetails(frameDetails);
//     }
//     int VideoDecoderReadFrame(void *obj, Frame *frame)
//     {
//         return ((VideoDecoder *)obj)->readFrame(frame);
//     }
//     void VideoDecoderClose(void *obj)
//     {
//         ((VideoDecoder *)obj)->close();
//     }
//     void VideoDecoderFreeFrame(void *obj, Frame *frame)
//     {
//         ((VideoDecoder *)obj)->freeFrame(frame);
//     }
// }
