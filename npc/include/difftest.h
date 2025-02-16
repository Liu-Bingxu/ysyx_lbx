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

#ifndef __CPU_DIFFTEST_H__
#define __CPU_DIFFTEST_H__

#include <common.h>
#include "debug.h"

#ifdef CONFIG_DIFFTEST
void difftest_skip_ref();
void difftest_skip_dut(int nr_ref, int nr_dut);
void difftest_set_patch(void (*fn)(void *arg), void *arg);
void difftest_step(vaddr_t pc, vaddr_t npc);
void difftest_detach();
void difftest_attach();
#else
static inline void difftest_skip_ref() {}
static inline void difftest_skip_dut(int nr_ref, int nr_dut) {}
static inline void difftest_set_patch(void (*fn)(void *arg), void *arg) {}
static inline void difftest_step(vaddr_t pc, vaddr_t npc) {}
static inline void difftest_detach() {}
static inline void difftest_attach() {}
#endif

extern void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction);
extern void (*ref_difftest_regcpy)(void *dut, bool direction);
extern void (*ref_difftest_exec)(uint64_t n);
extern void (*ref_difftest_raise_intr)(uint64_t NO);

static inline bool difftest_check_reg(const char *name, vaddr_t pc, word_t ref, word_t dut) {
  if (ref != dut) {
      Log("%s is different after executing instruction at pc = " FMT_WORD
          ", right = " FMT_WORD ", wrong = " FMT_WORD ", diff = " FMT_WORD,
          name, pc, ref, dut, ref ^ dut);
      return false;
  }
  return true;
}

typedef enum
{
    PRV_U = 0,
    PRV_S = 1,
    PRV_M = 3
} privileged;

typedef struct{
    word_t gpr[32];
    word_t pc;
    privileged privilege;
    word_t mtvec, mstatus, mcause, mepc, mtval;
    word_t stvec, sepc, scause, stval;
    word_t medeleg, mideleg;
    word_t mip, mie;
    word_t mcycle, minstret, mhpmcounter[29], mhpmevent[29];
    uint32_t mcountinhibit;
    word_t mscratch;
    word_t sscratch;
    word_t satp;
#ifndef CONFIG_RV64
    word_t mcycleh, minstreth, mhpmcounterh[29], mhpmeventh[29];
#endif
    //? RW but RO in imp
    word_t misa, menvcfg, mseccfg;
    word_t senvcfg;
    uint32_t mcounteren;
    uint32_t scounteren;
#ifndef CONFIG_RV64
    word_t mstatush;
    word_t menvcfgh, mseccfgh;
#endif
    //? RO reg
    word_t mvendorid, marchid, mimpid, mhartid, mconfigptr;
}CPU_state;

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

#endif
