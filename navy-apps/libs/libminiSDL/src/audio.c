#include <NDL.h>
#include <SDL.h>
#include "assert.h"

static void *buf = NULL;
static long len = 0;

typedef void (*sb_callback)(void *userdata, Uint8 *stream, int len);

static void *userdata = NULL;
static sb_callback callback = NULL;

static bool is_playing = false;

static int time_lag = 0;
static int size = 0;

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
    // assert(0);
    // assert(obtained == NULL);
    assert(desired->callback != NULL);
    assert(desired->format == AUDIO_S16SYS);
    assert((desired->channels == 1) || (desired->channels == 2));
    assert((desired->samples % 2) == 0);
    userdata = desired->userdata;
    callback = desired->callback;
    NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
    size = desired->channels * desired->samples * desired->format / 8;
    time_lag = desired->samples / desired->freq * 1000;
    printf("the freq is %d, the channels is %d, samples is %d, size is %d\n", desired->freq, desired->channels, desired->samples,size);
    return 0;
}

void SDL_CloseAudio() {
    // assert(0);
    NDL_CloseAudio();
}

void SDL_PauseAudio(int pause_on) {
    // assert(0);
    if(pause_on==0){
        is_playing = true;
    }
    else{
        is_playing = false;
    }
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
    assert(0);
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
    // assert(0);
    return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
    // assert(0);
}

void SDL_LockAudio() {
    // assert(0);
}

void SDL_UnlockAudio() {
    // assert(0);
}

void SDL_audio_help_callback(){
    static uint32_t prev = 0;
    if (prev == 0){
        prev = SDL_GetTicks();
    }
    uint32_t now = SDL_GetTicks();
    if (is_playing == false){
        return;
    }
    if((now - prev) < time_lag){
        return;
    }
    int count = NDL_QueryAudio();
    if (size > count){
        // printf("Hello,%d\n",count);
        return;
    }
    prev = now;
    uint8_t *sbuf = (uint8_t *)malloc(size);
    callback(userdata, sbuf, size);
    NDL_PlayAudio(sbuf, size);
    free(sbuf);
}
