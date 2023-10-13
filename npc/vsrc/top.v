module top#(parameter DATA_LEN=32) (
    input                   sys_clk,
    input                   sys_rst_n
    // input  [DATA_LEN-1:0]   inst_in,
    // output [DATA_LEN-1:0]   PC_out
);

// wire [DATA_LEN-1:0] PC_out;
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
wire [15:0]         	control_sign;
wire [3:0]          	store_sign;
wire                	inst_jump_flag;
wire                    jump_without;
wire                	ebreak;
wire                	op;
wire [11:0]         	CSR_addr;
wire [2:0]          	csr_sign;
wire                	CSR_ren;
wire                	CSR_wen;
wire                	inst_ready;
wire                	decode_valid;
wire [4:0]          	load_sign;

// wire [DATA_LEN-1:0] 	PC_S;
// wire [DATA_LEN-1:0] 	PC_D;
wire [DATA_LEN-1:0] 	PC_now;
wire [DATA_LEN-1:0] 	inst_fetch;
wire [2:0]              ifu_rresp;
wire                    ifu_arready;
wire                    ifu_rvalid;
wire                	ifu_arvalid;
wire [DATA_LEN-1:0] 	PC_to_sram;
wire                	ifu_rready;
wire                	inst_valid;

wire                	dest_wen;
wire                	Jump_flag;
wire [DATA_LEN-1:0] 	dest_data;
wire [DATA_LEN-1:0] 	Jump_PC;
wire [DATA_LEN-1:0] 	addr_load;
// wire [DATA_LEN-1:0]     pre_data;
wire [DATA_LEN-1:0]     csr_wdata;
wire [DATA_LEN-1:0]     cause;
wire                	unusual_flag;

wire                	decode_ready;
wire                	ls_ready;
wire [DATA_LEN-1:0] 	csr_rdata;
wire [DATA_LEN-1:0] 	mepc;
wire [DATA_LEN-1:0] 	mtvec;

wire                    	ls_valid;
wire [DATA_LEN-1:0]     	load_data;
wire                    	load_wen;
wire                    	lsu_awvalid;
wire [DATA_LEN-1:0]     	lsu_waddr;
wire                    	lsu_wvalid;
wire [DATA_BIT_NUM-1:0] 	lsu_wstrob;
wire [DATA_LEN-1:0]     	lsu_wdata;
wire                    	lsu_bready;
wire                    	lsu_arvalid;
wire [DATA_LEN-1:0]     	lsu_raddr;
wire                    	lsu_rready;
wire [2:0]                  lsu_bresp;
wire [2:0]                  lsu_rresp;
wire [DATA_LEN-1:0]         lsu_rdata;
wire                        lsu_awready;
wire                        lsu_wready;
wire                        lsu_bvalid;
wire                        lsu_arready;
wire                        lsu_rvalid;

localparam DATA_STROB_LEN = DATA_LEN/8;
wire                 	    sram_arvalid;
wire                        sram_arready;
wire [DATA_LEN-1:0]  	    sram_raddr;
wire                        sram_rvalid;
wire                 	    sram_rready;
wire [2:0]                  sram_rresp;
wire [DATA_LEN-1:0]         sram_rdata;
wire                 	    sram_awvalid;
wire                        sram_awready;
wire [DATA_LEN-1:0]  	    sram_waddr;
wire                 	    sram_wvalid;
wire                        sram_wready;
wire [DATA_STROB_LEN-1:0] 	sram_wstrob;
wire [DATA_LEN-1:0]  	    sram_wdata;
wire                        sram_bvalid;
wire                 	    sram_bready;
wire [2:0]                  sram_bresp;

// assign PC_out = PC_now;

regs #(DATA_LEN)u_regs(
    .clk       	( sys_clk               ),
    .rst_n     	( rst_n                 ),
    .dest_wen  	( dest_wen|load_wen     ),
    .rs1       	( rs1                   ),
    .rs2       	( rs2                   ),
    .rd        	( rd                    ),
    .dest_data 	( dest_data             ),
    .src1      	( src1                  ),
    .src2      	( src2                  )
);

csr #(DATA_LEN)u_csr(
    .clk          	( sys_clk       ),
    .rst_n        	( rst_n         ),
    .wen          	( CSR_wen       ),
    .ren          	( CSR_ren       ),
    .unusual_flag 	( unusual_flag  ),
    .inst_valid     ( inst_valid    ),
    .addr         	( CSR_addr      ),
    .wdata        	( csr_wdata     ),
    .PC           	( PC_now        ),
    .cause        	( cause         ),
    .rdata        	( csr_rdata     ),
    .mepc           ( mepc          ),
    .mtvec          ( mtvec         )
);


