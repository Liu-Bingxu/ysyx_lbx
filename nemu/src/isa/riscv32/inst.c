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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include "memory/paddr.h"
#include "stdio.h"
#include "stdlib.h"
#include <cpu/difftest.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

//myself
#define FTRACE_JUMP(pc) IFDEF(CONFIG_FTRACE,ftrce_text_jump(pc))
#define FTRACE_RETU(pc) IFDEF(CONFIG_FTRACE,ftrce_text_retu(pc))
extern void ftrce_text_jump(paddr_t pc);
extern void ftrce_text_retu(paddr_t pc);

// static int GPR1_name = MUXDEF(CONFIG_RVE, 15, 17);
extern void init_ftrace(const char *ELF_FILE);
// myself

enum
{
    TYPE_I, TYPE_U, TYPE_S,
    TYPE_N, // none
    // myself
    TYPE_J,
    TYPE_R,
    TYPE_B,
    TYPE_CR,
    TYPE_CI,
    TYPE_CSS,
    TYPE_CIW,
    TYPE_CL,
    TYPE_CS,
    TYPE_CB,
    TYPE_CA,
    // myself
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12);} while(0)
#define immU()                                  \
    do                                          \
    {                                           \
        *imm = SEXT(BITS(i, 31, 12), 20) << 12; \
    } while (0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)

//myself
#define immJ()                                                                                                                   \
    do                                                                                                                           \
    {                                                                                                                            \
        *imm = (SEXT((BITS(i, 31, 31) << 20) | (BITS(i, 30, 21) << 1) | (BITS(i, 20, 20) << 11) | (BITS(i, 19, 12) << 12), 21)); \
    } while (0)
#define immB()                                                                                                           \
    do                                                                                                                   \
    {                                                                                                                    \
        *imm = (SEXT((BITS(i, 31, 31) << 12) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1) | (BITS(i, 7, 7) << 11),13)); \
    } while (0)
#define srcCR_CI_CSS()            \
    do                        \
    {                         \
        rs1 = BITS(i, 11, 7); \
        rs2 = BITS(i, 6, 2);  \
    } while (0)
#define srcCIW_CL_CS_CB()       \
    do                             \
    {                              \
        rs1 = (BITS(i, 9, 7) + 8); \
        rs2 = (BITS(i, 4, 2) + 8); \
        *rd = rs2;                 \
    } while (0)
#define srcCA()       \
    do                             \
    {                              \
        rs1 = (BITS(i, 9, 7) + 8); \
        rs2 = (BITS(i, 4, 2) + 8); \
        *rd = rs1;                 \
    } while (0)
