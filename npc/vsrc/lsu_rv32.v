`include "./define.v"
module lsu_rv32 #(parameter DATA_LEN=32,DATA_BIT_NUM=4)(
    input                       clk,
    input                       rst_n,
//interface with exu 
    // output                      ls_valid,
    // input                       ls_ready,
    // input  [4:0]                load_sign,
    // input  [3:0]                store_sign,
    // output [DATA_LEN-1:0]       load_data,
//interface to GPR 
    // output                      load_wen,
//interface with sram
    //the write addr channel 
    output                      lsu_awvalid,
    input                       lsu_awready,
    output [DATA_LEN-1:0]       lsu_waddr,
    // input  [DATA_LEN-1:0]       addr_store,
    //the write data channel
    output                      lsu_wvalid,
    input                       lsu_wready,
    output [DATA_BIT_NUM-1:0]   lsu_wstrob,
    output [DATA_LEN-1:0]       lsu_wdata,
    // input  [DATA_LEN-1:0]       store_data,
    //the write resp channel
    input                       lsu_bvalid,
    output                      lsu_bready,
    input   [2:0]               lsu_bresp, 
    //the read addr channel 
    output                      lsu_arvalid,
    input                       lsu_arready,
    output [DATA_LEN-1:0]       lsu_raddr,
    // input  [DATA_LEN-1:0] 	    addr_load,
    //the read data channel 
    input                       lsu_rvalid,
    input  [2:0]                lsu_rresp,
    output                      lsu_rready,
    input  [DATA_LEN-1:0]       lsu_rdata,
//interface with exu 
    input                   EX_LS_reg_execute_valid,
    input  [DATA_LEN-1:0]   EX_LS_reg_dest_data,
    input  [DATA_LEN-1:0]   EX_LS_reg_csr_wdata,
    input  [DATA_LEN-1:0]   EX_LS_reg_cause,
    input  [DATA_LEN-1:0]   EX_LS_reg_addr_load,
    input  [DATA_LEN-1:0]   EX_LS_reg_store_data,
    input  [4 :0]           EX_LS_reg_rd,
    input  [1 :0]           EX_LS_reg_csr_wfunc,
    input  [11:0]           EX_LS_reg_CSR_addr,
    input  [4:0]            EX_LS_reg_load_sign,
    input  [3:0]            EX_LS_reg_store_sign,
    input                   EX_LS_reg_unusual_flag,
`ifdef RISCV64
    input  [1:0]            EX_LS_reg_control_sign_word,
`endif
    input                   EX_LS_reg_ebreak,
    input                   EX_LS_reg_CSR_ren,
    input                   EX_LS_reg_CSR_wen,
    input                   EX_LS_reg_dest_wen,
//interface to GPR 
    output                  LS_WB_reg_ls_valid,
    output                  LS_MON_ls_valid,
    input                   LS_reg_load_store_enable,
`ifdef SIM
    input  [31:0]           EX_LS_reg_inst,
    input  [DATA_LEN-1:0]   EX_LS_reg_PC,
    output [31:0]           LS_WB_reg_inst,
    output [DATA_LEN-1:0]   LS_WB_reg_PC,
