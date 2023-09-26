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
    // sys_open,
    // sys_read,
    // sys_write,
    // sys_kill,
    // sys_getpid,
    // sys_close,
    // sys_lseek,
    // sys_brk,
    // sys_fstat,
    // sys_time,
    // sys_signal,
    // sys_execve,
    // sys_fork,
    // sys_link,
    // sys_unlink,
    // sys_wait,
    // sys_times,
    // sys_gettimeofday
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
        Log("%s", sys_call_name[a[0]]);
        c->mepc += 4;
        return;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
  }
}

