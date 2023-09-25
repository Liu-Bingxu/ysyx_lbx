#ifndef __REGS_H__
#define __REGS_H__

#include "difftest.h"
#include "Vtop.h"
#include "Vtop___024root.h"

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