#ifndef __PMEM_H__
#define __PMEM_H__

#include "common.h"
// #include VTOP_DPI_H

#ifdef __cplusplus
extern "C" {
#endif

// #define PC_RST     0x80000000

#define PMEM_START 0x80000000
#define PMEM_SIZE  0x20000000

#define MROM_START 0x20000000
#define MROM_SIZE  0x1000

void *guest_to_host(paddr_t addr);

void pmem_read(word_t raddr, word_t *rdata);

void pmem_write(uint64_t waddr, uint64_t wdata, uint8_t wmask);

void init_mem(const char *img_file);

void sim_sram_read(uint64_t raddr, uint64_t *rdata);
void sim_sram_write(uint64_t waddr, uint64_t wdata, uint8_t wmask);

// extern "C" void flash_read(int32_t addr, int32_t *data);
// extern "C" void mrom_read(int32_t addr, int32_t *data);

#ifdef __cplusplus
}
#endif

#endif
