#include <am.h>
#include <nemu.h>
#include<klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

char *play_buf = (char *)AUDIO_SBUF_ADDR;

static uint32_t playcpy(uint32_t dst, char *src, uint32_t n){
    dst %= 0x10000;
    for (int i = 0; i < n; i++){
        play_buf[dst] = (*src);
        src++;
        dst++;
        dst %= 0x10000;
    }
    return dst;
}

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
    outl(AUDIO_FREQ_ADDR, ctrl->freq);
    outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
    outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
    outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
    stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
    static int last_len=0;
    // static int first_flag=0;
    // static uint32_t sb_size;
    // if (first_flag == 0){
    //     first_flag = 1;
    //     last_len = inl(AUDIO_SBUF_SIZE_ADDR);
    //     sb_size = inl(AUDIO_SBUF_SIZE_ADDR);
    // }
    volatile uint32_t count = inl(AUDIO_COUNT_ADDR);
    printf("the start addr is 0x%x, end addr is 0x%x\n", ctl->buf.start, ctl->buf.end);
    long len;
    void *start;
    if (ctl->buf.end < ctl->buf.start){
        start = ctl->buf.end;
        len = ctl->buf.start - ctl->buf.end;
    }
    else{
        len = ctl->buf.end - ctl->buf.start;
        start = ctl->buf.start;
    }
    while (count < len){
        uint32_t text = count;
        count = inl(AUDIO_COUNT_ADDR);
        if(text!=count)printf("I am %lu, target is %d\n", len, count);   
    }
    // if (last_len < len){
    //     playcpy((void *)AUDIO_SBUF_ADDR+sb_size-last_len, start, last_len);
    //     last_len = sb_size;
    //     start += last_len;
    //     len -= last_len;
    //     printf("Hello\n");
    // }
    // printf("Now last_len is %d\n", last_len);
    // playcpy((void *)AUDIO_SBUF_ADDR+sb_size-last_len, start, len);
    printf("Hello\n");
    // last_len = (last_len == len) ? sb_size : last_len - len;
    last_len = playcpy(last_len, start, len);
}
