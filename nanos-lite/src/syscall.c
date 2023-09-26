#include <common.h>
#include "syscall.h"
int SYS_yield(){
    yield();
    return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case sys_yield:
        c->GPRx=SYS_yield();
        return;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
  }
}

