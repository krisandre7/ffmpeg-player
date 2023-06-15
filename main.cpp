/*
 * Read video frame with FFmpeg and convert to OpenCV image
 *
 * Copyright (c) 2016 yohhoy
 */
#include <iostream>
#include <vector>

// FFmpeg
extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

// Converte um frame do FFmpeg para uma imagem do OpenCV
Mat avframeToCvmat(const AVFrame *frame)
{
    // cria uma imagem do OpenCV
    int width = frame->width;
    int height = frame->height;
    Mat image(height, width, CV_8UC3);

    // copia os dados do frame para a imagem
    int cvLinesizes[1];
    cvLinesizes[0] = image.step1();

    // se o frame não tiver formato, retorna a imagem vazia
    if (frame->format == -1)
    {
        return image;
    }

    // converte o frame para o formato BGR
    // O formato BGR é o formato padrão do OpenCV
    SwsContext *conversion = sws_getContext(
        width, height, (AVPixelFormat)frame->format, width, height,
        AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    // o sws_scale converte o frame para o formato BGR
    sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data,
              cvLinesizes);

    // libera memória
    sws_freeContext(conversion);

    return image;
}

// Referência
// https://github.com/leandromoreira/ffmpeg-libav-tutorial#intro
int main(int argc, char *argv[])
{
    const char *filename = "video.mp4";

    // aloca um contexto de formato
    AVFormatContext *pFormatContext = avformat_alloc_context();

    // abre o arquivo de entrada
    if (avformat_open_input(&pFormatContext, filename, NULL, NULL) != 0)
    {
        cerr << "Error: Couldn't open input file." << endl;
        return 1;
    }

    // recupera informações do formato
    if (avformat_find_stream_info(pFormatContext, NULL) < 0)
    {
        cerr << "Error: Couldn't find stream information." << endl;
        return 1;
    }

    // encontra o primeiro stream de vídeo
    int videoStream = -1;
    for (unsigned int i = 0; i < pFormatContext->nb_streams; i++)
    {
        // recupera o codec parâmetros para o stream
        AVCodecParameters *pLocalCodecParameters = pFormatContext->streams[i]->codecpar;

        // se for o stream de vídeo, salva o índice
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            AVCodec *pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

            // imprime informações sobre o codec
            cout << "Codec: " << pLocalCodec->long_name << endl;

            break;
        }
    }

    // se não encontrar o stream de vídeo, retorna erro
    if (videoStream == -1)
    {
        cerr << "Error: Didn't find a video stream." << endl;
        return 1;
    }

    // recupera um ponteiro para o codec
    AVCodecParameters *pCodecParameters = pFormatContext->streams[videoStream]->codecpar;

    // encontra o decoder para o codec
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);

    // aloca um contexto do codec
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecContext, pCodecParameters);

    // abre o codec
    avcodec_open2(pCodecContext, pCodec, NULL);

    // aloca um pacote de dados e um frame
    AVPacket *pPacket = av_packet_alloc();
    AVFrame *pFrame = av_frame_alloc();

    // calcula o frame rate
    int frame_rate = pFormatContext->streams[videoStream]->avg_frame_rate.num / pFormatContext->streams[videoStream]->avg_frame_rate.den;

    // exibe informações sobre o vídeo
    while (av_read_frame(pFormatContext, pPacket) >= 0)
    {
        if (pPacket->stream_index == videoStream)
        {
            // decodifica o frame
            avcodec_send_packet(pCodecContext, pPacket);
            avcodec_receive_frame(pCodecContext, pFrame);

            Mat image = avframeToCvmat(pFrame);

            // se o frame não for vazio, exibe-o
            if (!image.empty())
            {
                // show frame according to frame rate
                int delay = 1000 / frame_rate;

                // exibe o frame
                imshow("Frame", image);

                // aguarda delay
                waitKey(delay);

                // libera memória
                image.release();

                // se pressionar ESC, sai do loop
                if (waitKey(1) == 27)
                    break;
            }
        }

        // libera o pacote
        av_packet_unref(pPacket);

        // libera o frame
        av_frame_unref(pFrame);
    }

    // libera memória
    avformat_close_input(&pFormatContext);
    avformat_free_context(pFormatContext);
    avcodec_free_context(&pCodecContext);

    return 0;
}