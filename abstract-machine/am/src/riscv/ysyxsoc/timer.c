#include <am.h>
#include "ysyxsoc.h"

#include <riscv/riscv.h> // the macro `ISA_H` is defined in CFLAGS

void __am_timer_init() {
    // outl(TIMER_ADDR, 0);
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    // outl(TIMER_ADDR, 0);
    // uptime->us = 0;
    // uptime->us = inl(TIMER_ADDR);
    // uptime->us |= ((uint64_t)inl(TIMER_ADDR + 4) << 32);
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
    // outl(TIMER_ADDR, 1);
    // uint64_t time = 0;
    // time = inl(TIMER_ADDR);
    // time |= ((uint64_t)inl(TIMER_ADDR + 4) << 32);
    // rtc->second = (time&0x3f);
    // time >>= 6;
    // rtc->minute = (time&0x3f);
    // time >>= 6;
    // rtc->hour = (time&0x1f);
    // time >>= 5;
    // rtc->day = (time&0x1f);
    // time >>= 5;
    // rtc->month = (time&0xf);
    // time >>= 4;
    // rtc->year = (time&0x7ff);
}
