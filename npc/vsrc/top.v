`include "define.v"
module top#(parameter DATA_LEN=32) (
`ifndef HAS_AXI_BUS_ARBITER
    output                  unuse,
`endif
`ifdef HAS_ICACHE
    output [1:0]            ifu_addr_offset,
`endif
// `ifdef HAS_DCACHE
//     output [1:0]            lsu_read_addr_offset,
//     output [1:0]            lsu_write_addr_offset,
// `endif
    input                   sys_clk,
    input                   sys_rst_n
    // input  [DATA_LEN-1:0]   inst_in,
    // output [DATA_LEN-1:0]   PC_out
);

localparam DATA_STROB_LEN = DATA_LEN/8;

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

wire                        icache_arvalid;
wire                        icache_arready;
wire [DATA_LEN-1:0]         icache_raddr;
wire                        icache_rvalid;
wire                        icache_rready;
wire [DATA_LEN-1:0]         icache_rdata;
wire [2:0]                  icache_rresp;

wire                 	    dcache_arvalid;
wire                        dcache_arready;
wire [DATA_LEN-1:0]  	    dcache_raddr;
wire                        dcache_rvalid;
wire                 	    dcache_rready;
wire [2:0]                  dcache_rresp;
wire [DATA_LEN-1:0]         dcache_rdata;
wire                 	    dcache_awvalid;
wire                        dcache_awready;
wire [DATA_LEN-1:0]  	    dcache_waddr;
wire                 	    dcache_wvalid;
wire                        dcache_wready;
wire [DATA_STROB_LEN-1:0] 	dcache_wstrob;
wire [DATA_LEN-1:0]  	    dcache_wdata;
wire                        dcache_bvalid;
wire                 	    dcache_bready;
wire [2:0]                  dcache_bresp;

`ifdef HAS_AXI_BUS_ARBITER

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

`endif

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

lsu_rv32 #(DATA_LEN,DATA_BIT_NUM)u_lsu(
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
`ifndef HAS_AXI_BUS_ARBITER
// localparam DATA_STROB_LEN = DATA_LEN/8;
wire ifu_awready,ifu_wready,ifu_bvalid;
wire [2:0]  ifu_bresp;
assign unuse = ifu_awready&ifu_wready&ifu_bvalid&(ifu_bresp==3'b000);
sram #(DATA_LEN,DATA_STROB_LEN,DATA_LEN)ifu_sram(
    .clk     	( sys_clk                   ),
    .rst_n   	( rst_n                     ),
    .awvalid 	( 1'b0                      ),
    .awready 	( ifu_awready               ),
    .waddr   	( {DATA_LEN{1'b0}}          ),
    .wvalid  	( 1'b0                      ),
    .wready  	( ifu_wready                ),
    .wdata   	( {DATA_LEN{1'b0}}          ),
    .wstrob  	( {DATA_STROB_LEN{1'b0}}    ),
    .bvalid  	( ifu_bvalid                ),
    .bready  	( 1'b0                      ),
    .bresp   	( ifu_bresp                 ),

    .arvalid 	( icache_arvalid            ),
    .arready 	( icache_arready            ),
    .raddr   	( icache_raddr              ),
    .rvalid  	( icache_rvalid             ),
    .rready  	( icache_rready             ),
    .rdata   	( icache_rdata              ),
    .rresp   	( icache_rresp              )
);

sram #(DATA_LEN,DATA_STROB_LEN,DATA_LEN)lsu_sram(
    .clk     	( sys_clk           ),
    .rst_n   	( rst_n             ),
    .awvalid 	( dcache_awvalid    ),
    .awready 	( dcache_awready    ),
    .waddr   	( dcache_waddr      ),
    .wvalid  	( dcache_wvalid     ),
    .wready  	( dcache_wready     ),
    .wdata   	( dcache_wdata      ),
    .wstrob  	( dcache_wstrob     ),
    .bvalid  	( dcache_bvalid     ),
    .bready  	( dcache_bready     ),
    .bresp   	( dcache_bresp      ),
    .arvalid 	( dcache_arvalid    ),
    .arready 	( dcache_arready    ),
    .raddr   	( dcache_raddr      ),
    .rvalid  	( dcache_rvalid     ),
    .rready  	( dcache_rready     ),
    .rdata   	( dcache_rdata      ),
    .rresp   	( dcache_rresp      )
);

`else

