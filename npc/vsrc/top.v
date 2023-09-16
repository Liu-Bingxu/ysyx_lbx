module top#(parameter DATA_LEN=32) (
    input                   sys_clk,
    input                   sys_rst_n,
    input  [DATA_LEN-1:0]   inst_in,
    // output [DATA_LEN-1:0]   vaddr,
    output [DATA_LEN-1:0]   PC_out,

    output [DATA_LEN-1:0]   src2_use

);

reg  rst_n_s1, rst_n_s2;
wire rst_n;
always @ (posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n) begin
        rst_n_s2 <= 1'b0;
        rst_n_s1 <= 1'b0;
    end
    else begin
        rst_n_s1 <= 1'b1;
        rst_n_s2 <= rst_n_s1;
    end
end
assign rst_n = rst_n_s2;

wire [DATA_LEN-1:0] 	src1;
wire [DATA_LEN-1:0] 	src2;

wire [4:0]  	        rs1;
wire [4:0]  	        rs2;
wire [4:0]  	        rd;
wire [DATA_LEN-1:0] 	operand1;
wire [DATA_LEN-1:0] 	operand2;
wire        	        op;

wire [DATA_LEN-1:0] 	pc_out;
wire [DATA_LEN-1:0] 	inst_fetch;

wire                	dest_wen;
wire                	Jump_flag;
wire [DATA_LEN-1:0] 	dest_data;
wire [DATA_LEN-1:0] 	Jump_PC;

wire ebreak;

assign PC_out = pc_out;
//
assign src2_use = src2;
//
regs #(DATA_LEN)u_regs(
    .clk       	( sys_clk    ),
    .rst_n     	( rst_n      ),
    .dest_wen  	( dest_wen   ),
    .rs1       	( rs1        ),
    .rs2       	( rs2        ),
    .rd        	( rd         ),
    .dest_data 	( dest_data  ),
    .src1      	( src1       ),
    .src2      	( src2       )
);


idu #(DATA_LEN)u_idu(
    // .clk      	( sys_clk   ),
    // .rst_n    	( rst_n     ),
    .inst     	( inst_fetch),
    .src1     	( src1      ),
    // .src2     	( src2      ),
    .rs1      	( rs1       ),
    .rs2      	( rs2       ),
    .rd       	( rd        ),
    .operand1 	( operand1  ),
    .operand2 	( operand2  ),
    .ebreak     ( ebreak    ),
    .op       	( op        )
);


ifu #(DATA_LEN)u_ifu(
    .clk        	( sys_clk     ),
    .rst_n      	( rst_n       ),
    .Jump_flag  	( Jump_flag   ),
    .Jump_PC    	( Jump_PC     ),
    .inst_in    	( inst_in     ),
    .pc_out     	( pc_out      ),
    .inst_fetch 	( inst_fetch  )
);

exu #(DATA_LEN)u_exu(
    .clk        ( sys_clk    ),
    .operand1  	( operand1   ),
    .operand2  	( operand2   ),
    .op        	( ~op        ),
    .dest_wen  	( dest_wen   ),
    .Jump_flag 	( Jump_flag  ),
    .dest_data 	( dest_data  ),
    .ebreak     ( ebreak     ),
    .Jump_PC   	( Jump_PC    )
);


endmodule //top