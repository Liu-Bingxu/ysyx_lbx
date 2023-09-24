#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "common.h"

#define DEVICE_ADDR     0xa0000000

#define SERIAL_ADDR     (DEVICE_ADDR+0x000)
#define TIMER_ADDR      (DEVICE_ADDR+0x100)

void serial_out(word_t ch);
void get_rtc();
void get_uptime();
uint32_t get_timer_reg(int offest);

#endif