axi_bus_matrix u_axi_bus_matrix(
    .clk          	( sys_clk           ),
    .rst_n        	( rst_n             ),
    .ifu_arvalid  	( icache_arvalid    ),
    .ifu_arready  	( icache_arready    ),
    .ifu_raddr    	( icache_raddr      ),
    .ifu_rvalid   	( icache_rvalid     ),
    .ifu_rready   	( icache_rready     ),
    .ifu_rresp    	( icache_rresp      ),
    .ifu_rdata    	( icache_rdata      ),
    .lsu_arvalid  	( dcache_arvalid    ),
    .lsu_arready  	( dcache_arready    ),
    .lsu_raddr    	( dcache_raddr      ),
    .lsu_rvalid   	( dcache_rvalid     ),
    .lsu_rready   	( dcache_rready     ),
    .lsu_rresp    	( dcache_rresp      ),
    .lsu_rdata    	( dcache_rdata      ),
    .lsu_awvalid  	( dcache_awvalid    ),
    .lsu_awready  	( dcache_awready    ),
    .lsu_waddr    	( dcache_waddr      ),
    .lsu_wvalid   	( dcache_wvalid     ),
    .lsu_wready   	( dcache_wready     ),
    .lsu_strob    	( dcache_wstrob     ),
    .lsu_wdata    	( dcache_wdata      ),
    .lsu_bvalid   	( dcache_bvalid     ),
    .lsu_bready   	( dcache_bready     ),
    .lsu_bresp    	( dcache_bresp      ),
    .sram_arvalid 	( sram_arvalid      ),
    .sram_arready 	( sram_arready      ),
    .sram_raddr   	( sram_raddr        ),
    .sram_rvalid  	( sram_rvalid       ),
    .sram_rready  	( sram_rready       ),
    .sram_rresp   	( sram_rresp        ),
    .sram_rdata   	( sram_rdata        ),
    .sram_awvalid 	( sram_awvalid      ),
    .sram_awready 	( sram_awready      ),
    .sram_waddr   	( sram_waddr        ),
    .sram_wvalid  	( sram_wvalid       ),
    .sram_wready  	( sram_wready       ),
    .sram_strob   	( sram_wstrob       ),
    .sram_wdata   	( sram_wdata        ),
    .sram_bvalid  	( sram_bvalid       ),
    .sram_bready  	( sram_bready       ),
    .sram_bresp   	( sram_bresp        )
);
sram #(DATA_LEN,DATA_STROB_LEN,DATA_LEN)u_sram(
    .clk     	( sys_clk        ),
    .rst_n   	( rst_n          ),
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

`endif

`ifdef HAS_ICACHE

icache #(4,1,DATA_LEN)u_icache(
    .clk            	( sys_clk                   ),
    .rst_n          	( rst_n                     ),
    .ifu_arvalid    	( ifu_arvalid               ),
    .ifu_arready    	( ifu_arready               ),
    .ifu_raddr      	( PC_to_sram[DATA_LEN-1:2]  ),
    .ifu_rvalid     	( ifu_rvalid                ),
    .ifu_rready     	( ifu_rready                ),
    .ifu_rdata      	( inst_in                   ),
    .ifu_rresp      	( ifu_rresp                 ),
    .icache_arvalid 	( icache_arvalid            ),
    .icache_arready 	( icache_arready            ),
    .icache_raddr   	( icache_raddr              ),
    .icache_rvalid  	( icache_rvalid             ),
    .icache_rready  	( icache_rready             ),
    .icache_rresp   	( icache_rresp              ),
    .icache_rdata   	( icache_rdata              )
);

assign ifu_addr_offset = PC_to_sram[1:0];

