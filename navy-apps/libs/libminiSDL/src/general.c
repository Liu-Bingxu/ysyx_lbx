#include <NDL.h>
#include "assert.h"
#include "stdio.h"
#include "sdl-timer.h"
#include "sdl-general.h"

int SDL_Init(uint32_t flags) {
    // assert(0);
    printf("now sdl init\n");
    if(flags&SDL_INIT_TIMER){
        SDL_GetTicks();
    }
    return NDL_Init(flags);
}

void SDL_Quit() {
    assert(0);
    NDL_Quit();
}

char *SDL_GetError() {
    // assert(0);
    return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
    assert(0);
    return -1;
}

int SDL_ShowCursor(int toggle) {
    assert(0);
    return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
    // assert(0);
}
