#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

// #define CACHE_ENABLE
#define BLOCK_WIDTH 6
#define BLOCK_SIZE 64

typedef struct cache_line{
    bool valid;
    paddr_t tag;
    union {
        uint32_t cache_mem[BLOCK_SIZE / 4];
        uint64_t cache_mem_64[BLOCK_SIZE / 8];
    };
} cache_line;

#define ASS_NUM 128
#define ASS_SIZE 8

void init_cache(void);
uint32_t cache_read(paddr_t addr, size_t len);
void cache_write(paddr_t addr, size_t len, uint32_t data);

#endif