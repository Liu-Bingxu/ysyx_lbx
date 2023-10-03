#include<stdio.h>
#include<stdlib.h>
#include "sys/time.h"
#include "stdbool.h"
#include "assert.h"
#include "stdint.h"
#include "SDL.h"
#include "string.h"
#include "stdint.h"

#define MUSIC_PATH "/share/music/red.pcm"

static size_t red_size = 0;
void *buf = NULL;

void sb_callback(void *userdata, Uint8 *stream, int len){
    static size_t nplay = 0;
    memcpy(stream, 0, len);
    printf("%s %d,Hello\n", __func__, __LINE__);
    len = (red_size - nplay > len ? len : red_size - nplay);
    memcpy(stream, buf + nplay, len);
    nplay += len;
    printf("paly %d, Already play %d/%d bytes of data\n", len, nplay, red_size);
    if(nplay>=red_size){
        nplay = 0;
    }
}

int main(int argc , char* argv[]){

    // NDL_OpenAudio(44100, 2, 4096);
    SDL_Init(SDL_INIT_TIMER);
    FILE *fp = fopen(MUSIC_PATH, "r");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    red_size = size;
    buf = malloc(size);
    assert(buf);
    fseek(fp, 0, SEEK_SET);
    int ret = fread(buf, size, 1, fp);
    assert(ret == 1);
    fclose(fp);

    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.channels = 2;
    spec.samples = 1024;
    spec.format = AUDIO_S16SYS;
    spec.userdata = NULL;
    spec.callback = sb_callback;
    SDL_OpenAudio(&spec, NULL);

    SDL_PauseAudio(0);

    while(1){
        SDL_Event ev;
        SDL_PollEvent(&ev);
    }

    exit(0);
}