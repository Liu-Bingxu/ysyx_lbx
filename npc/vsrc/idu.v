`include "./define.v"
module idu#(parameter DATA_LEN=64) (
    input                   clk,
    input                   rst_n,
    // input                   unusual_flag,
    // input  [DATA_LEN-1:0]   PC_S,
//interface with ifu
    input  [31:0]           IF_ID_reg_inst,
    input  [DATA_LEN-1:0]   IF_ID_reg_PC,
    input                   IF_ID_reg_inst_valid,
    // output                  ID_IF_reg_inst_ready,
//interface with exu
    output                  ID_EX_reg_decode_valid,
    input                   ID_reg_decode_enable,
    input                   EX_reg_execute_enable,
    input                   ID_reg_decode_flush,
    input                   MON_ID_src_block_flag,
    input  [DATA_LEN-1:0]   src1,
    input  [DATA_LEN-1:0]   src2,
    // input  [DATA_LEN-1:0]   csr_rdata,
    output [4 :0]           rs1,
    output                  rs1_valid,
    output [4 :0]           rs2,
    output                  rs2_valid,
    output [11:0]           ID_EX_reg_CSR_addr,
    output [4 :0]           ID_EX_reg_rd,
    output [1 :0]           ID_EX_reg_csr_wfunc,
    output [DATA_LEN-1:0]   ID_EX_reg_PC,
`ifdef SIM
    output  [31:0]          ID_EX_reg_inst,
`endif
`ifdef RISCV64
    output [2:0]            ID_EX_reg_control_sign_word,
`endif
    output [DATA_LEN-1:0]   ID_EX_reg_operand1,   
    output [DATA_LEN-1:0]   ID_EX_reg_operand2, 
    output [DATA_LEN-1:0]   ID_EX_reg_operand3,   
    output [DATA_LEN-1:0]   ID_EX_reg_operand4,
    output [DATA_LEN-1:0]   ID_EX_reg_store_data,
    output [13:0]           ID_EX_reg_control_sign, 
    output [1:0]            ID_EX_reg_csr_sign,
    output                  ID_EX_reg_inst_jump_flag,
    output                  ID_EX_reg_jump_without,
    output [4:0]            ID_EX_reg_load_sign,
    output [3:0]            ID_EX_reg_store_sign,
    output                  ID_EX_reg_ebreak,
    output                  ID_EX_reg_op,
//interfaace with the register module 
    // output                  csr_ren,
    output                  ID_EX_reg_CSR_ren,
    output                  ID_EX_reg_CSR_wen,
    output                  ID_EX_reg_dest_wen
);

localparam FILLER_LEN = 20 + $clog2(DATA_LEN);
localparam CSR_FILLER_LEN = DATA_LEN-5;

// reg [31:0]  IF_ID_reg_inst;
// reg         inst_valid_reg;
// reg [DATA_LEN-1:0]  PC;
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         IF_ID_reg_inst<=`NOP;
//         PC<=`RST_PC;
//     end
//     else if(inst_valid&inst_ready)begin
//         IF_ID_reg_inst<=inst_in;
//         PC<=PC_now;
//     end
// end
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         inst_valid_reg<=1'b0;
//     end
//     else if(inst_valid)begin
//         inst_valid_reg<=1'b1;
//     end
//     else begin
//         inst_valid_reg<=1'b0;
//     end
// end

wire [4 :0]           rd;
wire [DATA_LEN-1:0]   operand1;
wire [DATA_LEN-1:0]   operand2;
wire [DATA_LEN-1:0]   operand3;
wire [DATA_LEN-1:0]   operand4;
wire [13:0]           control_sign;
wire [1:0]            csr_sign;
wire                  inst_jump_flag;
wire                  jump_without;
wire [4:0]            load_sign;
wire [3:0]            store_sign;
wire                  ebreak;
wire                  op;
wire                  CSR_ren;
wire                  CSR_wen;
wire                  dest_wen;

wire [DATA_LEN-1:0] imm;
wire [DATA_LEN-1:0] imm_I,imm_J,imm_U,imm_B,imm_S,CSR_imm;

wire [DATA_LEN-1:0]  CSR_operand1;
// wire [DATA_LEN-1:0]  CSR_operand2;
wire [1:0]  csr_wfunc;
wire [11:0] CSR_addr;