//myself

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
    uint32_t i = s->isa.inst.val;
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    *rd = BITS(i, 11, 7);
    switch (type){
        case TYPE_I: src1R();          immI(); break;
        case TYPE_U:                   immU(); break;
        case TYPE_S: src1R(); src2R(); immS(); break;
        //myself
        case TYPE_J:                   immJ(); break;
        case TYPE_R: src1R(); src2R();         break;
        case TYPE_B: src1R(); src2R(); immB(); break;
        case TYPE_CR: srcCR_CI_CSS();    src1R(); src2R(); break;
        case TYPE_CI: srcCR_CI_CSS();    src1R();          break;
        case TYPE_CSS:srcCR_CI_CSS();             src2R(); break;
        case TYPE_CIW:srcCIW_CL_CS_CB();                   break;
        case TYPE_CL: srcCIW_CL_CS_CB(); src1R();          break;
        case TYPE_CS: srcCIW_CL_CS_CB(); src1R(); src2R(); break;
        case TYPE_CB: srcCIW_CL_CS_CB(); src1R();          break;
        case TYPE_CA: srcCA();           src1R(); src2R(); break;
        //myself
    }
}
//myself
static char fsming_path[64];
void init_fsming_path(){
    char *nvay_path = getenv("NAVY_HOME");
    assert(nvay_path);
    sprintf(fsming_path, "%s/fsimg", nvay_path);
}
static word_t lr_sc(vaddr_t vaddr, int len, int is_lr, word_t wdata){
    static vaddr_t vaddr_reservation = 0;
    static int reservation_valid = 0;
    static int reservation_len = 0;
    word_t ret;
    if (is_lr){
        vaddr_reservation = vaddr;
        reservation_valid = 1;
        reservation_len = len;
        ret = (len == 4) ? SEXT(Mr(vaddr, len), 32) :
            Mr(vaddr, len);
    }else{
        if ((reservation_valid == 1) & (vaddr_reservation == vaddr) & (reservation_len == len)){
            ret = 0;
            Mw(vaddr, len, wdata);
        }
        else{
            ret = 1;
        }
        reservation_valid = 0;
    }
    return ret;
}
#define c_addi4spn_imm ((BITS(c_inst, 12, 11) << 4) | (BITS(c_inst, 10, 7) << 6) | (BITS(c_inst, 6, 6) << 2) | (BITS(c_inst, 5, 5) << 3))
#define c_lsw_imm ((BITS(c_inst, 12, 10) << 3) | (BITS(c_inst, 6, 6) << 2) | (BITS(c_inst, 5, 5) << 6))
#define c_lsd_imm ((BITS(c_inst, 12, 10) << 3) | (BITS(c_inst, 6, 5) << 6))
#define c_addi_addiw_andi_li_imm (SEXT(((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 2))), 6))
#define c_addi16sp_imm (SEXT(((BITS(c_inst, 12, 12) << 9) | (BITS(c_inst, 6, 6) << 4) | (BITS(c_inst, 5, 5) << 6) | (BITS(c_inst, 4, 3) << 7) | (BITS(c_inst, 2, 2) << 5)), 10))
#define c_lui_imm (SEXT(((BITS(c_inst, 12, 12) << 17) | (BITS(c_inst, 6, 2) << 12)), 18))
#define c_j_jal_imm (SEXT(((BITS(c_inst, 12, 12) << 11) | (BITS(c_inst, 11, 11) << 4) | (BITS(c_inst, 10, 9) << 8) | (BITS(c_inst, 8, 8) << 10) | (BITS(c_inst, 7, 7) << 6) | (BITS(c_inst, 6, 6) << 7) | (BITS(c_inst, 5, 3) << 1) | (BITS(c_inst, 2, 2) << 5)), 12))
#define c_b_imm (SEXT(((BITS(c_inst, 12, 12) << 8) | (BITS(c_inst, 11, 10) << 3) | (BITS(c_inst, 6, 5) << 6) | (BITS(c_inst, 4, 3) << 1) | (BITS(c_inst, 2, 2) << 5)), 9))
#define c_srli_srai_slli_imm ((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 2)))
#define c_lwsp_imm ((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 4) << 2) | (BITS(c_inst, 3, 2) << 6))
#define c_swsp_imm ((BITS(c_inst, 12, 9) << 2) | (BITS(c_inst, 8, 7) << 6))
#define c_ldsp_imm ((BITS(c_inst, 12, 12) << 5) | (BITS(c_inst, 6, 5) << 3) | (BITS(c_inst, 4, 2) << 6))
#define c_sdsp_imm ((BITS(c_inst, 12, 10) << 3) | (BITS(c_inst, 9, 7) << 6))

static bool page_fault_flag;
static int page_fault_type;
static bool misalign_fault_flag;
static int misalign_fault_type;
static vaddr_t error_vaddr;
void isa_page_fault(int type, vaddr_t vaddr){
    page_fault_flag = true;
    page_fault_type = type;
    error_vaddr     = vaddr;
}
void isa_misalign_fault(int type, vaddr_t vaddr){
    misalign_fault_flag = true;
    misalign_fault_type = type;
    error_vaddr     = vaddr;
}
static bool can_not_diasssemble;
bool isa_can_not_disassemble(){
    return can_not_diasssemble;
}
// myself

