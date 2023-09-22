#ifndef __REGS_H__
#define __REGS_H__

#include "difftest.h"
#include "Vtop.h"
#include "Vtop___024root.h"

#define gpr(a) concat3(top->rootp->top__DOT__u_regs__DOT__x, a, __DOT__data_out_reg)

typedef struct{
    word_t *GPR[32];
    word_t *pc;
} REGS;

// extern REGS reg;
void init_gpr(Vtop *top);
word_t get_gpr(int i);
word_t isa_reg_str2val(const char *name, bool *test);
void isa_reg_display(void);
bool isa_difftest_checkregs(CPU_state *ref,paddr_t pc);

#endif