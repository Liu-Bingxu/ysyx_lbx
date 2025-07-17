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
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <memory/paddr.h>
#include "cpu/decode.h"

__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
    if (direction == DIFFTEST_TO_REF){
        for (int i = 0; i < n/4;i++){
            uint32_t _data;
            _data = (*((uint32_t *)buf));
            paddr_write(addr, 4, _data);
            addr += 4;
            buf += 4;
        }
        if((n%4)!=0){
            uint32_t _data;
            if ((n % 4) == 1)
                _data = (uint32_t)(*((uint8_t *)buf));
            else if((n%4)==2)
                _data = (uint32_t)(*((uint16_t *)buf));
            else
                assert(0);
            paddr_write(addr, n % 4, _data);
        }
        // memcpy()
    }
    // else {
    //     assert(0);
    // }
}

__EXPORT void difftest_regcpy(void *dut, bool direction) {
    CPU_state *dut_regs;
    dut_regs = (CPU_state *)dut;
    if (direction == DIFFTEST_TO_REF){
        for (int i = 0; i < 32;i++){
            cpu.gpr[i] = dut_regs->gpr[i];
        }
        cpu.pc = dut_regs->pc;
        cpu.privilege = dut_regs->privilege;
        cpu.mepc = dut_regs->mepc;
        cpu.mtvec = dut_regs->mtvec;
        cpu.mstatus = dut_regs->mstatus;
        cpu.mcause = dut_regs->mcause;
        cpu.mtval = dut_regs->mtval;
        cpu.mscratch = dut_regs->mscratch;
        cpu.mideleg = dut_regs->mideleg;
        cpu.medeleg = dut_regs->medeleg;
        cpu.mip = dut_regs->mip;
        cpu.mie = dut_regs->mie;
        cpu.sepc = dut_regs->sepc;
        cpu.stvec = dut_regs->stvec;
        cpu.scause = dut_regs->scause;
        cpu.stval = dut_regs->stval;
        cpu.sscratch = dut_regs->sscratch;
        cpu.satp = dut_regs->satp;
    }
    else if (direction == DIFFTEST_TO_DUT)
    {
        for (int i = 0; i < 32; i++)
        {
            dut_regs->gpr[i] = cpu.gpr[i];
        }
        dut_regs->pc = cpu.pc;
        dut_regs->privilege = cpu.privilege;
        dut_regs->mepc = cpu.mepc;
        dut_regs->mtvec = cpu.mtvec;
        dut_regs->mstatus = cpu.mstatus;
        dut_regs->mcause = cpu.mcause;
        dut_regs->mtval = cpu.mtval;
        dut_regs->mscratch = cpu.mscratch;
        dut_regs->mideleg = cpu.mideleg;
        dut_regs->medeleg = cpu.medeleg;
        dut_regs->mip = cpu.mip;
        dut_regs->mie = cpu.mie;
        dut_regs->sepc = cpu.sepc;
        dut_regs->stvec = cpu.stvec;
        dut_regs->scause = cpu.scause;
        dut_regs->stval = cpu.stval;
        dut_regs->sscratch = cpu.sscratch;
        dut_regs->satp = cpu.satp;
    }
    else{
        assert(0);
    }
    //   assert(0);
}

__EXPORT void difftest_exec(uint64_t n) {
    cpu_exec(n);
    //   assert(0);
}

__EXPORT void difftest_raise_intr(word_t NO) {
    Decode s;
    s.pc = cpu.pc;
    s.snpc = cpu.pc;
    s.dnpc = cpu.pc;
    isa_raise_intr(&s, NO, s.pc, 0);
    cpu.pc = s.dnpc;
    //   assert(0);
}
__EXPORT CPU_state *_nemu_cpu = &cpu;
__EXPORT uint8_t *_nemu_pmem;

__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
  _nemu_pmem = guest_to_host(0x80000000);
}
