#include "regs.h"
#include "pmem.h"

extern "C" void difftest_ArchIntRegState(word_t *out_io_value_0, word_t *out_io_value_1, word_t *out_io_value_2, word_t *out_io_value_3, word_t *out_io_value_4,
                                         word_t *out_io_value_5, word_t *out_io_value_6, word_t *out_io_value_7, word_t *out_io_value_8, word_t *out_io_value_9,
                                         word_t *out_io_value_10, word_t *out_io_value_11, word_t *out_io_value_12, word_t *out_io_value_13, word_t *out_io_value_14,
                                         word_t *out_io_value_15, word_t *out_io_value_16, word_t *out_io_value_17, word_t *out_io_value_18, word_t *out_io_value_19,
                                         word_t *out_io_value_20, word_t *out_io_value_21, word_t *out_io_value_22, word_t *out_io_value_23, word_t *out_io_value_24,
                                         word_t *out_io_value_25, word_t *out_io_value_26, word_t *out_io_value_27, word_t *out_io_value_28, word_t *out_io_value_29,
                                         word_t *out_io_value_30, word_t *out_io_value_31);

extern "C" void difftest_CSRState(word_t *out_io_privilegeMode, word_t *out_io_mstatus, word_t *out_io_sstatus, word_t *out_io_mepc, word_t *out_io_sepc, word_t *out_io_mtval,
                                  word_t *out_io_stval, word_t *out_io_mtvec, word_t *out_io_stvec, word_t *out_io_mcause, word_t *out_io_scause, word_t *out_io_satp,
                                  word_t *out_io_mip, word_t *out_io_mie, word_t *out_io_mscratch, word_t *out_io_sscratch, word_t *out_io_mideleg, word_t *out_io_medeleg);

REGS reg;
#define nop 0x00000013

const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

static const uint32_t img[] = {
    0x00000297, // auipc t0,0
    // nop,
    // nop,
    // nop,
    // nop,
    0x00028A23, // sb  zero,20(t0)
    0x00100513, // addi a0,zero,1
    0x0142c503, // lbu a0,20(t0)
    // 0x00c0006f, // j   0x8000002c
    // 0x00100513, // addi a0,zero,1
    // 0x4,
    // 0x00000513, // addi a0,zero,0
    0x00100073, // ebreak (used as nemu_trap)
    0xdeadbeef, // some data
};

void update_reg(void){
    svSetScope(svGetScopeFromName("TOP." TOPNAME ".u_DifftestArchIntRegState"));
    difftest_ArchIntRegState(&reg.GPR[0], &reg.GPR[1], &reg.GPR[2], &reg.GPR[3], &reg.GPR[4], &reg.GPR[5], &reg.GPR[6], &reg.GPR[7], &reg.GPR[8], &reg.GPR[9],
                             &reg.GPR[10], &reg.GPR[11], &reg.GPR[12], &reg.GPR[13], &reg.GPR[14], &reg.GPR[15], &reg.GPR[16], &reg.GPR[17], &reg.GPR[18], &reg.GPR[19],
                             &reg.GPR[20], &reg.GPR[21], &reg.GPR[22], &reg.GPR[23], &reg.GPR[24], &reg.GPR[25], &reg.GPR[26], &reg.GPR[27], &reg.GPR[28], &reg.GPR[29],
                             &reg.GPR[30], &reg.GPR[31]);
    // printf("after update s5 is " FMT_WORD "\n", reg.GPR[21]);
    word_t privilegeMode;
    svSetScope(svGetScopeFromName("TOP." TOPNAME ".u_DifftestCSRState"));
    difftest_CSRState(&privilegeMode, &reg.mstatus, &reg.sstatus, &reg.mepc, &reg.sepc, &reg.mtval, &reg.stval, &reg.mtvec, &reg.stvec, &reg.mcause, &reg.scause, &reg.satp,
                      &reg.mip, &reg.mie, &reg.mscratch, &reg.sscratch, &reg.mideleg, &reg.medeleg);
    // reg.privilege = privilegeMode;
    switch (privilegeMode){
        case 0:
            reg.privilege = PRV_U;
            break;
        case 1:
            reg.privilege = PRV_S;
            break;
        case 3:
            reg.privilege = PRV_M;
            break;
        default:
            printf("privilegeMode is error\n");
            assert(0);
            break;
    }
}

