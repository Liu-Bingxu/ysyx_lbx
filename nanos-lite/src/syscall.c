#include <common.h>
#include "syscall.h"

int SYS_exit(int code){
    halt(code);
}

int SYS_yield(){
    yield();
    return 0;
}

const char *sys_call_name[] = {
    TOSTRING(sys_exit),
    TOSTRING(sys_yield),
    TOSTRING(sys_open),
    TOSTRING(sys_read),
    TOSTRING(sys_write),
    TOSTRING(sys_kill),
    TOSTRING(sys_getpid),
    TOSTRING(sys_close),
    TOSTRING(sys_lseek),
    TOSTRING(sys_brk),
    TOSTRING(sys_fstat),
    TOSTRING(sys_time),
    TOSTRING(sys_signal),
    TOSTRING(sys_execve),
    TOSTRING(sys_fork),
    TOSTRING(sys_link),
    TOSTRING(sys_unlink),
    TOSTRING(sys_wait),
    TOSTRING(sys_times),
    TOSTRING(sys_gettimeofday)
};

// #define str(x) #x

void do_syscall(Context *c) {
    uintptr_t a[4];
    a[0] = c->GPR1;
    a[1] = c->GPR2;
    a[2] = c->GPR3;
    a[3] = c->GPR4;

    switch (a[0]){
    case sys_exit:
        SYS_exit(a[1]);
    case sys_yield:
        c->GPRx=SYS_yield();
        c->mepc += 4;
        break;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
    Log("syscall the %s with %d %d %d return the %d", sys_call_name[a[0]], a[1], a[2], a[3], c->GPRx);
}

