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

#include <memory/host.h>
#include <memory/paddr.h>
#include "memory/cache.h"
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

static uint8_t mrom[MROM_SIZE] PG_ALIGN = {};
static uint8_t sram[SRAM_SIZE] PG_ALIGN = {};
static uint8_t flash[FLASH_SIZE] PG_ALIGN = {};

// uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
static inline bool in_mrom(paddr_t addr){
    return ((addr >= MROM_START) && (addr < (MROM_START + MROM_SIZE)));
}

static inline bool in_sram(paddr_t addr){
    return ((addr >= SRAM_START) && (addr < (SRAM_START + SRAM_SIZE)));
}

static inline bool in_flash(paddr_t addr){
    return ((addr >= FLASH_START) && (addr < (FLASH_START + FLASH_SIZE)));
}

uint8_t *guest_to_host(paddr_t paddr){
    if(in_mrom(paddr)){
        return (mrom + paddr - MROM_START);
    }
    else if(in_sram(paddr)){
        return (sram + paddr - SRAM_START);
    }
    else if(in_flash(paddr)){
        return (flash + paddr - FLASH_START);
    }
    else{
        return (pmem + paddr - CONFIG_MBASE);
    }
}
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }
#ifndef CACHE_ENABLE
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  IFDEF(CONFIG_MTRACE, _Log_mem(addr,"Read  Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr, ret));
  return ret;
}
#endif
#ifndef CACHE_ENABLE
static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
  IFDEF(CONFIG_MTRACE, _Log_mem(addr,"Write Addr: " FMT_PADDR " Data: " FMT_WORD "\n", addr, data));
}
#endif
static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
  #ifdef CACHE_ENABLE
    init_cache();
  #endif
}

word_t paddr_read(paddr_t addr, int len) {
  if (likely((in_pmem(addr) | (in_mrom(addr)) | in_sram(addr) | in_flash(addr)))){
  #ifdef CACHE_ENABLE
      return cache_read(addr,len);
    #else
      return pmem_read(addr, len);
    #endif
  }
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely((in_pmem(addr) | in_sram(addr)))) { 
    #ifdef CACHE_ENABLE
      cache_write(addr, len, data);
    #else
      pmem_write(addr, len, data);
    #endif
      return; 
    }
    IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
    out_of_bound(addr);
}
