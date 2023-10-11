// import "DPI-C" function void Log_mem_read(int unsigned addr);
// import "DPI-C" function void Log_mem_wirte(int unsigned addr, int unsigned data,byte wmask);
// import "DPI-C" function void pmem_read(
//   input int unsigned raddr, output int unsigned rdata);
// import "DPI-C" function void pmem_write(
//   input int unsigned waddr, input int unsigned wdata, input byte wmask);
module sram#(parameter DATA_LEN=32, STORB_LEN=4 ,ADDR_LEN=32) (
//write addr channel 
    input                   awvalid,
    output                  awready,
    input   [ADDR_LEN-1:0]  waddr,
//write data channel
    input                   wvalid,
    output                  wready,
    input   [DATA_LEN-1:0]  wdata,
    input   [STORB_LEN-1:0] wstrob,
//write resp channel 
    output                  bvalid,
    input                   bready,
    output  [2:0]           bresp,
//read addr channel 
    input                   arvalid,
    output                  arready,
    input   [ADDR_LEN-1:0]  raddr,
//read data channel
    output                  rvalid,
    input                   rready,
    output  [DATA_LEN-1:0]  rdata,
    output  [2:0]           rresp
);
//write fsm 
localparam AXI_WRITE_IDLE = 0;
localparam AXI_WRITE_ = ;

//read fsm 
localparam AXI_READ_IDLE = 0;

endmodule //sram
