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
#include <utils.h>
#include <device/alarm.h>
#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>
#endif

void init_map();
void init_serial();
void init_timer();
void init_vga();
void init_i8042();
void init_audio();
void init_disk();
void init_sdcard();
void init_alarm();

void send_key_sbi_serial(uint8_t, bool);
void init_sbi_serial();
void init_sbi_clint();
void init_sbi_plic();
void init_sbi_disk();
void update_sbi_time(uint64_t us);

void send_key(uint8_t, bool);
void vga_update_screen();

static inline void send_key_to_device(uint8_t scancode, bool is_keydown){
    // printf("now input is %d\n", scancode);
#ifdef CONFIG_HAS_KEYBOARD
    send_key(scancode, is_keydown);
#endif
#ifdef CONFIG_HAS_SBI_SERIAL
    send_key_sbi_serial(scancode, is_keydown);
#endif
}

void device_update() {
  static uint64_t last = 0;
  uint64_t now = get_time();
  IFDEF(CONFIG_HAS_SBI_CLINT, update_sbi_time(now));
  if (now - last < 1000000 / TIMER_HZ) {
    return;
  }
  last = now;

  IFDEF(CONFIG_HAS_VGA, vga_update_screen());

#ifndef CONFIG_TARGET_AM
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        nemu_state.state = NEMU_QUIT;
        break;
#if defined(CONFIG_HAS_KEYBOARD) || defined(CONFIG_HAS_SBI_SERIAL)
      // If a key was pressed
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        uint8_t k = event.key.keysym.scancode;
        bool is_keydown = (event.key.type == SDL_KEYDOWN);
        send_key_to_device(k, is_keydown);
        break;
      }
#endif
      default: break;
    }
  }
#endif
}

void sdl_clear_event_queue() {
#ifndef CONFIG_TARGET_AM
  SDL_Event event;
  while (SDL_PollEvent(&event));
#endif
}

void init_device() {
  IFDEF(CONFIG_TARGET_AM, ioe_init());
  init_map();

  IFDEF(CONFIG_HAS_SERIAL, init_serial());
  IFDEF(CONFIG_HAS_TIMER, init_timer());
  IFDEF(CONFIG_HAS_VGA, init_vga());
  IFDEF(CONFIG_HAS_KEYBOARD, init_i8042());
  IFDEF(CONFIG_HAS_AUDIO, init_audio());
  IFDEF(CONFIG_HAS_DISK, init_disk());
  IFDEF(CONFIG_HAS_SDCARD, init_sdcard());

  // myself
  IFDEF(CONFIG_HAS_SBI_SERIAL, init_sbi_serial());
  IFDEF(CONFIG_HAS_SBI_CLINT, init_sbi_clint());
  IFDEF(CONFIG_HAS_SBI_PLIC, init_sbi_plic());
  IFDEF(CONFIG_HAS_SBI_DISK, init_sbi_disk());
  // myself

  IFNDEF(CONFIG_TARGET_AM, init_alarm());
}
