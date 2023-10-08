#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
    char *empty[] = {NULL};
    // uintptr_t argc = (*args);
    // args++;
    // char **argv = (char **)args;
    // char **envp = (((char **)args) + argc + 1);
    // environ = empty;
    // environ = envp;
    __libc_init_array();
    // exit(main(argc, argv, envp));
    // assert(0);
    exit(main(0, empty, empty));
    assert(0);
}
