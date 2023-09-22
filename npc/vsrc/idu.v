module idu#(parameter DATA_LEN=32) (
    // input                   clk,
    // input                   rst_n,
    input  [31:0]           inst,
    input  [DATA_LEN-1:0]   PC_S,
    input  [DATA_LEN-1:0]   PC,
    input  [DATA_LEN-1:0]   src1,
    // input  [DATA_LEN-1:0]   src2,
    output [4 :0]           rs1,
    output [4 :0]           rs2,
    output [4 :0]           rd,
    output [DATA_LEN-1:0]   operand1,   
    output [DATA_LEN-1:0]   operand2, 
    output [DATA_LEN-1:0]   operand3,   
    output [DATA_LEN-1:0]   operand4, 
    output                  inst_jump_flag,
    output                  ebreak,
    output                  op1,  
    output                  op2
);

wire [31:0] imm;
wire [31:0] imm_I,imm_J,imm_U;//imm_B,imm_S;

// wire [6:0] funct7;
// wire [2:0] funct3;

wire I_flag,J_flag,U_flag;//B_flag,R_flag,S_flag;
wire load_flag,arith_flag;

wire lui,auipc;
wire jal;
wire jalr;
// wire addi;
// wire ebreak;

assign rs1 = inst[19:15];
assign rs2 = inst[24:20];
assign rd  = inst[11:7 ];

// assign funct3 = inst[14:12];
// assign funct7 = inst[31:25];

assign imm_I = {{20{inst[31]}},inst[31:20]};
// assign imm_S = {{20{inst[31]}},inst[31:25],inst[11:7]};
// assign imm_B = {{20{inst[31]}},inst[7],inst[30:25],inst[11:8],1'b0};
assign imm_U = {inst[31:12],12'h0};
assign imm_J = {{12{inst[31]}},inst[19:12],inst[20],inst[30:21],1'b0};
// 
// assign R_flag = (inst[6:0]==7'b0110011)?1'b1:1'b0;
// assign S_flag = (inst[6:0]==7'b0100011)?1'b1:1'b0;
assign I_flag = (load_flag|arith_flag|jalr);
// assign B_flag = (inst[6:0]==7'b1100011)?1'b1:1'b0;
assign U_flag = (lui|auipc);
assign J_flag = jal;

assign imm = (I_flag)?imm_I:(
    (U_flag)?imm_U:(
        (J_flag)?imm_J:{DATA_LEN{1'b0}}
    )
);

assign load_flag  = (inst[6:0]==7'b0000011)?1'b1:1'b0;
assign arith_flag = (inst[6:0]==7'b0010011)?1'b1:1'b0;

assign lui      =   (inst[6:0]  ==  7'b0110111  ) ? 1'b1 : 1'b0;
assign auipc    =   (inst[6:0]  ==  7'b0010111  ) ? 1'b1 : 1'b0;
assign jal      =   (inst[6:0]  ==  7'b1101111  ) ? 1'b1 : 1'b0;
assign jalr     =   (inst[6:0]  ==  7'b1100111  ) ? 1'b1 : 1'b0;
// assign addi     =   (arith_flag&(funct3==3'b000)) ? 1'b1 : 1'b0;

assign ebreak   =   (inst       ==  32'h00100073) ? 1'b1 : 1'b0;

assign operand1 = ((auipc)?PC:((J_flag|jalr)?PC_S:((I_flag)?src1:32'h0)));
assign operand2 = ((jalr)?32'h0:(I_flag|U_flag)?imm:(32'h0));
assign op1 = 1'b1;

assign operand3 = (jalr)?src1:PC;
assign operand4 = (I_flag|U_flag|J_flag)?imm:(32'h0);
assign op2 = 1'b0;

assign inst_jump_flag = jal|jalr;

endmodule //idu
