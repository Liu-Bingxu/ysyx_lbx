module idu#(parameter DATA_LEN=32) (
    // input                   clk,
    // input                   rst_n,
    input                   unusual_flag,
    input  [31:0]           inst,
    input  [DATA_LEN-1:0]   PC_S,
    input  [DATA_LEN-1:0]   PC,
    input  [DATA_LEN-1:0]   src1,
    input  [DATA_LEN-1:0]   src2,
    input  [DATA_LEN-1:0]   csr_rdata,
    output [4 :0]           rs1,
    output [4 :0]           rs2,
    output [4 :0]           rd,
    output [11:0]           CSR_addr,
    output [DATA_LEN-1:0]   operand1,   
    output [DATA_LEN-1:0]   operand2, 
    output [DATA_LEN-1:0]   operand3,   
    output [DATA_LEN-1:0]   operand4,
    output [17:0]           control_sign, 
    output [2:0]            csr_sign,
    output                  inst_jump_flag,
    output                  jump_without,
    output [3:0]            store_sign,
    output                  ebreak,
    output                  CSR_ren,
    output                  CSR_wen,
    output                  dest_wen,
    output                  op
);

localparam FILLER_LEN = 20 + $clog2(DATA_LEN);
localparam CSR_FILLER_LEN = DATA_LEN-5;

wire [DATA_LEN-1:0] imm;
wire [DATA_LEN-1:0] imm_I,imm_J,imm_U,imm_B,imm_S,CSR_imm;

wire [DATA_LEN-1:0]  CSR_operand1;
wire [DATA_LEN-1:0]  CSR_operand2;

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
wire is_load;
wire is_beq;
wire is_bne;
wire is_blt;
wire is_bge;
wire is_bltu;
wire is_bgeu;


wire ecall;
wire mret;  

assign rs1 = inst[19:15];
assign rs2 = inst[24:20];
assign rd  = inst[11:7 ];

assign funct3 = inst[14:12];
assign funct7 = inst[31:25];

assign imm_I = {{20{inst[31]}},inst[31:20]};
assign imm_S = {{20{inst[31]}},inst[31:25],inst[11:7]};
assign imm_B = {{20{inst[31]}},inst[7],inst[30:25],inst[11:8],1'b0};
assign imm_U = {inst[31:12],12'h0};
assign imm_J = {{12{inst[31]}},inst[19:12],inst[20],inst[30:21],1'b0};
assign CSR_imm = {{CSR_FILLER_LEN{1'b0}},rs1};
// 
assign R_flag   = (inst[6:0]==7'b0110011)?1'b1:1'b0;
assign S_flag   = (inst[6:0]==7'b0100011)?1'b1:1'b0;
assign I_flag   = (load_flag|arith_flag|jalr);
assign B_flag   = (inst[6:0]==7'b1100011)?1'b1:1'b0;
assign U_flag   = (lui|auipc);
assign J_flag   = jal;
assign CSR_flag = ((inst[6:0]==7'b1110011)&(~ebreak)&(~ecall))?1'b1:1'b0;

assign imm = (I_flag)?imm_I:(
    (U_flag)?imm_U:(
        (J_flag)?imm_J:(
            (S_flag)?imm_S:imm_B
        )
    )
);

assign load_flag    = (inst[6:0]==7'b0000011)?1'b1:1'b0;
assign arith_flag   = (inst[6:0]==7'b0010011)?1'b1:1'b0;

assign lui      =   (inst[6:0]  ==  7'b0110111  ) ? 1'b1 : 1'b0;
assign auipc    =   (inst[6:0]  ==  7'b0010111  ) ? 1'b1 : 1'b0;
assign jal      =   (inst[6:0]  ==  7'b1101111  ) ? 1'b1 : 1'b0;
assign jalr     =   (inst[6:0]  ==  7'b1100111  ) ? 1'b1 : 1'b0;
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
assign slli     =   (arith_flag&({inst[31:FILLER_LEN],funct3}=='h001))?1'b1:1'b0;
assign srl      =   (R_flag&({funct7,funct3}==10'h005 ))?1'b1:1'b0;
assign srli     =   (arith_flag&({inst[31:FILLER_LEN],funct3}=='h005))?1'b1:1'b0;
assign sra      =   (R_flag&({funct7,funct3}==10'h105 ))?1'b1:1'b0;
assign srai     =   (arith_flag&({inst[31:FILLER_LEN],funct3}=='h105))?1'b1:1'b0;

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

assign mret     =   (inst       ==  32'h30200073) ? 1'b1 : 1'b0;
assign ecall    =   (inst       ==  32'h00000073) ? 1'b1 : 1'b0;
assign ebreak   =   (inst       ==  32'h00100073) ? 1'b1 : 1'b0;

assign operand1 = ((csr_rw_flag)?CSR_operand1:((auipc)?PC:((J_flag|jalr|lui)?32'h0:src1)));
assign operand2 = ((csr_rw_flag)?CSR_operand2:((jalr|jal)?PC_S:((B_flag|R_flag)?src2:imm)));
assign op       = (B_flag|is_cmp|sub);

assign operand3 = (csr_rw_flag==1'b1)?csr_rdata:((jalr)?src1:PC);
assign operand4 = (csr_rw_flag==1'b1)?0:(imm);

assign inst_jump_flag = (B_flag);
assign jump_without   = (jal|jalr);

assign dest_wen = (!(B_flag|S_flag|(CSR_flag&(~CSR_ren))|unusual_flag|ebreak));

assign is_or    = OR    |   ori |   csrrc   |   csrrci  |   csrrs   |   csrrsi;
assign is_xor   = XOR   |   xori;
assign is_and   = AND   |   andi;
assign is_cmp   = slt|slti|sltiu|sltu;
assign is_unsign= sltiu|sltu|lbu|lhu;
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
assign AL       = inst[30];

assign control_sign = {is_word,is_half,is_byte,is_load,is_bgeu,is_bge,is_bne,is_beq,is_bltu,is_blt,
                        is_cmp,is_unsign,is_shift,AL,LR,is_and,is_xor,is_or};          
// assign control_sign = {is_or,is_xor,is_and,LR,AL,is_shift,is_unsign,is_cmp,is_blt,is_bltu,
//                         is_beq,is_bne,is_bge,is_bgeu,is_load,is_byte,is_half,is_word};

assign store_sign = {sw,sh,sb,S_flag};

assign CSR_operand1 = (inst[14])?CSR_imm:src1;
assign CSR_operand2 = (inst[13:12]==2'b01)?0:((inst[13:12]==2'b10)?csr_rdata:(~csr_rdata));
assign csr_rw_flag = (csrrc|csrrci|csrrw|csrrwi|csrrs|csrrsi);
assign CSR_addr = inst[31:20];
wire csrrw_with_rd0;
assign csrrw_with_rd0 = ((csrrw|csrrwi)&(rd==0));
assign CSR_ren = ( ( ~csrrw_with_rd0 ) & csr_rw_flag);
wire csrr_with_rs0,csrr_with_imm0;
assign csrr_with_rs0 = ((rs1==0)&(csrrc|csrrs));
assign csrr_with_imm0 = ((CSR_imm==0)&(csrrci|csrrsi));
assign CSR_wen = ( ( ~ ( csrr_with_imm0 | csrr_with_rs0 ) ) & csr_rw_flag );
assign csr_sign = {ecall,mret,csr_rw_flag};

endmodule //idu
