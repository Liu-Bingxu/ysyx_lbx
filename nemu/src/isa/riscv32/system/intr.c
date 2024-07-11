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

word_t get_csr(word_t csr_num, bool *csr_success){
    csr_num &= 0xfff;
    if(!check_csr_addr(csr_num)){
        *csr_success = false;
        return 0;
    }
    switch (csr_num){
    case 0x301:difftest_skip_ref();return cpu.misa;
    case 0xF11:difftest_skip_ref();return cpu.mvendorid;
    case 0xF12:difftest_skip_ref();return cpu.marchid;
    case 0xF13:difftest_skip_ref();return cpu.mimpid;
    case 0xF14:difftest_skip_ref();return cpu.mhartid;
    case 0x300:return cpu.mstatus;
    case 0x305:return cpu.mtvec;
    case 0x341:return cpu.mepc;
    case 0x342:return cpu.mcause;
    case 0x302:return cpu.medeleg;
    case 0x303:return cpu.mideleg;
    case 0x344:return cpu.mip;
    case 0x304:return cpu.mie;
    case 0xB00:case 0xC00:return cpu.mcycle;
    case 0xB02:case 0xC02:return cpu.minstret;
    case 0xB03:case 0xC03:difftest_skip_ref();return cpu.mhpmcounter[0];
    case 0xB04:case 0xC04:difftest_skip_ref();return cpu.mhpmcounter[1];
    case 0xB05:case 0xC05:difftest_skip_ref();return cpu.mhpmcounter[2];
    case 0xB06:case 0xC06:difftest_skip_ref();return cpu.mhpmcounter[3];
    case 0xB07:case 0xC07:difftest_skip_ref();return cpu.mhpmcounter[4];
    case 0xB08:case 0xC08:difftest_skip_ref();return cpu.mhpmcounter[5];
    case 0xB09:case 0xC09:difftest_skip_ref();return cpu.mhpmcounter[6];
    case 0xB0A:case 0xC0A:difftest_skip_ref();return cpu.mhpmcounter[7];
    case 0xB0B:case 0xC0B:difftest_skip_ref();return cpu.mhpmcounter[8];
    case 0xB0C:case 0xC0C:difftest_skip_ref();return cpu.mhpmcounter[9];
    case 0xB0D:case 0xC0D:difftest_skip_ref();return cpu.mhpmcounter[10];
    case 0xB0E:case 0xC0E:difftest_skip_ref();return cpu.mhpmcounter[11];
    case 0xB0F:case 0xC0F:difftest_skip_ref();return cpu.mhpmcounter[12];
    case 0xB10:case 0xC10:difftest_skip_ref();return cpu.mhpmcounter[13];
    case 0xB11:case 0xC11:difftest_skip_ref();return cpu.mhpmcounter[14];
    case 0xB12:case 0xC12:difftest_skip_ref();return cpu.mhpmcounter[15];
    case 0xB13:case 0xC13:difftest_skip_ref();return cpu.mhpmcounter[16];
    case 0xB14:case 0xC14:difftest_skip_ref();return cpu.mhpmcounter[17];
    case 0xB15:case 0xC15:difftest_skip_ref();return cpu.mhpmcounter[18];
    case 0xB16:case 0xC16:difftest_skip_ref();return cpu.mhpmcounter[19];
    case 0xB17:case 0xC17:difftest_skip_ref();return cpu.mhpmcounter[20];
    case 0xB18:case 0xC18:difftest_skip_ref();return cpu.mhpmcounter[21];
    case 0xB19:case 0xC19:difftest_skip_ref();return cpu.mhpmcounter[22];
    case 0xB1A:case 0xC1A:difftest_skip_ref();return cpu.mhpmcounter[23];
    case 0xB1B:case 0xC1B:difftest_skip_ref();return cpu.mhpmcounter[24];
    case 0xB1C:case 0xC1C:difftest_skip_ref();return cpu.mhpmcounter[25];
    case 0xB1D:case 0xC1D:difftest_skip_ref();return cpu.mhpmcounter[26];
    case 0xB1E:case 0xC1E:difftest_skip_ref();return cpu.mhpmcounter[27];
    case 0xB1F:case 0xC1F:difftest_skip_ref();return cpu.mhpmcounter[28];
    case 0x323:return cpu.mhpmevent[0];
    case 0x324:return cpu.mhpmevent[1];
    case 0x325:return cpu.mhpmevent[2];
    case 0x326:return cpu.mhpmevent[3];
    case 0x327:return cpu.mhpmevent[4];
    case 0x328:return cpu.mhpmevent[5];
    case 0x329:return cpu.mhpmevent[6];
    case 0x32A:return cpu.mhpmevent[7];
    case 0x32B:return cpu.mhpmevent[8];
    case 0x32C:return cpu.mhpmevent[9];
    case 0x32D:return cpu.mhpmevent[10];
    case 0x32E:return cpu.mhpmevent[11];
    case 0x32F:return cpu.mhpmevent[12];
    case 0x330:return cpu.mhpmevent[13];
    case 0x331:return cpu.mhpmevent[14];
    case 0x332:return cpu.mhpmevent[15];
    case 0x333:return cpu.mhpmevent[16];
    case 0x334:return cpu.mhpmevent[17];
    case 0x335:return cpu.mhpmevent[18];
    case 0x336:return cpu.mhpmevent[19];
    case 0x337:return cpu.mhpmevent[20];
    case 0x338:return cpu.mhpmevent[21];
    case 0x339:return cpu.mhpmevent[22];
    case 0x33A:return cpu.mhpmevent[23];
    case 0x33B:return cpu.mhpmevent[24];
    case 0x33C:return cpu.mhpmevent[25];
    case 0x33D:return cpu.mhpmevent[26];
    case 0x33E:return cpu.mhpmevent[27];
    case 0x33F:return cpu.mhpmevent[28];
    case 0x306:
        //read but ignore the read value
        if (ref_difftest_exec)
            ref_difftest_exec(1);
        difftest_skip_ref();
        return cpu.mcounteren;
    case 0x320:difftest_skip_ref();return cpu.mcountinhibit;
    case 0x340:return cpu.mscratch;
    case 0x343:difftest_skip_ref();return cpu.mtval;
    case 0xF15:return cpu.mconfigptr;
    case 0x30A:return cpu.menvcfg;
    case 0x747:return cpu.mseccfg;
    case 0x100:return cpu.mstatus & SSTATUS_MASK;
    case 0x105:return cpu.stvec;
    case 0x144:return cpu.mip & cpu.mideleg;
    case 0x104:return cpu.mie & cpu.mideleg;
    case 0x106:return cpu.scounteren;
    case 0x140:return cpu.sscratch;
    case 0x141:return cpu.sepc;
    case 0x142:return cpu.scause;
    case 0x143:difftest_skip_ref();return cpu.stval;
    case 0x10A:return cpu.senvcfg;
    case 0x180:return cpu.satp;
#ifndef CONFIG_RV64
    case 0x310:return cpu.mstatush;
    case 0x31A:return cpu.menvcfgh;
    case 0x757:return cpu.mseccfgh;
    case 0xB80:case 0xC80:return cpu.mcycleh;
    case 0xB82:case 0xC82:return cpu.minstreth;
    case 0xB83:case 0xC83:return cpu.mhpmcounterh[0];
    case 0xB84:case 0xC84:return cpu.mhpmcounterh[1];
    case 0xB85:case 0xC85:return cpu.mhpmcounterh[2];
    case 0xB86:case 0xC86:return cpu.mhpmcounterh[3];
    case 0xB87:case 0xC87:return cpu.mhpmcounterh[4];
    case 0xB88:case 0xC88:return cpu.mhpmcounterh[5];
    case 0xB89:case 0xC89:return cpu.mhpmcounterh[6];
    case 0xB8A:case 0xC8A:return cpu.mhpmcounterh[7];
    case 0xB8B:case 0xC8B:return cpu.mhpmcounterh[8];
    case 0xB8C:case 0xC8C:return cpu.mhpmcounterh[9];
    case 0xB8D:case 0xC8D:return cpu.mhpmcounterh[10];
    case 0xB8E:case 0xC8E:return cpu.mhpmcounterh[11];
    case 0xB8F:case 0xC8F:return cpu.mhpmcounterh[12];
    case 0xB90:case 0xC90:return cpu.mhpmcounterh[13];
    case 0xB91:case 0xC91:return cpu.mhpmcounterh[14];
    case 0xB92:case 0xC92:return cpu.mhpmcounterh[15];
    case 0xB93:case 0xC93:return cpu.mhpmcounterh[16];
    case 0xB94:case 0xC94:return cpu.mhpmcounterh[17];
    case 0xB95:case 0xC95:return cpu.mhpmcounterh[18];
    case 0xB96:case 0xC96:return cpu.mhpmcounterh[19];
    case 0xB97:case 0xC97:return cpu.mhpmcounterh[20];
    case 0xB98:case 0xC98:return cpu.mhpmcounterh[21];
    case 0xB99:case 0xC99:return cpu.mhpmcounterh[22];
    case 0xB9A:case 0xC9A:return cpu.mhpmcounterh[23];
    case 0xB9B:case 0xC9B:return cpu.mhpmcounterh[24];
    case 0xB9C:case 0xC9C:return cpu.mhpmcounterh[25];
    case 0xB9D:case 0xC9D:return cpu.mhpmcounterh[26];
    case 0xB9E:case 0xC9E:return cpu.mhpmcounterh[27];
    case 0xB9F:case 0xC9F:return cpu.mhpmcounterh[28];
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
    case 0x300:
        cpu.mstatus |= (mask & (~MSTATUS_CLR_BIT));
        return;
    case 0x305:cpu.mtvec|=mask;     return;
    case 0x341:cpu.mepc|=mask;      return;
    case 0x342:cpu.mcause|=mask;    return;
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
    case 0xB00:cpu.mcycle|=mask;          return;
    case 0xB02:cpu.minstret|=mask;        return;
    case 0xB03:cpu.mhpmcounter[0]|=mask;  return;
    case 0xB04:cpu.mhpmcounter[1]|=mask;  return;
    case 0xB05:cpu.mhpmcounter[2]|=mask;  return;
    case 0xB06:cpu.mhpmcounter[3]|=mask;  return;
    case 0xB07:cpu.mhpmcounter[4]|=mask;  return;
    case 0xB08:cpu.mhpmcounter[5]|=mask;  return;
    case 0xB09:cpu.mhpmcounter[6]|=mask;  return;
    case 0xB0A:cpu.mhpmcounter[7]|=mask;  return;
    case 0xB0B:cpu.mhpmcounter[8]|=mask;  return;
    case 0xB0C:cpu.mhpmcounter[9]|=mask;  return;
    case 0xB0D:cpu.mhpmcounter[10]|=mask; return;
    case 0xB0E:cpu.mhpmcounter[11]|=mask; return;
    case 0xB0F:cpu.mhpmcounter[12]|=mask; return;
    case 0xB10:cpu.mhpmcounter[13]|=mask; return;
    case 0xB11:cpu.mhpmcounter[14]|=mask; return;
    case 0xB12:cpu.mhpmcounter[15]|=mask; return;
    case 0xB13:cpu.mhpmcounter[16]|=mask; return;
    case 0xB14:cpu.mhpmcounter[17]|=mask; return;
    case 0xB15:cpu.mhpmcounter[18]|=mask; return;
    case 0xB16:cpu.mhpmcounter[19]|=mask; return;
    case 0xB17:cpu.mhpmcounter[20]|=mask; return;
    case 0xB18:cpu.mhpmcounter[21]|=mask; return;
    case 0xB19:cpu.mhpmcounter[22]|=mask; return;
    case 0xB1A:cpu.mhpmcounter[23]|=mask; return;
    case 0xB1B:cpu.mhpmcounter[24]|=mask; return;
    case 0xB1C:cpu.mhpmcounter[25]|=mask; return;
    case 0xB1D:cpu.mhpmcounter[26]|=mask; return;
    case 0xB1E:cpu.mhpmcounter[27]|=mask; return;
    case 0xB1F:cpu.mhpmcounter[28]|=mask; return;
    case 0x323:cpu.mhpmevent[0]|=mask;    return;
    case 0x324:cpu.mhpmevent[1]|=mask;    return;
    case 0x325:cpu.mhpmevent[2]|=mask;    return;
    case 0x326:cpu.mhpmevent[3]|=mask;    return;
    case 0x327:cpu.mhpmevent[4]|=mask;    return;
    case 0x328:cpu.mhpmevent[5]|=mask;    return;
    case 0x329:cpu.mhpmevent[6]|=mask;    return;
    case 0x32A:cpu.mhpmevent[7]|=mask;    return;
    case 0x32B:cpu.mhpmevent[8]|=mask;    return;
    case 0x32C:cpu.mhpmevent[9]|=mask;    return;
    case 0x32D:cpu.mhpmevent[10]|=mask;   return;
    case 0x32E:cpu.mhpmevent[11]|=mask;   return;
    case 0x32F:cpu.mhpmevent[12]|=mask;   return;
    case 0x330:cpu.mhpmevent[13]|=mask;   return;
    case 0x331:cpu.mhpmevent[14]|=mask;   return;
    case 0x332:cpu.mhpmevent[15]|=mask;   return;
    case 0x333:cpu.mhpmevent[16]|=mask;   return;
    case 0x334:cpu.mhpmevent[17]|=mask;   return;
    case 0x335:cpu.mhpmevent[18]|=mask;   return;
    case 0x336:cpu.mhpmevent[19]|=mask;   return;
    case 0x337:cpu.mhpmevent[20]|=mask;   return;
    case 0x338:cpu.mhpmevent[21]|=mask;   return;
    case 0x339:cpu.mhpmevent[22]|=mask;   return;
    case 0x33A:cpu.mhpmevent[23]|=mask;   return;
    case 0x33B:cpu.mhpmevent[24]|=mask;   return;
    case 0x33C:cpu.mhpmevent[25]|=mask;   return;
    case 0x33D:cpu.mhpmevent[26]|=mask;   return;
    case 0x33E:cpu.mhpmevent[27]|=mask;   return;
    case 0x33F:cpu.mhpmevent[28]|=mask;   return;
    case 0x320:cpu.mcountinhibit|=(mask&(~0x2));   return;
    case 0x340:cpu.mscratch|=mask;        return;
    case 0x343:cpu.mtval|=mask;           return;
    case 0x100:
        cpu.mstatus |= ((mask & SSTATUS_MASK) & (~MSTATUS_CLR_BIT));
        return;
    case 0x105:cpu.stvec|=mask;           return;
    case 0x144:cpu.mip|=(mask&cpu.mideleg);return;
    case 0x104:cpu.mie|=(mask&cpu.mideleg);return;
    case 0x140:cpu.sscratch|=mask;        return;
    case 0x141:cpu.sepc|=mask;            return;
    case 0x142:cpu.scause|=mask;          return;
    case 0x143:cpu.stval|=mask;           return;
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
    case 0xB80:cpu.mcycleh|=mask;         return;
    case 0xB82:cpu.minstreth|=mask;       return;
    case 0xB83:cpu.mhpmcounterh[0]|=mask; return;
    case 0xB84:cpu.mhpmcounterh[1]|=mask; return;
    case 0xB85:cpu.mhpmcounterh[2]|=mask; return;
    case 0xB86:cpu.mhpmcounterh[3]|=mask; return;
    case 0xB87:cpu.mhpmcounterh[4]|=mask; return;
    case 0xB88:cpu.mhpmcounterh[5]|=mask; return;
    case 0xB89:cpu.mhpmcounterh[6]|=mask; return;
    case 0xB8A:cpu.mhpmcounterh[7]|=mask; return;
    case 0xB8B:cpu.mhpmcounterh[8]|=mask; return;
    case 0xB8C:cpu.mhpmcounterh[9]|=mask; return;
    case 0xB8D:cpu.mhpmcounterh[10]|=mask;return;
    case 0xB8E:cpu.mhpmcounterh[11]|=mask;return;
    case 0xB8F:cpu.mhpmcounterh[12]|=mask;return;
    case 0xB90:cpu.mhpmcounterh[13]|=mask;return;
    case 0xB91:cpu.mhpmcounterh[14]|=mask;return;
    case 0xB92:cpu.mhpmcounterh[15]|=mask;return;
    case 0xB93:cpu.mhpmcounterh[16]|=mask;return;
    case 0xB94:cpu.mhpmcounterh[17]|=mask;return;
    case 0xB95:cpu.mhpmcounterh[18]|=mask;return;
    case 0xB96:cpu.mhpmcounterh[19]|=mask;return;
    case 0xB97:cpu.mhpmcounterh[20]|=mask;return;
    case 0xB98:cpu.mhpmcounterh[21]|=mask;return;
    case 0xB99:cpu.mhpmcounterh[22]|=mask;return;
    case 0xB9A:cpu.mhpmcounterh[23]|=mask;return;
    case 0xB9B:cpu.mhpmcounterh[24]|=mask;return;
    case 0xB9C:cpu.mhpmcounterh[25]|=mask;return;
    case 0xB9D:cpu.mhpmcounterh[26]|=mask;return;
    case 0xB9E:cpu.mhpmcounterh[27]|=mask;return;
    case 0xB9F:cpu.mhpmcounterh[28]|=mask;return;
#endif
    case 0x301:case 0x30A:case 0x747:case 0x10A:case 0x306:case 0x106:
#ifndef CONFIG_RV64
    case 0x757:case 0x31A:case 0x310:
#endif
        return;
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
    case 0x300:
        cpu.mstatus &= (mask | MSTATUS_SET_BIT);
        return;
    case 0x305:cpu.mtvec&=mask;     return;
    case 0x341:cpu.mepc&=mask;      return;
    case 0x342:cpu.mcause&=mask;    return;
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
    case 0xB00:cpu.mcycle&=mask;          return;
    case 0xB02:cpu.minstret&=mask;        return;
    case 0xB03:cpu.mhpmcounter[0]&=mask;  return;
    case 0xB04:cpu.mhpmcounter[1]&=mask;  return;
    case 0xB05:cpu.mhpmcounter[2]&=mask;  return;
    case 0xB06:cpu.mhpmcounter[3]&=mask;  return;
    case 0xB07:cpu.mhpmcounter[4]&=mask;  return;
    case 0xB08:cpu.mhpmcounter[5]&=mask;  return;
    case 0xB09:cpu.mhpmcounter[6]&=mask;  return;
    case 0xB0A:cpu.mhpmcounter[7]&=mask;  return;
    case 0xB0B:cpu.mhpmcounter[8]&=mask;  return;
    case 0xB0C:cpu.mhpmcounter[9]&=mask;  return;
    case 0xB0D:cpu.mhpmcounter[10]&=mask; return;
    case 0xB0E:cpu.mhpmcounter[11]&=mask; return;
    case 0xB0F:cpu.mhpmcounter[12]&=mask; return;
    case 0xB10:cpu.mhpmcounter[13]&=mask; return;
    case 0xB11:cpu.mhpmcounter[14]&=mask; return;
    case 0xB12:cpu.mhpmcounter[15]&=mask; return;
    case 0xB13:cpu.mhpmcounter[16]&=mask; return;
    case 0xB14:cpu.mhpmcounter[17]&=mask; return;
    case 0xB15:cpu.mhpmcounter[18]&=mask; return;
    case 0xB16:cpu.mhpmcounter[19]&=mask; return;
    case 0xB17:cpu.mhpmcounter[20]&=mask; return;
    case 0xB18:cpu.mhpmcounter[21]&=mask; return;
    case 0xB19:cpu.mhpmcounter[22]&=mask; return;
    case 0xB1A:cpu.mhpmcounter[23]&=mask; return;
    case 0xB1B:cpu.mhpmcounter[24]&=mask; return;
    case 0xB1C:cpu.mhpmcounter[25]&=mask; return;
    case 0xB1D:cpu.mhpmcounter[26]&=mask; return;
    case 0xB1E:cpu.mhpmcounter[27]&=mask; return;
    case 0xB1F:cpu.mhpmcounter[28]&=mask; return;
    case 0x323:cpu.mhpmevent[0]&=mask;    return;
    case 0x324:cpu.mhpmevent[1]&=mask;    return;
    case 0x325:cpu.mhpmevent[2]&=mask;    return;
    case 0x326:cpu.mhpmevent[3]&=mask;    return;
    case 0x327:cpu.mhpmevent[4]&=mask;    return;
    case 0x328:cpu.mhpmevent[5]&=mask;    return;
    case 0x329:cpu.mhpmevent[6]&=mask;    return;
    case 0x32A:cpu.mhpmevent[7]&=mask;    return;
    case 0x32B:cpu.mhpmevent[8]&=mask;    return;
    case 0x32C:cpu.mhpmevent[9]&=mask;    return;
    case 0x32D:cpu.mhpmevent[10]&=mask;   return;
    case 0x32E:cpu.mhpmevent[11]&=mask;   return;
    case 0x32F:cpu.mhpmevent[12]&=mask;   return;
    case 0x330:cpu.mhpmevent[13]&=mask;   return;
    case 0x331:cpu.mhpmevent[14]&=mask;   return;
    case 0x332:cpu.mhpmevent[15]&=mask;   return;
    case 0x333:cpu.mhpmevent[16]&=mask;   return;
    case 0x334:cpu.mhpmevent[17]&=mask;   return;
    case 0x335:cpu.mhpmevent[18]&=mask;   return;
    case 0x336:cpu.mhpmevent[19]&=mask;   return;
    case 0x337:cpu.mhpmevent[20]&=mask;   return;
    case 0x338:cpu.mhpmevent[21]&=mask;   return;
    case 0x339:cpu.mhpmevent[22]&=mask;   return;
    case 0x33A:cpu.mhpmevent[23]&=mask;   return;
    case 0x33B:cpu.mhpmevent[24]&=mask;   return;
    case 0x33C:cpu.mhpmevent[25]&=mask;   return;
    case 0x33D:cpu.mhpmevent[26]&=mask;   return;
    case 0x33E:cpu.mhpmevent[27]&=mask;   return;
    case 0x33F:cpu.mhpmevent[28]&=mask;   return;
    case 0x320:cpu.mcountinhibit&=mask;   return;
    case 0x340:cpu.mscratch&=mask;        return;
    case 0x343:cpu.mtval&=mask;           return;
    case 0x100:
        cpu.mstatus &= ((mask | (~SSTATUS_MASK)) | MSTATUS_SET_BIT);
        return;
    case 0x105:cpu.stvec&=mask;           return;
    case 0x144:cpu.mip&=(mask|(~cpu.mideleg));return;
    case 0x104:cpu.mie&=(mask|(~cpu.mideleg));return;
    case 0x140:cpu.sscratch&=mask;        return;
    case 0x141:cpu.sepc&=mask;            return;
    case 0x142:cpu.scause&=mask;          return;
    case 0x143:cpu.stval&=mask;           return;
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
    case 0xB80:cpu.mcycleh&=mask;         return;
    case 0xB82:cpu.minstreth&=mask;       return;
    case 0xB83:cpu.mhpmcounterh[0]&=mask; return;
    case 0xB84:cpu.mhpmcounterh[1]&=mask; return;
    case 0xB85:cpu.mhpmcounterh[2]&=mask; return;
    case 0xB86:cpu.mhpmcounterh[3]&=mask; return;
    case 0xB87:cpu.mhpmcounterh[4]&=mask; return;
    case 0xB88:cpu.mhpmcounterh[5]&=mask; return;
    case 0xB89:cpu.mhpmcounterh[6]&=mask; return;
    case 0xB8A:cpu.mhpmcounterh[7]&=mask; return;
    case 0xB8B:cpu.mhpmcounterh[8]&=mask; return;
    case 0xB8C:cpu.mhpmcounterh[9]&=mask; return;
    case 0xB8D:cpu.mhpmcounterh[10]&=mask;return;
    case 0xB8E:cpu.mhpmcounterh[11]&=mask;return;
    case 0xB8F:cpu.mhpmcounterh[12]&=mask;return;
    case 0xB90:cpu.mhpmcounterh[13]&=mask;return;
    case 0xB91:cpu.mhpmcounterh[14]&=mask;return;
    case 0xB92:cpu.mhpmcounterh[15]&=mask;return;
    case 0xB93:cpu.mhpmcounterh[16]&=mask;return;
    case 0xB94:cpu.mhpmcounterh[17]&=mask;return;
    case 0xB95:cpu.mhpmcounterh[18]&=mask;return;
    case 0xB96:cpu.mhpmcounterh[19]&=mask;return;
    case 0xB97:cpu.mhpmcounterh[20]&=mask;return;
    case 0xB98:cpu.mhpmcounterh[21]&=mask;return;
    case 0xB99:cpu.mhpmcounterh[22]&=mask;return;
    case 0xB9A:cpu.mhpmcounterh[23]&=mask;return;
    case 0xB9B:cpu.mhpmcounterh[24]&=mask;return;
    case 0xB9C:cpu.mhpmcounterh[25]&=mask;return;
    case 0xB9D:cpu.mhpmcounterh[26]&=mask;return;
    case 0xB9E:cpu.mhpmcounterh[27]&=mask;return;
    case 0xB9F:cpu.mhpmcounterh[28]&=mask;return;
#endif
    case 0x301:case 0x30A:case 0x747:case 0x10A:case 0x306:case 0x106:
#ifndef CONFIG_RV64
    case 0x757:case 0x31A:case 0x310:
#endif
        return;
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
    case 0x300:
        cpu.mstatus = mask;
        cpu.mstatus &= (~MSTATUS_CLR_BIT);
        cpu.mstatus |= MSTATUS_SET_BIT;
        return;
    case 0x305:cpu.mtvec=mask;     return;
    case 0x341:cpu.mepc=mask;      return;
    case 0x342:cpu.mcause=mask;    return;
    case 0x302:cpu.medeleg=(mask&MEDELEG_MASK);return;
    case 0x303:cpu.mideleg=(mask&MIDELEG_MASK);return;
    case 0x344:cpu.mip=(mask&MIP_MASK);  return;
    case 0x304:cpu.mie=(mask&MIE_MASK);  return;
    case 0xB00:cpu.mcycle=mask;          return;
    case 0xB02:cpu.minstret=mask;        return;
    case 0xB03:cpu.mhpmcounter[0]=mask;  return;
    case 0xB04:cpu.mhpmcounter[1]=mask;  return;
    case 0xB05:cpu.mhpmcounter[2]=mask;  return;
    case 0xB06:cpu.mhpmcounter[3]=mask;  return;
    case 0xB07:cpu.mhpmcounter[4]=mask;  return;
    case 0xB08:cpu.mhpmcounter[5]=mask;  return;
    case 0xB09:cpu.mhpmcounter[6]=mask;  return;
    case 0xB0A:cpu.mhpmcounter[7]=mask;  return;
    case 0xB0B:cpu.mhpmcounter[8]=mask;  return;
    case 0xB0C:cpu.mhpmcounter[9]=mask;  return;
    case 0xB0D:cpu.mhpmcounter[10]=mask; return;
    case 0xB0E:cpu.mhpmcounter[11]=mask; return;
    case 0xB0F:cpu.mhpmcounter[12]=mask; return;
    case 0xB10:cpu.mhpmcounter[13]=mask; return;
    case 0xB11:cpu.mhpmcounter[14]=mask; return;
    case 0xB12:cpu.mhpmcounter[15]=mask; return;
    case 0xB13:cpu.mhpmcounter[16]=mask; return;
    case 0xB14:cpu.mhpmcounter[17]=mask; return;
    case 0xB15:cpu.mhpmcounter[18]=mask; return;
    case 0xB16:cpu.mhpmcounter[19]=mask; return;
    case 0xB17:cpu.mhpmcounter[20]=mask; return;
    case 0xB18:cpu.mhpmcounter[21]=mask; return;
    case 0xB19:cpu.mhpmcounter[22]=mask; return;
    case 0xB1A:cpu.mhpmcounter[23]=mask; return;
    case 0xB1B:cpu.mhpmcounter[24]=mask; return;
    case 0xB1C:cpu.mhpmcounter[25]=mask; return;
    case 0xB1D:cpu.mhpmcounter[26]=mask; return;
    case 0xB1E:cpu.mhpmcounter[27]=mask; return;
    case 0xB1F:cpu.mhpmcounter[28]=mask; return;
    case 0x323:cpu.mhpmevent[0]=mask;    return;
    case 0x324:cpu.mhpmevent[1]=mask;    return;
    case 0x325:cpu.mhpmevent[2]=mask;    return;
    case 0x326:cpu.mhpmevent[3]=mask;    return;
    case 0x327:cpu.mhpmevent[4]=mask;    return;
    case 0x328:cpu.mhpmevent[5]=mask;    return;
    case 0x329:cpu.mhpmevent[6]=mask;    return;
    case 0x32A:cpu.mhpmevent[7]=mask;    return;
    case 0x32B:cpu.mhpmevent[8]=mask;    return;
    case 0x32C:cpu.mhpmevent[9]=mask;    return;
    case 0x32D:cpu.mhpmevent[10]=mask;   return;
    case 0x32E:cpu.mhpmevent[11]=mask;   return;
    case 0x32F:cpu.mhpmevent[12]=mask;   return;
    case 0x330:cpu.mhpmevent[13]=mask;   return;
    case 0x331:cpu.mhpmevent[14]=mask;   return;
    case 0x332:cpu.mhpmevent[15]=mask;   return;
    case 0x333:cpu.mhpmevent[16]=mask;   return;
    case 0x334:cpu.mhpmevent[17]=mask;   return;
    case 0x335:cpu.mhpmevent[18]=mask;   return;
    case 0x336:cpu.mhpmevent[19]=mask;   return;
    case 0x337:cpu.mhpmevent[20]=mask;   return;
    case 0x338:cpu.mhpmevent[21]=mask;   return;
    case 0x339:cpu.mhpmevent[22]=mask;   return;
    case 0x33A:cpu.mhpmevent[23]=mask;   return;
    case 0x33B:cpu.mhpmevent[24]=mask;   return;
    case 0x33C:cpu.mhpmevent[25]=mask;   return;
    case 0x33D:cpu.mhpmevent[26]=mask;   return;
    case 0x33E:cpu.mhpmevent[27]=mask;   return;
    case 0x33F:cpu.mhpmevent[28]=mask;   return;
    case 0x320:cpu.mcountinhibit=(mask&(~0x2));   return;
    case 0x340:cpu.mscratch=mask;        return;
    case 0x343:cpu.mtval=mask;           return;
    case 0x100:
        cpu.mstatus &= (~SSTATUS_MASK);
        cpu.mstatus |= (mask & SSTATUS_MASK);
        cpu.mstatus &= (~MSTATUS_CLR_BIT);
        cpu.mstatus |= MSTATUS_SET_BIT;
        return;
    case 0x105:cpu.stvec=mask;           return;
    case 0x144:
        cpu.mip &= (~cpu.mideleg);
        cpu.mip |= (mask & cpu.mideleg);
        return;
    case 0x104:
        cpu.mie &= (~cpu.mideleg);
        cpu.mie |= (mask & cpu.mideleg);
        return;
    case 0x140:cpu.sscratch=mask;        return;
    case 0x141:cpu.sepc=mask;            return;
    case 0x142:cpu.scause=mask;          return;
    case 0x143:cpu.stval=mask;           return;
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
    case 0xB80:cpu.mcycleh=mask;         return;
    case 0xB82:cpu.minstreth=mask;       return;
    case 0xB83:cpu.mhpmcounterh[0]=mask; return;
    case 0xB84:cpu.mhpmcounterh[1]=mask; return;
    case 0xB85:cpu.mhpmcounterh[2]=mask; return;
    case 0xB86:cpu.mhpmcounterh[3]=mask; return;
    case 0xB87:cpu.mhpmcounterh[4]=mask; return;
    case 0xB88:cpu.mhpmcounterh[5]=mask; return;
    case 0xB89:cpu.mhpmcounterh[6]=mask; return;
    case 0xB8A:cpu.mhpmcounterh[7]=mask; return;
    case 0xB8B:cpu.mhpmcounterh[8]=mask; return;
    case 0xB8C:cpu.mhpmcounterh[9]=mask; return;
    case 0xB8D:cpu.mhpmcounterh[10]=mask;return;
    case 0xB8E:cpu.mhpmcounterh[11]=mask;return;
    case 0xB8F:cpu.mhpmcounterh[12]=mask;return;
    case 0xB90:cpu.mhpmcounterh[13]=mask;return;
    case 0xB91:cpu.mhpmcounterh[14]=mask;return;
    case 0xB92:cpu.mhpmcounterh[15]=mask;return;
    case 0xB93:cpu.mhpmcounterh[16]=mask;return;
    case 0xB94:cpu.mhpmcounterh[17]=mask;return;
    case 0xB95:cpu.mhpmcounterh[18]=mask;return;
    case 0xB96:cpu.mhpmcounterh[19]=mask;return;
    case 0xB97:cpu.mhpmcounterh[20]=mask;return;
    case 0xB98:cpu.mhpmcounterh[21]=mask;return;
    case 0xB99:cpu.mhpmcounterh[22]=mask;return;
    case 0xB9A:cpu.mhpmcounterh[23]=mask;return;
    case 0xB9B:cpu.mhpmcounterh[24]=mask;return;
    case 0xB9C:cpu.mhpmcounterh[25]=mask;return;
    case 0xB9D:cpu.mhpmcounterh[26]=mask;return;
    case 0xB9E:cpu.mhpmcounterh[27]=mask;return;
    case 0xB9F:cpu.mhpmcounterh[28]=mask;return;
#endif
    case 0x301:case 0x30A:case 0x747:case 0x10A:case 0x306:case 0x106:
#ifndef CONFIG_RV64
    case 0x757:case 0x31A:case 0x310:
#endif
        return;
    default:
        // panic("unkown CSR number: " FMT_WORD, csr_num);
        *csr_success = false;
        return;
    }
}
