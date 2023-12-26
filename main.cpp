#include <cstdlib>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

int main()
{
    const char *input_url
        = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4";
    const char *output_filename = "output3.mp4";

    fprintf(stdout, "Input URL: %s\n", input_url);

    // Initialize FFmpeg
    // av_register_all();

    // Open input video stream
    fprintf(stdout, "Open input video stream\n");
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    if (avformat_open_input(&inputFormatContext, input_url, nullptr, nullptr) != 0) {
        fprintf(stderr, "Error opening input video stream\n");
        return -1;
    }

    // Find input video stream info
    fprintf(stdout, "Find input video stream info\n");
    if (avformat_find_stream_info(inputFormatContext, nullptr) < 0) {
        fprintf(stderr, "Error finding input stream info\n");
        return -1;
    }

    // Find video stream index
    fprintf(stdout, "Find video stream index\n");

    int videoStreamIndex
        = av_find_best_stream(inputFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    /*
    for (unsigned int i = 0; i < inputFormatContext->nb_streams; i++) {
        if (inputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        fprintf(stderr, "Error finding video stream\n");
        return -1;
    }*/

    fprintf(stdout, "Video stream index: %d\n", videoStreamIndex);

    // Open output file
    AVFormatContext *outputFormatContext = avformat_alloc_context();
    if (avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, output_filename)
        < 0) {
        fprintf(stderr, "Error creating output file context\n");
        return -1;
    }

    // Find the video codec and copy its parameters
    AVCodec *codec = (AVCodec *) avcodec_find_encoder(
        inputFormatContext->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Error finding video codec\n");
        return -1;
    }

    AVStream *outStream = avformat_new_stream(outputFormatContext, codec);
    if (!outStream) {
        fprintf(stderr, "Error creating output stream\n");
        return -1;
    }

    if (avcodec_parameters_copy(outStream->codecpar,
                                inputFormatContext->streams[videoStreamIndex]->codecpar)
        < 0) {
        fprintf(stderr, "Error copying codec parameters\n");
        return -1;
    }

    // Open output file for writing
    if (avio_open(&outputFormatContext->pb, output_filename, AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "Error opening output file for writing\n");
        return -1;
    }

    // Write the output file header
    if (avformat_write_header(outputFormatContext, nullptr) < 0) {
        fprintf(stderr, "Error writing output file header\n");
        return -1;
    }

    // Read and write frames
    AVPacket packet;
    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;

    while (av_read_frame(inputFormatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            // Adjust packet timestamps and write to the output file
            av_packet_rescale_ts(&packet,
                                 inputFormatContext->streams[videoStreamIndex]->time_base,
                                 outStream->time_base);
            packet.stream_index = outStream->index;

            if (av_interleaved_write_frame(outputFormatContext, &packet) < 0) {
                fprintf(stderr, "Error writing video frame\n");
                return -1;
            }
        }

        av_packet_unref(&packet);
    }

    // Write the output file trailer
    if (av_write_trailer(outputFormatContext) < 0) {
        fprintf(stderr, "Error writing output file trailer\n");
        return -1;
    }

    // Clean up
    avformat_close_input(&inputFormatContext);
    if (outputFormatContext && !(outputFormatContext->oformat->flags & AVFMT_NOFILE))
        avio_closep(&outputFormatContext->pb);
    avformat_free_context(outputFormatContext);

    return 0;
}
