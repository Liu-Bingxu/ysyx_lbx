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
#include "cpu/decode.h"
#include "cpu/difftest.h"
#include "debug.h"
#include "utils.h"

#define SSTATUS_MASK MUXDEF(CONFIG_RV64, 0x80000003000DE762, 0x800DE762)
#define MSTATUS_SET_BIT MUXDEF(CONFIG_RV64, 0xa00000000, 0x0)
#define MSTATUS_CLR_BIT MUXDEF(CONFIG_RV64, 0x3fff81e655, 0xff81e655)
#define MEDELEG_MASK 0xB1ff
#define MIDELEG_MASK 0x222
#define MIE_MASK     0xAAA
#define MIP_MASK     0x222
#define INT_MASK    MUXDEF(CONFIG_RV64, (0x1UL << 63), (0x1UL << 31))

// static char *interrupt_desc[] = {
//     "Machine_mode external", "Machine_mode software", "Machine_mode time",
//     "Supervisor_mode external", "Supervisor_mode software", "Supervisor_mode time"
// };

int try_isa_raise_intr(struct Decode *s){
    word_t NO = 0;
    word_t pending_interrupts = cpu.mip & cpu.mie;
    if(!pending_interrupts){
        return 1;
    }
    word_t mie = ((cpu.mstatus >> 3) & 0x1);
    word_t m_enabled = (cpu.privilege < PRV_M) || (cpu.privilege == PRV_M && mie);
    word_t enabled_interrupts = ((pending_interrupts) & (~cpu.mideleg) & (-m_enabled));
    if(enabled_interrupts == 0){
        word_t deleg_to_hs = cpu.mideleg;
        word_t sie = ((cpu.mstatus >> 1) & 0x1);
        word_t hs_enabled = (cpu.privilege < PRV_S) || (cpu.privilege == PRV_S && sie);
        enabled_interrupts = pending_interrupts & deleg_to_hs & -hs_enabled;
        if(enabled_interrupts == 0){
            return 1;
        }
    }
    NO |= INT_MASK;
    int interrupt_list[] = {11, 3, 7, 9, 1, 5};
    for (int i = 0; i < ARRLEN(interrupt_list); i++){
        if (enabled_interrupts & (0x1UL << interrupt_list[i])){
            NO |= interrupt_list[i];
            isa_raise_intr(s, NO, s->pc, 0);
            if (ref_difftest_raise_intr)
                ref_difftest_raise_intr(NO);
            difftest_skip_ref();
            // printf("now raise a %s interrupt\n", interrupt_desc[i]);
            // static int cnt = 0;
            // cnt++;
            // if(cnt == 2){
            //     Assert(0, "Hello Bug");
            // }
            return 0;
        }
    }
    Assert(0, "unkown interrupt num\nmip is " FMT_WORD "\nmie is " FMT_WORD, cpu.mip, cpu.mie);
}