void init_gpr(VTOP *top){
    // for (int i = 1; i < 32;i++){
    //     reg.GPR[i] = ((word_t *)&(top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_wbu__DOT__u_gpr__DOT__riscv_reg[i - 1]));
    // }
    // reg.pc = (&top->rootp->top__DOT__u_ifu__DOT__PC_to_sram_reg);
    update_reg();
    reg.pc = PC_RST;
    // reg.mcause = ((word_t *)&top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_wbu__DOT__u_csr__DOT__u_csr_mcause__DOT__mcause_reg);
    // reg.mepc = ((word_t *)&top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_wbu__DOT__u_csr__DOT__u_csr_mepc__DOT__mepc_reg);
    // reg.mstatus = ((word_t *)&top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_wbu__DOT__u_csr__DOT__mstatus);
    // reg.mtvec = ((word_t *)&top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_wbu__DOT__u_csr__DOT__u_csr_mtvec__DOT__mtvec_reg);
    memcpy(guest_to_host(PC_RST), img, sizeof(img));
}

void set_pc(word_t pc){
    reg.pc = pc;
}

word_t get_gpr(int i)
{
    assert((i >= 0) && (i <= 32));
    if (i == 0)
        return 0;
    else if (i == 32)
        return reg.pc;
    else
        return reg.GPR[i];
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
        return reg.mepc;
    if (strcmp("mcause", name) == 0)
        return reg.mcause;
    if (strcmp("mstatus", name) == 0)
        return reg.mstatus;
    if (strcmp("mtvec", name) == 0)
        return reg.mtvec;
    printf("the register name is error\n");
    assert(0);
}

