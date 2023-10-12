// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Prototypes for DPI import and export functions.
//
// Verilator includes this file in all generated .cpp files that use DPI functions.
// Manually include this file where DPI .c import functions are declared to ensure
// the C functions match the expectations of the DPI imports.

#ifndef VERILATED_VTOP__DPI_H_
#define VERILATED_VTOP__DPI_H_  // guard

#include "svdpi.h"

#ifdef __cplusplus
extern "C" {
#endif


    // DPI IMPORTS
    // DPI import at /home/kuuga/ysyx-workbench/npc/vsrc/sram.sv:1:30
    extern void Log_mem_read(unsigned int addr);
    // DPI import at /home/kuuga/ysyx-workbench/npc/vsrc/sram.sv:2:30
    extern void Log_mem_wirte(unsigned int addr, unsigned int data, char wmask);
    // DPI import at /home/kuuga/ysyx-workbench/npc/vsrc/monitor.sv:1:30
    extern void halt(int code, int pc);
    // DPI import at /home/kuuga/ysyx-workbench/npc/vsrc/sram.sv:3:30
    extern void pmem_read(unsigned int raddr, unsigned int* rdata);
    // DPI import at /home/kuuga/ysyx-workbench/npc/vsrc/sram.sv:5:30
    extern void pmem_write(unsigned int waddr, unsigned int wdata, char wmask);

#ifdef __cplusplus
}
#endif

#endif  // guard