wire [6:0] funct7;
wire [2:0] funct3;

wire I_flag,J_flag,U_flag,B_flag,S_flag,R_flag,CSR_flag;
wire load_flag,arith_flag,csr_rw_flag;

wire lui,auipc;
wire jal;
wire jalr;
wire sub;
wire OR,ori;
wire XOR,xori;
wire AND,andi;
wire beq,bne,blt,bltu,bge,bgeu;
wire slt,sltu,slti,sltiu;
wire sll,srl,sra,slli,srli,srai;
wire lb,lbu,lh,lhu,lw;
wire sb,sh,sw;
wire csrrw,csrrwi;
wire csrrs,csrrsi;
wire csrrc,csrrci;
// wire addi;
// wire ebreak;

wire LR;
wire AL;
wire is_or;
wire is_xor;
wire is_and;
wire is_cmp;
wire is_unsign;
wire is_shift;
wire is_byte;
wire is_half;
wire is_word;
wire is_store;
wire is_load;
wire is_beq;
wire is_bne;
wire is_blt;
wire is_bge;
wire is_bltu;
wire is_bgeu;


wire ecall;
wire mret;  

assign rs1 = IF_ID_reg_inst[19:15];
assign rs2 = IF_ID_reg_inst[24:20];
assign rd  = IF_ID_reg_inst[11:7 ];

assign funct3 = IF_ID_reg_inst[14:12];
assign funct7 = IF_ID_reg_inst[31:25];