idu #(DATA_LEN)u_idu(
    // .clk      	    ( sys_clk           ),
    // .rst_n    	    ( rst_n             ),
    .unusual_flag   ( unusual_flag      ),
    .inst    	    ( inst_fetch        ),
    .src1     	    ( src1              ),
    .src2     	    ( src2              ),
    // .PC             ( PC_S              ),
    .PC             ( PC_now            ),
    .inst_valid     ( inst_valid        ),
    .inst_ready     ( inst_ready        ),
    .decode_valid   ( decode_valid      ),
    .decode_ready   ( decode_ready      ),
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
    .load_sign      ( load_sign         ),
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
    .arvalid    	( ifu_arvalid ),
    .arready    	( ifu_arready ),
    .PC_to_sram 	( PC_to_sram  ),
    .inst_in    	( inst_in     ),
    .rvalid     	( ifu_rvalid  ),
    .rresp      	( ifu_rresp   ),
    .rready     	( ifu_rready  ),
    .inst_valid 	( inst_valid  ),
    .inst_ready 	( inst_ready  ),
    // .PC_S       	( PC_S        ),
    // .PC_D       	( PC_D        ),
    .PC_now     	( PC_now      ),
    .inst_fetch 	( inst_fetch  )
);

exu #(DATA_LEN)u_exu(
    // .clk                ( sys_clk           ),
    // .rst_n              ( rst_n             ),
    .decode_valid   	( decode_valid      ),
    .decode_ready   	( decode_ready      ),
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
    .ls_valid       	( ls_valid          ),
    .ls_ready       	( ls_ready          ),
    .load_data      	( load_data         ),
    .Control_signal 	( control_sign      ),
    // .pre_data       	( pre_data          ),
    .unusual_flag   	( unusual_flag      ),
    .dest_data 	        ( dest_data         ),
    .Jump_PC   	        ( Jump_PC           ),
    .csr_wdata          ( csr_wdata         ),
    .cause              ( cause             )
);
localparam  DATA_BIT_NUM = DATA_LEN/8;

lsu #(DATA_LEN,DATA_BIT_NUM)u_lsu(
    .clk        	( sys_clk           ),
    .rst_n      	( rst_n             ),
    .ls_valid   	( ls_valid          ),
    .ls_ready   	( ls_ready          ),
    .load_sign  	( load_sign         ),
    .store_sign 	( store_sign        ),
    .load_data  	( load_data         ),
    .load_wen   	( load_wen          ),
    .awvalid    	( lsu_awvalid       ),
    .awready    	( lsu_awready       ),
    .waddr      	( lsu_waddr         ),
    .store_addr 	( dest_data         ),
    .wvalid     	( lsu_wvalid        ),
    .wready     	( lsu_wready        ),
    .wstrob     	( lsu_wstrob        ),
    .wdata      	( lsu_wdata         ),
    .store_data 	( src2              ),
    .bvalid     	( lsu_bvalid        ),
    .bready     	( lsu_bready        ),
    .bresp      	( lsu_bresp         ),
    .arvalid    	( lsu_arvalid       ),
    .arready    	( lsu_arready       ),
    .raddr      	( lsu_raddr         ),
    .addr_load  	( addr_load         ),
    .rvalid     	( lsu_rvalid        ),
    .rresp      	( lsu_rresp         ),
    .rready     	( lsu_rready        ),
    .pre_data   	( lsu_rdata         )
);


monitor u_monitor(
    .clk        	( sys_clk           ),
    // .store_sign 	( store_sign        ),
    // .store_addr 	( dest_data         ),
    // .store_data 	( src2              ),
    // .addr_load  	( addr_load         ),
    // .PC_out     	( PC_out            ),
    // .inst_in    	( inst_in           ),
    // .pre_data   	( pre_data          ),
    // .is_load        ( control_sign[14]  ),
    .ebreak     	( ebreak            )
);

// localparam DATA_STROB_LEN = DATA_LEN/8;
// wire ifu_awready,ifu_wready,ifu_bvalid;
// wire [2:0]  ifu_bresp;
// assign unuse = ifu_awready&ifu_wready&ifu_bvalid&(ifu_bresp==3'b000);
// sram #(DATA_LEN,DATA_STROB_LEN,DATA_LEN)ifu_sram(
//     .clk     	( sys_clk                   ),
//     .rst_n   	( rst_n                     ),
//     .awvalid 	( 1'b0                      ),
//     .awready 	( ifu_awready               ),
//     .waddr   	( {DATA_LEN{1'b0}}          ),
//     .wvalid  	( 1'b0                      ),
//     .wready  	( ifu_wready                ),
//     .wdata   	( {DATA_LEN{1'b0}}          ),
//     .wstrob  	( {DATA_STROB_LEN{1'b0}}    ),
//     .bvalid  	( ifu_bvalid                ),
//     .bready  	( 1'b0                      ),
//     .bresp   	( ifu_bresp                 ),
//     .arvalid 	( ifu_arvalid               ),
//     .arready 	( ifu_arready               ),
//     .raddr   	( PC_to_sram                ),
//     .rvalid  	( ifu_rvalid                ),
//     .rready  	( ifu_rready                ),
//     .rdata   	( inst_in                   ),
//     .rresp   	( ifu_rresp                 )
// );

