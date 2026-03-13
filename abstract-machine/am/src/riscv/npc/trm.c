#include <am.h>
#include <klib-macros.h>
#include "npc.h"

#include <riscv/riscv.h> // the macro `ISA_H` is defined in CFLAGS

extern char _heap_start;
int main(const char *args);

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
    outb(SERIAL_ADDR, ch);
}

void __am_uart_tx(AM_UART_TX_T *uart_tx) {
    putch(uart_tx->data);
}

void __am_uart_rx(AM_UART_RX_T *uart_rx) {
    // if ((inb(SERIAL_LSR_ADDR) & 0x1) == 0){
    //     uart_rx->data = -1;
    //     return;
    // }
    // uart_rx->data = inb(SERIAL_ADDR);
    uart_rx->data = -1;
}

void halt(int code) {
    asm volatile(".word 0xfc000073" : : "r"(code));
    // asm volatile("mv a0,%0; ebreak;" ::"r"(code));
    while (1);
}

void _trm_init() {
    int ret = main(mainargs);
    halt(ret);
}
