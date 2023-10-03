#include <NDL.h>
#include <SDL.h>
#include "sdl-event.h"
#include "assert.h"
#include "string.h"
#include "stdio.h"

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t status[SDLK_PAGEDOWN + 1] = {0};

int SDL_PushEvent(SDL_Event *ev) {
    assert(0);
    return 0;
}

int SDL_PollEvent(SDL_Event *event) {
    // assert(0);
    if(event==NULL){
        return 0;
    }
    char buf[64];
    // printf("hello\n");
    SDL_audio_help_callback();
    if (NDL_PollEvent(buf, sizeof(buf))){
        if (buf[1] == 'u'){
            // printf("UP\n");
            event->type = SDL_KEYUP;
        }
        else if (buf[1] == 'd'){
            // printf("down\n");
            event->type = SDL_KEYDOWN;
        }
        else{
            printf("%s\n", buf);
            assert(0);
        }
        // printf("%s\n", buf + 3);
        // printf("num is %d\n", sizeof(keyname) / sizeof(char *));
        for (int i = 0; i < 100; i++){
            if ((*(buf + 3 + i)) == '\n')
            {
                (*(buf + 3 + i)) = '\0';
                break;
            }
        }
        for (int i = 0; i < (sizeof(keyname) / sizeof(keyname[0])); i++){
            // printf("%s and %s res is %d\n", buf + 3, keyname[i], strcmp(buf + 3, keyname[i]));
            if (strcmp((buf + 3), keyname[i]) == 0)
            {
                event->key.keysym.sym = i;
                if (event->type == SDL_KEYDOWN){
                    status[i] = 1;
                }
                else if (event->type == SDL_KEYUP){
                    status[i] = 0;
                }
                else{
                    assert(0);
                }
                // printf("%s\n", buf);
                return 1;
            }
        }
    }
    return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
    if (event == NULL){
        return 0;
    }
    // assert(0);
    char buf[64];
    // printf("hello\n");
    while(1){
        if (NDL_PollEvent(buf, sizeof(buf))){
            // printf("hello\n");
            if (buf[1] == 'u'){
                // printf("UP\n");
                event->type = SDL_KEYUP;
            }
            else if (buf[1] == 'd'){
                // printf("down\n");
                event->type = SDL_KEYDOWN;
            }
            else{
                printf("%s\n", buf);
                assert(0);
            }
            // printf("%s\n", buf + 3);
            // printf("num is %d\n", sizeof(keyname) / sizeof(char *));
            for(int i=0;i<100;i++){
                if((*(buf+3+i))=='\n'){
                    (*(buf + 3 + i)) = '\0';
                    break;
                }
            }
            for (int i = 0; i < (sizeof(keyname) / sizeof(keyname[0])); i++){
                // printf("%s and %s res is %d\n", buf + 3, keyname[i], strcmp(buf + 3, keyname[i]));
                if (strcmp((buf + 3), keyname[i]) == 0){
                    event->key.keysym.sym = i;
                    if (event->type == SDL_KEYDOWN){
                        status[i] = 1;
                    }
                    else if (event->type == SDL_KEYUP){
                        status[i] = 0;
                    }
                    else{
                        assert(0);
                    }
                    // printf("%s\n", buf);
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
    // assert(0);
    // SDL_Event ev;
    // int ret = SDL_PollEvent(&ev);
    // if(ret==1){
    //     if (ev.type == SDL_KEYDOWN){
    //         status[ev.key.keysym.sym] = 1;
    //     }
    //     else if (ev.type == SDL_KEYUP){
    //         status[ev.key.keysym.sym] = 0;
    //     }
    //     else{
    //         assert(0);
    //     }
    // }
    // else{
    //     memset(status, '\0', sizeof(status));
    // }
    SDL_audio_help_callback();
    return status;
}
