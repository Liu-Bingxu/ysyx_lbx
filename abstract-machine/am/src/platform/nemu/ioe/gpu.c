#include <am.h>
#include <nemu.h>
#include "klib.h"

#define SYNC_ADDR (VGACTL_ADDR + 4)

static int width = 0;
static int height = 0;
static int vmemsz = 0;

void __am_gpu_init() {
    int i;
    int w = io_read(AM_GPU_CONFIG).width;
    int h = io_read(AM_GPU_CONFIG).height;
    printf("width: %d height: %d\n", w, h);
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    for (i = 0; i < w * h;i++)
        fb[i] = i;
    outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->has_accel = false;
    if(width==0){
        cfg->width = (inl(VGACTL_ADDR) >> 16);
        cfg->height = (inl(VGACTL_ADDR) & 0xffff);
        cfg->vmemsz = cfg->width * cfg->height;
        width = cfg->width;
        height = cfg->height;
        vmemsz = cfg->vmemsz;
    }
    else{
        cfg->width = width;
        cfg->height = height;
        cfg->vmemsz = vmemsz;
    }
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    // assert((ctl->x + ctl->w) < io_read(AM_GPU_CONFIG).width);
    // assert((ctl->y + ctl->h) < io_read(AM_GPU_CONFIG).height);
    if(ctl->pixels!=NULL){
        for (int i = 0; i < ctl->h; i++){
            memcpy(((void *)FB_ADDR + (ctl->y + i) * io_read(AM_GPU_CONFIG).width * 4 + ctl->x ), ctl->pixels, ctl->w * 4);
            ctl->pixels += ctl->w * 4;
        }
    }
    if (ctl->sync){
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
