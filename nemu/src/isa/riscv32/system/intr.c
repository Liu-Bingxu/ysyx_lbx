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

#include <isa.h>
#include "debug.h"

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */

  cpu.mepc = epc;
  cpu.mcause = NO;

  return cpu.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}

word_t get_csr(word_t csr_num){
    csr_num &= 0xfff;
    switch (csr_num){
    case 0x300:
        return cpu.mstatus;
    case 0x305:
        return cpu.mtvec;
    case 0x341:
        return cpu.mepc;
    case 0x342:
        return cpu.mcause;
    default:
        panic("unkown CSR number: %u", csr_num);
    }
}

void set_csr(word_t csr_num,word_t mask){
    csr_num &= 0xfff;
    switch (csr_num){
    case 0x300:
        cpu.mstatus|=mask;
        return;
    case 0x305:
        cpu.mtvec|=mask;
        return;
    case 0x341:
        cpu.mepc|=mask;
        return;
    case 0x342:
        cpu.mcause|=mask;
        return;
    default:
        panic("unkown CSR number: %u", csr_num);
    }
}

void clr_csr(word_t csr_num, word_t mask){
    csr_num &= 0xfff;
    mask = ~mask;
    switch (csr_num){
    case 0x300:
        cpu.mstatus &= mask;
        return;
    case 0x305:
        cpu.mtvec &= mask;
        return;
    case 0x341:
        cpu.mepc &= mask;
        return;
    case 0x342:
        cpu.mcause &= mask;
        return;
    default:
        panic("unkown CSR number: %u", csr_num);
    }
}

void wirte_csr(word_t csr_num,word_t num){
    csr_num &= 0xfff;
    switch (csr_num){
    case 0x300:
        cpu.mstatus = num;
        return;
    case 0x305:
        cpu.mtvec = num;
        return;
    case 0x341:
        cpu.mepc = num;
        return;
    case 0x342:
        cpu.mcause = num;
        return;
    default:
        panic("unkown CSR number: %u", csr_num);
    }
}