`else

assign icache_arvalid   = ifu_arvalid;
assign ifu_arready      = icache_arready;
assign icache_raddr     = PC_to_sram;
assign ifu_rvalid       = icache_rvalid;
assign icache_rready    = ifu_rready;
assign inst_in          = icache_rdata;
assign ifu_rresp        = icache_rresp;

`endif

`ifdef HAS_DCACHE

dcache #(4,1,DATA_LEN,DATA_BIT_NUM)u_dcache(
    .clk            	( sys_clk       ),
    .rst_n          	( rst_n         ),
    .lsu_arvalid    	( lsu_arvalid   ),
    .lsu_arready    	( lsu_arready   ),
    .lsu_raddr      	( lsu_raddr     ),
    .lsu_rvalid     	( lsu_rvalid    ),
    .lsu_rready     	( lsu_rready    ),
    .lsu_rdata      	( lsu_rdata     ),
    .lsu_rresp      	( lsu_rresp     ),
    .lsu_awvalid    	( lsu_awvalid   ),
    .lsu_awready    	( lsu_awready   ),
    .lsu_waddr      	( lsu_waddr     ),
    .lsu_wvalid     	( lsu_wvalid    ),
    .lsu_wready     	( lsu_wready    ),
    .lsu_wstrob     	( lsu_wstrob    ),
    .lsu_wdata      	( lsu_wdata     ),
    .lsu_bvalid     	( lsu_bvalid    ),
    .lsu_bready     	( lsu_bready    ),
    .lsu_bresp      	( lsu_bresp     ),
    .dcache_arvalid 	( dcache_arvalid),
    .dcache_arready 	( dcache_arready),
    .dcache_raddr   	( dcache_raddr  ),
    .dcache_rvalid  	( dcache_rvalid ),
    .dcache_rready  	( dcache_rready ),
    .dcache_rresp   	( dcache_rresp  ),
    .dcache_rdata   	( dcache_rdata  ),
    .dcache_awvalid 	( dcache_awvalid),
    .dcache_awready 	( dcache_awready),
    .dcache_waddr   	( dcache_waddr  ),
    .dcache_wvalid  	( dcache_wvalid ),
    .dcache_wready  	( dcache_wready ),
    .dcache_wstrob  	( dcache_wstrob ),
    .dcache_wdata   	( dcache_wdata  ),
    .dcache_bvalid  	( dcache_bvalid ),
    .dcache_bready  	( dcache_bready ),
    .dcache_bresp   	( dcache_bresp  )
);
//debug temp
// assign dcache_awvalid   = lsu_awvalid;
// assign lsu_awready      = dcache_awready;
// assign dcache_waddr     = lsu_waddr;

// assign dcache_wvalid    = lsu_wvalid;
// assign lsu_wready       = dcache_wready;
// assign dcache_wdata     = lsu_wdata;
// assign dcache_wstrob    = lsu_wstrob;

// assign lsu_bvalid       = dcache_bvalid;
// assign dcache_bready    = lsu_bready;
// assign lsu_bresp        = dcache_bresp;
//debug temp
// assign lsu_read_addr_offset = lsu_raddr[1:0];
// assign lsu_write_addr_offset = lsu_waddr[1:0];

`else

assign dcache_arvalid   = lsu_arvalid;
assign lsu_arready      = dcache_arready;
assign dcache_raddr     = lsu_raddr;

assign lsu_rvalid       = dcache_rvalid;
assign dcache_rready    = lsu_rready;
assign lsu_rdata        = dcache_rdata;
assign lsu_rresp        = dcache_rresp;

assign dcache_awvalid   = lsu_awvalid;
assign lsu_awready      = dcache_awready;
assign dcache_waddr     = lsu_waddr;

assign dcache_wvalid    = lsu_wvalid;
assign lsu_wready       = dcache_wready;
assign dcache_wdata     = lsu_wdata;
assign dcache_wstrob    = lsu_wstrob;

assign lsu_bvalid       = dcache_bvalid;
assign dcache_bready    = lsu_bready;
assign lsu_bresp        = dcache_bresp;

`endif

endmodule //top
