#ifndef __COMMON_H__
#define __COMMON_H__

#include "stdint.h"
#include "assert.h"
#include "macro.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "config.h"
#include <inttypes.h>

typedef MUXDEF(CONFIG_RV64, uint64_t, uint32_t) vaddr_t;
typedef MUXDEF(CONFIG_RV64, uint64_t, uint32_t) paddr_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)
#define FMT_WORD_U MUXDEF(CONFIG_ISA64, PRIuMAX, "u")

typedef MUXDEF(CONFIG_RV64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_RV64, int64_t, int32_t) sword_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)

#endif