static int decode_exec(Decode *s) {
    int rd = 0;
    word_t gpr_temp = 0;
    bool amo_flag = false;
    bool inst_success = true;
    can_not_diasssemble = false;
    word_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__ ; \
}

    // printf("inst is 0x%08x\n", s->isa.inst.val>>12);
    if(page_fault_flag){
        goto out;
    }

    INSTPAT_START();
    // myself
    if((INSTPAT_INST(s) & 0x3) != 0x3){
        //! rvc instruction
        s->dnpc -= 2;
        INSTPAT_INST(s) &= 0xffff;
        uint16_t c_inst = INSTPAT_INST(s);
        if((INSTPAT_INST(s) & 0x3) == 0x0){
            // INSTPAT("000 000 000 00 000 00", Illegal_instruction,   N,  INV(s->pc));
            INSTPAT("000 000 000 00 000 00", Illegal_instruction,   N,  can_not_diasssemble = true; inst_success = false);
            // INSTPAT("000 000 000 00 ??? 00", C.ADDI4SPN_res,        N,  INV(s->pc));
            INSTPAT("000 000 000 00 ??? 00", C.ADDI4SPN_res,        N,  can_not_diasssemble = true; inst_success = false);
            INSTPAT("000 ??? ??? ?? ??? 00", C.ADDI4SPN,            CIW,R(rd) = R(2) + c_addi4spn_imm);
            INSTPAT("010 ??? ??? ?? ??? 00", c.lw,                  CL, gpr_temp = R(rd);R(rd) = SEXT(Mr(src1 + c_lsw_imm, 4), 32));
            INSTPAT("110 ??? ??? ?? ??? 00", c.sw,                  CS, Mw(src1 + c_lsw_imm, 4, src2));
#ifdef CONFIG_RV64
            INSTPAT("011 ??? ??? ?? ??? 00", c.ld,                  CL, gpr_temp = R(rd); R(rd) = Mr(src1 + c_lsd_imm, 8));
            INSTPAT("111 ??? ??? ?? ??? 00", c.sd,                  CS, Mw(src1 + c_lsd_imm, 8, src2));
#endif
        }
        else if((INSTPAT_INST(s) & 0x3) == 0x1){
            // INSTPAT("000 000 000 00 000 01", c.nop_hint,            N);
            INSTPAT("000 ?00 000 ?? ??? 01", c.nop,                 N);
            // INSTPAT("000 0?? ??? 00 000 01", c.addi_hint,           N);
            INSTPAT("000 ??? ??? ?? ??? 01", c.addi,                CI, R(rd) = src1 + c_addi_addiw_andi_li_imm);
            // INSTPAT("010 ?00 000 ?? ??? 01", c.li_hint,             N);
            INSTPAT("010 ??? ??? ?? ??? 01", c.li,                  CI, R(rd) = c_addi_addiw_andi_li_imm);
            // INSTPAT("011 ?00 000 ?? ??? 01", c.lui_hint,            N);
            // INSTPAT("011 000 010 00 000 01", c.addi16sp_res,        N, INV(s->pc));
            INSTPAT("011 000 010 00 000 01", c.addi16sp_res,        N, can_not_diasssemble = true; inst_success = false);
            INSTPAT("011 ?00 010 ?? ??? 01", c.addi16sp,            CI, R(rd) = src1 + c_addi16sp_imm);
            // INSTPAT("011 0?? ??? 00 000 01", c.lui_res,             N, INV(s->pc));
            INSTPAT("011 0?? ??? 00 000 01", c.lui_res,             N, can_not_diasssemble = true; inst_success = false);
            INSTPAT("011 ??? ??? ?? ??? 01", c.lui,                 CI, R(rd) = c_lui_imm);
            INSTPAT("100 ?10 ??? ?? ??? 01", c.andi,                CA, R(rd) = src1 & c_addi_addiw_andi_li_imm);
            INSTPAT("100 011 ??? 00 ??? 01", c.sub,                 CA, R(rd) = src1 - src2);
            INSTPAT("100 011 ??? 01 ??? 01", c.xor,                 CA, R(rd) = src1 ^ src2);
            INSTPAT("100 011 ??? 10 ??? 01", c.or,                  CA, R(rd) = src1 | src2);
            INSTPAT("100 011 ??? 11 ??? 01", c.and,                 CA, R(rd) = src1 & src2);
            INSTPAT("101 ??? ??? ?? ??? 01", c.j,                   N, s->dnpc = s->pc + c_j_jal_imm);
            INSTPAT("110 ??? ??? ?? ??? 01", c.beqz,                CB, if (src1 == 0) s->dnpc = s->pc + c_b_imm);
            INSTPAT("111 ??? ??? ?? ??? 01", c.bnez,                CB, if (src1 != 0) s->dnpc = s->pc + c_b_imm);
#ifdef CONFIG_RV64
            // INSTPAT("001 ?00 000 ?? ??? 01", c.addiw_res,           N, INV(s->pc));
            INSTPAT("001 ?00 000 ?? ??? 01", c.addiw_res,           N, can_not_diasssemble = true; inst_success = false);
            INSTPAT("001 ??? ??? ?? ??? 01", c.addiw,               CI, R(rd) = SEXT((src1 + c_addi_addiw_andi_li_imm), 32));
            // INSTPAT("100 000 ??? 00 000 01", c.srli_hint,           N);
            INSTPAT("100 ?00 ??? ?? ??? 01", c.srli,                CA, R(rd) = src1 >> c_srli_srai_slli_imm);
            // INSTPAT("100 001 ??? 00 000 01", c.srai_hint,           N);
            INSTPAT("100 ?01 ??? ?? ??? 01", c.srai,                CA, R(rd) = (sword_t)src1 >> c_srli_srai_slli_imm);
            INSTPAT("100 111 ??? 00 ??? 01", c.subw,                CA, R(rd) = (int32_t)((uint32_t)src1 - (uint32_t)src2));
            INSTPAT("100 111 ??? 01 ??? 01", c.addw,                CA, R(rd) = (int32_t)((uint32_t)src1 + (uint32_t)src2));
#else
            INSTPAT("001 ??? ??? ?? ??? 01", c.jal,                 N, R(1) = s->dnpc; s->dnpc = s->pc + c_j_jal_imm);
            // INSTPAT("100 000 ??? 00 000 01", c.srli_hint,           N);
            INSTPAT("100 000 ??? ?? ??? 01", c.srli,                CA, R(rd) = src1 >> c_srli_srai_slli_imm);
            // INSTPAT("100 001 ??? 00 000 01", c.srai_hint,           N);
            INSTPAT("100 001 ??? ?? ??? 01", c.srai,                CA, R(rd) = (sword_t)src1 >> c_srli_srai_slli_imm);
#endif
        }
        else if((INSTPAT_INST(s) & 0x3) == 0x2){
            // INSTPAT("010 ?00 000 ?? ??? 10", c.lwsp_res,            N, INV(s->pc));
            INSTPAT("010 ?00 000 ?? ??? 10", c.lwsp_res,            N, can_not_diasssemble = true; inst_success = false);
            INSTPAT("010 ??? ??? ?? ??? 10", c.lwsp,                CI, gpr_temp = R(rd);R(rd) = SEXT(Mr(R(2) + c_lwsp_imm, 4), 32));
            // INSTPAT("100 000 000 00 000 10", c.jr_res,              N, INV(s->pc));
            INSTPAT("100 000 000 00 000 10", c.jr_res,              N, can_not_diasssemble = true; inst_success = false);
            INSTPAT("100 0?? ??? 00 000 10", c.jr,                  CI, s->dnpc = src1; s->dnpc &= ((word_t)-2));
            // INSTPAT("100 000 000 ?? ??? 10", c.mv_hint,             N);
            INSTPAT("100 0?? ??? ?? ??? 10", c.mv,                  CR, R(rd) = src2);
            // INSTPAT("100 100 000 00 000 10", c.ebreak,              N, s->dnpc=s->pc;NEMUBREAK(s->pc, R(10)));
            INSTPAT("100 100 000 00 000 10", c.ebreak,              N, isa_raise_intr(s, 3, s->pc, s->pc));
            INSTPAT("100 1?? ??? 00 000 10", c.jalr,                CR, R(1) = s->dnpc; s->dnpc = src1; s->dnpc &= ((word_t)-2); FTRACE_JUMP(s->dnpc));
            // INSTPAT("100 100 000 ?? ??? 10", c.add_hint,            N);
            INSTPAT("100 1?? ??? ?? ??? 10", c.add,                 CR, R(rd) = src1 + src2);
            INSTPAT("110 ??? ??? ?? ??? 10", c.swsp,                CSS, Mw(R(2) + c_swsp_imm, 4, src2));
#ifdef CONFIG_RV64
            // INSTPAT("000 0?? ??? 00 000 10", c.slli64_hint, N);
            // INSTPAT("000 ?00 000 ?? ??? 10", c.slli_hint, N);
            INSTPAT("000 ??? ??? ?? ??? 10", c.slli,                CI, R(rd) = src1 << c_srli_srai_slli_imm);
            // INSTPAT("011 ?00 000 ?? ??? 10", c.ldsp_res,            N, INV(s->pc));
            INSTPAT("011 ?00 000 ?? ??? 10", c.ldsp_res,            N, can_not_diasssemble = true; inst_success = false);
            INSTPAT("011 ??? ??? ?? ??? 10", c.ldsp,                CI, gpr_temp = R(rd);R(rd) = Mr(R(2) + c_ldsp_imm, 8));
            INSTPAT("111 ??? ??? ?? ??? 10", c.sdsp,                CSS, Mw(R(2) + c_sdsp_imm, 8, src2));
#else
            // INSTPAT("000 0?? ??? 00 000 10", c.slli64_hint,         N);
            // INSTPAT("000 ?00 000 ?? ??? 10", c.slli_hint,           N);
            INSTPAT("000 0?? ??? ?? ??? 10", c.slli,                CI, R(rd) = src1 << c_srli_srai_slli_imm);
#endif
        }
        // INSTPAT("??? ??? ??? ?? ??? ??", Illegal_instruction,   N,  INV(s->pc));
        INSTPAT("??? ??? ??? ?? ??? ??", Illegal_instruction,   N,  can_not_diasssemble = true; inst_success = false);
    }
    // myself
    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  ,  U, R(rd) = s->pc + imm);
    INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    ,  I, gpr_temp = R(rd);R(rd) = Mr(src1 + imm, 1));
    INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     ,  S, Mw(src1 + imm, 1, src2));

    //myself
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   ,  I, R(rd) = src1 + imm);
    INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   ,  I, R(rd) = src1 & imm);
    INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    ,  I, R(rd) = src1 | imm);
    INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   ,  I, R(rd) = src1 ^ imm);

    //! rv64 and rv32 diffest
