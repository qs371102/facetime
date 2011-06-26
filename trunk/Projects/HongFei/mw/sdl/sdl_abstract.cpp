#include "sdl_abstract.h"
#include "util_log.h"

//#define TEST_RGB565

SdlAbstract::SdlAbstract () : mOperationLock("SdlAbstract"), mpScreen(NULL), mpBmp(NULL), mpImgConvertCtx(NULL), miWidth(0), miHeight(0) {
    mDeque.clear ();
    mCurrentFrame.len = 0;
    mCurrentFrame.index = 0;
}

SdlAbstract::~SdlAbstract () {
    if (mpBmp) {
        SDL_FreeYUVOverlay(mpBmp);
        mpBmp = NULL;
    }
    SDL_Quit ();
}

int SdlAbstract::Init (AVStream *vstream, AVStream *astream) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        ERROR("Could not initialize SDL - %s", SDL_GetError());
        return -1;
    }
    DEBUG ("vstream:%p, astream:%p", vstream, astream);
    SDL_Surface* screen = NULL;
    // Make a screen to put our video
#ifndef __DARWIN__
    screen = SDL_SetVideoMode(640, 480, 0, 0);
#else
    screen = SDL_SetVideoMode(640, 480, 24, 0);
#endif
    if(screen == NULL) {
        ERROR("SDL: could not set video mode - exiting");
        return -1;
    }

    SDL_Overlay *bmp = NULL;
    int width = 0, height = 0;
    SwsContext *img_convert_ctx = NULL;
    if (vstream != NULL) {
        AVCodecContext *codecCtx = vstream->codec;
        DEBUG ("codec:%p", codecCtx);
        width = codecCtx->width;
        height = codecCtx->height;
        bmp = SDL_CreateYUVOverlay(width, height, SDL_YV12_OVERLAY, screen);

        img_convert_ctx = sws_getContext (width, height, codecCtx->pix_fmt,
                                          width, height,PIX_FMT_YUV420P,
                                          SWS_BICUBIC, NULL, NULL, NULL);
        #ifdef TEST_RGB565
        mpImgCvtTmp = sws_getContext (width, height, codecCtx->pix_fmt,
                                      width, height,PIX_FMT_RGB565,
                                      SWS_BICUBIC, NULL, NULL, NULL);
        mpBufTmp = (uint8_t*) malloc (width * height * 2);
        #endif
    }

    if (astream != NULL) {
        AVCodecContext *codecCtx = astream->codec;
        SDL_AudioSpec wanted_spec, spec;
        // Set audio settings from codec info
        wanted_spec.freq = codecCtx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = codecCtx->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_spec.callback = SdlAbstract::AudioCallbackStub;
        wanted_spec.userdata = this;
    
        DEBUG ("sample_rate: %d, channels:%d", wanted_spec.freq, wanted_spec.channels);
        if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
            ERROR("SDL_OpenAudio: %s", SDL_GetError());
            return -1;
        }
        SDL_PauseAudio(0);
    }

    mpScreen = screen;
    mpBmp = bmp;
    miWidth = width;
    miHeight = height;
    mpImgConvertCtx = img_convert_ctx;
    return 0;
}

int SdlAbstract::ShowPicture (AVFrame *pFrame) {
    if (mpScreen == NULL || mpBmp == NULL || mpImgConvertCtx == NULL) {
        ERROR ("maybe init error priorly!");
        return -1;
    }

    if (pFrame == NULL) {
        ERROR ("pFrame == NULL!");
        return -1;
    }

    //int dst_pix_fmt = PIX_FMT_YUV420P;
    AVPicture pict;
    SDL_LockYUVOverlay (mpBmp);
    
    /* point pict at the queue */
    pict.data[0] = mpBmp->pixels[0];
    pict.data[1] = mpBmp->pixels[2];
    pict.data[2] = mpBmp->pixels[1];
    
    pict.linesize[0] = mpBmp->pitches[0];
    pict.linesize[1] = mpBmp->pitches[2];
    pict.linesize[2] = mpBmp->pitches[1];

    #ifdef TEST_RGB565    
    if (mpImgCvtTmp && mpBufTmp) {
        AVPicture pict2;
        pict2.data[0] = mpBufTmp;
        pict2.linesize[0] = miWidth;
        DEBUG ("begin scale RGB565");
        sws_scale (mpImgCvtTmp, pFrame->data, pFrame->linesize,
                   0, miHeight, pict2.data, pict2.linesize);
        DEBUG ("end scale RGB565");
    }
    #endif
    // Convert the image into YUV format that SDL uses
    sws_scale (mpImgConvertCtx, pFrame->data, pFrame->linesize,
               0, miHeight, pict.data, pict.linesize);
    //DEBUG ("pict(%d, %d, %d, %d)", pict.linesize[0],pict.linesize[1],pict.linesize[2],pict.linesize[3]);
    
    SDL_UnlockYUVOverlay (mpBmp);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = miWidth;
    rect.h = miHeight;
    SDL_DisplayYUVOverlay(mpBmp, &rect);

    return 0;
}

int SdlAbstract::PlaySound (unsigned char* buffer, int len) {
    VERBOSE ("buffer=%p, len=%d", buffer, len);
    mOperationLock.Lock ();
    SoundFrame frame;
    frame.len = len;
    frame.index = 0;
    memcpy (frame.data, buffer, len);
    mDeque.push_back (frame);
    mOperationLock.Unlock ();
    return 0;
}

void SdlAbstract::AudioCallbackStub (void *userdata, Uint8 *stream, int len) {
    if (userdata) {
        SdlAbstract* sdl = (SdlAbstract*)userdata;
        sdl->AudioCallback (stream, len);
    }
}

void SdlAbstract::AudioCallback (Uint8 *stream, int len) {
    DEBUG ("require stream=%p, len=%d", stream, len);
    mOperationLock.Lock ();
    int pushed_len = 0;
    int total_len = len;

    while (pushed_len < total_len) {
        if (mCurrentFrame.len > 0) {
            if (mCurrentFrame.index >= mCurrentFrame.len - 1) {
                //DEBUG("a frame. get new frame!");
                if (mDeque.size() > 0) {
                    mCurrentFrame = mDeque.front ();
                    mDeque.pop_front ();
                } else {
                    //DEBUG ("no new frame. we need return");
                    break;
                }
            }
        } else {
            if (mDeque.size() > 0) {
                mCurrentFrame = mDeque.front ();
                mDeque.pop_front ();
            } else {
                break;
            }
        }

        // get a valid frame in mCurrentFrame.
        int need_len = total_len - pushed_len;
        int current_frame_len = mCurrentFrame.len - mCurrentFrame.index;
        if (current_frame_len >= need_len) {
            DEBUG ("sendlen=%d, frame info: data=%p, index=%d, len=%d", need_len, mCurrentFrame.data, mCurrentFrame.index, mCurrentFrame.len);
            memcpy (stream, mCurrentFrame.data + mCurrentFrame.index, need_len);
            stream += need_len;
            mCurrentFrame.index += need_len;
            pushed_len += need_len;
            // get enough data, we break;
            break;
        } else {
            DEBUG ("sendlen=%d, data=%p, len=%d, index=%d, need=%d, stream=%p, len=%d", current_frame_len, mCurrentFrame.data, mCurrentFrame.len, mCurrentFrame.index, current_frame_len, stream, need_len);
            memcpy (stream, mCurrentFrame.data + mCurrentFrame.index, current_frame_len);
            stream += current_frame_len;
            mCurrentFrame.index += current_frame_len;
            pushed_len += current_frame_len;
        }

    }
    mOperationLock.Unlock ();
}

