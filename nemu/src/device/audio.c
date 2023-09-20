/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

SDL_AudioSpec s = {};

static uint8_t *sb_start = NULL;
static uint8_t *sb_end = NULL;
static uint32_t sb_count = 0;

void audio_play(void *userdata, Uint8 *stream, int len){
    memset(stream, 0, len);
    int my_fifo_len = audio_base[5];
    if (my_fifo_len==0){
        return;
    }
    len = (len < my_fifo_len) ? len : my_fifo_len;
    uint8_t *new_sb_buf = malloc(len);
    if((sb_start+len-sbuf)>=CONFIG_SB_SIZE){
        int temp_len = CONFIG_SB_SIZE - (uint32_t)(new_sb_buf - sbuf);
        memcpy(new_sb_buf, sb_start, temp_len);
        memcpy(new_sb_buf + temp_len, sbuf, len - temp_len);
    }
    SDL_MixAudio(stream, new_sb_buf, len, SDL_MIX_MAXVOLUME);
    sb_start += len;
    if((sb_start-sbuf)>CONFIG_SB_SIZE)
        sb_start -= CONFIG_SB_SIZE;
    // sb_start = sbuf + (((sb_start - sbuf) + len) % CONFIG_SB_SIZE);
    audio_base[5] = (sb_end >= sb_start) ? (sb_end - sb_start ) : (CONFIG_SB_SIZE + sb_end - sb_start );
    free(new_sb_buf);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
    if((offset==0x10)&&(is_write)&&(audio_base[4]==1)){
        s.freq = audio_base[0];
        s.channels = audio_base[1];
        s.samples = audio_base[2];
        SDL_OpenAudio(&s, NULL);
        SDL_PauseAudio(0);
    }
}

static void call_of_sbuf(uint32_t offset, int len, bool is_write){
    sb_end = (sbuf + ((offset + len) % CONFIG_SB_SIZE));
    audio_base[5] = (sb_end >= sb_start) ? (sb_end - sb_start ) : (CONFIG_SB_SIZE + sb_end - sb_start );
}

void init_audio() {
    uint32_t space_size = sizeof(uint32_t) * nr_reg;
    audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
    add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

    sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
    add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, call_of_sbuf);

    audio_base[3] = CONFIG_SB_SIZE;
    audio_base[4] = 0;
    audio_base[5] = 0;

    s.format = AUDIO_S16SYS;
    s.userdata = NULL;
    s.silence = 0;
    s.callback = audio_play;
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    sb_start = sbuf;
    sb_end = sbuf;
    sb_count = CONFIG_SB_SIZE;
}