`endif
    output [DATA_LEN-1:0]   LS_WB_reg_dest_data,
    output [DATA_LEN-1:0]   LS_WB_reg_csr_wdata,
    output [DATA_LEN-1:0]   LS_WB_reg_cause,
    output [4 :0]           LS_WB_reg_rd,
    output [1 :0]           LS_WB_reg_csr_wfunc,
    output [11:0]           LS_WB_reg_CSR_addr,
    output                  LS_WB_reg_unusual_flag,
    output                  LS_WB_reg_ebreak,
    output                  LS_WB_reg_CSR_ren,
    output                  LS_WB_reg_CSR_wen,
    output                  LS_WB_reg_dest_wen
);
                //load part 
//temp
assign lsu_rready = 1'b1;
//temp
//sign decode 
wire is_load_byte;
wire is_load_half;
wire is_load_word;
wire is_load_sign;
wire is_load;
assign is_load = EX_LS_reg_load_sign[0];
assign is_load_sign = (~EX_LS_reg_load_sign[1]);
assign is_load_byte = EX_LS_reg_load_sign[2];
assign is_load_half = EX_LS_reg_load_sign[3];
assign is_load_word = EX_LS_reg_load_sign[4];
//load mask decode 
wire [1:0] load_mask;
assign load_mask = EX_LS_reg_addr_load[1:0];
//get offest load data 
wire [DATA_LEN-1:0] pre_move_data_0;
wire [DATA_LEN-1:0] pre_move_data_1;
wire [DATA_LEN-1:0] pre_move_data_2;
wire [DATA_LEN-1:0] pre_move_data_3;
reg  [DATA_LEN-1:0] pre_read_data_reg;
assign pre_move_data_0 = pre_read_data_reg;
assign pre_move_data_1 = { 8'h0,pre_move_data_0[DATA_LEN-1:8] };
assign pre_move_data_2 = {16'h0,pre_move_data_0[DATA_LEN-1:16]};
assign pre_move_data_3 = {24'h0,pre_move_data_0[DATA_LEN-1:24]};
reg  [DATA_LEN-1:0] pre_data_reg;
always @(*) begin
    if(load_mask==2'b00)begin
        pre_data_reg=pre_move_data_0;
    end
    else if(load_mask==2'b01)begin
        pre_data_reg=pre_move_data_1;
    end
    else if(load_mask==2'b10)begin
        pre_data_reg=pre_move_data_2;
    end
    else begin
        pre_data_reg=pre_move_data_3;
    end
end
//get the truly load data 
wire [DATA_LEN-1:0]       load_data;
memory_load #(DATA_LEN)u_memory_load(
    .pre_data 	( pre_data_reg                      ),
    .is_byte  	( is_load_byte                      ),
    .is_half  	( is_load_half                      ),
    .is_word    ( is_load_word                      ),
`ifdef RISCV64
    .is_double  ( EX_LS_reg_control_sign_word[1]    ),
