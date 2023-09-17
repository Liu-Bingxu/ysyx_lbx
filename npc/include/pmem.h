#ifndef __PMEM_H__
#define __PMEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define PMEM_SIZE 0x8000000

void pmem_read(uint32_t raddr,uint32_t *rdata);

void pmem_write(uint32_t waddr, uint32_t wdata,char wmask);

void init_mem(const char *img_file);

#ifdef __cplusplus
}
#endif

#endif
