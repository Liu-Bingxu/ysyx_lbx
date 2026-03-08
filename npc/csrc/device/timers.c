#include "device.h"
#include "difftest.h"
#include "utils.h"
#include "time.h"

static uint32_t timer_reg[2];

void get_rtc(){
    static time_t now_time;
    static struct tm *now_time_rtc;
    now_time = time(NULL);
    now_time_rtc = localtime(&now_time);
    (*((uint64_t *)timer_reg)) = 0;
    //sec
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_sec<<0);
    // min
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_min<<6);
    //hour
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_hour<<12);
    //day
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_mday<<17);
    //mon
    now_time_rtc->tm_mon += 1;
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_mon << 22);
    //year
    now_time_rtc->tm_year += 1900;
    (*((uint64_t *)timer_reg)) |= (now_time_rtc->tm_year << 26);
}

void get_uptime(){
    static uint64_t open_time=0;
    if(open_time==0)open_time=get_time();
    uint64_t now = get_time();
    (*((uint64_t *)timer_reg)) = (now-open_time);
}

uint32_t get_timer_reg(int offest){
    if(offest==0)return timer_reg[0];
    else if(offest==1) return timer_reg[1];
    else assert(0);
}
