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

static int GPR1_name = MUXDEF(CONFIG_RVE, 15, 17);
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
        *imm = (SEXT((BITS(i, 31, 31) << 20) | (BITS(i, 30, 21) << 1) | (BITS(i, 20, 20) << 11) | (BITS(i, 19, 12) << 12), 20)); \
    } while (0)
#define immB()                                                                                                           \
    do                                                                                                                   \
    {                                                                                                                    \
        *imm = (SEXT((BITS(i, 31, 31) << 12) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1) | (BITS(i, 7, 7) << 11),13)); \
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
// myself

static int decode_exec(Decode *s) {
    int rd = 0;
    word_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__ ; \
}

    // printf("inst is 0x%08x\n", s->isa.inst.val>>12);

    INSTPAT_START();
    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
    INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
    INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

    //myself
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 - imm);
    INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);
    INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(rd) = src1 | imm);
    INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
    INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, imm -= 0x400; R(rd) = ((sword_t)src1 >> imm););
    INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = (src1 >> imm););
    INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = (src1 << imm););
    INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(rd) = (((sword_t)src1 < (sword_t)imm) ? 1 : 0););
    INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = ((src1 < imm) ? 1 : 0););
    INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(rd) = s->snpc; s->dnpc = s->pc + imm;if(rd==1)FTRACE_JUMP(s->dnpc));
    INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(rd) = s->snpc; s->dnpc = src1 + imm; s->dnpc &= 0xfffffffe;if(rd==1)FTRACE_JUMP(s->dnpc));
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2););
    INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2););
    INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(rd) = (SEXT(Mr(src1 + imm, 1),8)););
    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = (SEXT(Mr(src1 + imm, 4), 32)););
    INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = (SEXT(Mr(src1 + imm, 2), 16)););
    INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2););
    INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2;);
    INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2;);
    INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = src1 * src2;);
    INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, long _src1 = (sword_t)src1; long _src2 = (sword_t)src2; R(rd) = ((_src1 * _src2) >> MUXDEF(CONFIG_ISA64, 64, 32)););
    INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, long _src1 = src1; long _src2 = src2; R(rd) = ((_src1 * _src2) >> MUXDEF(CONFIG_ISA64, 64, 32)););
    INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (sword_t)src1 / (sword_t)src2;);
    INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = src1 / src2;);
    INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (sword_t)src1 % (sword_t)src2;);
    INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = src1 % src2;);
    INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2;);
    INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2;);
    INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2;);
    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, src2 &= 0x1f; R(rd) = (sword_t)src1 >> src2;);
    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, src2 &= 0x1f; R(rd) = src1 >> src2;);
    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, src2 &= 0x1f; R(rd) = src1 << src2;);
    INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = (((sword_t)src1 < (sword_t)src2) ? 1 : 0););
    INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = ((src1 < src2) ? 1 : 0););
    INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, s->dnpc = (src1 == src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, s->dnpc = (src1 != src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, s->dnpc = ((sword_t)src1 < (sword_t)src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, s->dnpc = (src1 < src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, s->dnpc = ((sword_t)src1 >= (sword_t)src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, s->dnpc = (src1 >= src2) ? (s->pc + imm) : s->snpc;);
    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm;);

    INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I, if ((BITS(s->isa.inst.val, 19, 15)) != 0) R(rd) = get_csr(imm); wirte_csr(imm, src1););
    INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, R(rd) = get_csr(imm); if ((BITS(s->isa.inst.val, 19, 15)) != 0) set_csr(imm, src1););
    INSTPAT("??????? ????? ????? 011 ????? 11100 11", csrrc  , I, R(rd) = get_csr(imm); if ((BITS(s->isa.inst.val, 19, 15)) != 0) clr_csr(imm, src1););
    INSTPAT("??????? ????? ????? 101 ????? 11100 11", csrrwi , I, if ((BITS(s->isa.inst.val, 19, 15)) != 0) R(rd) = get_csr(imm); wirte_csr(imm, BITS(s->isa.inst.val, 19, 15)););
    INSTPAT("??????? ????? ????? 110 ????? 11100 11", csrrsi , I, R(rd) = get_csr(imm); if ((BITS(s->isa.inst.val, 19, 15)) != 0) set_csr(imm, BITS(s->isa.inst.val, 19, 15)););
    INSTPAT("??????? ????? ????? 111 ????? 11100 11", csrrci , I, R(rd) = get_csr(imm); if ((BITS(s->isa.inst.val, 19, 15)) != 0) clr_csr(imm, BITS(s->isa.inst.val, 19, 15)););

    INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , N, s->dnpc = cpu.mepc ;);

    INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , N, char _path[500]; if (gpr(GPR1_name) != -2) s->dnpc = isa_raise_intr(11, s->pc); else{sprintf(_path,"%s%s", fsming_path,(char *)guest_to_host(gpr(5)));difftest_skip_ref();IFDEF(CONFIG_FTRACE,init_ftrace(_path));});
    // myself

    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    // printf("now PC is 0x%x\n", s->dnpc);
    if(INSTPAT_INST(s)==0x00008067){
        // printf("Hello world\n");
        FTRACE_RETU(s->pc);
    }
    return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s);
}
