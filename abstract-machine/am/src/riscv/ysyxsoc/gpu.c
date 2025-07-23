#include <am.h>
#include "klib.h"
#include "ysyxsoc.h"
#include <riscv/riscv.h>

void __am_gpu_init() {
    int i;
    int w = 640;
    int h = 480;
    printf("width: %d height: %d\n", w, h);
    uint32_t *fb = (uint32_t *)(uintptr_t)VMEM_BASE;
    for (i = 0; i < w * h;i++)
        fb[i] = i;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->has_accel = false;
    cfg->width = 640;
    cfg->height = 480;
    cfg->vmemsz = cfg->width * cfg->height;
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    assert((ctl->x + ctl->w) <= 640);
    assert((ctl->y + ctl->h) <= 480);
    const void *pixels = ctl->pixels;
    if (pixels != NULL){
        for (int i = 0; i < ctl->h; i++){
            memcpy(((void *)VMEM_BASE + (ctl->y + i) * 640 * 4 + ctl->x * 4), pixels, ctl->w * 4);
            pixels += ctl->w * 4;
        }
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
