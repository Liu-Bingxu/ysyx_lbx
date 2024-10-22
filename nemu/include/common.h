/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include <generated/autoconf.h>
#include "macro.h"

#ifdef CONFIG_TARGET_AM
#include <klib.h>
#else
#include <assert.h>
#include <stdlib.h>
#endif

#if CONFIG_MBASE + CONFIG_MSIZE > 0x100000000ul
#define PMEM64 1
#endif

typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t)  sword_t;
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)

//myself
#define FMT_WORD_INT MUXDEF(CONFIG_ISA64, "%-21lu", "%-21u")
//myself

typedef word_t vaddr_t;
// typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
// #define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)
typedef vaddr_t paddr_t;
#define FMT_PADDR FMT_WORD
typedef uint16_t ioaddr_t;

#include <debug.h>

typedef struct token {
  int type;
  char str[32];
} Token;

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;
    struct watchpoint *prev;

  /* TODO: Add more members if necessary */

    int Hitnum;
    word_t old_value;
    int wp_nr_token;
    Token wp_tokens[100];

} WP;

bool check_watchpoint();

#endif
