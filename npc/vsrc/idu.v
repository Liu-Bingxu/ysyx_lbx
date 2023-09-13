module idu#(parameter DATA_LEN=32) (
    // input                   clk,
    // input                   rst_n,
    input  [31:0]           inst,
    input  [DATA_LEN-1:0]   src1,
    // input  [DATA_LEN-1:0]   src2,
    output [4 :0]           rs1,
    output [4 :0]           rs2,
    output [4 :0]           rd,
    output [DATA_LEN-1:0]   operand1,   
    output [DATA_LEN-1:0]   operand2, 
    output                  ebreak,
    output                  op  
);

wire [31:0] /*imm_B,*/imm_I;//,imm_J,imm_U,imm_S;

// wire [6:0] funct7;
wire [2:0] funct3;

// wire R_flag,I_flag,B_flag,J_flag,U_flag,S_flag;
wire /*load_flag,*/arith_flag;

// wire lui,auipc;
// wire jal;
// wire jalr;
wire addi;
// wire ebreak;

assign rs1 = inst[19:15];
assign rs2 = inst[24:20];
assign rd  = inst[11:7 ];

assign funct3 = inst[14:12];
// assign funct7 = inst[31:25];

assign imm_I = {{20{inst[31]}},inst[31:20]};
// assign imm_S = {{20{inst[31]}},inst[31:25],inst[11:7]};
// assign imm_B = {{20{inst[31]}},inst[7],inst[30:25],inst[11:8],1'b0};
// assign imm_U = {inst[31:12],12'h0};
// assign imm_J = {{12{inst[31]}},inst[19:12],inst[20],inst[30:21],1'b0};
// 
// assign R_flag = (inst[6:0]==7'b0110011)?1'b1:1'b0;
// assign S_flag = (inst[6:0]==7'b0100011)?1'b1:1'b0;
// assign I_flag = (load_flag|arith_flag|jalr);
// assign B_flag = (inst[6:0]==7'b1100011)?1'b1:1'b0;
// assign U_flag = (lui|auipc);
// assign J_flag = jal;

// assign load_flag  = (inst[6:0]==7'b0000011)?1'b1:1'b0;
assign arith_flag = (inst[6:0]==7'b0010011)?1'b1:1'b0;

// assign lui      =   (inst[6:0]  ==  7'b0110111  ) ? 1'b1 : 1'b0;
// assign auipc    =   (inst[6:0]  ==  7'b0010111  ) ? 1'b1 : 1'b0;
// assign jal      =   (inst[6:0]  ==  7'b1101111  ) ? 1'b1 : 1'b0;
// assign jalr     =   (inst[6:0]  ==  7'b1100111  ) ? 1'b1 : 1'b0;

assign addi     =   (arith_flag&(funct3==3'b000)) ? 1'b1 : 1'b0;

assign ebreak   =   (inst       ==  32'h00100073) ? 1'b1 : 1'b0;

assign operand1 = (addi)?src1:32'h0;
assign operand2 = (addi)?imm_I:32'h0;
assign op = (addi)?1'b1:1'b0;

endmodule //idu