void isa_reg_display(void){
    printf("npc is :\n");
    for (int i = 0; i < 32; i++)
        printf("%-7s   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", regs[i], get_gpr(i), get_gpr(i));
    printf("pc        : %-20" FMT_WORD_U "(" FMT_WORD ")\n", get_gpr(32), get_gpr(32));
    printf("privilege : %-20s\n", (reg.privilege == PRV_M) ? "PRV_M" : (reg.privilege == PRV_S) ? "PRV_S" : (reg.privilege == PRV_U) ? "PRV_U" : "UNKOWN PRIV");
    printf("mepc      : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mepc,     reg.mepc);
    printf("mtvec     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mtvec,    reg.mtvec);
    printf("mcause    : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mcause,   reg.mcause);
    printf("mstatus   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mstatus,  reg.mstatus);
    printf("mtval     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mtval,    reg.mtval);
    printf("mscratch  : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mscratch, reg.mscratch);
    printf("mideleg   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mideleg,  reg.mideleg);
    printf("medeleg   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.medeleg,  reg.medeleg);
    printf("mip       : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mip,      reg.mip);
    printf("mie       : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.mie,      reg.mie);
    printf("sepc      : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.sepc,     reg.sepc);
    printf("stvec     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.stvec,    reg.stvec);
    printf("scause    : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.scause,   reg.scause);
    printf("sstatus   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.sstatus,  reg.sstatus);
    printf("stval     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.stval,    reg.stval);
    printf("sscratch  : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.sscratch, reg.sscratch);
    printf("satp      : %-20" FMT_WORD_U "(" FMT_WORD ")\n", reg.satp,     reg.satp);
}

void isa_ref_reg_display(CPU_state *ref){
    printf("ref is :\n");
    for (int i = 0; i < 32; i++)
        printf("%-7s   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", regs[i], ref->gpr[i], ref->gpr[i]);
    printf("pc        : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->pc,ref->pc);
    printf("privilege : %-20s\n", (ref->privilege == PRV_M) ? "PRV_M" : (ref->privilege == PRV_S) ? "PRV_S" : (ref->privilege == PRV_U) ? "PRV_U" : "UNKOWN PRIV");
    printf("mepc      : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mepc,ref->mepc);
    printf("mtvec     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mtvec,ref->mtvec);
    printf("mcause    : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mcause,ref->mcause);
    printf("mstatus   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mstatus,ref->mstatus);
    printf("mtval     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mtval,    ref->mtval);
    printf("mscratch  : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mscratch, ref->mscratch);
    printf("mideleg   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mideleg,  ref->mideleg);
    printf("medeleg   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->medeleg,  ref->medeleg);
    printf("mip       : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mip,      ref->mip);
    printf("mie       : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->mie,      ref->mie);
    printf("sepc      : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->sepc,     ref->sepc);
    printf("stvec     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->stvec,    ref->stvec);
    printf("scause    : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->scause,   ref->scause);
    // printf("sstatus   : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->sstatus,  ref->sstatus);
    printf("stval     : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->stval,    ref->stval);
    printf("sscratch  : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->sscratch, ref->sscratch);
    printf("satp      : %-20" FMT_WORD_U "(" FMT_WORD ")\n", ref->satp,     ref->satp);
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
    if (ref->privilege != reg.privilege){
        printf("The privilege diff\n");
        this_text=false;
    }
    if (ref->mcause != reg.mcause){
        printf("The mcause diff\n");
        this_text=false;
    }
    if (ref->mepc != reg.mepc){
        printf("The mepc diff\n");
        this_text=false;
    }
    if (ref->mstatus != reg.mstatus){
        printf("The mstatus diff\n");
        this_text=false;
    }
    if (ref->mtvec != reg.mtvec){
        printf("The mtvec diff\n");
        this_text=false;
    }
    if (ref->mtval != reg.mtval){
        printf("The mtval diff\n");
        this_text=false;
    }
    if (ref->scause != reg.scause){
        printf("The scause diff\n");
        this_text=false;
    }
    if (ref->sepc != reg.sepc){
        printf("The sepc diff\n");
        this_text=false;
    }
    // if (ref->sstatus != reg.sstatus){
    //     printf("The sstatus diff\n");
    //     this_text=false;
    // }
    if (ref->stvec != reg.stvec){
        printf("The stvec diff\n");
        this_text=false;
    }
    if (ref->stval != reg.stval){
        printf("The stval diff\n");
        this_text=false;
    }
    if (ref->satp != reg.satp){
        printf("The satp diff\n");
        this_text=false;
    }
    if (ref->mip != reg.mip){
        printf("The mip diff\n");
        this_text=false;
    }
    if (ref->mie != reg.mie){
        printf("The mie diff\n");
        this_text=false;
    }
    if (ref->mscratch != reg.mscratch){
        printf("The mscratch diff\n");
        this_text=false;
    }
    if (ref->sscratch != reg.sscratch){
        printf("The sscratch diff\n");
        this_text=false;
    }
    if (ref->mideleg != reg.mideleg){
        printf("The mideleg diff\n");
        this_text=false;
    }
    if (ref->medeleg != reg.medeleg){
        printf("The medeleg diff\n");
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
    for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++){
        cpu->gpr[i] = get_gpr(i);
    }
    cpu->pc = get_gpr(32);
    cpu->privilege = reg.privilege;
    cpu->mepc    = reg.mepc;
    cpu->mtvec   = reg.mtvec;
    cpu->mcause  = reg.mcause;
    cpu->mstatus = reg.mstatus;
    cpu->mtval   = reg.mtval;
    cpu->sepc    = reg.sepc;
    cpu->stval   = reg.stval;
    cpu->stvec   = reg.stvec;
    cpu->scause  = reg.scause;
    cpu->satp    = reg.satp;
    cpu->mip     = reg.mip;
    cpu->mie     = reg.mie;
    cpu->mscratch= reg.mscratch;
    cpu->sscratch= reg.sscratch;
    cpu->mideleg = reg.mideleg;
    cpu->medeleg = reg.medeleg;
}
