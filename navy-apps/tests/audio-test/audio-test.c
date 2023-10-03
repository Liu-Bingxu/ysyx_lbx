#include<stdio.h>
#include<stdlib.h>
#include "sys/time.h"
#include "stdbool.h"
#include "assert.h"
#include "stdint.h"
#include "NDL.h"

#define MUSIC_PATH "/share/music/red.pcm"

int main(int argc , char* argv[]){

    NDL_OpenAudio(44100, 2, 4096);
    FILE *fp = fopen(MUSIC_PATH, "r");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    void *buf = malloc(size);
    assert(buf);
    fseek(fp, 0, SEEK_SET);
    int ret = fread(buf, size, 1, fp);
    assert(ret == 1);
    fclose(fp);

    size_t nplay = 0;
    size_t audio_len = size;

    while(1){
        int len = (audio_len - nplay > 4096 ? 4096 : audio_len - nplay);
        NDL_PlayAudio(buf + nplay, len);
        nplay += len;
        printf("paly %d, Already play %d/%d bytes of data\n", len,nplay, audio_len);
        if (nplay >= audio_len){
            nplay = 0;
        }
    }

    exit(0);
}