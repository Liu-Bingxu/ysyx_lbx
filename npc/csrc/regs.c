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
    reg.pc = (&top->rootp->top__DOT__u_ifu__DOT__PC_to_sram_reg);
    reg.mcause = (&top->rootp->top__DOT__u_csr__DOT__MCAUSE__DOT__data_out_reg);
    reg.mepc = (&top->rootp->top__DOT__u_csr__DOT__MEPC__DOT__data_out_reg);
    reg.mstatus = (&top->rootp->top__DOT__u_csr__DOT__MSTATUS__DOT__data_out_reg);
    reg.mtvec = (&top->rootp->top__DOT__u_csr__DOT__MTVEC__DOT__data_out_reg);
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
    if(strcmp("mepc",name)==0)
    return (*reg.mepc);
    if (strcmp("mcause", name) == 0)
        return (*reg.mcause);
    if (strcmp("mstatus", name) == 0)
        return (*reg.mstatus);
    if (strcmp("mtvec", name) == 0)
        return (*reg.mtvec);
    printf("the register name is error\n");
    assert(0);
}

void isa_reg_display(void)
{
    bool text = true;
    for (int i = 0; i < 32; i++)
        printf("%-7s : %-12u(" FMT_WORD ")\n", regs[i], get_gpr(i), get_gpr(i));
    printf("pc      : %-12u(" FMT_WORD ")\n", get_gpr(32), get_gpr(32));
    printf("mepc    : %-12u(" FMT_WORD ")\n", isa_reg_str2val("mepc", &text), isa_reg_str2val("mepc", &text));
    printf("mtvec   : %-12u(" FMT_WORD ")\n", isa_reg_str2val("mtvec", &text), isa_reg_str2val("mtvec", &text));
    printf("mcause  : %-12u(" FMT_WORD ")\n", isa_reg_str2val("mcause", &text), isa_reg_str2val("mcause", &text));
    printf("mstatus : %-12u(" FMT_WORD ")\n", isa_reg_str2val("mstatus", &text), isa_reg_str2val("mstatus", &text));
}

void isa_ref_reg_display(CPU_state *ref)
{
    bool text = true;
    for (int i = 0; i < 32; i++)
        printf("%-7s : %-12u(" FMT_WORD ")\n", regs[i], ref->gpr[i], ref->gpr[i]);
    printf("pc      : %-12u(" FMT_WORD ")\n", ref->pc,ref->pc);
    printf("mepc    : %-12u(" FMT_WORD ")\n", ref->mepc,ref->mepc);
    printf("mtvec   : %-12u(" FMT_WORD ")\n", ref->mtvec,ref->mtvec);
    printf("mcause  : %-12u(" FMT_WORD ")\n", ref->mcause,ref->mcause);
    printf("mstatus : %-12u(" FMT_WORD ")\n", ref->mstatus,ref->mstatus);
}

bool isa_difftest_checkregs(CPU_state *ref,paddr_t pc){
    word_t val;
    pmem_read(pc, &val);
    bool text = true;
    bool this_text = true;
    // for (int i = 0; i < 32; i++)
    //     printf("%-4s : %-12u(" FMT_WORD ")\n", regs[i], ref->gpr[i], ref->gpr[i]);
    // printf("pc   : %-12u(" FMT_WORD ")\n", ref->pc,ref->pc);
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++){
        if (ref->gpr[i] != get_gpr(i)){
            printf("The %s diff\n",regs[i]);
            this_text=false;
        }
    }
    if (ref->pc != get_gpr(32)){
        printf("The pc diff\n");
        this_text=false;
    }
    if (ref->mcause != isa_reg_str2val("mcause",&text)){
        printf("The mcause diff\n");
        this_text=false;
    }
    if (ref->mepc != isa_reg_str2val("mepc",&text)){
        printf("The mepc diff\n");
        this_text=false;
    }
    if (ref->mstatus != isa_reg_str2val("mstatus",&text)){
        printf("The mstatus diff\n");
        this_text=false;
    }
    if (ref->mtvec != isa_reg_str2val("mtvec",&text)){
        printf("The mtvec diff\n");
        this_text=false;
    }
    if(this_text==false){
        printf("error inst: "
               "\n" FMT_PADDR ": " FMT_WORD "\n",
               pc, val);
        isa_ref_reg_display(ref);
        printf("-------------------------------------------------------------------------\n");
    }
    return this_text;
}

void init_ref(CPU_state *cpu){
    bool text = true;
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++)
    {
        cpu->gpr[i] = get_gpr(i);
    }
    cpu->pc = get_gpr(32);
    cpu->mepc = isa_reg_str2val("mepc", &text);
    cpu->mtvec=isa_reg_str2val("mtvec", &text);
    cpu->mcause=isa_reg_str2val("mcause", &text);
    cpu->mstatus=isa_reg_str2val("mstatus", &text);
}