void isa_raise_intr(struct Decode *s, word_t NO, vaddr_t epc, word_t tval){
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * Then return the address of the interrupt/exception vector.
     */
    word_t status_temp = cpu.mstatus;

    word_t bits = (NO & (~INT_MASK));
    word_t sdeleg;

    // assert((NO & INT_MASK) == 0);
    if(NO & INT_MASK){
        sdeleg = cpu.mideleg;
    }else{
        sdeleg = cpu.medeleg;
    }

    bool success_flag = false;

    NO = (NO != 11) ? NO : 8 + cpu.privilege;

    IFDEF(CONFIG_ETRACE, Log_func(ANSI_FMT("ENTER the irq in the PC is: ", ANSI_FG_BLUE) ANSI_FMT(FMT_WORD, ANSI_FG_YELLOW) ANSI_FMT(", the macuse is ", ANSI_FG_BLUE) ANSI_FMT(FMT_WORD, ANSI_FG_GREEN) "\n", epc, NO));

    if (NO == 0xb && (isa_reg_str2val("a7", &success_flag)==-1)){
        IFDEF(CONFIG_ETRACE, Log_func(ANSI_FMT("yeild sp is ", ANSI_FG_BLUE) ANSI_FMT(FMT_WORD, ANSI_FG_YELLOW) "\n", (isa_reg_str2val("sp", &success_flag))));
    }
    // printf("%s:%d:enter now is " FMT_WORD " \n", __func__, __LINE__, cpu.mstatus);

    if((cpu.privilege != PRV_M) && ((sdeleg >> bits) & 0x1)){
        //? smode trap
        cpu.sepc        = epc;
        cpu.scause      = NO;
        cpu.stval       = tval;
        s->dnpc         = cpu.stvec;
        cpu.pc          = cpu.stvec;
        cpu.mstatus     = (((cpu.mstatus) & (~(0x0122UL))) | (BITS(status_temp, 1, 1) << 5) | ((cpu.privilege & 0x1) << 8));
        cpu.privilege   = PRV_S;
        // printf("%s:%d:now is " FMT_WORD " \n", __func__,__LINE__,cpu.mstatus);
        return;
    }

    //? mmode trap
    cpu.mepc        = epc;
    cpu.mcause      = NO;
    cpu.mtval       = tval;
    s->dnpc         = cpu.mtvec;
    cpu.pc          = cpu.mtvec;
    cpu.mstatus     = (((cpu.mstatus) & (~(0x01888UL))) | (BITS(status_temp, 3, 3) << 7) | (cpu.privilege << 11));
    cpu.privilege = PRV_M;
    // printf("%s:%d:now is " FMT_WORD " \n", __func__, __LINE__, cpu.mstatus);
}

void mret(struct Decode *s){
    word_t mstatus_temp = cpu.mstatus;
    // printf("%s:%d:enter now is " FMT_WORD " \n", __func__, __LINE__, cpu.mstatus);
    if(cpu.privilege != PRV_M){
        isa_raise_intr(s, 2, s->pc, s->isa.inst.val);
        return;
    }
    cpu.pc = cpu.mepc;
    s->dnpc = cpu.mepc;
    cpu.privilege = ((cpu.mstatus >> 11) & 0x3);
    cpu.mstatus = (((cpu.mstatus) & (~(0x01808UL))) | (0x80));
    if(cpu.privilege != PRV_M)
        cpu.mstatus &= (~(0x1 << 17));
    cpu.mstatus |= (((mstatus_temp >> 7) & 0x1) << 3);
    // printf("%s:%d:now is " FMT_WORD " \n", __func__, __LINE__, cpu.mstatus);
}

void sret(struct Decode *s){
    word_t mstatus_temp = cpu.mstatus;
    // printf("%s:%d:enter now is " FMT_WORD " \n", __func__, __LINE__, cpu.mstatus);
    if(cpu.privilege < PRV_S){
        isa_raise_intr(s, 2, s->pc, s->isa.inst.val);
        return;
    }
    cpu.pc = cpu.sepc;
    s->dnpc = cpu.sepc;
    cpu.privilege = ((cpu.mstatus >> 8) & 0x1);
    cpu.mstatus = (((cpu.mstatus & (~(0x0102UL)) & (~(0x1 << 17))) | (0x20)));
    cpu.mstatus |= (((mstatus_temp >> 5) & 0x1) << 1);
    // printf("%s:%d:now is " FMT_WORD " \n", __func__, __LINE__, cpu.mstatus);
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}

