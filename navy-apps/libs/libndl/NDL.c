#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "assert.h"
#include "sys/time.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int start_x = 0, start_y = 0;
static int width = 0, height = 0;

uint32_t NDL_GetTicks() {
    struct timeval tv;
    int succes = gettimeofday(&tv, NULL);
    assert(succes == 0);
    uint32_t msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return msec;
}

int NDL_PollEvent(char *buf, int len) {
    // FILE *event_fp = fopen("/dev/events", "r");
    // int read_len = fread(buf, len, 1, event_fp);
    // fclose(event_fp);
    int event_fp = open("/dev/events", 0, 0);
    int read_len = read(event_fp, buf, len);
    close(event_fp);
    // printf("%d\n", read_len);
    if (read_len == 0)
        return 0;
    else
        return 1;
}

void NDL_OpenCanvas(int *w, int *h) {
    if (getenv("NWM_APP")) {
        int fbctl = 4;
        fbdev = 5;
        screen_w = *w;
        screen_h = *h;
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        // let NWM resize the window and create the frame buffer
        write(fbctl, buf, len);
        while (1){
            // 3 = evtdev
            int nread = read(3, buf, sizeof(buf) - 1);
            if (nread <= 0)
                continue;
            buf[nread] = '\0';
            if (strcmp(buf, "mmap ok") == 0)
                break;
        }
        close(fbctl);
    }
    else{
        FILE *dispinfo = fopen("/proc/dispinfo", "r");
        assert(dispinfo);
        char char_buf[64];
        fread(char_buf, sizeof(char_buf), 1, dispinfo);
        fclose(dispinfo);
        sscanf(char_buf, "WIDTH:%d\nHEIGHT:%d", &width, &height);
        // printf("%s\n", char_buf);
        assert((*w) <= width);
        assert((*h) <= height);
        if(*w==0){
            (*w) = width;
        }
        if(*h==0){
            (*h) = height;
        }
        screen_w = (*w);
        screen_h = (*h);
        start_x = (width - screen_w) / 2;
        start_y = (height - screen_h) / 2;
        printf("the size of width  is %d\n", width );
        printf("the size of height is %d\n", height);
        printf("the size of input width  is %d\n", *w);
        printf("the size of input height is %d\n", *h);
        printf("the size of start width  is %d\n", start_x);
        printf("the size of start height is %d\n", start_y);
    }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
    int fb_fp = open("/dev/fb", 0, 0);
    // printf("befoe x is %d, y is %d\n", x, y);
    x += start_x;
    y += start_y;
    if((w==0)&&(h==0)){
        w = width;
        h = height;
    }
    // printf("after x is %d, y is %d\n", x, y);
    for (int i = 0; i < h;i++){
        lseek(fb_fp, (((y + i) * width * 4) + (x * 4)), SEEK_SET);
        write(fb_fp, pixels, w * 4);
        pixels += w;
    }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