`endif
    .is_sign  	( is_load_sign                      ),
    .data     	( load_data                         )
);

//AXI load FSM
localparam AXI_LOAD_IDLE            = 0;
localparam AXI_LOAD_WAIT_ARREADY    = 1;
localparam AXI_LOAD_WAIT_RVALID     = 2;
localparam AXI_LOAD_WAIT_LS_READY   = 3;
reg arvalid_reg;
reg load_vaild_reg;
// reg load_wen_reg;
reg [1:0]   axi_load_fsm;
wire load_success;
//test
// reg [7:0] lsu_load_lfsr; 
//test
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        arvalid_reg<=1'b0;
        load_vaild_reg<=1'b0;
        // load_wen_reg<=1'b0;
        axi_load_fsm<=AXI_LOAD_IDLE;
        // lsu_load_lfsr<=1;
    end
    else begin
        case (axi_load_fsm)
            AXI_LOAD_IDLE: begin
                if(is_load&EX_LS_reg_execute_valid)begin
                    // repeat({24'h0,lsu_load_lfsr})begin
                        // @(posedge clk);
                    // end
                    // lsu_load_lfsr<={(lsu_load_lfsr[4]^lsu_load_lfsr[3]^lsu_load_lfsr[2]^lsu_load_lfsr[0]),lsu_load_lfsr[7:1]};
                    arvalid_reg<=1'b1;
                    axi_load_fsm<=AXI_LOAD_WAIT_ARREADY;
                end
            end
            AXI_LOAD_WAIT_ARREADY:begin
                // if(lsu_arready&lsu_rvalid&lsu_rready&(lsu_rresp==3'b000))begin
                //     load_vaild_reg<=1'b1;
                //     arvalid_reg<=1'b0;
                //     load_wen_reg<=1'b1;
                //     axi_load_fsm<=AXI_LOAD_WAIT_LS_READY;
                // end
                // else if(lsu_arready&lsu_rvalid&lsu_rready&(lsu_rresp!=3'b000))begin
                //     arvalid_reg<=1'b0;
                //     axi_load_fsm<=AXI_LOAD_IDLE;
                // end
                if(lsu_arready)begin
                    arvalid_reg<=1'b0;
                    axi_load_fsm<=AXI_LOAD_WAIT_RVALID;
                end
            end
            AXI_LOAD_WAIT_RVALID:begin
                if(lsu_rvalid&lsu_rready&(lsu_rresp==3'b000))begin
                    load_vaild_reg<=1'b1;
                    // load_wen_reg<=1'b1;
                    pre_read_data_reg<=lsu_rdata;
                    axi_load_fsm<=AXI_LOAD_WAIT_LS_READY;
                end
                else if(lsu_rvalid&lsu_rready&(lsu_rresp!=3'b000))begin
                    axi_load_fsm<=AXI_LOAD_IDLE;
                end
            end
            AXI_LOAD_WAIT_LS_READY:begin
                if(LS_reg_load_store_enable)begin
                    load_vaild_reg<=1'b0;
                    // load_wen_reg<=1'b0;
                    axi_load_fsm<=AXI_LOAD_IDLE;
                end
                // else begin
                    // load_wen_reg<=1'b0;
                // end
            end
            default: begin
                arvalid_reg<=1'b0;
                load_vaild_reg<=1'b0;
                // load_wen_reg<=1'b0;
                axi_load_fsm<=AXI_LOAD_IDLE;
            end
        endcase
    end
end
assign lsu_arvalid  = arvalid_reg;
assign  lsu_raddr   = EX_LS_reg_addr_load;
assign load_success = (~is_load)|load_vaild_reg;
// assign load_wen = load_wen_reg;

                    //store part 
//sign decode 
wire is_store;
wire is_store_byte;
wire is_store_half;
wire is_store_word;
assign is_store         = EX_LS_reg_store_sign[0];
assign is_store_byte    = EX_LS_reg_store_sign[1];
assign is_store_half    = EX_LS_reg_store_sign[2];
assign is_store_word    = EX_LS_reg_store_sign[3];

//store addr
wire  [DATA_LEN-1:0]       addr_store;
assign addr_store = EX_LS_reg_dest_data;

//store_mask decode 
wire [1:0] store_mask;
assign store_mask = addr_store[1:0];
//get wstrb by control sign 
reg [3:0] byte_wstrob,half_wstrob,word_wstrob;
reg [DATA_LEN-1:0]  lsu_wdata_reg;
always @(*) begin
    case (store_mask)
        2'b00: lsu_wdata_reg = EX_LS_reg_store_data;
        2'b01: lsu_wdata_reg = {EX_LS_reg_store_data[DATA_LEN-9:0],8'h0};
        2'b10: lsu_wdata_reg = {EX_LS_reg_store_data[DATA_LEN-17:0],16'h0};
        2'b11: lsu_wdata_reg = {EX_LS_reg_store_data[DATA_LEN-25:0],24'h0};
        // default: 
    endcase
end
always @(*) begin
    case (store_mask)
        2'b00: byte_wstrob=4'b0001;
        2'b01: byte_wstrob=4'b0010;
        2'b10: byte_wstrob=4'b0100;
        2'b11: byte_wstrob=4'b1000;
        default: byte_wstrob=4'b0000;
    endcase
end
always @(*) begin
    case (store_mask)
        2'b00: half_wstrob=4'b0011;
        2'b10: half_wstrob=4'b1100;
        default: half_wstrob=4'b0000;
    endcase
end
always @(*) begin
    case (store_mask)
        2'b00: word_wstrob=4'b1111;
        default: word_wstrob=4'b0000;
    endcase
end
assign lsu_wstrob = (is_store_byte)?byte_wstrob:((is_store_half)?half_wstrob:((is_store_word)?word_wstrob:{DATA_BIT_NUM{1'b0}}));

//AXI store FSM 
localparam AXI_STORE_IDLE                   = 0;
localparam AXI_STORE_WITE_AEREADY_WREADY    = 1;
localparam AXI_STORE_WAIT_AWREADY           = 2;
localparam AXI_STORE_WAIT_WREADY            = 3;
localparam AXI_STORE_WAIT_BVALID            = 4;
localparam AXI_STORE_WAIT_LS_READY          = 5;
reg store_valid_reg;
reg awvalid_reg;
reg wvalid_reg;
reg [2:0] axi_store_fsm;
wire store_success;
//test
// reg [7:0] lsu_store_lfsr; 
//test
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        store_valid_reg<=1'b0;
        awvalid_reg<=1'b0;
        wvalid_reg<=1'b0;
        axi_store_fsm<=AXI_STORE_IDLE;
        // lsu_store_lfsr<=1;
    end
    else begin
        case (axi_store_fsm)
            AXI_STORE_IDLE:begin
                if(is_store&EX_LS_reg_execute_valid)begin
                    // repeat({24'h0,lsu_store_lfsr})begin
                        // @(posedge clk);
                    // end
                    // lsu_store_lfsr<={(lsu_store_lfsr[4]^lsu_store_lfsr[3]^lsu_store_lfsr[2]^lsu_store_lfsr[0]),lsu_store_lfsr[7:1]};
                    awvalid_reg<=1'b1;
                    wvalid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WITE_AEREADY_WREADY;
                end
            end 
            AXI_STORE_WITE_AEREADY_WREADY:begin
                // if(lsu_awready&lsu_wready&lsu_bvalid&lsu_bready&(lsu_bresp==3'b000))begin
                //     awvalid_reg<=1'b0;
                //     wvalid_reg<=1'b0;
                //     store_valid_reg<=1'b1;
                //     axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                // end
                // else if(lsu_awready&lsu_wready&lsu_bvalid&lsu_bready&(lsu_bresp!=3'b000))begin
                //     awvalid_reg<=1'b0;
                //     wvalid_reg<=1'b0;
                //     axi_store_fsm<=AXI_STORE_IDLE;
                // end
                if(lsu_awready&lsu_wready)begin
                    awvalid_reg<=1'b0;
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_BVALID;
                end
                else if(lsu_awready)begin
                    awvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_WREADY;
                end
                else if(lsu_wready)begin
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_AWREADY;
                end
            end
            AXI_STORE_WAIT_AWREADY:begin
                // if(lsu_awready&lsu_bvalid&lsu_bready&(lsu_bresp==3'b000))begin
                //     awvalid_reg<=1'b0;
                //     store_valid_reg<=1'b1;
                //     axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                // end
                // else if(lsu_awready&lsu_bvalid&lsu_bready&(lsu_bresp!=3'b000))begin
                //     awvalid_reg<=1'b0;
                //     axi_store_fsm<=AXI_STORE_IDLE;
                // end
                if(lsu_awready)begin
                    awvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_BVALID;
                end
            end
            AXI_STORE_WAIT_WREADY:begin
                // if(lsu_wready&lsu_bvalid&lsu_bready&(lsu_bresp==3'b000))begin
                //     wvalid_reg<=1'b0;
                //     store_valid_reg<=1'b1;
                //     axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                // end
                // else if(lsu_wready&lsu_bvalid&lsu_bready&(lsu_bresp!=3'b000))begin
                //     wvalid_reg<=1'b0;
                //     axi_store_fsm<=AXI_STORE_IDLE;
                // end
                if(lsu_wready)begin
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_BVALID;
                end
            end
            AXI_STORE_WAIT_BVALID:begin
                if(lsu_bvalid&lsu_bready&(lsu_bresp==3'b000))begin
                    store_valid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                end
                else if(lsu_bvalid&lsu_bready&(lsu_bresp!=3'b000))begin
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
            end
            AXI_STORE_WAIT_LS_READY:begin
                if(LS_reg_load_store_enable)begin
                    store_valid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
            end
            default: begin
                store_valid_reg<=1'b0;
                awvalid_reg<=1'b0;
                wvalid_reg<=1'b0;
                axi_store_fsm<=AXI_STORE_IDLE;
            end
        endcase
    end
end
assign lsu_awvalid  = awvalid_reg;
assign lsu_waddr    = addr_store;
assign lsu_wvalid   = wvalid_reg;
assign lsu_wdata    = lsu_wdata_reg;
assign lsu_bready   = 1'b1;
assign store_success = (~is_store)|store_valid_reg;

//last
// assign ls_valid = load_vaild_reg|store_valid_reg;
wire ls_valid;
assign ls_valid = load_success&store_success&EX_LS_reg_execute_valid;

FF_D #(1,0)                       u_LS_WB_reg_ls_valid   (clk,rst_n,LS_reg_load_store_enable,ls_valid,LS_WB_reg_ls_valid);
assign LS_MON_ls_valid = ls_valid;
FF_D_without_asyn_rst #(1)        u_LS_WB_reg_CSR_ren    (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_CSR_ren,LS_WB_reg_CSR_ren);
FF_D_without_asyn_rst #(1)        u_LS_WB_reg_CSR_wen    (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_CSR_wen,LS_WB_reg_CSR_wen);
FF_D_without_asyn_rst #(1)        u_LS_WB_reg_dest_wen   (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_dest_wen,LS_WB_reg_dest_wen);
FF_D_without_asyn_rst #(1)        u_LS_WB_reg_ecall      (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_unusual_flag,LS_WB_reg_unusual_flag);
FF_D_without_asyn_rst #(1)        u_LS_WB_reg_ebreak     (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_ebreak,LS_WB_reg_ebreak);
FF_D_without_asyn_rst #(5)        u_LS_WB_reg_rd         (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_rd,LS_WB_reg_rd);
FF_D_without_asyn_rst #(2)        u_CSR_wfunc            (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_csr_wfunc,LS_WB_reg_csr_wfunc);
FF_D_without_asyn_rst #(12)       u_CSR_addr             (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_CSR_addr,LS_WB_reg_CSR_addr);
`ifdef SIM
FF_D_without_asyn_rst #(32)       u_inst                 (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_inst,LS_WB_reg_inst);
FF_D_without_asyn_rst #(DATA_LEN) u_PC                   (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_PC,LS_WB_reg_PC);
`endif
FF_D_without_asyn_rst #(DATA_LEN) u_LS_WB_reg_csr_wdata  (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_csr_wdata,LS_WB_reg_csr_wdata);
FF_D_without_asyn_rst #(DATA_LEN )u_csr_cause            (clk,LS_reg_load_store_enable&ls_valid,EX_LS_reg_cause,LS_WB_reg_cause);
FF_D_without_asyn_rst #(DATA_LEN) u_LS_WB_reg_dest_data  (clk,LS_reg_load_store_enable&ls_valid,(is_load)?load_data:EX_LS_reg_dest_data,LS_WB_reg_dest_data);

endmodule //lsu_rv32

/* verilator lint_off DECLFILENAME */
module memory_load#(parameter DATA_LEN = 32)(
    input   [DATA_LEN-1:0]  pre_data,
    input                   is_byte,
    input                   is_half,
    input                   is_word,
`ifdef RISCV64
    input                   is_double,