static bool check_csr_addr(word_t csr_num){

    if(cpu.privilege < BITS(csr_num, 9, 8)){
        return false;
    }
    if((csr_num >= 0xC00) && (csr_num <= 0xC1F)){
        int num = csr_num - 0xC00;
        switch (cpu.privilege)
        {
        case PRV_M:
            break;
        case PRV_S:
            if(num == 1){
                return false;
            }
            break;
        case PRV_U:
            return false;
        default:
            panic("error: unkonwn privilege");
        }
    }
#ifndef CONFIG_RV64
    if((csr_num >= 0xC80) && (csr_num <= 0xC8F)){
        int num = csr_num - 0xC80;
        switch (cpu.privilege)
        {
        case PRV_M:
            break;
        case PRV_S:
            if(num == 1){
                return false;
            }
            break;
        case PRV_U:
            return false;
        default:
            panic("error: unkonwn privilege");
        }
    }
#endif
    return true;
} 


#define __PM_NUM_MAP(f,csr,arg) f(csr,3,arg) f(csr,4,arg) f(csr,5,arg) f(csr,6,arg) f(csr,7,arg) f(csr,8,arg) f(csr,9,arg) f(csr,10,arg) \
            f(csr,11,arg) f(csr,12,arg) f(csr,13,arg) f(csr,14,arg) f(csr,15,arg) f(csr,16,arg) f(csr,17,arg) \
            f(csr,18,arg) f(csr,19,arg) f(csr,20,arg) f(csr,21,arg) f(csr,22,arg) f(csr,23,arg) f(csr,24,arg) f(csr,25,arg) \
            f(csr,26,arg) f(csr,27,arg) f(csr,28,arg) f(csr,29,arg) f(csr,30,arg) f(csr,31,arg)
#define PM_USE_NUM(csr,num,f) f(csr##num,csr,[num - 3]) 
#define PM_NUM_MAP(f,csr) __PM_NUM_MAP(PM_USE_NUM,csr,f)

word_t get_csr(word_t csr_num, bool *csr_success){
    csr_num &= 0xfff;
    if(!check_csr_addr(csr_num)){
        *csr_success = false;
        return 0;
    }
    switch (csr_num){
        #define RO_CSR_READ(f) f(mvendorid) f(marchid) f(mimpid) f(mhartid) f(mconfigptr)
        #define RW_CSR_READ(f) f(mstatus) f(mtvec) f(mepc) f(mcause) f(medeleg) f(mideleg) f(mie) f(mip) f(mscratch) f(stvec) f(sepc) f(scause) f(sscratch) f(satp)
        #define RW_BUT_RO_CSR_READ(f) f(misa) f(mcounteren) f(scounteren) f(menvcfg) f(mseccfg) f(senvcfg)
        #define PM_CSR_READ(f) f(cycle, cycle, ) f(instret, instret, ) PM_NUM_MAP(f,hpmcounter)
        #define PM_EVENT_CSR_READ(f) PM_NUM_MAP(f,mhpmevent)

        #define READ_RO_CASE_GEN(csr) case CSR_##csr:difftest_skip_ref();return cpu.csr;
        #define READ_RW_CASE_GEN(csr) case CSR_##csr:return cpu.csr;
        #define READ_PM_CASE_GEN(csr_num, csr, suffix) case CSR_##csr_num:case CSR_m##csr_num:difftest_skip_ref();return cpu.m##csr suffix;
        #define READ_PM_EVENT_CASE_GEN(csr_num, csr, suffix) case CSR_##csr_num:difftest_skip_ref();return cpu.csr suffix;

        RO_CSR_READ(READ_RO_CASE_GEN)
        RW_CSR_READ(READ_RW_CASE_GEN)
        RW_BUT_RO_CSR_READ(READ_RO_CASE_GEN)
        PM_CSR_READ(READ_PM_CASE_GEN)
        PM_EVENT_CSR_READ(READ_PM_EVENT_CASE_GEN)

        case 0x320:difftest_skip_ref();return cpu.mcountinhibit;
        case 0x343:difftest_skip_ref();return cpu.mtval;
        case 0x100:return cpu.mstatus & SSTATUS_MASK;
        case 0x144:return cpu.mip & cpu.mideleg;
        case 0x104:return cpu.mie & cpu.mideleg;
        case 0x143:difftest_skip_ref();return cpu.stval;
#ifndef CONFIG_RV64
        #define RWH_CSR_READ(f) f(mstatush)
        #define RWH_BUT_RO_CSR_READ(f) f(menvcfgh) f(mseccfgh)
        #define PMH_CSR_READ(f) f(cycleh, cycleh, ) f(instreth, instreth, ) PM_NUM_MAP(f,hpmcounterh)
        #define PMH_EVENT_CSR_READ(f) PM_NUM_MAP(f,mhpmeventh)
        
        RWH_CSR_READ(READ_RW_CASE_GEN)
        RWH_BUT_RO_CSR_READ(READ_RO_CASE_GEN)
        PMH_CSR_READ(READ_PM_CASE_GEN)
        PMH_EVENT_CSR_READ(READ_PM_EVENT_CASE_GEN)
#endif
        default:
            // panic("unkown CSR number: " FMT_WORD, csr_num);
            *csr_success = false;
            return 0;
    }
}

