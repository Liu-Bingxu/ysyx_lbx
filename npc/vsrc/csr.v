`include "./define.v"
module csr #(parameter DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
    // input                   wen,
    // input                   ren,
    // input   [DATA_LEN-1:0]  wdata,
    // input                   inst_valid,
    // input   [11:0]          CSR_addr,
    // input                   unusual_flag,
    // input   [DATA_LEN-1:0]  PC,
    // input   [DATA_LEN-1:0]  cause,
    output  [DATA_LEN-1:0]  csr_rdata,
    output  [DATA_LEN-1:0]  mepc,
    output  [DATA_LEN-1:0]  mtvec,
//interface with lsu
    input                   LS_WB_reg_ls_valid,
    input                   LS_WB_reg_unusual_flag,
    input  [DATA_LEN-1:0]   LS_WB_reg_csr_wdata,
    input  [DATA_LEN-1:0]   LS_WB_reg_cause,
    input  [DATA_LEN-1:0]   LS_WB_reg_PC,
    input  [1 :0]           LS_WB_reg_csr_wfunc,
    input  [11:0]           LS_WB_reg_CSR_addr,
    input                   LS_WB_reg_CSR_ren,
    input                   LS_WB_reg_CSR_wen
);

localparam MSTATUS_ADDR = 12'h300;
localparam MCAUSE_ADDR  = 12'h342;
localparam MTVEC_ADDR   = 12'h305;
localparam MEPC_ADDR    = 12'h341;

// `ifdef RISCV32
// localparam MSTATUS_RST_DATA = 32'h1800;
// `else 
// localparam MSTATUS_RST_DATA = 64'ha0001800;
// `endif

reg [DATA_LEN-1:0]  rdata_reg;

wire [DATA_LEN-1:0] mstatus,mcause;
wire [DATA_LEN-1:0] mstatus_wdata,mtvec_wdata,mepc_wdata,mcause_wdata;

wire nomal_wen;
wire mstatus_wen,mtvec_wen,mepc_wen,mcause_wen;

assign nomal_wen    = (LS_WB_reg_CSR_wen&LS_WB_reg_ls_valid&(~LS_WB_reg_unusual_flag));
assign mstatus_wen  = (LS_WB_reg_CSR_addr==MSTATUS_ADDR )&nomal_wen;
assign mtvec_wen    = (LS_WB_reg_CSR_addr==MTVEC_ADDR   )&nomal_wen;
assign mcause_wen   = ((LS_WB_reg_CSR_addr==MCAUSE_ADDR  )&LS_WB_reg_CSR_wen&LS_WB_reg_ls_valid)| LS_WB_reg_unusual_flag;
assign mepc_wen     = ((LS_WB_reg_CSR_addr==MEPC_ADDR    )&LS_WB_reg_CSR_wen&LS_WB_reg_ls_valid)| LS_WB_reg_unusual_flag;

assign mstatus_wdata    = LS_WB_reg_csr_wdata;
assign mtvec_wdata      = LS_WB_reg_csr_wdata;
assign mcause_wdata     = (LS_WB_reg_unusual_flag)?LS_WB_reg_cause:LS_WB_reg_csr_wdata;
assign mepc_wdata       = (LS_WB_reg_unusual_flag)?LS_WB_reg_PC:LS_WB_reg_csr_wdata;

wire [1:0] mstatus_csr_wfunc;
wire [1:0] mtvec_csr_wfunc;
wire [1:0] mcause_csr_wfunc;
wire [1:0] mepc_csr_wfunc;

assign mstatus_csr_wfunc = LS_WB_reg_csr_wfunc;
assign mtvec_csr_wfunc = LS_WB_reg_csr_wfunc;
assign mcause_csr_wfunc = (LS_WB_reg_unusual_flag)?2'b01:LS_WB_reg_csr_wfunc;
assign mepc_csr_wfunc = (LS_WB_reg_unusual_flag)?2'b01:LS_WB_reg_csr_wfunc;

