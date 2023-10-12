`include "./define.v"
module csr #(parameter DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
    input                   wen,
    input                   ren,
    input                   unusual_flag,
    input                   inst_valid,
    input   [11:0]          addr,
    input   [DATA_LEN-1:0]  wdata,
    input   [DATA_LEN-1:0]  PC,
    input   [DATA_LEN-1:0]  cause,
    output  [DATA_LEN-1:0]  rdata,
    output  [DATA_LEN-1:0]  mepc,
    output  [DATA_LEN-1:0]  mtvec
);

localparam MSTATUS_ADDR = 12'h300;
localparam MCAUSE_ADDR  = 12'h342;
localparam MTVEC_ADDR   = 12'h305;
localparam MEPC_ADDR    = 12'h341;

// `ifdef RISCV32
localparam MSTATUS_RST_DATA = 32'h1800;
// `else 
// localparam MSTATUS_RST_DATA = 64'ha0001800;
// `endif

reg [DATA_LEN-1:0]  rdata_reg;

wire [DATA_LEN-1:0] mstatus,mcause;
wire [DATA_LEN-1:0] mstatus_wdata,mtvec_wdata,mepc_wdata,mcause_wdata;

wire nomal_wen;
wire mstatus_wen,mtvec_wen,mepc_wen,mcause_wen;

assign nomal_wen    = (wen&(~unusual_flag));
assign mstatus_wen  = ((addr==MSTATUS_ADDR )&nomal_wen)&inst_valid;
assign mtvec_wen    = ((addr==MTVEC_ADDR   )&nomal_wen)&inst_valid;
assign mcause_wen   = (((addr==MCAUSE_ADDR  )&wen)| unusual_flag )&inst_valid;
assign mepc_wen     = (((addr==MEPC_ADDR    )&wen)| unusual_flag )&inst_valid;

assign mstatus_wdata    = wdata;
assign mtvec_wdata      = wdata;
assign mcause_wdata     = (unusual_flag)?cause:wdata;
assign mepc_wdata       = (unusual_flag)?PC:wdata;

FF_D #(DATA_LEN,MSTATUS_RST_DATA) MSTATUS  (clk,rst_n,mstatus_wen  , mstatus_wdata ,mstatus );
FF_D #(DATA_LEN,0               ) MTVEC    (clk,rst_n,mtvec_wen    , mtvec_wdata   ,mtvec   );
FF_D #(DATA_LEN,0               ) MCAUSE   (clk,rst_n,mcause_wen   , mcause_wdata  ,mcause  );
FF_D #(DATA_LEN,0               ) MEPC	   (clk,rst_n,mepc_wen     , mepc_wdata    ,mepc    );

always @(*) begin
    if(unusual_flag)begin
        rdata_reg=mtvec;
    end
    else if(ren)begin
        case (addr)
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

assign rdata = rdata_reg;

endmodule //csr
