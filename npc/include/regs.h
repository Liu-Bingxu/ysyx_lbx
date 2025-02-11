#ifndef __REGS_H__
#define __REGS_H__

#include "difftest.h"
#include VTOP_H
#include "svdpi.h"
// #include VTOP_DPI_H

typedef struct{
    word_t GPR[32];
    word_t pc;
    privileged privilege;
    word_t mtvec, mstatus, mcause, mepc, mtval;
    word_t stvec, sstatus, sepc, scause, stval;
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
} REGS;

// extern REGS reg;
void update_reg(void);
void init_gpr(VTOP *top);
word_t get_gpr(int i);
void set_pc(word_t pc);
word_t isa_reg_str2val(const char *name, bool *test);
void isa_reg_display(void);
bool isa_difftest_checkregs(CPU_state *ref,paddr_t pc);
void isa_ref_reg_display(CPU_state *ref);

#endif