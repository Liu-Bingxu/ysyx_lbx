#include <am.h>
#include <klib-macros.h>
#include "ysyxsoc.h"

#include <riscv/riscv.h> // the macro `ISA_H` is defined in CFLAGS

extern char _heap_start;
int main(const char *args);

// extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
// #define PMEM_END ((uintptr_t) & _pmem_start + PMEM_SIZE)
#define HEAP_END ((uintptr_t) & _heap_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, HEAP_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
    outb(SERIAL_ADDR, ch);
}

void halt(int code) {
    asm volatile(".word 0xfc000073" : : "r"(code));
    // asm volatile("mv a0,%0; ebreak;" ::"r"(code));
    while (1);
}

void _trm_init() {
    extern char _erodata, _data_start, _data_end, _bss_start, _bss_end;
    volatile char *src = &_erodata;
    volatile char *dst = &_data_start;

    /* ROM has data at end of text; copy it.  */
    while (dst < &_data_end)
        *dst++ = *src++;

    /* Zero bss.  */
    for (dst = &_bss_start; dst < &_bss_end; dst++)
        *dst = 0;
    int ret = main(mainargs);
    halt(ret);
}