#ifdef CONFIG_RV64
    INSTPAT("010000? ????? ????? 101 ????? 00100 11", srai   ,  I, imm -= 0x400; R(rd) = ((sword_t)src1 >> imm););
    INSTPAT("000000? ????? ????? 101 ????? 00100 11", srli   ,  I, R(rd) = (src1 >> imm););
    INSTPAT("000000? ????? ????? 001 ????? 00100 11", slli   ,  I, R(rd) = (src1 << imm););
    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    ,  R, src2 &= 0x3f; R(rd) = (sword_t)src1 >> src2;);
    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    ,  R, src2 &= 0x3f; R(rd) = src1 >> src2;);
    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    ,  R, src2 &= 0x3f; R(rd) = src1 << src2;);
#else
    INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   ,  I, imm -= 0x400; R(rd) = ((sword_t)src1 >> imm););
    INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   ,  I, R(rd) = (src1 >> imm););
    INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   ,  I, R(rd) = (src1 << imm););
    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    ,  R, src2 &= 0x1f; R(rd) = (sword_t)src1 >> src2;);
    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    ,  R, src2 &= 0x1f; R(rd) = src1 >> src2;);
    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    ,  R, src2 &= 0x1f; R(rd) = src1 << src2;);
#endif

    INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   ,  I, R(rd) = (((sword_t)src1 < (sword_t)imm) ? 1 : 0););
    INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  ,  I, R(rd) = ((src1 < imm) ? 1 : 0););
    INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    ,  J, R(rd) = s->snpc; s->dnpc = s->pc + imm;/* printf("rd is %d, snpc is "FMT_PADDR", dnpc is "FMT_PADDR", PC is "FMT_PADDR", imm is %d\n",rd,s->snpc,s->dnpc,s->pc,imm); */if(rd==1)FTRACE_JUMP(s->dnpc));
    INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   ,  I, R(rd) = s->snpc; s->dnpc = src1 + imm; s->dnpc &= ((word_t)-2);if(rd==1)FTRACE_JUMP(s->dnpc));
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     ,  S, Mw(src1 + imm, 4, src2););
    INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     ,  S, Mw(src1 + imm, 2, src2););
    INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     ,  I, gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1 + imm, 1),8)););
    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     ,  I, gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1 + imm, 4), 32)););
    INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     ,  I, gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1 + imm, 2), 16)););
    INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    ,  I, gpr_temp = R(rd);R(rd) = Mr(src1 + imm, 2););
    INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    ,  R, R(rd) = src1 + src2;);
    INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    ,  R, R(rd) = src1 - src2;);
    INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    ,  R, R(rd) = src1 & src2;);
    INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     ,  R, R(rd) = src1 | src2;);
    INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    ,  R, R(rd) = src1 ^ src2;);
    INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    ,  R, R(rd) = (((sword_t)src1 < (sword_t)src2) ? 1 : 0););
    INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   ,  R, R(rd) = ((src1 < src2) ? 1 : 0););
    INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    ,  B, s->dnpc = (src1 == src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    ,  B, s->dnpc = (src1 != src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    ,  B, s->dnpc = ((sword_t)src1 < (sword_t)src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   ,  B, s->dnpc = (src1 < src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    ,  B, s->dnpc = ((sword_t)src1 >= (sword_t)src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   ,  B, s->dnpc = (src1 >= src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    ,  U, R(rd) = imm;);

#ifdef CONFIG_RV64
    INSTPAT("??????? ????? ????? 110 ????? 00000 11", lwu,      I, gpr_temp = R(rd);R(rd) = Mr(src1 + imm, 4););
    INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld,       I, gpr_temp = R(rd);R(rd) = Mr(src1 + imm, 8););
    INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd,       S, Mw(src1 + imm, 8, src2););
    INSTPAT("??????? ????? ????? 000 ????? 00110 11", addiw,    I, R(rd) = (int32_t)((uint32_t)src1 + (uint32_t)imm););
    INSTPAT("0000000 ????? ????? 001 ????? 00110 11", slliw,    I, R(rd) = (int32_t)((uint32_t)src1 << (uint32_t)imm););
    INSTPAT("0000000 ????? ????? 101 ????? 00110 11", srliw,    I, R(rd) = (int32_t)((uint32_t)src1 >> (uint32_t)imm););
    INSTPAT("0100000 ????? ????? 101 ????? 00110 11", sraiw,    I, imm -= 0x400; R(rd) = (int32_t)((int32_t)src1 >> (uint32_t)imm););
    INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw,     R, R(rd) = (int32_t)((uint32_t)src1 + (uint32_t)src2););
    INSTPAT("0100000 ????? ????? 000 ????? 01110 11", subw,     R, R(rd) = (int32_t)((uint32_t)src1 - (uint32_t)src2););
    INSTPAT("0000000 ????? ????? 001 ????? 01110 11", sllw,     R, src2 &= 0x1f; R(rd) = (int32_t)((uint32_t)src1 << (uint32_t)src2););
    INSTPAT("0000000 ????? ????? 101 ????? 01110 11", srlw,     R, src2 &= 0x1f; R(rd) = (int32_t)((uint32_t)src1 >> (uint32_t)src2););
    INSTPAT("0100000 ????? ????? 101 ????? 01110 11", sraw,     R, src2 &= 0x1f; R(rd) = (int32_t)((int32_t)src1 >> (uint32_t)src2););

    INSTPAT("0000001 ????? ????? 000 ????? 01110 11", mulw,     R, R(rd) = (int32_t)((uint32_t)src1 * (uint32_t)src2););
    INSTPAT("0000001 ????? ????? 100 ????? 01110 11", divw,     R, if ((uint32_t)src2 == 0) R(rd) = (word_t)-1; else if (((uint32_t)src1 == (uint32_t)0x80000000) & ((int32_t)src2 == -1)) R(rd) = (int32_t)0x80000000; else R(rd) = (int32_t)src1 / (int32_t)src2;);
    INSTPAT("0000001 ????? ????? 101 ????? 01110 11", divuw,    R, if ((uint32_t)src2 == 0) R(rd) = (word_t)-1; else R(rd) = (int32_t)((uint32_t)src1 / (uint32_t)src2););
    INSTPAT("0000001 ????? ????? 110 ????? 01110 11", remw,     R, if ((uint32_t)src2 == 0) R(rd) = (int32_t)src1; else if (((uint32_t)src1 == (uint32_t)0x80000000) & ((int32_t)src2 == -1)) R(rd) = 0; else R(rd) = (int32_t)src1 % (int32_t)src2;);
    INSTPAT("0000001 ????? ????? 111 ????? 01110 11", remuw,    R, if ((uint32_t)src2 == 0) R(rd) = (int32_t)src1; else R(rd) = (int32_t)((uint32_t)src1 % (uint32_t)src2););

    INSTPAT("00010?? 00000 ????? 011 ????? 01011 11", lr.d,     R, amo_flag = true;gpr_temp = R(rd);R(rd) = lr_sc(src1, 8, true, 0););
    INSTPAT("00011?? ????? ????? 011 ????? 01011 11", sd.d,     R, amo_flag = true;gpr_temp = R(rd);R(rd) = lr_sc(src1, 8, false, src2););
    INSTPAT("00001?? ????? ????? 011 ????? 01011 11", amoswap.d,R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, src2););
    INSTPAT("00000?? ????? ????? 011 ????? 01011 11", amoadd.d, R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, src2 + R(rd)););
    INSTPAT("00100?? ????? ????? 011 ????? 01011 11", amoxor.d, R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, src2 ^ R(rd)););
    INSTPAT("01100?? ????? ????? 011 ????? 01011 11", amoand.d, R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, src2 & R(rd)););
    INSTPAT("01000?? ????? ????? 011 ????? 01011 11", amoor.d,  R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, src2 | R(rd)););
    INSTPAT("10000?? ????? ????? 011 ????? 01011 11", amomin.d, R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, ((sword_t)src2 < (sword_t)R(rd)) ? src2 : R(rd)););
    INSTPAT("10100?? ????? ????? 011 ????? 01011 11", amomax.d, R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, ((sword_t)src2 > (sword_t)R(rd)) ? src2 : R(rd)););
    INSTPAT("11000?? ????? ????? 011 ????? 01011 11", amominu.d,R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, (src2 < R(rd)) ? src2 : R(rd)););
    INSTPAT("11100?? ????? ????? 011 ????? 01011 11", amomaxu.d,R, amo_flag = true;gpr_temp = R(rd);R(rd) = Mr(src1, 8); Mw(src1, 8, (src2 > R(rd)) ? src2 : R(rd)););
