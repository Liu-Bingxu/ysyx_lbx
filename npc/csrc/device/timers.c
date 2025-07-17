#include "device.h"
#include "difftest.h"
#include "utils.h"
#include "time.h"

static uint32_t timer_reg[2];

void get_rtc(){
    // difftest_skip_ref();
    static time_t now_time;
    static struct tm *now_time_rtc;
    now_time = time(NULL);
    now_time_rtc = localtime(&now_time);
    (*((uint64_t *)timer_reg)) = 0;
    //sec
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_sec<<0);
    // printf("sec: %lu\n", (*((uint64_t *)timer_reg)));
    // min
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_min<<6);
    // printf("min: %lu\n", (*((uint64_t *)timer_reg)));
    //hour
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_hour<<12);
    // printf("hour: %lu\n", (*((uint64_t *)timer_reg)));
    //day
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_mday<<17);
    // printf("day: %lu\n", (*((uint64_t *)timer_reg)));
    //mon
    now_time_rtc->tm_mon += 1;
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_mon << 22);
    // printf("mon: %lu\n", (*((uint64_t *)timer_reg)));
    //year
    now_time_rtc->tm_year += 1900;
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_year << 26);
    // printf("year: %lu\n", (*((uint64_t *)timer_reg)));
    // printf("%d-%d-%d %02d:%02d:%02d GMT (\n", now_time_rtc->tm_year,now_time_rtc->tm_mon, now_time_rtc->tm_mday, now_time_rtc->tm_hour, now_time_rtc->tm_min,now_time_rtc->tm_sec);
    // uint64_t now=(*((uint64_t *)timer_reg));
    // int second, minute, hour, day, month, year;
    // second = (now & 0x3f);
    // now >>= 6;
    // minute = (now & 0x3f);
    // now >>= 6;
    // hour = (now & 0x1f);
    // now >>= 5;
    // day = (now & 0x1f);
    // now >>= 5;
    // month = (now & 0xf);
    // now >>= 4;
    // year = (now&0xfff);
    // printf("%d-%d-%d %02d:%02d:%02d GMT (\n", year,month,day,hour,minute,second);
}

void get_uptime(){
    // difftest_skip_ref();
    static uint64_t open_time=0;
    if(open_time==0)open_time=get_time();
    uint64_t now = get_time();
    (*((uint64_t *)timer_reg)) = (now-open_time);
}

uint32_t get_timer_reg(int offest){
    // difftest_skip_ref();
    if(offest==0)return timer_reg[0];
    else if(offest==1) return timer_reg[1];
    else assert(0);
}
