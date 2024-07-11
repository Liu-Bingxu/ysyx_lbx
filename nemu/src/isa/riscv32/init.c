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
#include <memory/paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb  zero,16(t0)
  0x0102c503,  // lbu a0,16(t0)
  0x00100073,  // ebreak (used as nemu_trap)
  0xdeadbeef,  // some data
};

static void restart() {
  /* Set the initial program counter. */
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  cpu.gpr[0] = 0;

  cpu.mstatus = MUXDEF(CONFIG_ISA64, 0xa00001800, 0x1800);

  cpu.mvendorid = (word_t)0x79737978;
  cpu.marchid =  (word_t)23060081;
  cpu.mimpid = (word_t)0x797379786C627800;
  cpu.mhartid = 0;
  cpu.mconfigptr = 0;
  cpu.menvcfg = 0;
  cpu.mseccfg = 0;
  cpu.senvcfg = 0;
  cpu.mcounteren = 0xfffffffd;
  cpu.scounteren = 0x0;
  cpu.misa = MUXDEF(CONFIG_ISA64, 0x8000000000141105, 0x80141105);
#ifndef CONFIG_RV64
  cpu.mstatush = 0;
  cpu.mstatush = 0;
  cpu.menvcfgh = 0;
  cpu.mseccfgh = 0;
#endif

  cpu.privilege = PRV_M;
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}