#endif

    INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul,      R, R(rd) = src1 * src2;);
    INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh,     R, __uint128_t _src1 = (sword_t)src1; __uint128_t _src2 = (sword_t)src2; R(rd) = ((_src1 * _src2) >> MUXDEF(CONFIG_ISA64, 64, 32)););
    INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu,   R, __uint128_t _src1 = (sword_t)src1; __uint128_t _src2 = src2; R(rd) = ((_src1 * _src2) >> MUXDEF(CONFIG_ISA64, 64, 32)););
    INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu,    R, __uint128_t _src1 = src1; __uint128_t _src2 = src2; R(rd) = ((_src1 * _src2) >> MUXDEF(CONFIG_ISA64, 64, 32)););
    INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div,      R, if (src2 == 0) R(rd) = (word_t)-1; else if ((src1 == MUXDEF(CONFIG_ISA64, 0x8000000000000000, 0x80000000)) & (src2 == -1)) R(rd) = src1; else R(rd) = (sword_t)src1 / (sword_t)src2;);
    INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu,     R, if (src2 == 0) R(rd) = (word_t)-1; else R(rd) = src1 / src2;);
    INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem,      R, if (src2 == 0) R(rd) = src1; else if ((src1 == MUXDEF(CONFIG_ISA64, 0x8000000000000000, 0x80000000)) & (src2 == -1)) R(rd) = 0; else R(rd) = (sword_t)src1 % (sword_t)src2;);
    INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu,     R, if (src2 == 0) R(rd) = src1; else R(rd) = src1 % src2;);

    INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  ,  I, word_t temp = 0;if (rd != 0) temp = get_csr(imm, &inst_success); if(inst_success) wirte_csr(imm, src1, &inst_success);if(inst_success) R(rd) = temp);
    INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  ,  I, word_t temp = 0;temp = get_csr(imm, &inst_success); if (((BITS(s->isa.inst.val, 19, 15)) != 0) && inst_success) set_csr(imm, src1, &inst_success);if(inst_success) R(rd) = temp);
    INSTPAT("??????? ????? ????? 011 ????? 11100 11", csrrc  ,  I, word_t temp = 0;temp = get_csr(imm, &inst_success); if (((BITS(s->isa.inst.val, 19, 15)) != 0) && inst_success) clr_csr(imm, src1, &inst_success);if(inst_success) R(rd) = temp);
    INSTPAT("??????? ????? ????? 101 ????? 11100 11", csrrwi ,  I, word_t temp = 0;if (rd != 0) temp = get_csr(imm, &inst_success); if(inst_success) wirte_csr(imm, BITS(s->isa.inst.val, 19, 15), &inst_success);if(inst_success) R(rd) = temp);
    INSTPAT("??????? ????? ????? 110 ????? 11100 11", csrrsi ,  I, word_t temp = 0;temp = get_csr(imm, &inst_success); if (((BITS(s->isa.inst.val, 19, 15)) != 0) && inst_success) set_csr(imm, BITS(s->isa.inst.val, 19, 15), &inst_success);if(inst_success) R(rd) = temp);
    INSTPAT("??????? ????? ????? 111 ????? 11100 11", csrrci ,  I, word_t temp = 0;temp = get_csr(imm, &inst_success); if (((BITS(s->isa.inst.val, 19, 15)) != 0) && inst_success) clr_csr(imm, BITS(s->isa.inst.val, 19, 15), &inst_success);if(inst_success) R(rd) = temp);

    INSTPAT("00010?? 00000 ????? 010 ????? 01011 11", lr.w,     R, amo_flag = true;gpr_temp = R(rd);R(rd) = lr_sc(src1, 4, true, 0););
    INSTPAT("00011?? ????? ????? 010 ????? 01011 11", sd.w,     R, amo_flag = true;gpr_temp = R(rd);R(rd) = lr_sc(src1, 4, false, src2););
    INSTPAT("00001?? ????? ????? 010 ????? 01011 11", amoswap.w,R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, src2););
    INSTPAT("00000?? ????? ????? 010 ????? 01011 11", amoadd.w, R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, src2 + R(rd)););
    INSTPAT("00100?? ????? ????? 010 ????? 01011 11", amoxor.w, R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, src2 ^ R(rd)););
    INSTPAT("01100?? ????? ????? 010 ????? 01011 11", amoand.w, R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, src2 & R(rd)););
    INSTPAT("01000?? ????? ????? 010 ????? 01011 11", amoor.w,  R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, src2 | R(rd)););
    INSTPAT("10000?? ????? ????? 010 ????? 01011 11", amomin.w, R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, ((sword_t)src2 < (sword_t)R(rd)) ? src2 : R(rd)););
    INSTPAT("10100?? ????? ????? 010 ????? 01011 11", amomax.w, R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, ((sword_t)src2 > (sword_t)R(rd)) ? src2 : R(rd)););
    INSTPAT("11000?? ????? ????? 010 ????? 01011 11", amominu.w,R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, (src2 < R(rd)) ? src2 : R(rd)););
    INSTPAT("11100?? ????? ????? 010 ????? 01011 11", amomaxu.w,R, amo_flag = true;gpr_temp = R(rd);R(rd) = (SEXT(Mr(src1, 4), 32)); Mw(src1, 4, (src2 > R(rd)) ? src2 : R(rd)););

    INSTPAT("0011000 00010 00000 000 00000 11100 11", mret,     N, mret(s));
    INSTPAT("0001000 00010 00000 000 00000 11100 11", sret,     N, sret(s));
    INSTPAT("0001000 00101 00000 000 00000 11100 11", wfi,      N, difftest_skip_ref(););
    INSTPAT("0001001 ????? ????? 000 00000 11100 11", sfence.vma,N);

    // INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall, N, char _path[500]; if (gpr(GPR1_name) != -2) s->dnpc = isa_raise_intr(11, s->pc); else {sprintf(_path,"%s%s", fsming_path,(char *)guest_to_host(gpr(5)));difftest_skip_ref();IFDEF(CONFIG_FTRACE,init_ftrace(_path)); });
    INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall,    N, isa_raise_intr(s, 8 + cpu.privilege, s->pc, 0));
    INSTPAT("??????? ????? ????? 000 ????? 00011 11", fence,    N);
    INSTPAT("??????? ????? ????? 001 ????? 00011 11", fence.i,  N);
    // myself

    // INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak ,  N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
    // INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak ,  N, s->dnpc=s->pc; NEMUBREAK(s->pc, R(10))); // R(10) is $a0
    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak ,  N, isa_raise_intr(s, 3, s->pc, s->pc)); // R(10) is $a0
    INSTPAT("1111110 00000 00000 000 00000 11100 11", halt   ,  N, can_not_diasssemble = true; s->dnpc=s->pc; NEMUBREAK(s->pc, R(10))); // R(10) is $a0
    // INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    ,  N, INV(s->pc));
    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    ,  N, can_not_diasssemble = true; inst_success = false);
    INSTPAT_END();

