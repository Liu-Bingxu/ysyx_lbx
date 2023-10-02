#include <am.h>
#include "assert.h"
#include "sys/time.h"
#include "stdlib.h"
#include "string.h"

#define keyname(k) #k,

static const char *keyname[] = {
    "NONE",
    AM_KEYS(keyname)
};

void __am_timer_init();
void __am_gpu_init();
void __am_audio_init();
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);
void __am_disk_config(AM_DISK_CONFIG_T *cfg);
void __am_disk_status(AM_DISK_STATUS_T *stat);
void __am_disk_blkio(AM_DISK_BLKIO_T *io);

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __am_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }
static void __am_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __am_timer_config,
  [AM_TIMER_RTC   ] = __am_timer_rtc,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,
  [AM_GPU_STATUS  ] = __am_gpu_status,
  [AM_UART_CONFIG ] = __am_uart_config,
  [AM_AUDIO_CONFIG] = __am_audio_config,
  [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
  [AM_AUDIO_STATUS] = __am_audio_status,
  [AM_AUDIO_PLAY  ] = __am_audio_play,
  [AM_DISK_CONFIG ] = __am_disk_config,
  [AM_DISK_STATUS ] = __am_disk_status,
  [AM_DISK_BLKIO  ] = __am_disk_blkio,
  [AM_NET_CONFIG  ] = __am_net_config,
};

bool ioe_init() {
  return true;
}

void ioe_read (int reg, void *buf) {
    ((handler_t)lut[reg])(buf);
}

void ioe_write(int reg, void *buf) {
    ((handler_t)lut[reg])(buf);
}

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd){
    char buf[20];
    int ret = NDL_PollEvent(buf, 20);
    if (ret == 0){
        kbd->keydown = 0;
        kbd->keycode = AM_KEY_NONE;
        return;
    }
    if(buf[1]=='u'){
        kbd->keydown = 0;
    }
    else if(buf[1]=='d'){
        kbd->keydown = 0;
    }
    else{
        assert(0);
    }
    for (int i = 0; i < 20;i++){
        if(buf[i]=='\n'){
            buf[i] = '\0';
            break;
        }
    }
    for (int i = 1; i < (sizeof(keyname) / sizeof(keyname[0]) - 1); i++){
        if (strcmp(buf + 3, keyname[i]) == 0){
            kbd->keycode = i;
            return;
        }
    }
    assert(0);
}

void __am_timer_init(){}
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime){
    struct timeval tv;
    int succes = gettimeofday(&tv, NULL);
    assert(succes == 0);
    uptime->us = tv.tv_usec + (tv.tv_sec * 1000000);
}
void __am_timer_rtc(AM_TIMER_RTC_T *rtc){}

static int record_width = 0, record_height = 0;
void __am_gpu_init(){

    int i;
    int w = (record_width == 0) ? (io_read(AM_GPU_CONFIG).width) : record_width;
    int h = (record_height == 0) ? (io_read(AM_GPU_CONFIG).height) : record_height;
    uint32_t *pixels = (uint32_t *)malloc(sizeof(uint32_t) * w * h);
    for (i = 0; i < w * h; i++)
        pixels[i] = i;
    int fb_fp = open("/dev/fb", 0, 0);
    write(fb_fp, pixels, w * h * 4);
    close(fb_fp);
}
void __am_gpu_config(AM_GPU_CONFIG_T *cfg){
    cfg->present = true;
    cfg->has_accel = false;
    int width, height;
    FILE *dispinfo = fopen("/proc/dispinfo", "r");
    assert(dispinfo);
    char char_buf[64];
    fread(char_buf, sizeof(char_buf), 1, dispinfo);
    fclose(dispinfo);
    sscanf(char_buf, "WIDTH:%d\nHEIGHT:%d", &width, &height);
    cfg->width = width;
    cfg->height = height;
    cfg->vmemsz = cfg->width * cfg->height;
    if(record_width==0){
        record_width = width;
        record_height = height;
    }
}
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl){
    assert((ctl->x + ctl->w) <= (record_width==0)?(io_read(AM_GPU_CONFIG).width):record_width);
    assert((ctl->y + ctl->h) <= (record_height==0)?(io_read(AM_GPU_CONFIG).height):record_height);
    const uint32_t *pixels = (uint32_t *)ctl->pixels;
    int fb_fp = open("/dev/fb", 0, 0);
    for (int i = 0; i < ctl->h; i++){
        lseek(fb_fp, (((ctl->y + i) * ctl->w * 4) + (ctl->x * 4)), SEEK_SET);
        write(fb_fp, pixels, ctl->w * 4);
        pixels += ctl->w;
    }
    close(fb_fp);
}
void __am_gpu_status(AM_GPU_STATUS_T *status){
    status->ready = true;
}

void __am_audio_init(){}
void __am_audio_config(AM_AUDIO_CONFIG_T *){}
void __am_audio_ctrl(AM_AUDIO_CTRL_T *){}
void __am_audio_status(AM_AUDIO_STATUS_T *){}
void __am_audio_play(AM_AUDIO_PLAY_T *){}
void __am_disk_config(AM_DISK_CONFIG_T *cfg){}
void __am_disk_status(AM_DISK_STATUS_T *stat){}
void __am_disk_blkio(AM_DISK_BLKIO_T *io){}
