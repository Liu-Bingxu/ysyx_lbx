#ifndef __PMEM_H__
#define __PMEM_H__

#include "common.h"
#include "VysyxSoCFull__Dpi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_RST 0x20000000

#define PMEM_SIZE 0x8000000

void *guest_to_host(paddr_t addr);

void pmem_read(uint32_t raddr, uint32_t *rdata);

void pmem_write(uint32_t waddr, uint32_t wdata, char wmask);

void init_mem(const char *img_file);

extern "C" void flash_read(int32_t addr, int32_t *data);
extern "C" void mrom_read(int32_t addr, int32_t *data);

#ifdef __cplusplus
}
#endif

#endif
