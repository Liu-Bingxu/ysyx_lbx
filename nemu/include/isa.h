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

#ifndef __ISA_H__
#define __ISA_H__

// Located at src/isa/$(GUEST_ISA)/include/isa-def.h
#include <isa-def.h>

// The macro `__GUEST_ISA__` is defined in $(CFLAGS).
// It will be expanded as "x86" or "mips32" ...
typedef concat(__GUEST_ISA__, _CPU_state) CPU_state;
typedef concat(__GUEST_ISA__, _ISADecodeInfo) ISADecodeInfo;

// monitor
extern unsigned char isa_logo[];
void init_isa();

// reg
extern CPU_state cpu;
void isa_reg_display();
word_t isa_reg_str2val(const char *name, bool *success);

// exec
struct Decode;
int isa_exec_once(struct Decode *s);
bool isa_can_not_disassemble();

// memory
enum { MMU_DIRECT, MMU_TRANSLATE, MMU_FAIL };
enum { MEM_TYPE_IFETCH, MEM_TYPE_READ, MEM_TYPE_WRITE };
enum { MEM_RET_OK, MEM_RET_FAIL, MEM_RET_CROSS_PAGE };
#ifndef isa_mmu_check
int isa_mmu_check(vaddr_t vaddr, int len, int type);
#endif
int isa_mmu_translate(paddr_t *paddr, vaddr_t vaddr, int len, int type);

// interrupt/exception
// vaddr_t isa_raise_intr(word_t NO, vaddr_t epc);
#define INTR_EMPTY ((word_t)-1)
word_t isa_query_intr();
// myself
int try_isa_raise_intr(struct Decode *s);
void isa_raise_intr(struct Decode *s, word_t NO, vaddr_t epc, word_t tval);
// myself
void isa_page_fault(int type, vaddr_t vaddr);
void isa_misalign_fault(int type, vaddr_t vaddr);
// myself
void mret(struct Decode *s);
void sret(struct Decode *s);
word_t get_csr(word_t csr_num, bool *csr_success);
void set_csr(word_t csr_num, word_t mask, bool *csr_success);
void clr_csr(word_t csr_num, word_t mask, bool *csr_success);
void wirte_csr(word_t csr_num, word_t num, bool *csr_success);
// #define MY_DEBUG
#ifdef MY_DEBUG
#define debug_info printf
#else
#define debug_info(fmt, ...)
#endif
// myself

// difftest
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc);
void isa_difftest_attach();

//expr
long my_atoi(const char *args);
word_t isa_reg_str2val(const char *s, bool *success);

#endif