// FF_D #(DATA_LEN,MSTATUS_RST_DATA) MSTATUS  (clk,rst_n,mstatus_wen  , mstatus_wdata ,mstatus );
// FF_D #(DATA_LEN,0               ) MTVEC    (clk,rst_n,mtvec_wen    , mtvec_wdata   ,mtvec   );
// FF_D #(DATA_LEN,0               ) MCAUSE   (clk,rst_n,mcause_wen   , mcause_wdata  ,mcause  );
// FF_D #(DATA_LEN,0               ) MEPC	   (clk,rst_n,mepc_wen     , mepc_wdata    ,mepc    );
mstatus #(DATA_LEN)u_mstatus(clk,rst_n,mstatus_wen,mstatus_csr_wfunc,mstatus_wdata,mstatus);
mtvec   #(DATA_LEN)u_mtvec  (clk,rst_n,mtvec_wen  ,mtvec_csr_wfunc,mtvec_wdata  ,mtvec  );
mcause  #(DATA_LEN)u_mcause (clk,rst_n,mcause_wen ,mcause_csr_wfunc,mcause_wdata ,mcause );
mepc    #(DATA_LEN)u_mepc   (clk,rst_n,mepc_wen   ,mepc_csr_wfunc,mepc_wdata   ,mepc   );
always @(*) begin
    if(LS_WB_reg_CSR_ren)begin
        case (LS_WB_reg_CSR_addr)
            MCAUSE_ADDR :rdata_reg=mcause;
            MEPC_ADDR   :rdata_reg=mepc;
            MSTATUS_ADDR:rdata_reg=mstatus;
            MTVEC_ADDR  :rdata_reg=mtvec;
            default: rdata_reg={DATA_LEN{1'b0}};
        endcase
    end
    else begin
        rdata_reg={DATA_LEN{1'b0}};
    end
end

assign csr_rdata = rdata_reg;

endmodule //csr

/* verilator lint_off DECLFILENAME */
module mstatus#(parameter DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
    input                   wen,
    input  [1:0]            wfunc,
    input  [DATA_LEN-1:0]   wdata,
    output [DATA_LEN-1:0]   mstatus_val
);

wire [DATA_LEN-1:0] mstatus_wdata;
assign mstatus_wdata = (wfunc==2'b01)?wdata:(mstatus_val|((wfunc==2'b10)?wdata:(~wdata)));

generate

if(DATA_LEN==32)begin:RV32_mstatus_init
    localparam  MSTATUS_RST_DATA = 32'h1800;
    FF_D #(DATA_LEN,MSTATUS_RST_DATA) u_mstatus(clk,rst_n,wen,mstatus_wdata,mstatus_val);
end
else begin: RV64_mstatus_init
    localparam MSTATUS_RST_DATA=64'hA0001800;
    FF_D #(DATA_LEN,MSTATUS_RST_DATA) u_mstatus(clk,rst_n,wen,mstatus_wdata,mstatus_val);
end

endgenerate

endmodule

module mtvec#(parameter DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
    input                   wen,
    input  [1:0]            wfunc,
    input  [DATA_LEN-1:0]   wdata,
    output [DATA_LEN-1:0]   mtvec_val
);

wire [DATA_LEN-1:0] mtvec_wdata;
assign mtvec_wdata = (wfunc==2'b01)?wdata:(mtvec_val|((wfunc==2'b10)?wdata:(~wdata)));

FF_D #(DATA_LEN,0) u_mtvec(clk,rst_n,wen,mtvec_wdata,mtvec_val);

endmodule

module mcause#(parameter DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
    input                   wen,
    input  [1:0]            wfunc,
    input  [DATA_LEN-1:0]   wdata,
    output [DATA_LEN-1:0]   mcause_val
);

wire [DATA_LEN-1:0] mcause_wdata;
assign mcause_wdata = (wfunc==2'b01)?wdata:(mcause_val|((wfunc==2'b10)?wdata:(~wdata)));

FF_D #(DATA_LEN,0) u_mcause(clk,rst_n,wen,mcause_wdata,mcause_val);

endmodule

module mepc#(parameter DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
    input                   wen,
    input  [1:0]            wfunc,
    input  [DATA_LEN-1:0]   wdata,
    output [DATA_LEN-1:0]   mepc_val
);

wire [DATA_LEN-1:0] mepc_wdata;
assign mepc_wdata = (wfunc==2'b01)?wdata:(mepc_val|((wfunc==2'b10)?wdata:(~wdata)));

FF_D #(DATA_LEN,0) u_mepc(clk,rst_n,wen,mepc_wdata,mepc_val);

endmodule
