`include "./define.v"
module exu#(parameter DATA_LEN=32) (
    input                   clk,
    input                   rst_n,
//interface with idu    
    // input                   decode_valid,
    // output                  decode_ready,
    // input  [DATA_LEN-1:0]   operand1,
    // input  [DATA_LEN-1:0]   operand2,
    // input  [DATA_LEN-1:0]   operand3,
    // input  [DATA_LEN-1:0]   operand4,
    // input                   op,
    // input   [2:0]           csr_sign,
    // input                   inst_jump_flag,
    // input                   jump_without,
    // input  [15:0]           Control_signal,
//interface with LSU
    // input  [DATA_LEN-1:0]   pre_data,
    // output [DATA_LEN-1:0]   addr_load,
    // input                   ls_valid,
    // output                  ls_ready,
    // input  [DATA_LEN-1:0]   load_data,
//get the csr: mepc, mtvec 
    input  [DATA_LEN-1:0]   mepc,
    input  [DATA_LEN-1:0]   mtvec,
    // output                  unusual_flag,
//interface with idu    
    input                   ID_EX_reg_decode_valid,
    input [4 :0]            ID_EX_reg_rd,
    input [1 :0]            ID_EX_reg_csr_wfunc,
    input [11:0]            ID_EX_reg_CSR_addr,
    input [DATA_LEN-1:0]    ID_EX_reg_PC,
    input [DATA_LEN-1:0]    ID_EX_reg_operand1,   
    input [DATA_LEN-1:0]    ID_EX_reg_operand2, 
    input [DATA_LEN-1:0]    ID_EX_reg_operand3,   
    input [DATA_LEN-1:0]    ID_EX_reg_operand4,
    input [DATA_LEN-1:0]    ID_EX_reg_store_data,
    input [13:0]            ID_EX_reg_control_sign, 
    input [1:0]             ID_EX_reg_csr_sign,
    input                   ID_EX_reg_inst_jump_flag,
    input                   ID_EX_reg_jump_without,
    input                   ID_EX_reg_op,
    input [4:0]             ID_EX_reg_load_sign,
    input [3:0]             ID_EX_reg_store_sign,
    input                   ID_EX_reg_ebreak,
    input                   ID_EX_reg_CSR_ren,
    input                   ID_EX_reg_CSR_wen,
    input                   ID_EX_reg_dest_wen,
//interface with ifu    
    output                  EX_MON_reg_Jump_flag,
    output [DATA_LEN-1:0]   EX_IF_reg_Jump_PC,
//interface with LSU
    output                  EX_LS_reg_execute_valid,
    input                   EX_reg_execute_enable,
`ifdef SIM
    input  [31:0]           ID_EX_reg_inst,
    output [31:0]           EX_LS_reg_inst,
    output [DATA_LEN-1:0]   EX_LS_reg_PC,
