#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>
#include "assert.h"

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
    assert(0);
    return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
    assert(0);
    return 1;
}

uint32_t SDL_GetTicks() {
    // assert(0);
    static uint32_t boot = 0;
    if(boot==0){
        boot=NDL_GetTicks();
        printf("Init time is %d\n", boot);
    }
    uint32_t now;
    now = NDL_GetTicks();
    return (now-boot);
}

void SDL_Delay(uint32_t ms) {
    // assert(0);
    uint32_t start = SDL_GetTicks();
    while(1){
        uint32_t now;
        now = SDL_GetTicks();
        if ((now - start) >= ms){
            return;
        }
    }
}