// sram #(DATA_LEN,DATA_STROB_LEN,DATA_LEN)lsu_sram(
//     .clk     	( sys_clk       ),
//     .rst_n   	( rst_n         ),
//     .awvalid 	( lsu_awvalid   ),
//     .awready 	( lsu_awready   ),
//     .waddr   	( lsu_waddr     ),
//     .wvalid  	( lsu_wvalid    ),
//     .wready  	( lsu_wready    ),
//     .wdata   	( lsu_wdata     ),
//     .wstrob  	( lsu_wstrob    ),
//     .bvalid  	( lsu_bvalid    ),
//     .bready  	( lsu_bready    ),
//     .bresp   	( lsu_bresp     ),
//     .arvalid 	( lsu_arvalid   ),
//     .arready 	( lsu_arready   ),
//     .raddr   	( lsu_raddr     ),
//     .rvalid  	( lsu_rvalid    ),
//     .rready  	( lsu_rready    ),
//     .rdata   	( lsu_rdata     ),
//     .rresp   	( lsu_rresp     )
// );

axi_bus_matrix u_axi_bus_matrix(
    .clk          	( sys_clk           ),
    .rst_n        	( rst_n         ),
    .ifu_arvalid  	( ifu_arvalid   ),
    .ifu_arready  	( ifu_arready   ),
    .ifu_raddr    	( PC_to_sram     ),
    .ifu_rvalid   	( ifu_rvalid    ),
    .ifu_rready   	( ifu_rready    ),
    .ifu_rresp    	( ifu_rresp     ),
    .ifu_rdata    	( inst_in     ),
    .lsu_arvalid  	( lsu_arvalid   ),
    .lsu_arready  	( lsu_arready   ),
    .lsu_raddr    	( lsu_raddr     ),
    .lsu_rvalid   	( lsu_rvalid    ),
    .lsu_rready   	( lsu_rready    ),
    .lsu_rresp    	( lsu_rresp     ),
    .lsu_rdata    	( lsu_rdata     ),
    .lsu_awvalid  	( lsu_awvalid   ),
    .lsu_awready  	( lsu_awready   ),
    .lsu_waddr    	( lsu_waddr     ),
    .lsu_wvalid   	( lsu_wvalid    ),
    .lsu_wready   	( lsu_wready    ),
    .lsu_strob    	( lsu_wstrob     ),
    .lsu_wdata    	( lsu_wdata     ),
    .lsu_bvalid   	( lsu_bvalid    ),
    .lsu_bready   	( lsu_bready    ),
    .lsu_bresp    	( lsu_bresp     ),
    .sram_arvalid 	( sram_arvalid  ),
    .sram_arready 	( sram_arready  ),
    .sram_raddr   	( sram_raddr    ),
    .sram_rvalid  	( sram_rvalid   ),
    .sram_rready  	( sram_rready   ),
    .sram_rresp   	( sram_rresp    ),
    .sram_rdata   	( sram_rdata    ),
    .sram_awvalid 	( sram_awvalid  ),
    .sram_awready 	( sram_awready  ),
    .sram_waddr   	( sram_waddr    ),
    .sram_wvalid  	( sram_wvalid   ),
    .sram_wready  	( sram_wready   ),
    .sram_strob   	( sram_wstrob    ),
    .sram_wdata   	( sram_wdata    ),
    .sram_bvalid  	( sram_bvalid   ),
    .sram_bready  	( sram_bready   ),
    .sram_bresp   	( sram_bresp    )
);
sram #(DATA_LEN,DATA_STROB_LEN,DATA_LEN)u_sram(
    .clk     	( sys_clk       ),
    .rst_n   	( rst_n         ),
    .awvalid 	( sram_awvalid   ),
    .awready 	( sram_awready   ),
    .waddr   	( sram_waddr     ),
    .wvalid  	( sram_wvalid    ),
    .wready  	( sram_wready    ),
    .wdata   	( sram_wdata     ),
    .wstrob  	( sram_wstrob    ),
    .bvalid  	( sram_bvalid    ),
    .bready  	( sram_bready    ),
    .bresp   	( sram_bresp     ),
    .arvalid 	( sram_arvalid   ),
    .arready 	( sram_arready   ),
    .raddr   	( sram_raddr     ),
    .rvalid  	( sram_rvalid    ),
    .rready  	( sram_rready    ),
    .rdata   	( sram_rdata     ),
    .rresp   	( sram_rresp     )
);


endmodule //top
