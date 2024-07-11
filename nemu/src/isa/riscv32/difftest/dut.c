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
#include <cpu/difftest.h>
#include "../local-include/reg.h"
#include "memory/vaddr.h"

extern const char *regs[];

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
    bool error = true;
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++){
        if(ref_r->gpr[i]!=cpu.gpr[i]){
            // for (int i = 0; i < 32;i++){
            //     printf("%-4s : %-12u(0x%08x)\n", regs[i], ref_r->gpr[i], ref_r->gpr[i]);
            // }
            // printf("pc   : %-12u(0x%08x)\n", ref_r->pc,ref_r->pc);
            // printf("error inst: "
            //        "\n" FMT_PADDR ": " FMT_WORD "\n",
            //        pc, vaddr_read(pc, 4));
            // return false;
            error = false;
            printf("%-6s is diff\n", regs[i]);
        }
    }
    if(ref_r->pc!=cpu.pc){
        // return false;
        error = false;
        printf("%-6s is diff\n", "PC");
    }
    if(error==false){
        for (int i = 0; i < 32; i++){
            printf("%-7s  : " FMT_WORD_INT "(" FMT_WORD ")\n", regs[i], ref_r->gpr[i], ref_r->gpr[i]);
        }
        printf("pc       : " FMT_WORD_INT "(" FMT_WORD ")\n", ref_r->pc, ref_r->pc);
        printf("error inst: "
               "\n" FMT_WORD ": 0x%08x\n",
               pc, (uint32_t)vaddr_ifetch(pc, 4));
    }
    return error;
}

void isa_difftest_attach() {
}
