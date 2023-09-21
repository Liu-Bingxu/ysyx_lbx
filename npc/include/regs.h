#ifndef __REGS_H__
#define __REGS_H__

#include "common.h"
#include "Vtop.h"
#include "Vtop___024root.h"

const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

#define gpr(a) concat3(top->rootp->top__DOT__u_regs__DOT__x, a, __DOT__data_out_reg)

typedef struct{
    word_t *GPR[32];
    word_t *pc;
} REGS;

static REGS reg;

void init_gpr(Vtop *top){
    reg.GPR[1]  = (&gpr(1) );
    reg.GPR[2]  = (&gpr(2) );
    reg.GPR[3]  = (&gpr(3) );
    reg.GPR[4]  = (&gpr(4) );
    reg.GPR[5]  = (&gpr(5) );
    reg.GPR[6]  = (&gpr(6) );
    reg.GPR[7]  = (&gpr(7) );
    reg.GPR[8]  = (&gpr(8) );
    reg.GPR[9]  = (&gpr(9) );
    reg.GPR[10] = (&gpr(10));
    reg.GPR[11] = (&gpr(11));
    reg.GPR[12] = (&gpr(12));
    reg.GPR[13] = (&gpr(13));
    reg.GPR[14] = (&gpr(14));
    reg.GPR[15] = (&gpr(15));
    reg.GPR[16] = (&gpr(16));
    reg.GPR[17] = (&gpr(17));
    reg.GPR[18] = (&gpr(18));
    reg.GPR[19] = (&gpr(19));
    reg.GPR[20] = (&gpr(20));
    reg.GPR[21] = (&gpr(21));
    reg.GPR[22] = (&gpr(22));
    reg.GPR[23] = (&gpr(23));
    reg.GPR[24] = (&gpr(24));
    reg.GPR[25] = (&gpr(25));
    reg.GPR[26] = (&gpr(26));
    reg.GPR[27] = (&gpr(27));
    reg.GPR[28] = (&gpr(28));
    reg.GPR[29] = (&gpr(29));
    reg.GPR[30] = (&gpr(30));
    reg.GPR[31] = (&gpr(31));
    reg.pc = (&top->rootp->top__DOT__u_ifu__DOT__PC);
}

word_t get_gpr(int i){
    assert((i >= 0) && (i <= 32));
    if(i==0)
        return 0;
    else if(i==32)
        return (*(reg.pc));
    else
        return (*(reg.GPR[i]));
}

word_t isa_reg_str2val(const char *name, bool *test)
{
    for (int i = 0; i < 32; i++)
    {
        if (strcmp(regs[i], name) == 0)
        {
            return get_gpr(i);
        }
    }
    if (strcmp("pc", name) == 0)
        return get_gpr(32);
    printf("the register name is error\n");
    assert(0);
}

void isa_reg_display(void)
{
    for (int i = 0; i < 32; i++)
        printf("%-4s : %-12u(" FMT_WORD ")\n", regs[i], get_gpr(i),get_gpr(i));
    printf("pc   : %-12u(" FMT_WORD ")\n", get_gpr(32), get_gpr(32));
}

#endif