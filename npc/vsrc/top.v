module top#(parameter DATA_LEN=32) (
    input                   sys_clk,
    input                   sys_rst_n
    // input  [DATA_LEN-1:0]   inst_in,
    // output [DATA_LEN-1:0]   PC_out
);

wire [DATA_LEN-1:0] PC_out;
wire [DATA_LEN-1:0] inst_in;

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
wire [DATA_LEN-1:0] 	operand3;
wire [DATA_LEN-1:0] 	operand4;
wire [17:0]         	control_sign;
wire [3:0]          	store_sign;
wire                	inst_jump_flag;
wire                    jump_without;
wire                	ebreak;
wire                	op;
wire [11:0]         	CSR_addr;
wire [2:0]          	csr_sign;
wire                	CSR_ren;
wire                	CSR_wen;

wire [DATA_LEN-1:0] 	PC_S;
// wire [DATA_LEN-1:0] 	PC_D;
wire [DATA_LEN-1:0] 	PC_now;
wire [DATA_LEN-1:0] 	inst_fetch;

wire                	dest_wen;
wire                	Jump_flag;
wire [DATA_LEN-1:0] 	dest_data;
wire [DATA_LEN-1:0] 	Jump_PC;
wire [DATA_LEN-1:0] 	addr_load;
wire [DATA_LEN-1:0]     pre_data;
wire [DATA_LEN-1:0]     csr_wdata;
wire [DATA_LEN-1:0]     cause;
wire                	unusual_flag;

wire [DATA_LEN-1:0] 	csr_rdata;
wire [DATA_LEN-1:0] 	mepc;
wire [DATA_LEN-1:0] 	mtvec;

assign PC_out = PC_now;

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

csr #(DATA_LEN)u_csr(
    .clk          	( sys_clk       ),
    .rst_n        	( rst_n         ),
    .wen          	( CSR_wen       ),
    .ren          	( CSR_ren       ),
    .unusual_flag 	( unusual_flag  ),
    .addr         	( CSR_addr      ),
    .wdata        	( csr_wdata     ),
    .PC           	( PC_now        ),
    .cause        	( cause         ),
    .rdata        	( csr_rdata     ),
    .mepc           ( mepc          ),
    .mtvec          ( mtvec         )
);



idu #(DATA_LEN)u_idu(
    // .clk      	( sys_clk           ),
    // .rst_n    	( rst_n             ),
    .unusual_flag   ( unusual_flag      ),
    .inst     	    ( inst_fetch        ),
    .src1     	    ( src1              ),
    .src2     	    ( src2              ),
    .PC_S           ( PC_S              ),
    .PC             ( PC_now            ),
    .rs1      	    ( rs1               ),
    .rs2      	    ( rs2               ),
    .rd       	    ( rd                ),
    .csr_rdata      ( csr_rdata         ),
    .CSR_addr       ( CSR_addr          ),
    .operand1 	    ( operand1          ),
    .operand2 	    ( operand2          ),
    .operand3       ( operand3          ),
    .operand4       ( operand4          ),
    .control_sign   ( control_sign      ),
    .csr_sign       ( csr_sign          ),
    .inst_jump_flag ( inst_jump_flag    ),
    .jump_without   ( jump_without      ),
    .store_sign     ( store_sign        ),
    .ebreak         ( ebreak            ),
    .CSR_ren        ( CSR_ren           ),
    .CSR_wen        ( CSR_wen           ),
    .dest_wen       ( dest_wen          ),
    .op             ( op                )
);

ifu #(DATA_LEN)u_ifu(
    .clk        	( sys_clk     ),
    .rst_n      	( rst_n       ),
    .Jump_flag  	( Jump_flag   ),
    .Jump_PC    	( Jump_PC     ),
    .inst_in    	( inst_in     ),
    .PC_S       	( PC_S        ),
    // .PC_D       	( PC_D        ),
    .PC_now     	( PC_now      ),
    .inst_fetch 	( inst_fetch  )
);

exu #(DATA_LEN)u_exu(
    .operand1  	        ( operand1          ),
    .operand2  	        ( operand2          ),
    .operand3       	( operand3          ),
    .operand4       	( operand4          ),
    .mepc           	( mepc              ),
    .mtvec          	( mtvec             ),
    .op             	( op                ),
    .csr_sign       	( csr_sign          ),
    .inst_jump_flag 	( inst_jump_flag    ),
    .jump_without       ( jump_without      ),
    .Jump_flag 	        ( Jump_flag         ),
    .addr_load      	( addr_load         ),
    .Control_signal 	( control_sign      ),
    .pre_data       	( pre_data          ),
    .unusual_flag   	( unusual_flag      ),
    .dest_data 	        ( dest_data         ),
    .Jump_PC   	        ( Jump_PC           ),
    .csr_wdata          ( csr_wdata         ),
    .cause              ( cause             )
);

monitor u_monitor(
    .clk        	( sys_clk           ),
    .store_sign 	( store_sign        ),
    .store_addr 	( dest_data         ),
    .store_data 	( src2              ),
    .addr_load  	( addr_load         ),
    .PC_out     	( PC_out            ),
    .inst_in    	( inst_in           ),
    .pre_data   	( pre_data          ),
    .is_load        ( control_sign[14]  ),
    .ebreak     	( ebreak            )
);



endmodule //top
