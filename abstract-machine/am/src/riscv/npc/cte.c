#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
        case 11:
            ev.event = EVENT_YIELD;
            if (c->GPR1 != -1)
                ev.event = EVENT_SYSCALL;
            c->mepc += 4;
            break;
        default:
            ev.event = EVENT_ERROR;
            break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

#if __riscv_xlen == 64
#define RST_STATUS 0xa00001800
#define XLEN 8
#else
#define RST_STATUS 0x1800
#define XLEN 4
#endif

#ifndef __riscv_e
#define NR_REGS 32
#else
#define NR_REGS 16
#endif

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
    Context *context = (Context *)(kstack.end - (NR_REGS + 3 + 1) * XLEN);
    for (int i = 0; i < NR_REGS; i++)
    {
        context->gpr[i] = 0;
    }
    context->GPR2 = (uintptr_t)arg;
    context->mcause = 0;
    context->mstatus = RST_STATUS;
    context->mepc = (uintptr_t)entry;
    return context;
    // return NULL;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
