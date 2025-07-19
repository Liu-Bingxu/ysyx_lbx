#include <am.h>
#include <klib-macros.h>
#include "ysyxsoc.h"

#include <riscv/riscv.h> // the macro `ISA_H` is defined in CFLAGS

extern char _heap_start;
extern char _heap_end;
int main(const char *args);

// extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
// #define PMEM_END ((uintptr_t) & _pmem_start + PMEM_SIZE)
// #define HEAP_END ((uintptr_t) & _heap_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, &_heap_end);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
    while ((inb(SERIAL_LSR_ADDR) & 0x20) == 0)
        ;

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

    uint16_t bdiv = 2;

    outb(SERIAL_LCR_ADDR, 0x80);

    if (bdiv) {
		/* Set divisor low byte */
        outb(SERIAL_DLL_ADDR, bdiv & 0xff);
        /* Set divisor high byte */
        outb(SERIAL_DLM_ADDR, (bdiv >> 8) & 0xff);
    }

	/* 8 bits, no parity, one stop bit */
    outb(SERIAL_LCR_ADDR, 0x03);

    outw(SPI_DIV_ADDR, 0x0);

    uintptr_t mvendorid, marchid;
    asm volatile(
        "csrr %0,mvendorid\n"
        "csrr %1,marchid\n"
        : "=r"(mvendorid), // %1
          "=r"(marchid)    // %2
        :
        : );
    char *vendorid = (char *)&mvendorid;
    const char *str1 = "my mvendorid is ";
    for (int i = 0; str1[i]; i++){
        putch(str1[i]);
    }for (int i = 0; i < 4; i++){
        putch(vendorid[3 - i]);
    }
    const char *str2 = "; my marhid is ";
    for (int i = 0; str2[i]; i++){
        putch(str2[i]);
    }
    char archid[8] = {0};
    for (int i = 0; i < 8; i++){
        archid[7 - i] = (marchid % 10) + '0';
        marchid /= 10;
    }for (int i = 0; i < 8; i++){
        putch(archid[i]);
    }
    putch('\n');

    int ret = main(mainargs);
    halt(ret);
}
