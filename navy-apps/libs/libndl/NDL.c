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
        int width, height;
        fscanf(dispinfo, "WIDTH:%d", &width);
        fscanf(dispinfo, "HEIGHT:%d", &height);
        fclose(dispinfo);
        // assert((*w) <= width);
        // assert((*h) <= height);
        screen_w = (*w)?(*w):width;
        screen_h = (*h)?(*h):height;
        start_x = (width - screen_w) / 2;
        start_y = (height - screen_h) / 2;
        printf("the size of width  is %d\n", width );
        printf("the size of height is %d\n", height);
    }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
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
