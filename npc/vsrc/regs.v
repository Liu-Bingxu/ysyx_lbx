module regs#(parameter DATA_LEN=32)(
	input 			        clk,
	input 			        rst_n,
	input 			        dest_wen,
	input  [4:0]		    rs1,
	input  [4:0]		    rs2,
	input  [4:0]		    rd ,
	input  [DATA_LEN-1:0]	dest_data,
	output [DATA_LEN-1:0]	src1,
	output [DATA_LEN-1:0]	src2
);

wire [DATA_LEN-1:0] riscv_reg [0:31];
wire x1_wen,x2_wen,x3_wen,x4_wen,x5_wen,x6_wen,x7_wen,x8_wen,x9_wen,x10_wen,x11_wen;
wire x12_wen,x13_wen,x14_wen,x15_wen,x16_wen,x17_wen,x18_wen,x19_wen,x20_wen,x21_wen;
wire x22_wen,x23_wen,x24_wen,x25_wen,x26_wen,x27_wen,x28_wen,x29_wen,x30_wen,x31_wen;

assign x1_wen	    = ((rd == 5'd1 )&dest_wen);
assign x2_wen	    = ((rd == 5'd2 )&dest_wen);
assign x3_wen	    = ((rd == 5'd3 )&dest_wen);
assign x4_wen	    = ((rd == 5'd4 )&dest_wen);
assign x5_wen	    = ((rd == 5'd5 )&dest_wen);
assign x6_wen	    = ((rd == 5'd6 )&dest_wen);
assign x7_wen	    = ((rd == 5'd7 )&dest_wen);
assign x8_wen	    = ((rd == 5'd8 )&dest_wen);
assign x9_wen	    = ((rd == 5'd9 )&dest_wen);
assign x10_wen      = ((rd == 5'd10)&dest_wen);
assign x11_wen      = ((rd == 5'd11)&dest_wen);
assign x12_wen      = ((rd == 5'd12)&dest_wen);
assign x13_wen      = ((rd == 5'd13)&dest_wen);
assign x14_wen      = ((rd == 5'd14)&dest_wen);
assign x15_wen      = ((rd == 5'd15)&dest_wen);
assign x16_wen      = ((rd == 5'd16)&dest_wen);
assign x17_wen      = ((rd == 5'd17)&dest_wen);
assign x18_wen      = ((rd == 5'd18)&dest_wen);
assign x19_wen      = ((rd == 5'd19)&dest_wen);
assign x20_wen      = ((rd == 5'd20)&dest_wen);
assign x21_wen      = ((rd == 5'd21)&dest_wen);
assign x22_wen      = ((rd == 5'd22)&dest_wen);
assign x23_wen      = ((rd == 5'd23)&dest_wen);
assign x24_wen      = ((rd == 5'd24)&dest_wen);
assign x25_wen      = ((rd == 5'd25)&dest_wen);
assign x26_wen      = ((rd == 5'd26)&dest_wen);
assign x27_wen      = ((rd == 5'd27)&dest_wen);
assign x28_wen      = ((rd == 5'd28)&dest_wen);
assign x29_wen      = ((rd == 5'd29)&dest_wen);
assign x30_wen      = ((rd == 5'd30)&dest_wen);
assign x31_wen      = ((rd == 5'd31)&dest_wen);

assign src1=riscv_reg[rs1];
assign src2=riscv_reg[rs2];

assign riscv_reg[0]={DATA_LEN{1'b0}};
FF_D #(DATA_LEN,0) x1	(clk,rst_n,x1_wen, dest_data,riscv_reg[1] );
FF_D #(DATA_LEN,0) x2	(clk,rst_n,x2_wen, dest_data,riscv_reg[2] );
FF_D #(DATA_LEN,0) x3	(clk,rst_n,x3_wen, dest_data,riscv_reg[3] );
FF_D #(DATA_LEN,0) x4	(clk,rst_n,x4_wen, dest_data,riscv_reg[4] );
FF_D #(DATA_LEN,0) x5	(clk,rst_n,x5_wen, dest_data,riscv_reg[5] );
FF_D #(DATA_LEN,0) x6	(clk,rst_n,x6_wen, dest_data,riscv_reg[6] );
FF_D #(DATA_LEN,0) x7	(clk,rst_n,x7_wen, dest_data,riscv_reg[7] );
FF_D #(DATA_LEN,0) x8	(clk,rst_n,x8_wen, dest_data,riscv_reg[8] );
FF_D #(DATA_LEN,0) x9	(clk,rst_n,x9_wen, dest_data,riscv_reg[9] );
FF_D #(DATA_LEN,0) x10	(clk,rst_n,x10_wen,dest_data,riscv_reg[10]);
FF_D #(DATA_LEN,0) x11	(clk,rst_n,x11_wen,dest_data,riscv_reg[11]);
FF_D #(DATA_LEN,0) x12	(clk,rst_n,x12_wen,dest_data,riscv_reg[12]);
FF_D #(DATA_LEN,0) x13	(clk,rst_n,x13_wen,dest_data,riscv_reg[13]);
FF_D #(DATA_LEN,0) x14	(clk,rst_n,x14_wen,dest_data,riscv_reg[14]);
FF_D #(DATA_LEN,0) x15	(clk,rst_n,x15_wen,dest_data,riscv_reg[15]);
FF_D #(DATA_LEN,0) x16	(clk,rst_n,x16_wen,dest_data,riscv_reg[16]);
FF_D #(DATA_LEN,0) x17	(clk,rst_n,x17_wen,dest_data,riscv_reg[17]);
FF_D #(DATA_LEN,0) x18	(clk,rst_n,x18_wen,dest_data,riscv_reg[18]);
FF_D #(DATA_LEN,0) x19	(clk,rst_n,x19_wen,dest_data,riscv_reg[19]);
FF_D #(DATA_LEN,0) x20	(clk,rst_n,x20_wen,dest_data,riscv_reg[20]);
FF_D #(DATA_LEN,0) x21	(clk,rst_n,x21_wen,dest_data,riscv_reg[21]);
FF_D #(DATA_LEN,0) x22	(clk,rst_n,x22_wen,dest_data,riscv_reg[22]);
FF_D #(DATA_LEN,0) x23	(clk,rst_n,x23_wen,dest_data,riscv_reg[23]);
FF_D #(DATA_LEN,0) x24	(clk,rst_n,x24_wen,dest_data,riscv_reg[24]);
FF_D #(DATA_LEN,0) x25	(clk,rst_n,x25_wen,dest_data,riscv_reg[25]);
FF_D #(DATA_LEN,0) x26	(clk,rst_n,x26_wen,dest_data,riscv_reg[26]);
FF_D #(DATA_LEN,0) x27	(clk,rst_n,x27_wen,dest_data,riscv_reg[27]);
FF_D #(DATA_LEN,0) x28	(clk,rst_n,x28_wen,dest_data,riscv_reg[28]);
FF_D #(DATA_LEN,0) x29	(clk,rst_n,x29_wen,dest_data,riscv_reg[29]);
FF_D #(DATA_LEN,0) x30	(clk,rst_n,x30_wen,dest_data,riscv_reg[30]);
FF_D #(DATA_LEN,0) x31	(clk,rst_n,x31_wen,dest_data,riscv_reg[31]);

endmodule
