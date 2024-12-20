`define NOP 32'h13
`define RST_PC 32'h80000000
`define HAS_AXI_BUS
`define HAS_AXI_BUS_ARBITER
`define HAS_ICACHE
`define HAS_DCACHE
// `define ICACHE_LINE_MULIT_SRAM
// `define DCACHE_LINE_MULIT_SRAM
`define SIM
`define DPI_C
// `define ADD_USE_ADD_BASE
// `define RISCV64
`ifdef RISCV64
    `define DATA_LEN 64
`else
    `define DATA_LEN 32
`endif
