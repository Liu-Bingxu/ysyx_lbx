#ifndef __COMMON_H__
#define __COMMON_H__

#include "stdint.h"
#include "assert.h"
#include "macro.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"

typedef MUXDEF(RISCV64, uint64_t, uint32_t) vaddr_t;
typedef MUXDEF(RISCV64, uint64_t, uint32_t) paddr_t;

typedef MUXDEF(RISCV64, uint64_t, uint32_t) word_t;
typedef MUXDEF(RISCV64, int64_t, int32_t) sword_t;

#endif