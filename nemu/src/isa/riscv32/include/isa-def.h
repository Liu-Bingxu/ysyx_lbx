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

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef enum
{
    PRV_U = 0,
    PRV_S = 1,
    PRV_M = 3
}privileged;

typedef struct
{
    word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
    // word_t gpr[32];
    vaddr_t pc;
    privileged privilege;
    word_t mtvec,mstatus, mcause, mepc, mtval;
    word_t stvec, sepc, scause, stval;
    word_t medeleg, mideleg;
    word_t mip, mie;
    word_t mcycle, minstret, mhpmcounter[29], mhpmevent[29];
    uint32_t mcountinhibit;
    word_t mscratch;
    word_t sscratch;
    word_t satp;
#ifndef CONFIG_RV64
    word_t mcycleh, minstreth, mhpmcounterh[29];
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
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

// #define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