out:

    R(0) = 0; // reset $zero to 0

    // printf("now PC is 0x%x\n", s->dnpc);
    if((INSTPAT_INST(s)==0x00008067) || (BITS(INSTPAT_INST(s), 31, 0)==0x8082)){
        // printf("Hello world\n");
        FTRACE_RETU(s->pc);
    }

    if(!inst_success){
        // printf("%s-%s-%d:0x%x\n", __func__, __FILE__, __LINE__, INSTPAT_INST(s));
        isa_raise_intr(s, 2, s->pc, INSTPAT_INST(s));
        // printf("%s-%s-%d:0x%x\n", __func__, __FILE__, __LINE__, INSTPAT_INST(s));
        if (ref_difftest_raise_intr)
            ref_difftest_raise_intr(2);
        // printf("%s-%s-%d:0x%x\n", __func__, __FILE__, __LINE__, INSTPAT_INST(s));
        difftest_skip_ref();
        // printf("%s-%s-%d:0x%x\n", __func__, __FILE__, __LINE__, INSTPAT_INST(s));
    }

    if(page_fault_flag){
        word_t NO;
        switch (page_fault_type){
            case MEM_TYPE_IFETCH:
                NO = 12;
                break;
            case MEM_TYPE_READ:
                R(rd) = gpr_temp;
                NO = 13;
                break;
            case MEM_TYPE_WRITE:
                if(amo_flag)
                    R(rd) = gpr_temp;
                NO = 15;
                break;
            default:
                Assert(0, "%s-%d:unkown type %d\n", __func__, __LINE__, page_fault_type);
                break;
        }
        debug_info("%s-%d:now have a %ld page fault trap happend\nnow pc is "FMT_WORD"\n", __func__, __LINE__, NO, cpu.pc);
        isa_raise_intr(s, NO, s->pc, error_vaddr);
    }
    if(misalign_fault_flag){
        word_t NO;
        switch (misalign_fault_type){
            case MEM_TYPE_IFETCH:
                NO = 0;
                break;
            case MEM_TYPE_READ:
                R(rd) = gpr_temp;
                NO = 4;
                break;
            case MEM_TYPE_WRITE:
                if(amo_flag)
                    R(rd) = gpr_temp;
                NO = 6;
                break;
            default:
                Assert(0,"%s-%d:unkown type %d\n",__func__, __LINE__, misalign_fault_type);
                break;
        }
        debug_info("%s-%d:now have a %ld misalign fault trap happend\nnow pc is "FMT_WORD"\n", __func__, __LINE__, NO, cpu.pc);
        isa_raise_intr(s, NO, s->pc, error_vaddr);
        if (ref_difftest_raise_intr)
            ref_difftest_raise_intr(NO);
        difftest_skip_ref();
    }

    return 0;
}

int isa_exec_once(Decode *s){
    page_fault_flag = false;
    misalign_fault_flag = false;
    if(!try_isa_raise_intr(s)){
        return 0;
    }
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
