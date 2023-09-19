#include <am.h>
#include <nemu.h>
#include<klib.h>

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    outl(RTC_ADDR, 0xffffffff);
    uptime->us = inl(RTC_ADDR);
    uptime->us += ((uint64_t)inl(RTC_ADDR + 4) << 32);
}

int months_common[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int months_leap[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
    outl(RTC_ADDR, 0x0);
    uint64_t us = 0;
    us = inl(RTC_ADDR);
    us += ((uint64_t)inl(RTC_ADDR + 4) << 32);
    printf("%ld\n", us);
    us /= 1000000;
    rtc->second = us % 60;
    us /= 60;
    rtc->minute = us % 60;
    us /= 60;
    rtc->hour = us % 24;
    us /= 24;
    rtc->year = 1970;
    int year_day = 365;
    while(us>=year_day){
        rtc->year++;
        if((((rtc->year%4)==0)&&((rtc->year%100)!=0))||(rtc->year%400==0)){
            year_day = 366;
        }
        else{
            year_day = 365;
        }
        us -= year_day;
    }
    rtc->month = 0;
    if (year_day == 365){
        int month = 0;
        while (us>=months_common[month]){
            us -= months_common[month];
            rtc->month++;
            month++;
        }
    }
    else{
        int month = 0;
        while (us >= months_common[month])
        {
            us -= months_common[month];
            rtc->month++;
            month++;
        }
    }
    rtc->day = (us == 0) ? 1 : us;
}
