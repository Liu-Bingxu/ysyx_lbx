#include <am.h>
#include <nemu.h>
#include<klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

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
    static int last_len;
    static int first_flag=0;
    static uint32_t sb_size;
    if (first_flag == 0){
        first_flag = 1;
        last_len = inl(AUDIO_SBUF_SIZE_ADDR);
        sb_size = inl(AUDIO_SBUF_SIZE_ADDR);
    }
    volatile uint32_t count = inl(AUDIO_COUNT_ADDR);
    int len = ctl->buf.end - ctl->buf.start;
    void *start = ctl->buf.start;
    while (count < len){
        count = inl(AUDIO_COUNT_ADDR);
    }
    if(last_len<len){
        memcpy((void *)AUDIO_SBUF_ADDR+sb_size-last_len, start, last_len);
        last_len = sb_size;
        start += last_len;
        len -= last_len;
    }
    memcpy((void *)AUDIO_SBUF_ADDR+sb_size-last_len, start, len);
    last_len = (last_len == len) ? sb_size : last_len - len;
}
