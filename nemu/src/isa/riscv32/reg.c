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
#include "local-include/reg.h"
#include "stdio.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

static const char *privileges[] = {
    "Uesr_mode", "Supervisor_mode", NULL, "Machine_mode"
};

void isa_reg_display() {
	for (int i = 0; i < 32;i++)
        printf("%-7s  : " FMT_WORD_INT "(" FMT_WORD ")\n", regs[i], cpu.gpr[i], cpu.gpr[i]);
    printf("pc       : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.pc, cpu.pc);
    printf("mtvec    : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.mtvec, cpu.mtvec);
    printf("mstatus  : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.mstatus, cpu.mstatus);
    printf("mcause   : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.mcause, cpu.mcause);
    printf("mepc     : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.mepc, cpu.mepc);
    printf("stvec    : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.stvec, cpu.stvec);
    printf("scause   : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.scause, cpu.scause);
    printf("sepc     : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.sepc, cpu.sepc);
    printf("satp     : " FMT_WORD_INT "(" FMT_WORD ")\n", cpu.satp, cpu.satp);
    printf("privilege: %s\n", privileges[cpu.privilege]);
}

word_t isa_reg_str2val(const char *s, bool *success) {
    for (int i = 0; i < 32;i++){
        // printf("%s vs %s\n", regs[i], s);
        if (strcmp(regs[i], s)==0){
            return gpr(i);
        }
    }
    if(strcmp("pc",s)==0)
        return cpu.pc;
    if (strcmp("mtvec", s) == 0)
        return cpu.mtvec;
    if (strcmp("mstatus", s) == 0)
        return cpu.mstatus;
    if (strcmp("mcause", s) == 0)
        return cpu.mcause;
    if (strcmp("mepc", s) == 0)
        return cpu.mepc;
    printf("the register name is error\n");
    assert(0);
}
