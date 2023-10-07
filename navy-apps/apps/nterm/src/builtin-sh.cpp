#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

static void sh_handle_cmd(char *cmd) {
    int res=memcmp(cmd, "setenv", 6);
    if(res==0){
        char *val=strtok(cmd, " ");
        val = strtok(NULL, " ");
        // printf("%s\n", val);
        if(val!=NULL){
            res = setenv("PATH", val, 0);
            assert(res == 0);
        }
        return;
    }
    char *filename = strtok(cmd, " ");
    char **argv = (char **)malloc(sizeof(char *) * 10);
    assert(argv);
    int _cnt = 0;
    while (1){
        char *val = strtok(NULL, " ");
        if (_cnt < (sizeof(argv) / sizeof(char *))){
            argv[_cnt] = val;
        }
        else{
            char **_argv = (char **)realloc((void *)argv, sizeof(argv) + sizeof(char *) * 1);
            free(argv);
            argv = _argv;
            argv[_cnt] = val;
        }
        _cnt++;
        if(val==NULL){
            break;
        }
    }
    char *env[] = {NULL};
    // execve(cmd, argv,env);
    execl("/bin/pal", "--skip");
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
            char cmd[100];
            strcpy(cmd, res);
            for (int i = 0; i < strlen(cmd); i++){
                if (cmd[i] == '\n'){
                    cmd[i] = '\0';
                    break;
                }
            }
          sh_handle_cmd(cmd);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
