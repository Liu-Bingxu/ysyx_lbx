#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "common.h"

#define DEVICE_ADDR     0xa0000000

#define SERIAL_ADDR     (DEVICE_ADDR+0x000)
#define TIMER_ADDR      (DEVICE_ADDR+0x100)

#define CONFIG_SBI_DISK_MMIO   0x10100000
#define CONFIG_SBI_SERIAL_MMIO 0x10000000
#define CONFIG_SBI_CLINT_MMIO  0x2000000
#define CONFIG_SBI_PLIC_MMIO   0xc000000

#define CONFIG_SBI_PLIC_CONTEXT_COUNT 2

void serial_out(char ch);
void get_rtc();
void get_uptime();
uint32_t get_timer_reg(int offest);
void sbi_serial_io_handler_r(uint64_t raddr, uint64_t *rdata);
void sbi_serial_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_disk_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_disk_io_handler_r(uint64_t raddr, uint64_t *rdata);
void sbi_clint_io_handler_r(uint64_t raddr, uint64_t *rdata);
void sbi_clint_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_plic_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_plic_io_handler_r(uint64_t raddr, uint64_t *rdata);

extern void set_skip_ref_flag(void);

#endif