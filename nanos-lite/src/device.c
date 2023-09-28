#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    const char *char_buf = (const char *)buf;
    for (int i = 0; i < len; i++){
        putch(*char_buf);
        char_buf++;
    }
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
    AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
    if(ev.keycode==0)return 0;
    char char_buf[40];
    int write_len = sprintf(char_buf, "%s %s\n", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
    // printf("%d\n", write_len);
    len = ((write_len + 1) < len) ? write_len + 1 : len;
    printf("%d\n", len);
    memcpy(buf, char_buf, len);
    return len;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T gpu_cfg = io_read(AM_GPU_CONFIG);
    char char_buf[40];
    int write_len = sprintf(char_buf, "WIDTH:%d\n""HEIGHT:%d\n", gpu_cfg.width,gpu_cfg.height);
    // Log("the size of width is %d, the size of height is %d", gpu_cfg.width, gpu_cfg.height);
    // Log("%s", char_buf);
    len = ((write_len + 1) < len) ? write_len + 1 : len;
    memcpy(buf, char_buf, len);
    return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
    AM_GPU_CONFIG_T gpu_cfg = io_read(AM_GPU_CONFIG);
    // printf("%d\n", offset);
    offset /= 4;
    // printf("%d\n", offset);
    int x = (offset % gpu_cfg.width);
    int y = (offset / gpu_cfg.width);
    io_write(AM_GPU_FBDRAW, x, y, buf, (len) / 4, 1, true);
    return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