`endif
    output [DATA_LEN-1:0]   EX_LS_reg_dest_data,
    output [DATA_LEN-1:0]   EX_LS_reg_csr_wdata,
    output [DATA_LEN-1:0]   EX_LS_reg_addr_load,
    output [DATA_LEN-1:0]   EX_LS_reg_store_data,
    output [4 :0]           EX_LS_reg_rd,
    output [1 :0]           EX_LS_reg_csr_wfunc,
    output [11:0]           EX_LS_reg_CSR_addr,
    output [DATA_LEN-1:0]   EX_LS_reg_cause,
    output [4:0]            EX_LS_reg_load_sign,
    output [3:0]            EX_LS_reg_store_sign,
    output                  EX_LS_reg_unusual_flag,
    output                  EX_LS_reg_ebreak,
    output                  EX_LS_reg_CSR_ren,
    output                  EX_LS_reg_CSR_wen,
    output                  EX_LS_reg_dest_wen
);

// outports wire
wire                    Jump_flag;
wire [DATA_LEN-1:0]     Jump_PC;

wire [DATA_LEN-1:0]     addr_load;

wire [DATA_LEN-1:0]     csr_wdata;
wire [DATA_LEN-1:0]     cause;
wire                    unusual_flag;

wire [DATA_LEN-1:0]     dest_data;
wire [DATA_LEN-1:0] 	res1;
wire [DATA_LEN-1:0] 	res2;
wire                	out_eq;
wire                	out_ge;
wire                    out_lt;
wire                    out_ltu;
wire                	out_neq;
wire                	out_geu;
// wire [DATA_LEN-1:0] 	data;
wire [DATA_LEN-1:0] 	res_alu;

wire LR;
wire AL;
wire is_or;
wire is_xor;
wire is_and;
wire is_cmp;
wire is_sign;
wire is_shift;
// wire is_byte;
// wire is_half;
// wire is_word;
// wire is_load;
// wire is_store;

wire is_beq;
wire is_bne;
wire is_blt;
wire is_bge;
wire is_bltu;
wire is_bgeu;

wire beq;
wire bne;
wire blt;
wire bge;
wire bltu;
wire bgeu;

wire ret,ecall;

// assign csr_rw_flag = ID_EX_reg_csr_sign[0];
assign ret         = ID_EX_reg_csr_sign[0];
assign ecall       = ID_EX_reg_csr_sign[1];

assign is_or    = ID_EX_reg_control_sign[0];
assign is_xor   = ID_EX_reg_control_sign[1];
assign is_and   = ID_EX_reg_control_sign[2];
assign LR       = ID_EX_reg_control_sign[3];
assign AL       = ID_EX_reg_control_sign[4];
assign is_shift = ID_EX_reg_control_sign[5];
assign is_sign  = (~ID_EX_reg_control_sign[6]);
assign is_cmp   = ID_EX_reg_control_sign[7];
assign is_blt   = ID_EX_reg_control_sign[8];
assign is_bltu  = ID_EX_reg_control_sign[9];
assign is_beq   = ID_EX_reg_control_sign[10];
assign is_bne   = ID_EX_reg_control_sign[11];
assign is_bge   = ID_EX_reg_control_sign[12];
assign is_bgeu  = ID_EX_reg_control_sign[13];
// assign is_load  = ID_EX_reg_control_sign[14];
// assign is_store = ID_EX_reg_control_sign[15];
// assign is_byte  = ID_EX_reg_control_sign[15];
// assign is_half  = ID_EX_reg_control_sign[16];
// assign is_word  = ID_EX_reg_control_sign[17];

alu #(DATA_LEN)u_alu(
    .NUM_A    	( ID_EX_reg_operand1    ),
    .NUM_B    	( ID_EX_reg_operand2    ),
    .OP       	( ID_EX_reg_op          ),
    .LR       	( LR                    ),
    .AL         ( AL                    ),
    .is_or    	( is_or                 ),
    .is_xor   	( is_xor                ),
    .is_and   	( is_and                ),
    .is_cmp   	( is_cmp                ),
    .is_sign  	( is_sign               ),
    .is_shift 	( is_shift              ),
    .out_eq   	( out_eq                ),
    .out_ge   	( out_ge                ),
    .out_lt     ( out_lt                ),
    .out_ltu    ( out_ltu               ),
    .out_neq  	( out_neq               ),
    .out_geu  	( out_geu               ),
    .res      	( res_alu               )
);

// memory_load_rv32 #(DATA_LEN)u_memory_load_rv32(
//     .pre_data 	( pre_data  ),
//     .is_byte  	( is_byte   ),
//     .is_half  	( is_half   ),
//     .is_word    ( is_word   ),
//     .is_sign  	( is_sign   ),
//     .data     	( data      )
// );

add_without_Cin #(DATA_LEN)PCadd(
    .OP_A 	( ID_EX_reg_operand3),
    .OP_B 	( ID_EX_reg_operand4),
    .Sum  	( res2              )
);

assign addr_load = res_alu;

assign res1 = res_alu;

assign dest_data = res1;

assign Jump_PC = (unusual_flag)?mtvec:((ret)?mepc:res2);
assign csr_wdata = ID_EX_reg_operand1;

assign beq  = is_beq&out_eq;
assign bne  = is_bne&out_neq;
assign blt  = is_blt&out_lt;
assign bge  = is_bge&out_ge;
assign bltu = is_bltu&out_ltu;
assign bgeu = is_bgeu&out_geu;
assign Jump_flag = ((ID_EX_reg_inst_jump_flag&((beq|bne|blt|bltu|bge|bgeu)))|ID_EX_reg_jump_without|ret|unusual_flag);

//短期
assign unusual_flag = ecall;
assign cause = 11;

// assign decode_ready     = ls_valid|(~(is_load|is_store));
// assign ls_ready   = 1'b1|decode_valid;
//短期

// wire valid_flush_flag;
// assign valid_flush_flag = EX_LS_reg_execute_valid&EX_MON_reg_Jump_flag;
assign EX_MON_reg_Jump_flag = Jump_flag&ID_EX_reg_decode_valid;
assign EX_IF_reg_Jump_PC = Jump_PC;

FF_D                  #(1,0      )u_execute_valid (clk,rst_n,EX_reg_execute_enable,ID_EX_reg_decode_valid,EX_LS_reg_execute_valid);
// FF_D                  #(1,0      )u_Jump_flag     (clk,rst_n,EX_reg_execute_enable,Jump_flag,EX_MON_reg_Jump_flag);
`ifdef SIM
FF_D_without_asyn_rst #(32       )u_inst          (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_inst,EX_LS_reg_inst);
`endif
FF_D_without_asyn_rst #(DATA_LEN )u_PC            (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_PC,EX_LS_reg_PC);
// FF_D_without_asyn_rst #(DATA_LEN )u_Jump_PC       (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,Jump_PC,EX_IF_reg_Jump_PC);
FF_D_without_asyn_rst #(DATA_LEN )u_dest_data     (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,dest_data,EX_LS_reg_dest_data);
FF_D_without_asyn_rst #(DATA_LEN )u_csr_wdata     (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,csr_wdata,EX_LS_reg_csr_wdata);
FF_D_without_asyn_rst #(DATA_LEN )u_csr_cause     (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,cause,EX_LS_reg_cause);
FF_D_without_asyn_rst #(DATA_LEN )u_addr_load     (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,addr_load,EX_LS_reg_addr_load);
FF_D_without_asyn_rst #(5        )u_rd            (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_rd,EX_LS_reg_rd);
FF_D_without_asyn_rst #(5        )u_load_sign     (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_load_sign,EX_LS_reg_load_sign);
FF_D_without_asyn_rst #(4        )u_store_sign    (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_store_sign,EX_LS_reg_store_sign);
FF_D_without_asyn_rst #(1        )u_ecall         (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,unusual_flag,EX_LS_reg_unusual_flag);
FF_D_without_asyn_rst #(1        )u_ebreak        (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_ebreak,EX_LS_reg_ebreak);
FF_D_without_asyn_rst #(2        )u_CSR_wfunc     (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_csr_wfunc,EX_LS_reg_csr_wfunc);
FF_D_without_asyn_rst #(12       )u_CSR_addr      (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_CSR_addr,EX_LS_reg_CSR_addr);
FF_D_without_asyn_rst #(1        )u_CSR_ren       (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_CSR_ren,EX_LS_reg_CSR_ren);
FF_D_without_asyn_rst #(1        )u_CSR_wen       (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_CSR_wen,EX_LS_reg_CSR_wen);
FF_D_without_asyn_rst #(1        )u_dest_wen      (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_dest_wen,EX_LS_reg_dest_wen);
FF_D_without_asyn_rst #(DATA_LEN )u_store_data    (clk,EX_reg_execute_enable&ID_EX_reg_decode_valid,ID_EX_reg_store_data,EX_LS_reg_store_data);

endmodule //exu


