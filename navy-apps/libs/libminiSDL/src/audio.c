#include <NDL.h>
#include <SDL.h>
#include "assert.h"
#include "string.h"
#include "stdint.h"

static void *buf = NULL;
static long len = 0;

typedef void (*sb_callback)(void *userdata, Uint8 *stream, int len);

static void *userdata = NULL;
static sb_callback callback = NULL;

static bool is_playing = false;

static int time_lag = 0;
static int size = 0;

#define __attribute__unpacked__ __attribute__((packed))

typedef struct RIFFChunk
{
    char RIFF[4];
    uint32_t CK_size;
    char wav_ID[4];
} __attribute__unpacked__ RIFFChunk;

typedef struct StandardPCMFmtChunk
{
    char ckID[4];
    uint32_t ckSize;
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
} __attribute__unpacked__ StandardPCMFmtChunk;

typedef struct DATAChunk
{
    char ckID[4];
    uint32_t CK_size;
} __attribute__unpacked__ DATAChunk;

typedef struct WAV
{
    RIFFChunk Riffchunk;
    StandardPCMFmtChunk Fmtchunt;
    DATAChunk Datachunk;
} __attribute__unpacked__ WAV;

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
    // assert(0);
    int shift = SDL_MIX_MAXVOLUME / volume;
    int16_t *dest = (int16_t *)dst;
    int16_t *source = (int16_t *)src;
    len /= 2;
    for (int i = 0; i < len;i++){
        int16_t res = (*dest) + ((*source) / shift);
        if(((*dest)>0)&&((*source)>0)&&(res<0)){
            (*dest) = 32767;
        }
        else if (((*dest) < 0) && ((*source) < 0) && (res > 0)){
            (*dest) = -32768;
        }
        else{
            (*dest) = res;
        }
        dest++;
        source++;
    }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
    // assert(0);
    FILE *wav_fp = fopen(file, "rb");
    assert(wav_fp);
    WAV wav;
    int ret = fread(&wav, sizeof(WAV), 1, wav_fp);
    assert(ret == 1);
    assert(memcmp(wav.Riffchunk.RIFF, "RIFF", 4) == 0);
    assert(memcmp(wav.Riffchunk.wav_ID, "WAVE", 4) == 0);
    assert(memcmp(wav.Fmtchunt.ckID, "fmt ", 4) == 0);
    assert(memcmp(wav.Datachunk.ckID, "data", 4) == 0);
    assert((wav.Fmtchunt.nChannels == 1) || (wav.Fmtchunt.nChannels == 2));
    assert(wav.Fmtchunt.wFormatTag == 1);
    assert(wav.Fmtchunt.wBitsPerSample == 16);
    assert(memcmp(wav.Datachunk.ckID, "data", 4) == 0);
    printf("freq is %d, channels is %d\n", wav.Fmtchunt.nAvgBytesPerSec, wav.Fmtchunt.nChannels);
    assert(spec != NULL);
    // assert(0);
    spec->freq = wav.Fmtchunt.nAvgBytesPerSec;
    spec->channels = wav.Fmtchunt.nChannels;
    spec->samples = 1024;
    spec->format = AUDIO_S16SYS;
    (*audio_len) = wav.Datachunk.CK_size;
    (*audio_buf) = malloc(wav.Datachunk.CK_size);
    return spec;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
    // assert(0);
    free(audio_buf);
}

void SDL_LockAudio() {
    // assert(0);
}

void SDL_UnlockAudio() {
    // assert(0);
}

void SDL_audio_help_callback(){
    static bool is_recall = false;
    if(is_recall==true){
        return;
    }
    else{
        is_recall = true;
    }
    static uint32_t prev = 0;
    if (prev == 0){
        prev = SDL_GetTicks();
    }
    uint32_t now = SDL_GetTicks();
    if (is_playing == false){
        printf("sdl-audio return by not open\n");
        return;
    }
    if((now - prev) < time_lag){
        printf("sdl-audio return by time\n");
        return;
    }
    int count = NDL_QueryAudio();
    if (size > count){
        printf("sdl-audio return by not enough space\n");
        // printf("Hello,%d\n",count);
        return;
    }
    prev = now;
    uint8_t *sbuf = (uint8_t *)malloc(size);
    callback(userdata, sbuf, size);
    NDL_PlayAudio(sbuf, size);
    free(sbuf);
    is_recall = false;
}
