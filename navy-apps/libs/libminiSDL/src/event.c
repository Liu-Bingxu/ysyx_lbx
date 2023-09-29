#include <NDL.h>
#include <SDL.h>
#include "assert.h"
#include "string.h"
#include "stdio.h"

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
    assert(0);
    return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
    assert(0);
    return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
    // assert(0);
    char buf[64];
    while(1){
        if (NDL_PollEvent(buf, sizeof(buf))){
            printf("hello\n");
            if (buf[1] == 'u'){
                event->type = SDL_KEYUP;
            }
            else if (buf[1] == 'd'){
                event->type = SDL_KEYDOWN;
            }
            else{
                printf("%s\n", buf);
                assert(0);
            }
            for (int i = 0; i < (sizeof(keyname) / sizeof(char *));i++){
                if(strcmp((buf+3),keyname[i])==0){
                    event->key.keysym.sym = i;
                    printf("%s\n", buf);
                    return 1;
                }
            }
            return 0;
        }
    }
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
    assert(0);
    return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
    assert(0);
    return NULL;
}
