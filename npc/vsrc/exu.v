module exu#(parameter DATA_LEN=32) (
    // input                   clk,
    // input                   rst_n,
//interface with idu    
    input                   decode_valid,
    output                  decode_ready,
    input  [DATA_LEN-1:0]   operand1,
    input  [DATA_LEN-1:0]   operand2,    
    input  [DATA_LEN-1:0]   operand3,
    input  [DATA_LEN-1:0]   operand4,
    input                   op,
    input   [2:0]           csr_sign,
    input                   inst_jump_flag,
    input                   jump_without,
    input  [15:0]           Control_signal,
//interface with LSU
    // input  [DATA_LEN-1:0]   pre_data,
    output [DATA_LEN-1:0]   addr_load,
    input                   ls_valid,
    output                  ls_ready,
    input  [DATA_LEN-1:0]   load_data,
    output                  unusual_flag,

    output                  Jump_flag,
    output [DATA_LEN-1:0]   Jump_PC,
    output [DATA_LEN-1:0]   dest_data,
//get the csr: mepc, mtvec 
    input  [DATA_LEN-1:0]   mepc,
    input  [DATA_LEN-1:0]   mtvec,
    output [DATA_LEN-1:0]   csr_wdata,
    output [DATA_LEN-1:0]   cause
);

// outports wire
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
wire is_load;
wire is_store;

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

wire ret,csr_rw_flag,ecall;

assign csr_rw_flag = csr_sign[0];
assign ret         = csr_sign[1];
assign ecall       = csr_sign[2];

assign is_or    = Control_signal[0];
assign is_xor   = Control_signal[1];
assign is_and   = Control_signal[2];
assign LR       = Control_signal[3];
assign AL       = Control_signal[4];
assign is_shift = Control_signal[5];
assign is_sign  = (~Control_signal[6]);
assign is_cmp   = Control_signal[7];
assign is_blt   = Control_signal[8];
assign is_bltu  = Control_signal[9];
assign is_beq   = Control_signal[10];
assign is_bne   = Control_signal[11];
assign is_bge   = Control_signal[12];
assign is_bgeu  = Control_signal[13];
assign is_load  = Control_signal[14];
assign is_store = Control_signal[15];
// assign is_byte  = Control_signal[15];
// assign is_half  = Control_signal[16];
// assign is_word  = Control_signal[17];

alu #(DATA_LEN)u_alu(
    .NUM_A    	( operand1      ),
    .NUM_B    	( operand2      ),
    .OP       	( op            ),
    .LR       	( LR            ),
    .AL         ( AL            ),
    .is_or    	( is_or         ),
    .is_xor   	( is_xor        ),
    .is_and   	( is_and        ),
    .is_cmp   	( is_cmp        ),
    .is_sign  	( is_sign       ),
    .is_shift 	( is_shift      ),
    .out_eq   	( out_eq        ),
    .out_ge   	( out_ge        ),
    .out_lt     ( out_lt        ),
    .out_ltu    ( out_ltu       ),
    .out_neq  	( out_neq       ),
    .out_geu  	( out_geu       ),
    .res      	( res_alu       )
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
    .OP_A 	( operand3  ),
    .OP_B 	( operand4  ),
    .Sum  	( res2      )
);

assign addr_load = res_alu;

assign res1 = ((csr_rw_flag)?res2:((is_load)?load_data:res_alu));

assign dest_data = res1;

assign Jump_PC = (unusual_flag)?mtvec:((ret)?mepc:((csr_rw_flag)?res_alu:res2));
assign csr_wdata = Jump_PC;

assign beq  = is_beq&out_eq;
assign bne  = is_bne&out_neq;
assign blt  = is_blt&out_lt;
assign bge  = is_bge&out_ge;
assign bltu = is_bltu&out_ltu;
assign bgeu = is_bgeu&out_geu;
assign Jump_flag = ((inst_jump_flag&((beq|bne|blt|bltu|bge|bgeu)))|jump_without|ret|unusual_flag);

//短期
assign unusual_flag = ecall;
assign cause = 11;

assign decode_ready     = ls_valid|(~(is_load|is_store));
assign ls_ready   = 1'b1|decode_valid;
//短期

endmodule //exu