`endif
    input                   is_sign,
    output  [DATA_LEN-1:0]  data
);

localparam FILLER_LEN_BYTE = DATA_LEN-8 ;
localparam FILLER_LEN_HALF = DATA_LEN-16;
`ifdef RISCV64
    localparam FILLER_LEN_DOUBLE = DATA_LEN-32;
`endif

wire [DATA_LEN-1:0] data_byte;
wire [DATA_LEN-1:0] data_half;
wire [DATA_LEN-1:0] data_word;
wire [DATA_LEN-1:0] data_signed_byte;
wire [DATA_LEN-1:0] data_signed_half;
wire [DATA_LEN-1:0] data_unsigned_byte;
wire [DATA_LEN-1:0] data_unsigned_half;
`ifdef RISCV64
    wire [DATA_LEN-1:0] data_double;
    wire [DATA_LEN-1:0] data_signed_double;
    wire [DATA_LEN-1:0] data_unsigned_double;
`endif

assign data_signed_byte = {{FILLER_LEN_BYTE{pre_data[7]} },pre_data[7:0] };
assign data_signed_half = {{FILLER_LEN_HALF{pre_data[15]}},pre_data[15:0]};

assign data_unsigned_byte = {{FILLER_LEN_BYTE{1'b0}},pre_data[7:0] };
assign data_unsigned_half = {{FILLER_LEN_HALF{1'b0}},pre_data[15:0]};

`ifdef RISCV64
    assign data_signed_double = {{FILLER_LEN_DOUBLE{pre_data[31]}},pre_data[31:0] };
    assign data_unsigned_double = {{FILLER_LEN_DOUBLE{1'b0}},pre_data[31:0]};
`endif

assign data_byte = (is_sign)?data_signed_byte:data_unsigned_byte;
assign data_half = (is_sign)?data_signed_half:data_unsigned_half;
`ifdef RISCV64
    assign data_word    = (is_sign)?data_signed_double:data_unsigned_double;
    assign data_double  = pre_data;
`else
    assign data_word = pre_data;
`endif

`ifdef RISCV64
    assign data = (is_byte)?data_byte:((is_half)?data_half:((is_word)?data_word:((is_double)?data_double:{DATA_LEN{1'b0}})));
`else
    assign data = (is_byte)?data_byte:((is_half)?data_half:((is_word)?data_word:{DATA_LEN{1'b0}}));
`endif

endmodule //memory_load

