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

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef enum
{
    PRV_U = 0,
    PRV_S = 1,
    PRV_M = 3
}privileged;

typedef struct
{
    word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
    // word_t gpr[32];
    vaddr_t pc;
    privileged privilege;
    word_t mtvec,mstatus, mcause, mepc, mtval;
    word_t stvec, sepc, scause, stval;
    word_t medeleg, mideleg;
    word_t mip, mie;
    word_t mcycle, minstret, mhpmcounter[29], mhpmevent[29];
    uint32_t mcountinhibit;
    word_t mscratch;
    word_t sscratch;
    word_t satp;
#ifndef CONFIG_RV64
    word_t mcycleh, minstreth, mhpmcounterh[29], mhpmeventh[29];
#endif
    //? RW but RO in imp
    word_t misa, menvcfg, mseccfg;
    word_t senvcfg;
    uint32_t mcounteren;
    uint32_t scounteren;
#ifndef CONFIG_RV64
    word_t mstatush;
    word_t menvcfgh, mseccfgh;
#endif
    //? RO reg
    word_t mvendorid, marchid, mimpid, mhartid, mconfigptr;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

// #define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#define CSR_fflags 0x1
#define CSR_frm 0x2
#define CSR_fcsr 0x3
#define CSR_vstart 0x8
#define CSR_vxsat 0x9
#define CSR_vxrm 0xa
#define CSR_vcsr 0xf
#define CSR_ssp 0x11
#define CSR_seed 0x15
#define CSR_jvt 0x17
#define CSR_cycle 0xc00
#define CSR_time 0xc01
#define CSR_instret 0xc02
#define CSR_hpmcounter3 0xc03
#define CSR_hpmcounter4 0xc04
#define CSR_hpmcounter5 0xc05
#define CSR_hpmcounter6 0xc06
#define CSR_hpmcounter7 0xc07
#define CSR_hpmcounter8 0xc08
#define CSR_hpmcounter9 0xc09
#define CSR_hpmcounter10 0xc0a
#define CSR_hpmcounter11 0xc0b
#define CSR_hpmcounter12 0xc0c
#define CSR_hpmcounter13 0xc0d
#define CSR_hpmcounter14 0xc0e
#define CSR_hpmcounter15 0xc0f
#define CSR_hpmcounter16 0xc10
#define CSR_hpmcounter17 0xc11
#define CSR_hpmcounter18 0xc12
#define CSR_hpmcounter19 0xc13
#define CSR_hpmcounter20 0xc14
#define CSR_hpmcounter21 0xc15
#define CSR_hpmcounter22 0xc16
#define CSR_hpmcounter23 0xc17
#define CSR_hpmcounter24 0xc18
#define CSR_hpmcounter25 0xc19
#define CSR_hpmcounter26 0xc1a
#define CSR_hpmcounter27 0xc1b
#define CSR_hpmcounter28 0xc1c
#define CSR_hpmcounter29 0xc1d
#define CSR_hpmcounter30 0xc1e
#define CSR_hpmcounter31 0xc1f
#define CSR_vl 0xc20
#define CSR_vtype 0xc21
#define CSR_vlenb 0xc22
#define CSR_sstatus 0x100
#define CSR_sedeleg 0x102
#define CSR_sideleg 0x103
#define CSR_sie 0x104
#define CSR_stvec 0x105
#define CSR_scounteren 0x106
#define CSR_senvcfg 0x10a
#define CSR_sstateen0 0x10c
#define CSR_sstateen1 0x10d
#define CSR_sstateen2 0x10e
#define CSR_sstateen3 0x10f
#define CSR_scountinhibit 0x120
#define CSR_sscratch 0x140
#define CSR_sepc 0x141
#define CSR_scause 0x142
#define CSR_stval 0x143
#define CSR_sip 0x144
#define CSR_stimecmp 0x14d
#define CSR_sctrctl 0x14e
#define CSR_sctrstatus 0x14f
#define CSR_siselect 0x150
#define CSR_sireg 0x151
#define CSR_sireg2 0x152
#define CSR_sireg3 0x153
#define CSR_sireg4 0x155
#define CSR_sireg5 0x156
#define CSR_sireg6 0x157
#define CSR_stopei 0x15c
#define CSR_sctrdepth 0x15f
#define CSR_satp 0x180
#define CSR_srmcfg 0x181
#define CSR_scontext 0x5a8
#define CSR_vsstatus 0x200
#define CSR_vsie 0x204
#define CSR_vstvec 0x205
#define CSR_vsscratch 0x240
#define CSR_vsepc 0x241
#define CSR_vscause 0x242
#define CSR_vstval 0x243
#define CSR_vsip 0x244
#define CSR_vstimecmp 0x24d
#define CSR_vsctrctl 0x24e
#define CSR_vsiselect 0x250
#define CSR_vsireg 0x251
#define CSR_vsireg2 0x252
#define CSR_vsireg3 0x253
#define CSR_vsireg4 0x255
#define CSR_vsireg5 0x256
#define CSR_vsireg6 0x257
#define CSR_vstopei 0x25c
#define CSR_vsatp 0x280
#define CSR_hstatus 0x600
#define CSR_hedeleg 0x602
#define CSR_hideleg 0x603
#define CSR_hie 0x604
#define CSR_htimedelta 0x605
#define CSR_hcounteren 0x606
#define CSR_hgeie 0x607
#define CSR_hvien 0x608
#define CSR_hvictl 0x609
#define CSR_henvcfg 0x60a
#define CSR_hstateen0 0x60c
#define CSR_hstateen1 0x60d
#define CSR_hstateen2 0x60e
#define CSR_hstateen3 0x60f
#define CSR_htval 0x643
#define CSR_hip 0x644
#define CSR_hvip 0x645
#define CSR_hviprio1 0x646
#define CSR_hviprio2 0x647
#define CSR_htinst 0x64a
#define CSR_hgatp 0x680
#define CSR_hcontext 0x6a8
#define CSR_hgeip 0xe12
#define CSR_vstopi 0xeb0
#define CSR_scountovf 0xda0
#define CSR_stopi 0xdb0
#define CSR_utvt 0x7
#define CSR_unxti 0x45
#define CSR_uintstatus 0x46
#define CSR_usratchcsw 0x48
#define CSR_usratchcswl 0x49
#define CSR_stvt 0x107
#define CSR_snxti 0x145
#define CSR_sintstatus 0x146
#define CSR_sscratchcsw 0x148
#define CSR_sscratchcswl 0x149
#define CSR_mtvt 0x307
#define CSR_mnxti 0x345
#define CSR_mintstatus 0x346
#define CSR_mscratchcsw 0x348
#define CSR_mscratchcswl 0x349
#define CSR_mstatus 0x300
#define CSR_misa 0x301
#define CSR_medeleg 0x302
#define CSR_mideleg 0x303
#define CSR_mie 0x304
#define CSR_mtvec 0x305
#define CSR_mcounteren 0x306
#define CSR_mvien 0x308
#define CSR_mvip 0x309
#define CSR_menvcfg 0x30a
#define CSR_mstateen0 0x30c
#define CSR_mstateen1 0x30d
#define CSR_mstateen2 0x30e
#define CSR_mstateen3 0x30f
#define CSR_mcountinhibit 0x320
#define CSR_mscratch 0x340
#define CSR_mepc 0x341
#define CSR_mcause 0x342
#define CSR_mtval 0x343
#define CSR_mip 0x344
#define CSR_mtinst 0x34a
#define CSR_mtval2 0x34b
#define CSR_mctrctl 0x34e
#define CSR_miselect 0x350
#define CSR_mireg 0x351
#define CSR_mireg2 0x352
#define CSR_mireg3 0x353
#define CSR_mireg4 0x355
#define CSR_mireg5 0x356
#define CSR_mireg6 0x357
#define CSR_mtopei 0x35c
#define CSR_mpmcfg0 0x3a0
#define CSR_mpmcfg1 0x3a1
#define CSR_mpmcfg2 0x3a2
#define CSR_mpmcfg3 0x3a3
#define CSR_mpmcfg4 0x3a4
#define CSR_mpmcfg5 0x3a5
#define CSR_mpmcfg6 0x3a6
#define CSR_mpmcfg7 0x3a7
#define CSR_mpmcfg8 0x3a8
#define CSR_mpmcfg9 0x3a9
#define CSR_mpmcfg10 0x3aa
#define CSR_mpmcfg11 0x3ab
#define CSR_mpmcfg12 0x3ac
#define CSR_mpmcfg13 0x3ad
#define CSR_mpmcfg14 0x3ae
#define CSR_mpmcfg15 0x3af
#define CSR_pmpaddr0 0x3b0
#define CSR_pmpaddr1 0x3b1
#define CSR_pmpaddr2 0x3b2
#define CSR_pmpaddr3 0x3b3
#define CSR_pmpaddr4 0x3b4
#define CSR_pmpaddr5 0x3b5
#define CSR_pmpaddr6 0x3b6
#define CSR_pmpaddr7 0x3b7
#define CSR_pmpaddr8 0x3b8
#define CSR_pmpaddr9 0x3b9
#define CSR_pmpaddr10 0x3ba
#define CSR_pmpaddr11 0x3bb
#define CSR_pmpaddr12 0x3bc
#define CSR_pmpaddr13 0x3bd
#define CSR_pmpaddr14 0x3be
#define CSR_pmpaddr15 0x3bf
#define CSR_pmpaddr16 0x3c0
#define CSR_pmpaddr17 0x3c1
#define CSR_pmpaddr18 0x3c2
#define CSR_pmpaddr19 0x3c3
#define CSR_pmpaddr20 0x3c4
#define CSR_pmpaddr21 0x3c5
#define CSR_pmpaddr22 0x3c6
#define CSR_pmpaddr23 0x3c7
#define CSR_pmpaddr24 0x3c8
#define CSR_pmpaddr25 0x3c9
#define CSR_pmpaddr26 0x3ca
#define CSR_pmpaddr27 0x3cb
#define CSR_pmpaddr28 0x3cc
#define CSR_pmpaddr29 0x3cd
#define CSR_pmpaddr30 0x3ce
#define CSR_pmpaddr31 0x3cf
#define CSR_pmpaddr32 0x3d0
#define CSR_pmpaddr33 0x3d1
#define CSR_pmpaddr34 0x3d2
#define CSR_pmpaddr35 0x3d3
#define CSR_pmpaddr36 0x3d4
#define CSR_pmpaddr37 0x3d5
#define CSR_pmpaddr38 0x3d6
#define CSR_pmpaddr39 0x3d7
#define CSR_pmpaddr40 0x3d8
#define CSR_pmpaddr41 0x3d9
#define CSR_pmpaddr42 0x3da
#define CSR_pmpaddr43 0x3db
#define CSR_pmpaddr44 0x3dc
#define CSR_pmpaddr45 0x3dd
#define CSR_pmpaddr46 0x3de
#define CSR_pmpaddr47 0x3df
#define CSR_pmpaddr48 0x3e0
#define CSR_pmpaddr49 0x3e1
#define CSR_pmpaddr50 0x3e2
#define CSR_pmpaddr51 0x3e3
#define CSR_pmpaddr52 0x3e4
#define CSR_pmpaddr53 0x3e5
#define CSR_pmpaddr54 0x3e6
#define CSR_pmpaddr55 0x3e7
#define CSR_pmpaddr56 0x3e8
#define CSR_pmpaddr57 0x3e9
#define CSR_pmpaddr58 0x3ea
#define CSR_pmpaddr59 0x3eb
#define CSR_pmpaddr60 0x3ec
#define CSR_pmpaddr61 0x3ed
#define CSR_pmpaddr62 0x3ee
#define CSR_pmpaddr63 0x3ef
#define CSR_mseccfg 0x747
#define CSR_tselect 0x7a0
#define CSR_tdata1 0x7a1
#define CSR_tdata2 0x7a2
#define CSR_tdata3 0x7a3
#define CSR_tinfo 0x7a4
#define CSR_tcontrol 0x7a5
#define CSR_mcontext 0x7a8
#define CSR_msccontext 0x7aa
#define CSR_dcsr 0x7b0
#define CSR_dpc 0x7b1
#define CSR_dscratch0 0x7b2
#define CSR_dscratch1 0x7b3
#define CSR_mcycle 0xb00
#define CSR_minstret 0xb02
#define CSR_mhpmcounter3 0xb03
#define CSR_mhpmcounter4 0xb04
#define CSR_mhpmcounter5 0xb05
#define CSR_mhpmcounter6 0xb06
#define CSR_mhpmcounter7 0xb07
#define CSR_mhpmcounter8 0xb08
#define CSR_mhpmcounter9 0xb09
#define CSR_mhpmcounter10 0xb0a
#define CSR_mhpmcounter11 0xb0b
#define CSR_mhpmcounter12 0xb0c
#define CSR_mhpmcounter13 0xb0d
#define CSR_mhpmcounter14 0xb0e
#define CSR_mhpmcounter15 0xb0f
#define CSR_mhpmcounter16 0xb10
#define CSR_mhpmcounter17 0xb11
#define CSR_mhpmcounter18 0xb12
#define CSR_mhpmcounter19 0xb13
#define CSR_mhpmcounter20 0xb14
#define CSR_mhpmcounter21 0xb15
#define CSR_mhpmcounter22 0xb16
#define CSR_mhpmcounter23 0xb17
#define CSR_mhpmcounter24 0xb18
#define CSR_mhpmcounter25 0xb19
#define CSR_mhpmcounter26 0xb1a
#define CSR_mhpmcounter27 0xb1b
#define CSR_mhpmcounter28 0xb1c
#define CSR_mhpmcounter29 0xb1d
#define CSR_mhpmcounter30 0xb1e
#define CSR_mhpmcounter31 0xb1f
#define CSR_mcyclecfg 0x321
#define CSR_minstretcfg 0x322
#define CSR_mhpmevent3 0x323
#define CSR_mhpmevent4 0x324
#define CSR_mhpmevent5 0x325
#define CSR_mhpmevent6 0x326
#define CSR_mhpmevent7 0x327
#define CSR_mhpmevent8 0x328
#define CSR_mhpmevent9 0x329
#define CSR_mhpmevent10 0x32a
#define CSR_mhpmevent11 0x32b
#define CSR_mhpmevent12 0x32c
#define CSR_mhpmevent13 0x32d
#define CSR_mhpmevent14 0x32e
#define CSR_mhpmevent15 0x32f
#define CSR_mhpmevent16 0x330
#define CSR_mhpmevent17 0x331
#define CSR_mhpmevent18 0x332
#define CSR_mhpmevent19 0x333
#define CSR_mhpmevent20 0x334
#define CSR_mhpmevent21 0x335
#define CSR_mhpmevent22 0x336
#define CSR_mhpmevent23 0x337
#define CSR_mhpmevent24 0x338
#define CSR_mhpmevent25 0x339
#define CSR_mhpmevent26 0x33a
#define CSR_mhpmevent27 0x33b
#define CSR_mhpmevent28 0x33c
#define CSR_mhpmevent29 0x33d
#define CSR_mhpmevent30 0x33e
#define CSR_mhpmevent31 0x33f
#define CSR_mvendorid 0xf11
#define CSR_marchid 0xf12
#define CSR_mimpid 0xf13
#define CSR_mhartid 0xf14
#define CSR_mconfigptr 0xf15
#define CSR_mtopi 0xfb0
#define CSR_sieh 0x114
#define CSR_siph 0x154
#define CSR_stimecmph 0x15d
#define CSR_vsieh 0x214
#define CSR_vsiph 0x254
#define CSR_vstimecmph 0x25d
#define CSR_htimedeltah 0x615
#define CSR_hidelegh 0x613
#define CSR_hvienh 0x618
#define CSR_henvcfgh 0x61a
#define CSR_hviph 0x655
#define CSR_hviprio1h 0x656
#define CSR_hviprio2h 0x657
#define CSR_hstateen0h 0x61c
#define CSR_hstateen1h 0x61d
#define CSR_hstateen2h 0x61e
#define CSR_hstateen3h 0x61f
#define CSR_cycleh 0xc80
#define CSR_timeh 0xc81
#define CSR_instreth 0xc82
#define CSR_hpmcounterh3 0xc83
#define CSR_hpmcounterh4 0xc84
#define CSR_hpmcounterh5 0xc85
#define CSR_hpmcounterh6 0xc86
#define CSR_hpmcounterh7 0xc87
#define CSR_hpmcounterh8 0xc88
#define CSR_hpmcounterh9 0xc89
#define CSR_hpmcounterh10 0xc8a
#define CSR_hpmcounterh11 0xc8b
#define CSR_hpmcounterh12 0xc8c
#define CSR_hpmcounterh13 0xc8d
#define CSR_hpmcounterh14 0xc8e
#define CSR_hpmcounterh15 0xc8f
#define CSR_hpmcounterh16 0xc90
#define CSR_hpmcounterh17 0xc91
#define CSR_hpmcounterh18 0xc92
#define CSR_hpmcounterh19 0xc93
#define CSR_hpmcounterh20 0xc94
#define CSR_hpmcounterh21 0xc95
#define CSR_hpmcounterh22 0xc96
#define CSR_hpmcounterh23 0xc97
#define CSR_hpmcounterh24 0xc98
#define CSR_hpmcounterh25 0xc99
#define CSR_hpmcounterh26 0xc9a
#define CSR_hpmcounterh27 0xc9b
#define CSR_hpmcounterh28 0xc9c
#define CSR_hpmcounterh29 0xc9d
#define CSR_hpmcounterh30 0xc9e
#define CSR_hpmcounterh31 0xc9f
#define CSR_mstatush 0x310
#define CSR_midelegh 0x313
#define CSR_mieh 0x314
#define CSR_mvienh 0x318
#define CSR_mviph 0x319
#define CSR_menvcfgh 0x31a
#define CSR_mstateen0h 0x31c
#define CSR_mstateen1h 0x31d
#define CSR_mstateen2h 0x31e
#define CSR_mstateen3h 0x31f
#define CSR_miph 0x354
#define CSR_mcyclecfgh 0x721
#define CSR_minstretcfgh 0x722
#define CSR_mhpmeventh3 0x723
#define CSR_mhpmeventh4 0x724
#define CSR_mhpmeventh5 0x725
#define CSR_mhpmeventh6 0x726
#define CSR_mhpmeventh7 0x727
#define CSR_mhpmeventh8 0x728
#define CSR_mhpmeventh9 0x729
#define CSR_mhpmeventh10 0x72a
#define CSR_mhpmeventh11 0x72b
#define CSR_mhpmeventh12 0x72c
#define CSR_mhpmeventh13 0x72d
#define CSR_mhpmeventh14 0x72e
#define CSR_mhpmeventh15 0x72f
#define CSR_mhpmeventh16 0x730
#define CSR_mhpmeventh17 0x731
#define CSR_mhpmeventh18 0x732
#define CSR_mhpmeventh19 0x733
#define CSR_mhpmeventh20 0x734
#define CSR_mhpmeventh21 0x735
#define CSR_mhpmeventh22 0x736
#define CSR_mhpmeventh23 0x737
#define CSR_mhpmeventh24 0x738
#define CSR_mhpmeventh25 0x739
#define CSR_mhpmeventh26 0x73a
#define CSR_mhpmeventh27 0x73b
#define CSR_mhpmeventh28 0x73c
#define CSR_mhpmeventh29 0x73d
#define CSR_mhpmeventh30 0x73e
#define CSR_mhpmeventh31 0x73f
#define CSR_mnscratch 0x740
#define CSR_mnepc 0x741
#define CSR_mncause 0x742
#define CSR_mnstatus 0x744
#define CSR_mseccfgh 0x757
#define CSR_mcycleh 0xb80
#define CSR_minstreth 0xb82
#define CSR_mhpmcounterh3 0xb83
#define CSR_mhpmcounterh4 0xb84
#define CSR_mhpmcounterh5 0xb85
#define CSR_mhpmcounterh6 0xb86
#define CSR_mhpmcounterh7 0xb87
#define CSR_mhpmcounterh8 0xb88
#define CSR_mhpmcounterh9 0xb89
#define CSR_mhpmcounterh10 0xb8a
#define CSR_mhpmcounterh11 0xb8b
#define CSR_mhpmcounterh12 0xb8c
#define CSR_mhpmcounterh13 0xb8d
#define CSR_mhpmcounterh14 0xb8e
#define CSR_mhpmcounterh15 0xb8f
#define CSR_mhpmcounterh16 0xb90
#define CSR_mhpmcounterh17 0xb91
#define CSR_mhpmcounterh18 0xb92
#define CSR_mhpmcounterh19 0xb93
#define CSR_mhpmcounterh20 0xb94
#define CSR_mhpmcounterh21 0xb95
#define CSR_mhpmcounterh22 0xb96
#define CSR_mhpmcounterh23 0xb97
#define CSR_mhpmcounterh24 0xb98
#define CSR_mhpmcounterh25 0xb99
#define CSR_mhpmcounterh26 0xb9a
#define CSR_mhpmcounterh27 0xb9b
#define CSR_mhpmcounterh28 0xb9c
#define CSR_mhpmcounterh29 0xb9d
#define CSR_mhpmcounterh30 0xb9e
#define CSR_mhpmcounterh31 0xb9f

#endif
