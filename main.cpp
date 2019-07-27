#include <iostream>
#include <unistd.h>

#include <SDL2/SDL.h>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


using namespace std;

int main(int argc,char *argv[])
{
    if(argc != 4){
        cout << "./ffmpeg-sdl-palyer fileName width height" << endl;
    }
    int windowWidth = 1920;
    int windowHeight = 1080;
    SDL_Window *pWindow = nullptr;
    SDL_Renderer *pRender = nullptr;
    SDL_Texture *pTexture = nullptr;
    SDL_Rect sdlRect;

    windowWidth = atoi(argv[2]);
    windowHeight = atoi(argv[3]);


    //ffmpeg
    AVFormatContext *p_avFormatCtx;
    AVCodec *p_avCodec;
    AVCodecContext *p_videoAvCodecCtx;
    int videoIndex = 0;

    //1.init
    av_register_all();
    avformat_network_init();

    cout << "ffmpeg init" << endl;
    //2.open input
    p_avFormatCtx = avformat_alloc_context();
    if(avformat_open_input(&p_avFormatCtx,argv[1],NULL,NULL) != 0){
        cout << "avformat_open_input error" << endl;
        return -1;
    }

    cout << "video format : " << p_avFormatCtx->iformat->name << endl;

    cout << "open input" << endl;
    //3.find stream
    if(avformat_find_stream_info(p_avFormatCtx,NULL) < 0){
        cout << "avformat_find_stream_info error" << endl;
        return -1;
    }

    av_dump_format(p_avFormatCtx,0,argv[1],0);

    //4.find video stream index
    for(int i = 0;i < p_avFormatCtx->nb_streams;i++){
        if(p_avFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            videoIndex = i;
            break;
        }
    }
    p_videoAvCodecCtx = p_avFormatCtx->streams[videoIndex]->codec;

    //5.find codec
    p_avCodec = avcodec_find_decoder(p_videoAvCodecCtx->codec_id);

    //6.open decoder
    AVCodecContext *p_decoderAvCodecCtx = avcodec_alloc_context3(p_avCodec);
    avcodec_copy_context(p_decoderAvCodecCtx,p_videoAvCodecCtx);

    if(avcodec_open2(p_decoderAvCodecCtx,p_avCodec,NULL) != 0){
        cout << "avcodec_open2 Error" << endl;
        return -1;
    }

    AVFrame *p_YUVFrame = avcodec_alloc_frame();

    AVPacket packet;

    //SDL

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        cout << "SDL_Init Error : " << SDL_GetError() << endl;
        return -1;
    }

    if((pWindow = SDL_CreateWindow("title",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,windowWidth,windowHeight,SDL_WINDOW_OPENGL)) == nullptr){
        cout << "SDL_CreateWindow Error : " << SDL_GetError() << endl;
        return -1;
    }

    if((pRender = SDL_CreateRenderer(pWindow, -1, 0)) == nullptr){
        cout <<"SDL_CreateRenderer Error" << endl;
        return -1;
    }

    pTexture = SDL_CreateTexture(pRender,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,p_videoAvCodecCtx->width,p_videoAvCodecCtx->height);

    if(pTexture == nullptr){
        cout << "SDL_CreateTexture Error" << endl;
        return -1;
    }

    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = p_videoAvCodecCtx->width;
    sdlRect.h = p_videoAvCodecCtx->height;

    //SDL

    //7.read frame
    while(av_read_frame(p_avFormatCtx,&packet) >= 0){
        cout << "read packet" << endl;
        if(packet.stream_index == videoIndex){
            int framDecodeFinish = 0;
            //8.decode frame
            avcodec_decode_video2(p_decoderAvCodecCtx,p_YUVFrame,&framDecodeFinish,&packet);
            if(framDecodeFinish != 0){
                //decode frame success
                cout << "present" << endl;

                SDL_UpdateYUVTexture(pTexture,&sdlRect,
                                     p_YUVFrame->data[0],p_YUVFrame->linesize[0],
                                     p_YUVFrame->data[1],p_YUVFrame->linesize[1],
                                     p_YUVFrame->data[2],p_YUVFrame->linesize[2]);
                SDL_RenderClear(pRender);
                SDL_RenderCopy(pRender,pTexture,nullptr,&sdlRect);
                SDL_RenderPresent(pRender);
                SDL_Delay(40);
            }
        }
    }

}
