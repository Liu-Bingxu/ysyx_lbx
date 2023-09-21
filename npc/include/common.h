#ifndef __COMMON_H__
#define __COMMON_H__

#include "stdint.h"
#include "assert.h"
#include "macro.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "config.h"

typedef MUXDEF(RISCV64, uint64_t, uint32_t) vaddr_t;
typedef MUXDEF(RISCV64, uint64_t, uint32_t) paddr_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" "lx", "0x%08" "x")

typedef MUXDEF(RISCV64, uint64_t, uint32_t) word_t;
typedef MUXDEF(RISCV64, int64_t, int32_t) sword_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" "lx", "0x%08" "x")


#endif