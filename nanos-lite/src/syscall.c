#include <common.h>
#include "syscall.h"

int SYS_exit(int code){
    halt(code);
}

int SYS_yield(){
    yield();
    return 0;
}

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
        return;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
  }
}

