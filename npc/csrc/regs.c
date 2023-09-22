#include "regs.h"
#include "pmem.h"

static REGS reg;

const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void init_gpr(Vtop *top){
    for (int i = 0; i < 32;i++){
        reg.GPR[i] = (&(top->rootp->top__DOT__u_regs__DOT__riscv_reg[i]));
    }
    reg.pc = (&top->rootp->PC_out);
}

word_t get_gpr(int i)
{
    assert((i >= 0) && (i <= 32));
    if (i == 0)
        return 0;
    else if (i == 32)
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
        printf("%-4s : %-12u(" FMT_WORD ")\n", regs[i], get_gpr(i), get_gpr(i));
    printf("pc   : %-12u(" FMT_WORD ")\n", get_gpr(32), get_gpr(32));
}

bool isa_difftest_checkregs(CPU_state *ref,paddr_t pc){
    word_t val;
    pmem_read(pc, &val);
    for (int i = 0; i < 32; i++)
        printf("%-4s : %-12u(" FMT_WORD ")\n", regs[i], ref->gpr[i], ref->gpr[i]);
    printf("pc   : %-12u(" FMT_WORD ")\n", ref->pc,ref->pc);
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++){
        if (ref->gpr[i] != get_gpr(i)){
            printf("error inst: "
                   "\n" FMT_PADDR ": " FMT_WORD,
                   pc, val);
            return false;
        }
    }
    if (ref->pc != get_gpr(32)){
        printf("error inst: "
               "\n" FMT_PADDR ": " FMT_WORD,
               pc, val);
        return false;
    }
    return true;
}

void init_ref(CPU_state *cpu){
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++){
        cpu->gpr[i] = get_gpr(i);
    }
    cpu->pc = get_gpr(32);
}
