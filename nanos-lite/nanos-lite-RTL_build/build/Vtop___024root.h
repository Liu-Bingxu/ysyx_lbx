// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design internal header
// See Vtop.h for the primary calling header

#ifndef VERILATED_VTOP___024ROOT_H_
#define VERILATED_VTOP___024ROOT_H_  // guard

#include "verilated.h"

class Vtop__Syms;
class Vtop___024unit;


class Vtop___024root final : public VerilatedModule {
  public:
    // CELLS
    Vtop___024unit* __PVT____024unit;

    // DESIGN SPECIFIC STATE
    // Anonymous structures to workaround compiler member-count bugs
    struct {
        VL_IN8(sys_clk,0,0);
        VL_IN8(sys_rst_n,0,0);
        CData/*0:0*/ top__DOT__rst_n_s2;
        CData/*0:0*/ top__DOT__rst_n_s1;
        CData/*0:0*/ top__DOT__jump_without;
        CData/*0:0*/ top__DOT__op;
        CData/*0:0*/ top__DOT__CSR_ren;
        CData/*0:0*/ top__DOT__CSR_wen;
        CData/*0:0*/ top__DOT__dest_wen;
        CData/*0:0*/ top__DOT__u_csr__DOT__nomal_wen;
        CData/*0:0*/ top__DOT__u_idu__DOT__CSR_flag;
        CData/*0:0*/ top__DOT__u_idu__DOT__sltu;
        CData/*0:0*/ top__DOT__u_idu__DOT__sltiu;
        CData/*0:0*/ top__DOT__u_idu__DOT__lbu;
        CData/*0:0*/ top__DOT__u_idu__DOT__lhu;
        CData/*0:0*/ top__DOT__u_idu__DOT__csrrw;
        CData/*0:0*/ top__DOT__u_idu__DOT__csrrwi;
        CData/*0:0*/ top__DOT__u_idu__DOT__csrrs;
        CData/*0:0*/ top__DOT__u_idu__DOT__csrrsi;
        CData/*0:0*/ top__DOT__u_idu__DOT__csrrc;
        CData/*0:0*/ top__DOT__u_idu__DOT__csrrci;
        CData/*0:0*/ top__DOT__u_idu__DOT__is_unsign;
        CData/*0:0*/ top__DOT__u_idu__DOT____VdfgTmp_hafb5073f__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__out_lt;
        CData/*0:0*/ top__DOT__u_exu__DOT__LR;
        CData/*0:0*/ top__DOT__u_exu__DOT__is_xor;
        CData/*0:0*/ top__DOT__u_exu__DOT__is_and;
        CData/*0:0*/ top__DOT__u_exu__DOT__is_cmp;
        CData/*0:0*/ top__DOT__u_exu__DOT__is_shift;
        CData/*0:0*/ top__DOT__u_exu__DOT__csr_rw_flag;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__Cout;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__overflow;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT____VdfgTmp_h5067d9b4__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__1__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__2__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__3__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__4__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__5__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__6__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__7__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__8__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__9__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__10__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__11__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__12__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__13__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__14__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__15__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__16__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__17__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__18__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__19__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__20__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__21__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__22__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__23__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__24__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__25__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__26__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__27__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__28__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__29__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__30__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT____Vcellinp__add__BRA__31__KET____DOT__genblk1__DOT__u_add_base__c;
    };
    struct {
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__0__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__1__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__2__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__3__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__4__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__5__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__6__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__7__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__8__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__9__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__10__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__11__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__12__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__13__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__14__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__15__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__16__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__17__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__18__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__19__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__20__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__21__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__22__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__23__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__24__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__25__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__26__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__27__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__28__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__29__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__add__BRA__30__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__1__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__1__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__2__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__2__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__3__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__3__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__4__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__4__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__5__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__5__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__6__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__6__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__7__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__7__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__8__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__8__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__9__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__9__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__10__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__10__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__11__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__11__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__12__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__12__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__13__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__13__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__14__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__14__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__15__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__15__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__16__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__16__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__17__KET____DOT__genblk1__DOT__u_add_base__c;
    };
    struct {
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__17__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__18__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__18__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__19__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__19__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__20__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__20__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__21__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__21__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__22__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__22__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__23__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__23__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__24__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__24__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__25__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__25__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__26__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__26__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__27__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__27__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__28__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__28__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__29__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__29__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellout__add__BRA__30__KET____DOT__genblk1__DOT__u_add_base__Cout;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__30__KET____DOT__genblk1__DOT__u_add_base__c;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT____Vcellinp__add__BRA__30__KET____DOT__genblk1__DOT__u_add_base__b;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__1__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__2__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__3__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__4__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__5__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__6__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__7__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__8__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__9__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__10__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__11__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__12__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__13__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__14__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__15__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__16__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__17__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__18__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__19__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__20__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__21__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__22__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__23__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__24__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__25__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__26__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__27__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__28__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__29__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ top__DOT__u_exu__DOT__PCadd__DOT__add__BRA__30__KET____DOT__genblk1__DOT__u_add_base__DOT____VdfgTmp_h36175ebf__0;
        CData/*0:0*/ __Vtrigrprev__TOP__sys_clk;
        CData/*0:0*/ __Vtrigrprev__TOP__sys_rst_n;
        CData/*0:0*/ __Vtrigrprev__TOP__top__DOT__rst_n_s2;
        CData/*0:0*/ __VactContinue;
        SData/*9:0*/ top__DOT__u_idu__DOT____VdfgTmp_h5126d8d4__0;
        IData/*31:0*/ top__DOT__PC_out;
    };
    struct {
        IData/*31:0*/ top__DOT__inst_in;
        IData/*31:0*/ top__DOT__src1;
        IData/*31:0*/ top__DOT__operand2;
        IData/*31:0*/ top__DOT__operand3;
        IData/*31:0*/ top__DOT__dest_data;
        IData/*31:0*/ top__DOT__Jump_PC;
        IData/*31:0*/ top__DOT__u_regs__DOT__x1__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x2__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x3__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x4__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x5__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x6__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x7__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x8__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x9__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x10__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x11__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x12__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x13__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x14__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x15__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x16__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x17__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x18__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x19__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x20__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x21__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x22__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x23__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x24__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x25__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x26__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x27__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x28__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x29__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x30__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_regs__DOT__x31__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_csr__DOT__rdata_reg;
        IData/*31:0*/ top__DOT__u_csr__DOT__MSTATUS__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_csr__DOT__MTVEC__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_csr__DOT__MCAUSE__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_csr__DOT__MEPC__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_idu__DOT__imm;
        IData/*31:0*/ top__DOT__u_ifu__DOT__PC;
        IData/*31:0*/ top__DOT__u_ifu__DOT__inst_reg;
        IData/*31:0*/ top__DOT__u_exu__DOT__res2;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__res_reg;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__a;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__b;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__add_sub__DOT__s;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__u_buck_shift__DOT__shift__BRA__0__KET____DOT__genblk1__DOT__u_shift_base__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__u_buck_shift__DOT__shift__BRA__1__KET____DOT__genblk1__DOT__u_shift_base__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__u_buck_shift__DOT__shift__BRA__2__KET____DOT__genblk1__DOT__u_shift_base__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_exu__DOT__u_alu__DOT__u_buck_shift__DOT__shift__BRA__3__KET____DOT__genblk1__DOT__u_shift_base__DOT__data_out_reg;
        IData/*31:0*/ top__DOT__u_monitor__DOT__pre_move_data_0;
        IData/*31:0*/ top__DOT__u_monitor__DOT__pre_data_reg;
        IData/*31:0*/ __VstlIterCount;
        IData/*31:0*/ __VactIterCount;
        VlUnpacked<IData/*31:0*/, 32> top__DOT__u_regs__DOT__riscv_reg;
        VlUnpacked<IData/*31:0*/, 4> top__DOT__u_exu__DOT__u_alu__DOT__u_buck_shift__DOT__temp;
        VlUnpacked<CData/*0:0*/, 4> __Vm_traceActivity;
    };
    VlTriggerVec<1> __VstlTriggered;
    VlTriggerVec<3> __VactTriggered;
    VlTriggerVec<3> __VnbaTriggered;

    // INTERNAL VARIABLES
    Vtop__Syms* const vlSymsp;

    // CONSTRUCTORS
    Vtop___024root(Vtop__Syms* symsp, const char* v__name);
    ~Vtop___024root();
    VL_UNCOPYABLE(Vtop___024root);

    // INTERNAL METHODS
    void __Vconfigure(bool first);
} VL_ATTR_ALIGNED(VL_CACHE_LINE_BYTES);


#endif  // guard
