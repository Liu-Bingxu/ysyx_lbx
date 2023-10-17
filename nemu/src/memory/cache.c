#include "memory/cache.h"
#include "memory/paddr.h"

static uint64_t cache_hit_num = 0;
static uint64_t cache_access_num = 0;
static uint64_t access_mem_time = 0;

uint64_t get_cache_hit_num(void){
    return cache_hit_num;
}

uint64_t get_cache_access_num(void){
    return cache_access_num;
}

uint64_t get_access_mem_time(void){
    return access_mem_time;
}

static cache_line cache[ASS_NUM][ASS_SIZE];

static void mem_read(uintptr_t block_num, uint8_t *buf){
    // printf("the block_num is 0x%08lx\n", block_num<<BLOCK_WIDTH);
    memcpy(buf, (void *)(guest_to_host(block_num << BLOCK_WIDTH)), BLOCK_SIZE);
}

static void mem_write(uintptr_t block_num, const uint8_t *buf){
    memcpy((void *)(guest_to_host(block_num << BLOCK_WIDTH)) , buf, BLOCK_SIZE);
}

static uint32_t __cache_read(uintptr_t addr){
    addr &= (~0x3);
    for (int i = 0; i < ASS_SIZE; i++){
        if ((cache[(addr / BLOCK_SIZE) % ASS_NUM][i].tag == ((addr / BLOCK_SIZE) / ASS_NUM)) && (cache[(addr / BLOCK_SIZE) % ASS_NUM][i].valid == 1)){
            cache_hit_num++;
            access_mem_time += 10;
            return cache[(addr / BLOCK_SIZE) % ASS_NUM][i].cache_mem[(addr % BLOCK_SIZE) / 4];
        }
    }
    access_mem_time += 110;
    // printf("cache miss\n");
    int rand_num = rand() % ASS_SIZE;
    // printf("start read data\n");
    mem_read((addr / BLOCK_SIZE), (void *)(cache[(addr / BLOCK_SIZE) % ASS_NUM][rand_num].cache_mem));
    // printf("finish read data\n");
    cache[(addr / BLOCK_SIZE) % ASS_NUM][rand_num].tag = (addr / BLOCK_SIZE) / ASS_NUM;
    cache[(addr / BLOCK_SIZE) % ASS_NUM][rand_num].valid = 1;
    return cache[(addr / BLOCK_SIZE) % ASS_NUM][rand_num].cache_mem[(addr % BLOCK_SIZE) / 4];
}

static void __cache_write(uintptr_t addr, uint32_t data, uint32_t wmask){
    addr &= (~0x3);
    for (int i = 0; i < ASS_SIZE; i++){
        if ((cache[(addr / BLOCK_SIZE) % ASS_NUM][i].tag == ((addr / BLOCK_SIZE) / ASS_NUM)) && (cache[(addr / BLOCK_SIZE) % ASS_NUM][i].valid == 1)){
            cache_hit_num++;
            access_mem_time += 10;
            cache[(addr / BLOCK_SIZE) % ASS_NUM][i].cache_mem[(addr % BLOCK_SIZE) / 4] = (cache[(addr / BLOCK_SIZE) % ASS_NUM][i].cache_mem[(addr % BLOCK_SIZE) / 4] & (~wmask)) | (data & (wmask));
            mem_write(cache[(addr / BLOCK_SIZE) % ASS_NUM][i].tag * ASS_NUM + ((addr / BLOCK_SIZE) % ASS_NUM), (void *)(cache[(addr / BLOCK_SIZE) % ASS_NUM][i].cache_mem));
            return;
        }
    }
    access_mem_time += 110;
    cache_line temp_write_cache_line;
    mem_read((addr / BLOCK_SIZE), (void *)(temp_write_cache_line.cache_mem));
    temp_write_cache_line.cache_mem[(addr % BLOCK_SIZE) / 4] = (temp_write_cache_line.cache_mem[(addr % BLOCK_SIZE) / 4] & (~wmask)) | (data & (wmask));
    mem_write((addr / BLOCK_SIZE), (void *)(temp_write_cache_line.cache_mem));
    return;
}

#define addr_offset_bit(addr) (((addr) & 0x3) * 8)

const uint32_t len2datamask[] = {0x0, 0xff, 0xffff, 0xffffff, 0xffffffff};

uint32_t cache_read(paddr_t addr, size_t len){
    // printf("the addr is " FMT_PADDR ", the len is %ld\n", addr, len);
    cache_access_num++;
    assert((addr % len) == 0);
    return (__cache_read(addr) >> addr_offset_bit(addr)) & len2datamask[len];
}

void cache_write(paddr_t addr, size_t len, uint32_t data){
    cache_access_num++;
    assert((addr % len) == 0);
    __cache_write(addr, data << addr_offset_bit(addr), len2datamask[len] << addr_offset_bit(addr));
}

void init_cache(void){
    for (int i = 0; i < ASS_NUM; i++){
        for (int y = 0; y < ASS_SIZE; y++){
            cache[i][y].valid = 0;
        }
    }
}