void set_csr(word_t csr_num,word_t mask, bool *csr_success){
    csr_num &= 0xfff;
    if (!check_csr_addr(csr_num)){
        *csr_success = false;
        return;
    }
    switch (csr_num){
        #define RW_CSR_SET(f) f(mtvec) f(mepc) f(mcause) f(mscratch) f(stvec) f(sepc) f(scause) f(sscratch) f(mtval) f(stval)
        #define RW_BUT_RO_CSR_SET(f) f(misa) f(mcounteren) f(scounteren) f(menvcfg) f(mseccfg) f(senvcfg)
        #define PM_CSR_SET(f) f(mcycle, mcycle, ) f(minstret, minstret, ) PM_NUM_MAP(f,mhpmcounter) PM_NUM_MAP(f,mhpmevent)

        #define SET_RO_CASE_GEN(csr) case CSR_##csr:difftest_skip_ref();return;
        #define SET_RW_CASE_GEN(csr) case CSR_##csr:cpu.csr|=mask;return;
        #define SET_PM_CASE_GEN(csr_num, csr, suffix) case CSR_##csr_num:difftest_skip_ref();cpu.csr suffix|=mask;return;

        RW_CSR_SET(SET_RW_CASE_GEN)
        RW_BUT_RO_CSR_SET(SET_RO_CASE_GEN)
        PM_CSR_SET(SET_PM_CASE_GEN)
        case 0x300:
            cpu.mstatus |= (mask & (~MSTATUS_CLR_BIT));
            return;
        case 0x302:cpu.medeleg|=(mask&MEDELEG_MASK);return;
        case 0x303:cpu.mideleg|=(mask&MIDELEG_MASK);return;
        case 0x344:
            // printf("set mip with " FMT_WORD "\n", mask);
            cpu.mip |= (mask & MIP_MASK);
            return;
        case 0x304:
            // printf("set mie with " FMT_WORD "\n", mask);
            cpu.mie |= (mask & MIE_MASK);
            return;
        case 0x320:cpu.mcountinhibit|=(mask&(~0x2));   return;
        case 0x100:
            cpu.mstatus |= ((mask & SSTATUS_MASK) & (~MSTATUS_CLR_BIT));
            return;
        case 0x144:cpu.mip|=(mask&cpu.mideleg);return;
        case 0x104:cpu.mie|=(mask&cpu.mideleg);return;
        case 0x180:
#ifdef CONFIG_RV64
            if ((BITS((mask | cpu.satp), 63, 60) != 0) && ((BITS((mask | cpu.satp), 63, 60) != 8))){
                //! only can set no mmu and RV39
                difftest_skip_ref();
                return;
            }
#endif
            cpu.satp |= mask; 
            return;
#ifndef CONFIG_RV64
        #define RWH_BUT_RO_CSR_SET(f) f(menvcfgh) f(mseccfgh)
        #define PMH_CSR_SET(f) f(mcycleh, mcycleh, ) f(minstreth, minstreth, ) PM_NUM_MAP(f,mhpmcounterh) PM_NUM_MAP(f,mhpmeventh)
        
        RWH_BUT_RO_CSR_SET(SET_RO_CASE_GEN)
        PMH_CSR_SET(SET_PM_CASE_GEN)
        case 0x310:
            return;
#endif
        default:
            // panic("unkown CSR number: " FMT_WORD, csr_num);
            *csr_success = false;
            return;
    }
}