assign imm_I = {{(DATA_LEN-12){IF_ID_reg_inst[31]}},IF_ID_reg_inst[31:20]};
assign imm_S = {{(DATA_LEN-12){IF_ID_reg_inst[31]}},IF_ID_reg_inst[31:25],IF_ID_reg_inst[11:7]};
assign imm_B = {{(DATA_LEN-12){IF_ID_reg_inst[31]}},IF_ID_reg_inst[7],IF_ID_reg_inst[30:25],IF_ID_reg_inst[11:8],1'b0};
assign imm_U = {{(DATA_LEN-31){IF_ID_reg_inst[31]}},IF_ID_reg_inst[30:12],12'h0};
assign imm_J = {{(DATA_LEN-20){IF_ID_reg_inst[31]}},IF_ID_reg_inst[19:12],IF_ID_reg_inst[20],IF_ID_reg_inst[30:21],1'b0};
assign CSR_imm = {{CSR_FILLER_LEN{1'b0}},rs1};
// 
assign R_flag   = (IF_ID_reg_inst[6:0]==7'b0110011)?1'b1:1'b0;
assign S_flag   = (IF_ID_reg_inst[6:0]==7'b0100011)?1'b1:1'b0;
assign I_flag   = (load_flag|arith_flag|jalr);
assign B_flag   = (IF_ID_reg_inst[6:0]==7'b1100011)?1'b1:1'b0;
assign U_flag   = (lui|auipc);
assign J_flag   = jal;
assign CSR_flag = ((IF_ID_reg_inst[6:0]==7'b1110011)&(~ebreak)&(~ecall))?1'b1:1'b0;

assign imm = (I_flag)?imm_I:(
    (U_flag)?imm_U:(
        (J_flag)?imm_J:(
            (S_flag)?imm_S:imm_B
        )
    )
);

assign load_flag    = (IF_ID_reg_inst[6:0]==7'b0000011)?1'b1:1'b0;
assign arith_flag   = (IF_ID_reg_inst[6:0]==7'b0010011)?1'b1:1'b0;

assign lui      =   (IF_ID_reg_inst[6:0]  ==  7'b0110111  ) ? 1'b1 : 1'b0;
assign auipc    =   (IF_ID_reg_inst[6:0]  ==  7'b0010111  ) ? 1'b1 : 1'b0;
assign jal      =   (IF_ID_reg_inst[6:0]  ==  7'b1101111  ) ? 1'b1 : 1'b0;
assign jalr     =   (IF_ID_reg_inst[6:0]  ==  7'b1100111  ) ? 1'b1 : 1'b0;
// assign addi     =   (arith_flag&(funct3==3'b000)) ? 1'b1 : 1'b0;
assign sub      =   (R_flag&({funct3,funct7}==10'h20))?1'b1:1'b0;

assign OR       =   (R_flag&({funct7,funct3}==10'h6 ))?1'b1:1'b0;
assign ori      =   (arith_flag & (funct3 == 3'h6))   ?1'b1:1'b0; 
assign AND      =   (R_flag&({funct7,funct3}==10'h7 ))?1'b1:1'b0;
assign andi     =   (arith_flag & (funct3 == 3'h7))   ?1'b1:1'b0; 
assign XOR      =   (R_flag&({funct7,funct3}==10'h4 ))?1'b1:1'b0;
assign xori     =   (arith_flag & (funct3 == 3'h4))   ?1'b1:1'b0;

assign slt      =   (R_flag&({funct7,funct3}==10'h2 ))?1'b1:1'b0;
assign slti     =   (arith_flag & (funct3 == 3'h2))   ?1'b1:1'b0;
assign sltu     =   (R_flag&({funct7,funct3}==10'h3 ))?1'b1:1'b0;
assign sltiu    =   (arith_flag & (funct3 == 3'h3))   ?1'b1:1'b0;

assign sll      =   (R_flag&({funct7,funct3}==10'h001 ))?1'b1:1'b0;
assign slli     =   (arith_flag&({IF_ID_reg_inst[31:FILLER_LEN],funct3}=='h001))?1'b1:1'b0;
assign srl      =   (R_flag&({funct7,funct3}==10'h005 ))?1'b1:1'b0;
assign srli     =   (arith_flag&({IF_ID_reg_inst[31:FILLER_LEN],funct3}=='h005))?1'b1:1'b0;
assign sra      =   (R_flag&({funct7,funct3}==10'h105 ))?1'b1:1'b0;
assign srai     =   (arith_flag&({IF_ID_reg_inst[31:FILLER_LEN],funct3}=='h105))?1'b1:1'b0;

assign beq      =   (B_flag&(funct3==3'b000))?1'b1:1'b0;
assign bne      =   (B_flag&(funct3==3'b001))?1'b1:1'b0;
assign blt      =   (B_flag&(funct3==3'b100))?1'b1:1'b0;
assign bge      =   (B_flag&(funct3==3'b101))?1'b1:1'b0;
assign bltu     =   (B_flag&(funct3==3'b110))?1'b1:1'b0;
assign bgeu     =   (B_flag&(funct3==3'b111))?1'b1:1'b0;

assign lb       =   (load_flag&(funct3==3'b000))?1'b1:1'b0;
assign lbu      =   (load_flag&(funct3==3'b100))?1'b1:1'b0;
assign lh       =   (load_flag&(funct3==3'b001))?1'b1:1'b0;
assign lhu      =   (load_flag&(funct3==3'b101))?1'b1:1'b0;
assign lw       =   (load_flag&(funct3==3'b010))?1'b1:1'b0;

assign sb       =   (S_flag&(funct3==3'b000))?1'b1:1'b0;
assign sh       =   (S_flag&(funct3==3'b001))?1'b1:1'b0;
assign sw       =   (S_flag&(funct3==3'b010))?1'b1:1'b0;

assign csrrw    =   (CSR_flag&(funct3==3'b001))?1'b1:1'b0;
assign csrrs    =   (CSR_flag&(funct3==3'b010))?1'b1:1'b0;
assign csrrc    =   (CSR_flag&(funct3==3'b011))?1'b1:1'b0;
assign csrrwi   =   (CSR_flag&(funct3==3'b101))?1'b1:1'b0;
assign csrrsi   =   (CSR_flag&(funct3==3'b110))?1'b1:1'b0;
assign csrrci   =   (CSR_flag&(funct3==3'b111))?1'b1:1'b0;

assign mret     =   (IF_ID_reg_inst ==  32'h30200073) ? 1'b1 : 1'b0;
assign ecall    =   (IF_ID_reg_inst ==  32'h00000073) ? 1'b1 : 1'b0;
assign ebreak   =   (IF_ID_reg_inst ==  32'h00100073) ? 1'b1 : 1'b0;

`ifdef RISCV64

wire IW_flag;
wire RW_flag;
assign IW_flag   = (IF_ID_reg_inst[6:0]==7'b0011011)?1'b1:1'b0;
assign RW_flag   = (IF_ID_reg_inst[6:0]==7'b0111011)?1'b1:1'b0;

wire ld,lwu,sd;
wire slliw,srliw,sraiw;
wire subw,sllw,srlw,sraw;
wire is_word;

wire [2:0] control_sign_word;

assign lwu      =   (load_flag&(funct3==3'b110))?1'b1:1'b0;
assign ld       =   (load_flag&(funct3==3'b011))?1'b1:1'b0;
assign sd       =   (S_flag   &(funct3==3'b011))?1'b1:1'b0;

assign sllw     =   (RW_flag&({funct7,funct3}==10'h001 ))?1'b1:1'b0;
assign slliw    =   (IW_flag&({funct7,funct3}==10'h001 ))?1'b1:1'b0;
assign srlw     =   (RW_flag&({funct7,funct3}==10'h005 ))?1'b1:1'b0;
assign srliw    =   (IW_flag&({funct7,funct3}==10'h005 ))?1'b1:1'b0;
assign sraw     =   (RW_flag&({funct7,funct3}==10'h105 ))?1'b1:1'b0;
assign sraiw    =   (IW_flag&({funct7,funct3}==10'h105 ))?1'b1:1'b0;

assign subw     =   (RW_flag&({funct3,funct7}==10'h20))?1'b1:1'b0;

assign is_word  =   RW_flag|IW_flag;

assign control_sign_word = {is_word,ld,sd};

assign operand1 = ((csr_rw_flag)?CSR_operand1:((auipc|jalr|jal)?IF_ID_reg_PC:((lui)?{DATA_LEN{1'b0}}:src1)));
assign operand2 = (jalr|jal)?4:((B_flag|R_flag|RW_flag)?src2:imm);
assign op       = (B_flag|is_cmp|sub|subw);

assign is_or    = OR    |   ori |   csrrc   |   csrrci  |   csrrs   |   csrrsi;
assign is_xor   = XOR   |   xori;
assign is_and   = AND   |   andi;
assign is_cmp   = slt|slti|sltiu|sltu;
assign is_unsign= sltiu|sltu|lbu|lhu|lwu;
assign is_store = S_flag;
assign is_load  = load_flag;
assign is_beq   = beq;
assign is_bne   = bne;
assign is_blt   = blt;
assign is_bge   = bge;
assign is_bltu  = bltu;
assign is_bgeu  = bgeu;
assign is_byte  = lb|lbu;
assign is_half  = lh|lhu;
assign is_word  = lw|lwu;
assign is_shift = sll|slli|srl|srli|sra|srai|sllw|slliw|srlw|srliw|sraw|sraiw;
assign LR       = sll|slli|sllw|slliw;
assign AL       = IF_ID_reg_inst[30];

`else

assign operand1 = ((csr_rw_flag)?CSR_operand1:((auipc|jalr|jal)?IF_ID_reg_PC:((lui)?{DATA_LEN{1'b0}}:src1)));
assign operand2 = (jalr|jal)?4:((B_flag|R_flag)?src2:imm);
assign op       = (B_flag|is_cmp|sub);

assign is_or    = OR    |   ori |   csrrc   |   csrrci  |   csrrs   |   csrrsi;
assign is_xor   = XOR   |   xori;
assign is_and   = AND   |   andi;
assign is_cmp   = slt|slti|sltiu|sltu;
assign is_unsign= sltiu|sltu|lbu|lhu;
assign is_store = S_flag;
assign is_load  = load_flag;
assign is_beq   = beq;
assign is_bne   = bne;
assign is_blt   = blt;
assign is_bge   = bge;
assign is_bltu  = bltu;
assign is_bgeu  = bgeu;
assign is_byte  = lb|lbu;
assign is_half  = lh|lhu;
assign is_word  = lw;
assign is_shift = sll|slli|srl|srli|sra|srai;
assign LR       = sll|slli;
assign AL       = IF_ID_reg_inst[30];

`endif

assign operand3 = (jalr)?src1:IF_ID_reg_PC;
assign operand4 = imm;

assign inst_jump_flag = (B_flag);
assign jump_without   = (jal|jalr);

assign dest_wen = ((!(B_flag|S_flag|(CSR_flag&(~CSR_ren))|ebreak|mret)));

assign control_sign = {is_bgeu,is_bge,is_bne,is_beq,is_bltu,is_blt,
                        is_cmp,is_unsign,is_shift,AL,LR,is_and,is_xor,is_or};          
// assign control_sign = {is_or,is_xor,is_and,LR,AL,is_shift,is_unsign,is_cmp,is_blt,is_bltu,
//                         is_beq,is_bne,is_bge,is_bgeu,is_load,is_byte,is_half,is_word};
assign  load_sign = {is_word,is_half,is_byte,is_unsign,is_load};
assign store_sign = {sw,sh,sb,is_store};

assign CSR_operand1 = (IF_ID_reg_inst[14])?CSR_imm:src1;
// assign CSR_operand2 = (IF_ID_reg_inst[13:12]==2'b01)?0:((IF_ID_reg_inst[13:12]==2'b10)?csr_rdata:(~csr_rdata));
assign csr_rw_flag = (csrrc|csrrci|csrrw|csrrwi|csrrs|csrrsi);
assign CSR_addr = IF_ID_reg_inst[31:20];
wire csrrw_with_rd0;
assign csrrw_with_rd0 = ((csrrw|csrrwi)&(rd==0));
assign CSR_ren = (( ( ~csrrw_with_rd0 ) & csr_rw_flag));
wire csrr_with_rs0,csrr_with_imm0;
assign csrr_with_rs0 = ((rs1==0)&(csrrc|csrrs));
assign csrr_with_imm0 = ((CSR_imm==0)&(csrrci|csrrsi));
assign CSR_wen = (( ( ~ ( csrr_with_imm0 | csrr_with_rs0 ) ) & csr_rw_flag ));
assign csr_sign = {ecall,mret};
assign csr_wfunc = IF_ID_reg_inst[13:12];

assign rs1_valid = R_flag|I_flag|S_flag|B_flag|csrrw|csrrs|csrrc;
assign rs2_valid = R_flag|S_flag|B_flag;

//temp because the idu decode but not need one cycle 
// assign inst_ready   = decode_ready;
// assign decode_valid = inst_valid;
FF_D_with_syn_rst #(1,0)           u_decode_valid    (clk,rst_n,ID_reg_decode_flush|(MON_ID_src_block_flag&EX_reg_execute_enable),ID_reg_decode_enable,IF_ID_reg_inst_valid,ID_EX_reg_decode_valid);
FF_D_without_asyn_rst #(5)         u_rd              (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,rd,ID_EX_reg_rd);
FF_D_without_asyn_rst #(14)        u_control_sign    (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,control_sign,ID_EX_reg_control_sign);
FF_D_without_asyn_rst #(2)         u_csr_sign        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,csr_sign,ID_EX_reg_csr_sign);
FF_D_without_asyn_rst #(1)         u_inst_jump_flag  (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,inst_jump_flag,ID_EX_reg_inst_jump_flag);
FF_D_without_asyn_rst #(1)         u_jump_without    (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,jump_without,ID_EX_reg_jump_without);
FF_D_without_asyn_rst #(5)         u_load_sign       (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,load_sign,ID_EX_reg_load_sign);
FF_D_without_asyn_rst #(4)         u_store_sign      (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,store_sign,ID_EX_reg_store_sign);
FF_D_without_asyn_rst #(1)         u_ebreak          (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,ebreak,ID_EX_reg_ebreak);
FF_D_without_asyn_rst #(1)         u_op              (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,op,ID_EX_reg_op);
FF_D_without_asyn_rst #(1)         u_CSR_ren         (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,CSR_ren,ID_EX_reg_CSR_ren);
FF_D_without_asyn_rst #(2)         u_CSR_wfunc       (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,csr_wfunc,ID_EX_reg_csr_wfunc);
FF_D_without_asyn_rst #(12)        u_CSR_addr        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,CSR_addr,ID_EX_reg_CSR_addr);
// assign csr_ren = CSR_ren;
FF_D_without_asyn_rst #(1)         u_CSR_wen         (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,CSR_wen,ID_EX_reg_CSR_wen);
FF_D_without_asyn_rst #(1)         u_dest_wen        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,dest_wen,ID_EX_reg_dest_wen);
`ifdef SIM
FF_D_without_asyn_rst #(32)        u_inst            (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,IF_ID_reg_inst,ID_EX_reg_inst);
`endif
FF_D_without_asyn_rst #(DATA_LEN)  u_PC              (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,IF_ID_reg_PC,ID_EX_reg_PC);
FF_D_without_asyn_rst #(DATA_LEN)  u_operand1        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,operand1,ID_EX_reg_operand1);
FF_D_without_asyn_rst #(DATA_LEN)  u_operand2        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,operand2,ID_EX_reg_operand2);
FF_D_without_asyn_rst #(DATA_LEN)  u_operand3        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,operand3,ID_EX_reg_operand3);
FF_D_without_asyn_rst #(DATA_LEN)  u_operand4        (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,operand4,ID_EX_reg_operand4);
FF_D_without_asyn_rst #(DATA_LEN)  u_store_data      (clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,src2,ID_EX_reg_store_data);

`ifdef RISCV64

FF_D_without_asyn_rst #(3)         u_control_sign_word(clk,ID_reg_decode_enable&IF_ID_reg_inst_valid,control_sign_word,ID_EX_reg_control_sign_word);

`endif

endmodule //idu
//the channel 1 is to GPR, channel 2 is to jump_pc
// U:
// LUI: 	0 	    +	    imm
// AUPIC:	PC	    +       imm
// J:
// JAL：	PC	    + 	    4		    PC	        +	imm
// I:
// JALR:	PC	    +	    4		    SRC1	    +	imm
// LB:	    SRC1	+	    imm
// LH:	    SRC1	+	    imm
// LW:	    SRC1	+	    imm
// LBU:	    SRC1	+	    imm
// LHU:	    SRC1	+	    imm
// ADDI:	SRC1	+	    imm
// SLTI:	SRC1	-	    imm
// SLTIU:	SRC1	-	    imm
// XORI:	SRC1	^	    imm
// ORI:	    SRC1	|	    imm
// ANDI:	SRC1	&	    imm
// SLLI:	SRC1	no_op	imm
// SRLI:	SRC1	no_op	imm
// SRAI:	SRC1	no_op	imm
// B:    
// BEQ:	    SRC1	-	    SRC2		PC	        +	imm
// BNE:	    SRC1	-	    SRC2		PC	        +	imm
// BLT:	    SRC1	-	    SRC2		PC	        +	imm
// BGE:	    SRC1	-	    SRC2		PC	        +	imm
// BLTU:	SRC1	-	    SRC2		PC	        +	imm
// BGEU:	SRC1	-	    SRC2		PC	        +	imm
// S:
// SB:	    SRC1	+	    imm
// SH:	    SRC1	+	    imm
// SW:	    SRC1	+	    imm
// R:   
// ADD:	    SRC1	+	    SRC2
// SUB:	    SRC1	-	    SRC2
// SLL:	    SRC1	no_op	SRC2
// SLT:	    SRC1	-	    SRC2
// SLTU:	SRC1	-	    SRC2
// XOR: 	SRC1	^	    SRC2
// SRL: 	SRC1	no_op	SRC2
// SRA: 	SRC1	no_op	SRC2
// OR:	    SRC1	|	    SRC2
// AND: 	SRC1	&	    SRC2

//the channel 1 is write CSR, channel 2 is to read CSR 
// CSR:
// CSRRW	SRC1	+	    0		    CSR_RDATA	+	0
// CSRRS	SRC1	|	    CSR_RDATA	CSR_RDATA	+	0
// CSRRC	SRC1	|	    ~CSR_RDATA	CSR_RDATA	+	0
// CSRRWI	CSR_imm	+	    0		    CSR_RDATA	+	0
// CSRRSI	CSR_imm	|	    CSR_RDATA	CSR_RDATA	+	0
// CSRRCI	CSR_imm	|	    ~CSR_RDATA	CSR_RDATA	+	0

//if unusual happen, Don't use the channels shutdown the wen and ren 
//if unusual return, Don't use the ALU but use the channel 2 to jump back and shutdown the wen and ren  
// unusual
// ECALL
// ret	