void clr_csr(word_t csr_num, word_t mask, bool *csr_success){
    csr_num &= 0xfff;
    if (!check_csr_addr(csr_num)){
        *csr_success = false;
        return;
    }
    mask = ~mask;
    switch (csr_num){
        #define RW_CSR_CLR(f) f(mtvec) f(mepc) f(mcause) f(mscratch) f(stvec) f(sepc) f(scause) f(sscratch) f(mtval) f(stval)
        #define RW_BUT_RO_CSR_CLR(f) f(misa) f(mcounteren) f(scounteren) f(menvcfg) f(mseccfg) f(senvcfg)
        #define PM_CSR_CLR(f) f(mcycle, mcycle, ) f(minstret, minstret, ) PM_NUM_MAP(f,mhpmcounter) PM_NUM_MAP(f,mhpmevent)

        #define CLR_RO_CASE_GEN(csr) case CSR_##csr:difftest_skip_ref();return;
        #define CLR_RW_CASE_GEN(csr) case CSR_##csr:cpu.csr&=mask;return;
        #define CLR_PM_CASE_GEN(csr_num, csr, suffix) case CSR_##csr_num:difftest_skip_ref();cpu.csr suffix&=mask;return;

        RW_CSR_CLR(CLR_RW_CASE_GEN)
        RW_BUT_RO_CSR_CLR(CLR_RO_CASE_GEN)
        PM_CSR_CLR(CLR_PM_CASE_GEN)
        case 0x300:
            cpu.mstatus &= (mask | MSTATUS_SET_BIT);
            return;
        case 0x302:cpu.medeleg&=(mask|(~MEDELEG_MASK));return;
        case 0x303:cpu.mideleg&=(mask|(~MIDELEG_MASK));return;
        case 0x344:
            // printf("clear mip with " FMT_WORD "\n", mask);
            cpu.mip &= (mask | (~MIP_MASK));
            return;
        case 0x304:
            // printf("clear mie with " FMT_WORD "\n", mask);
            cpu.mie&=(mask|(~MIE_MASK));  
        return;
        case 0x320:cpu.mcountinhibit&=mask;   return;
        case 0x100:
            cpu.mstatus &= ((mask | (~SSTATUS_MASK)) | MSTATUS_SET_BIT);
            return;
        case 0x144:cpu.mip&=(mask|(~cpu.mideleg));return;
        case 0x104:cpu.mie&=(mask|(~cpu.mideleg));return;
        case 0x180:
#ifdef CONFIG_RV64
            if ((BITS((mask & cpu.satp), 63, 60) != 0) && ((BITS((mask & cpu.satp), 63, 60) != 8))){
                //! only can set no mmu and RV39
                difftest_skip_ref();
                return;
            }
#endif
            cpu.satp &= mask; 
            return;
#ifndef CONFIG_RV64
        #define RWH_BUT_RO_CSR_CLR(f) f(menvcfgh) f(mseccfgh)
        #define PMH_CSR_CLR(f) f(mcycleh, mcycleh, ) f(minstreth, minstreth, ) PM_NUM_MAP(f,mhpmcounterh) PM_NUM_MAP(f,mhpmeventh)
        
        RWH_BUT_RO_CSR_CLR(CLR_RO_CASE_GEN)
        PMH_CSR_CLR(CLR_PM_CASE_GEN)
        case 0x310:
            return;
#endif
        default:
            // panic("unkown CSR number: " FMT_WORD, csr_num);
            *csr_success = false;
            return;
    }
}

void wirte_csr(word_t csr_num,word_t num, bool *csr_success){
    word_t mask = num;
    csr_num &= 0xfff;
    if (!check_csr_addr(csr_num)){
        *csr_success = false;
        return;
    }
    switch (csr_num){
        #define RW_CSR_WIRTE(f) f(mtvec) f(mepc) f(mcause) f(mscratch) f(stvec) f(sepc) f(scause) f(sscratch) f(mtval) f(stval)
        #define RW_BUT_RO_CSR_WIRTE(f) f(misa) f(mcounteren) f(scounteren) f(menvcfg) f(mseccfg) f(senvcfg)
        #define PM_CSR_WIRTE(f) f(mcycle, mcycle, ) f(minstret, minstret, ) PM_NUM_MAP(f,mhpmcounter) PM_NUM_MAP(f,mhpmevent)

        #define WIRTE_RO_CASE_GEN(csr) case CSR_##csr:difftest_skip_ref();return;
        #define WIRTE_RW_CASE_GEN(csr) case CSR_##csr:cpu.csr = mask;return;
        #define WIRTE_PM_CASE_GEN(csr_num, csr, suffix) case CSR_##csr_num:difftest_skip_ref();cpu.csr suffix = mask;return;

        RW_CSR_WIRTE(WIRTE_RW_CASE_GEN)
        RW_BUT_RO_CSR_WIRTE(WIRTE_RO_CASE_GEN)
        PM_CSR_WIRTE(WIRTE_PM_CASE_GEN)
        case 0x300:
            cpu.mstatus = mask;
            cpu.mstatus &= (~MSTATUS_CLR_BIT);
            cpu.mstatus |= MSTATUS_SET_BIT;
            return;
        case 0x302:cpu.medeleg=(mask&MEDELEG_MASK);return;
        case 0x303:cpu.mideleg=(mask&MIDELEG_MASK);return;
        case 0x344:cpu.mip=(mask&MIP_MASK);  return;
        case 0x304:cpu.mie=(mask&MIE_MASK);  return;
        case 0x320:cpu.mcountinhibit=(mask&(~0x2));   return;
        case 0x100:
            cpu.mstatus &= (~SSTATUS_MASK);
            cpu.mstatus |= (mask & SSTATUS_MASK);
            cpu.mstatus &= (~MSTATUS_CLR_BIT);
            cpu.mstatus |= MSTATUS_SET_BIT;
            return;
        case 0x144:
            cpu.mip &= (~cpu.mideleg);
            cpu.mip |= (mask & cpu.mideleg);
            return;
        case 0x104:
            cpu.mie &= (~cpu.mideleg);
            cpu.mie |= (mask & cpu.mideleg);
            return;
        case 0x180:
#ifdef CONFIG_RV64
            if ((BITS(mask, 63, 60) != 0) && ((BITS(mask, 63, 60) != 8))){
                //! only can set no mmu and RV39
                difftest_skip_ref();
                return;
            }
#endif
            cpu.satp=mask;            
        return;
#ifndef CONFIG_RV64
        #define RWH_BUT_RO_CSR_WIRTE(f) f(menvcfgh) f(mseccfgh)
        #define PMH_CSR_WIRTE(f) f(mcycleh, mcycleh, ) f(minstreth, minstreth, ) PM_NUM_MAP(f,mhpmcounterh) PM_NUM_MAP(f,mhpmeventh)

        RWH_BUT_RO_CSR_WIRTE(WIRTE_RO_CASE_GEN)
        PMH_CSR_WIRTE(WIRTE_PM_CASE_GEN)
        case 0x310:
            return;
#endif
        default:
            // panic("unkown CSR number: " FMT_WORD, csr_num);
            *csr_success = false;
            return;
    }
}
