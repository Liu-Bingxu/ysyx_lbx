`include "define.v"
module top#(parameter DATA_LEN=32) (
`ifndef HAS_AXI_BUS_ARBITER
    output                  unuse,
`endif
`ifdef HAS_ICACHE
    output [1:0]            ifu_addr_offset,
`endif
`ifdef SIM
    output [DATA_LEN-1:0]     LS_WB_reg_PC,
    output [31:0]             LS_WB_reg_inst,
`endif
    input                   sys_clk,
    input                   sys_rst_n
);

localparam DATA_STROB_LEN = DATA_LEN/8;

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

// ifu outports wire
wire                	ifu_arvalid;
wire [DATA_LEN-1:0] 	ifu_raddr;
wire                	ifu_rready;
wire                	IF_ID_reg_inst_valid;
wire [DATA_LEN-1:0] 	IF_ID_reg_inst;
wire [DATA_LEN-1:0] 	IF_ID_reg_PC;

// idu outports wire
wire                	ID_EX_reg_decode_valid;
wire [4:0]          	rs1;
wire [4:0]          	rs2;
wire                    rs1_valid;
wire                    rs2_valid;
wire [4:0]          	ID_EX_reg_rd;
wire [1:0]          	ID_EX_reg_csr_wfunc;
wire [11:0]             ID_EX_reg_CSR_addr;
wire [DATA_LEN-1:0] 	ID_EX_reg_operand1;
wire [DATA_LEN-1:0] 	ID_EX_reg_operand2;
wire [DATA_LEN-1:0] 	ID_EX_reg_operand3;
wire [DATA_LEN-1:0] 	ID_EX_reg_operand4;
wire [DATA_LEN-1:0] 	ID_EX_reg_store_data;
wire [13:0]         	ID_EX_reg_control_sign;
wire [1:0]          	ID_EX_reg_csr_sign;
wire                	ID_EX_reg_inst_jump_flag;
wire                	ID_EX_reg_jump_without;
wire [4:0]          	ID_EX_reg_load_sign;
wire [3:0]          	ID_EX_reg_store_sign;
wire                	ID_EX_reg_ebreak;
wire                	ID_EX_reg_op;
wire                	ID_EX_reg_CSR_ren;
wire                	ID_EX_reg_CSR_wen;
wire                	ID_EX_reg_dest_wen;
`ifdef SIM
wire [DATA_LEN-1:0]     ID_EX_reg_PC  ;
wire [31:0]             ID_EX_reg_inst;
`endif

// exu outports wire
wire                	EX_MON_reg_Jump_flag;
wire [DATA_LEN-1:0] 	EX_IF_reg_Jump_PC;
wire                	EX_LS_reg_execute_valid;
wire [DATA_LEN-1:0] 	EX_LS_reg_dest_data;
wire [DATA_LEN-1:0] 	EX_LS_reg_csr_wdata;
wire [DATA_LEN-1:0] 	EX_LS_reg_addr_load;
wire [DATA_LEN-1:0] 	EX_LS_reg_store_data;
wire [DATA_LEN-1:0]     EX_LS_reg_cause;
wire                    EX_LS_reg_unusual_flag;
wire [4:0]          	EX_LS_reg_rd;
wire [1:0]          	EX_LS_reg_csr_wfunc;
wire [11:0]             EX_LS_reg_CSR_addr;
wire [4:0]          	EX_LS_reg_load_sign;
wire [3:0]          	EX_LS_reg_store_sign;
wire                	EX_LS_reg_ebreak;
wire                	EX_LS_reg_CSR_wen;
wire                	EX_LS_reg_CSR_ren;
wire                	EX_LS_reg_dest_wen;
`ifdef SIM
wire [DATA_LEN-1:0]     EX_LS_reg_PC  ;
wire [31:0]             EX_LS_reg_inst;
`endif

// lsu outports wire
wire                    	lsu_awvalid;
wire [DATA_LEN-1:0]     	lsu_waddr;
wire                    	lsu_wvalid;
wire [DATA_BIT_NUM-1:0] 	lsu_wstrob;
wire [DATA_LEN-1:0]     	lsu_wdata;
wire                    	lsu_bready;
wire                    	lsu_arvalid;
wire [DATA_LEN-1:0]     	lsu_raddr;
wire                    	lsu_rready;
wire                    	LS_WB_reg_ls_valid;
wire                        LS_MON_ls_valid;
wire [DATA_LEN-1:0]     	LS_WB_reg_dest_data;
wire [DATA_LEN-1:0]     	LS_WB_reg_csr_wdata;
wire [4:0]              	LS_WB_reg_rd;
wire [1:0]              	LS_WB_reg_csr_wfunc;
wire [11:0]                 LS_WB_reg_CSR_addr;
wire                    	LS_WB_reg_ebreak;
wire [DATA_LEN-1:0]         LS_WB_reg_cause;
wire                        LS_WB_reg_unusual_flag;
wire                    	LS_WB_reg_CSR_wen;
wire                	    LS_WB_reg_CSR_ren;
wire                    	LS_WB_reg_dest_wen;

// gpr outports wire
wire [DATA_LEN-1:0] 	src1;
wire [DATA_LEN-1:0] 	src2;

// csr outports wire
wire [DATA_LEN-1:0] 	csr_rdata;
wire [DATA_LEN-1:0] 	mepc;
wire [DATA_LEN-1:0] 	mtvec;

//block monitor output wire
wire IF_reg_inst_enable;
wire ID_reg_decode_enable;
wire EX_reg_execute_enable;
wire LS_reg_load_store_enable;
wire IF_reg_inst_flush;
wire ID_reg_decode_flush;
wire src1_bypass_flag;
wire src2_bypass_flag;
wire MON_ID_src_block_flag;

//ifu sram output wire
wire [2:0]              ifu_rresp;
wire                    ifu_arready;
wire                    ifu_rvalid;
wire [DATA_LEN-1:0]     ifu_rdata;

//lsu sram output wire
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

ifu u_ifu(
    .clk                  	( sys_clk               ),
    .rst_n                	( rst_n                 ),
    .IF_reg_inst_flush      ( IF_reg_inst_flush     ),
    .EX_IF_reg_Jump_PC    	( EX_IF_reg_Jump_PC     ),
    .ifu_arvalid          	( ifu_arvalid           ),
    .ifu_arready          	( ifu_arready           ),
    .ifu_raddr            	( ifu_raddr             ),
    .ifu_rdata            	( ifu_rdata             ),
    .ifu_rvalid           	( ifu_rvalid            ),
    .ifu_rresp            	( ifu_rresp             ),
    .ifu_rready           	( ifu_rready            ),
    .IF_ID_reg_inst_valid 	( IF_ID_reg_inst_valid  ),
    .IF_reg_inst_enable   	( IF_reg_inst_enable    ),
    .IF_ID_reg_inst 	    ( IF_ID_reg_inst        ),
    .IF_ID_reg_PC     	    ( IF_ID_reg_PC          )
);

wire [DATA_LEN-1:0] dest_data_bypass;
wire [DATA_LEN-1:0] decode_get_src1;
wire [DATA_LEN-1:0] decode_get_src2;
assign decode_get_src1 = (src1_bypass_flag)?dest_data_bypass:src1;
assign decode_get_src2 = (src2_bypass_flag)?dest_data_bypass:src2;

idu #(DATA_LEN)u_idu(
    .clk                      	( sys_clk                   ),
    .rst_n                    	( rst_n                     ),
    .IF_ID_reg_inst           	( IF_ID_reg_inst            ),
    .IF_ID_reg_PC             	( IF_ID_reg_PC              ),
`ifdef SIM
    .ID_EX_reg_PC               ( ID_EX_reg_PC              ),
    .ID_EX_reg_inst             ( ID_EX_reg_inst            ),
`endif
    .IF_ID_reg_inst_valid     	( IF_ID_reg_inst_valid      ),
    .MON_ID_src_block_flag      ( MON_ID_src_block_flag     ),
    .ID_EX_reg_decode_valid   	( ID_EX_reg_decode_valid    ),
    .ID_reg_decode_enable     	( ID_reg_decode_enable      ),
    .EX_reg_execute_enable      ( EX_reg_execute_enable     ),
    .ID_reg_decode_flush        ( ID_reg_decode_flush       ),
    .src1                     	( decode_get_src1           ),
    .src2                     	( decode_get_src2           ),
    .rs1                      	( rs1                       ),
    .rs2                      	( rs2                       ),
    .rs1_valid                  ( rs1_valid                 ),
    .rs2_valid                  ( rs2_valid                 ),
    .ID_EX_reg_rd             	( ID_EX_reg_rd              ),
    .ID_EX_reg_csr_wfunc      	( ID_EX_reg_csr_wfunc       ),
    .ID_EX_reg_CSR_addr         ( ID_EX_reg_CSR_addr        ),
    .ID_EX_reg_operand1       	( ID_EX_reg_operand1        ),
    .ID_EX_reg_operand2       	( ID_EX_reg_operand2        ),
    .ID_EX_reg_operand3       	( ID_EX_reg_operand3        ),
    .ID_EX_reg_operand4       	( ID_EX_reg_operand4        ),
    .ID_EX_reg_store_data     	( ID_EX_reg_store_data      ),
    .ID_EX_reg_control_sign   	( ID_EX_reg_control_sign    ),
    .ID_EX_reg_csr_sign       	( ID_EX_reg_csr_sign        ),
    .ID_EX_reg_inst_jump_flag 	( ID_EX_reg_inst_jump_flag  ),
    .ID_EX_reg_jump_without   	( ID_EX_reg_jump_without    ),
    .ID_EX_reg_load_sign      	( ID_EX_reg_load_sign       ),
    .ID_EX_reg_store_sign     	( ID_EX_reg_store_sign      ),
    .ID_EX_reg_ebreak         	( ID_EX_reg_ebreak          ),
    .ID_EX_reg_op             	( ID_EX_reg_op              ),
    .ID_EX_reg_CSR_ren       	( ID_EX_reg_CSR_ren         ),
    .ID_EX_reg_CSR_wen        	( ID_EX_reg_CSR_wen         ),
    .ID_EX_reg_dest_wen       	( ID_EX_reg_dest_wen        )
);

exu #(DATA_LEN)u_exu(
    .clk                      	( sys_clk                   ),
    .rst_n                    	( rst_n                     ),
    .mepc                     	( mepc                      ),
    .mtvec                    	( mtvec                     ),
    // .unusual_flag             	( unusual_flag              ),
    // .cause                    	( cause                     ),
    .ID_EX_reg_decode_valid   	( ID_EX_reg_decode_valid    ),
    .ID_EX_reg_rd             	( ID_EX_reg_rd              ),
    .ID_EX_reg_csr_wfunc      	( ID_EX_reg_csr_wfunc       ),
    .ID_EX_reg_CSR_addr         ( ID_EX_reg_CSR_addr        ),
`ifdef SIM
    .ID_EX_reg_PC               ( ID_EX_reg_PC              ),
    .EX_LS_reg_PC               ( EX_LS_reg_PC              ),
    .ID_EX_reg_inst             ( ID_EX_reg_inst            ),
    .EX_LS_reg_inst             ( EX_LS_reg_inst            ),
`endif
    .ID_EX_reg_operand1       	( ID_EX_reg_operand1        ),
    .ID_EX_reg_operand2       	( ID_EX_reg_operand2        ),
    .ID_EX_reg_operand3       	( ID_EX_reg_operand3        ),
    .ID_EX_reg_operand4       	( ID_EX_reg_operand4        ),
    .ID_EX_reg_store_data     	( ID_EX_reg_store_data      ),
    .ID_EX_reg_control_sign   	( ID_EX_reg_control_sign    ),
    .ID_EX_reg_csr_sign       	( ID_EX_reg_csr_sign        ),
    .ID_EX_reg_inst_jump_flag 	( ID_EX_reg_inst_jump_flag  ),
    .ID_EX_reg_jump_without   	( ID_EX_reg_jump_without    ),
    .ID_EX_reg_op             	( ID_EX_reg_op              ),
    .ID_EX_reg_load_sign      	( ID_EX_reg_load_sign       ),
    .ID_EX_reg_store_sign     	( ID_EX_reg_store_sign      ),
    .ID_EX_reg_ebreak         	( ID_EX_reg_ebreak          ),
    .ID_EX_reg_CSR_wen        	( ID_EX_reg_CSR_wen         ),
    .ID_EX_reg_CSR_ren       	( ID_EX_reg_CSR_ren         ),
    .ID_EX_reg_dest_wen       	( ID_EX_reg_dest_wen        ),
    .EX_MON_reg_Jump_flag      	( EX_MON_reg_Jump_flag      ),
    .EX_IF_reg_Jump_PC        	( EX_IF_reg_Jump_PC         ),
    .EX_LS_reg_execute_valid  	( EX_LS_reg_execute_valid   ),
    .EX_reg_execute_enable    	( EX_reg_execute_enable     ),
    .EX_LS_reg_dest_data      	( EX_LS_reg_dest_data       ),
    .EX_LS_reg_csr_wdata      	( EX_LS_reg_csr_wdata       ),
    .EX_LS_reg_addr_load      	( EX_LS_reg_addr_load       ),
    .EX_LS_reg_store_data     	( EX_LS_reg_store_data      ),
    .EX_LS_reg_rd             	( EX_LS_reg_rd              ),
    .EX_LS_reg_csr_wfunc      	( EX_LS_reg_csr_wfunc       ),
    .EX_LS_reg_CSR_addr         ( EX_LS_reg_CSR_addr        ),
    .EX_LS_reg_load_sign      	( EX_LS_reg_load_sign       ),
    .EX_LS_reg_store_sign     	( EX_LS_reg_store_sign      ),
    .EX_LS_reg_ebreak         	( EX_LS_reg_ebreak          ),
    .EX_LS_reg_unusual_flag     ( EX_LS_reg_unusual_flag    ),
    .EX_LS_reg_cause            ( EX_LS_reg_cause           ),
    .EX_LS_reg_CSR_wen        	( EX_LS_reg_CSR_wen         ),
    .EX_LS_reg_CSR_ren       	( EX_LS_reg_CSR_ren         ),
    .EX_LS_reg_dest_wen       	( EX_LS_reg_dest_wen        )
);

localparam  DATA_BIT_NUM = DATA_LEN/8;

lsu_rv32 #(DATA_LEN,DATA_BIT_NUM)u_lsu_rv32(
    .clk                     	( sys_clk                  ),
    .rst_n                   	( rst_n                    ),
    .lsu_awvalid             	( lsu_awvalid              ),
    .lsu_awready             	( lsu_awready              ),
    .lsu_waddr               	( lsu_waddr                ),
    .lsu_wvalid              	( lsu_wvalid               ),
    .lsu_wready              	( lsu_wready               ),
    .lsu_wstrob              	( lsu_wstrob               ),
    .lsu_wdata               	( lsu_wdata                ),
    .lsu_bvalid              	( lsu_bvalid               ),
    .lsu_bready              	( lsu_bready               ),
    .lsu_bresp               	( lsu_bresp                ),
    .lsu_arvalid             	( lsu_arvalid              ),
    .lsu_arready             	( lsu_arready              ),
    .lsu_raddr               	( lsu_raddr                ),
    .lsu_rvalid              	( lsu_rvalid               ),
    .lsu_rresp               	( lsu_rresp                ),
    .lsu_rready              	( lsu_rready               ),
    .lsu_rdata               	( lsu_rdata                ),
    .EX_LS_reg_execute_valid 	( EX_LS_reg_execute_valid  ),
    .EX_LS_reg_dest_data     	( EX_LS_reg_dest_data      ),
    .EX_LS_reg_csr_wdata     	( EX_LS_reg_csr_wdata      ),
    .EX_LS_reg_addr_load     	( EX_LS_reg_addr_load      ),
    .EX_LS_reg_store_data    	( EX_LS_reg_store_data     ),
    .EX_LS_reg_rd            	( EX_LS_reg_rd             ),
    .EX_LS_reg_csr_wfunc     	( EX_LS_reg_csr_wfunc      ),
    .EX_LS_reg_CSR_addr         ( EX_LS_reg_CSR_addr       ),
    .EX_LS_reg_load_sign     	( EX_LS_reg_load_sign      ),
    .EX_LS_reg_store_sign    	( EX_LS_reg_store_sign     ),
    .EX_LS_reg_ebreak        	( EX_LS_reg_ebreak         ),
    .EX_LS_reg_unusual_flag     ( EX_LS_reg_unusual_flag   ),
    .EX_LS_reg_cause            ( EX_LS_reg_cause          ),
    .EX_LS_reg_CSR_wen       	( EX_LS_reg_CSR_wen        ),
    .EX_LS_reg_CSR_ren       	( EX_LS_reg_CSR_ren        ),
    .EX_LS_reg_dest_wen      	( EX_LS_reg_dest_wen       ),
    .LS_WB_reg_ls_valid      	( LS_WB_reg_ls_valid       ),
    .LS_MON_ls_valid            ( LS_MON_ls_valid          ),
    .LS_reg_load_store_enable   ( LS_reg_load_store_enable ),
`ifdef SIM
    .EX_LS_reg_PC               ( EX_LS_reg_PC              ),
    .LS_WB_reg_PC               ( LS_WB_reg_PC              ),
    .EX_LS_reg_inst             ( EX_LS_reg_inst            ),
    .LS_WB_reg_inst             ( LS_WB_reg_inst            ),
`endif
    .LS_WB_reg_dest_data     	( LS_WB_reg_dest_data      ),
    .LS_WB_reg_csr_wdata     	( LS_WB_reg_csr_wdata      ),
    .LS_WB_reg_rd            	( LS_WB_reg_rd             ),
    .LS_WB_reg_csr_wfunc     	( LS_WB_reg_csr_wfunc      ),
    .LS_WB_reg_CSR_addr         ( LS_WB_reg_CSR_addr       ),
    .LS_WB_reg_ebreak        	( LS_WB_reg_ebreak         ),
    .LS_WB_reg_unusual_flag     ( LS_WB_reg_unusual_flag   ),
    .LS_WB_reg_cause            ( LS_WB_reg_cause          ),
    .LS_WB_reg_CSR_wen       	( LS_WB_reg_CSR_wen        ),
    .LS_WB_reg_CSR_ren       	( LS_WB_reg_CSR_ren        ),
    .LS_WB_reg_dest_wen      	( LS_WB_reg_dest_wen       )
);


wire [DATA_LEN-1:0] dest_data;
assign dest_data = (LS_WB_reg_CSR_ren)?csr_rdata:LS_WB_reg_dest_data;
assign dest_data_bypass = dest_data;
gpr #(DATA_LEN)u_gpr(
    .clk                 	( sys_clk              ),
    .rst_n               	( rst_n                ),
    .rs1                 	( rs1                  ),
    .rs2                 	( rs2                  ),
    .src1                	( src1                 ),
    .src2                	( src2                 ),
    .LS_WB_reg_ls_valid  	( LS_WB_reg_ls_valid   ),
    .LS_WB_reg_dest_data 	( dest_data            ),
    .LS_WB_reg_rd        	( LS_WB_reg_rd         ),
    .LS_WB_reg_dest_wen  	( LS_WB_reg_dest_wen   )
);

csr #(DATA_LEN)u_csr(
    .clk                 	( sys_clk               ),
    .rst_n               	( rst_n                 ),
    // .unusual_flag        	( unusual_flag         ),
    // .PC                  	( {DATA_LEN{1'b0}}     ),
    // .cause               	( cause                ),
    .LS_WB_reg_unusual_flag ( LS_WB_reg_unusual_flag),
    .LS_WB_reg_PC           ( LS_WB_reg_PC          ),
    .LS_WB_reg_cause        ( LS_WB_reg_cause       ),
    .csr_rdata            	( csr_rdata             ),
    .mepc                	( mepc                  ),
    .mtvec               	( mtvec                 ),
    .LS_WB_reg_CSR_addr     ( LS_WB_reg_CSR_addr    ),
    .LS_WB_reg_CSR_ren  	( LS_WB_reg_CSR_ren     ),
    .LS_WB_reg_ls_valid  	( LS_WB_reg_ls_valid    ),
    .LS_WB_reg_csr_wdata 	( LS_WB_reg_csr_wdata   ),
    .LS_WB_reg_csr_wfunc 	( LS_WB_reg_csr_wfunc   ),
    .LS_WB_reg_CSR_wen   	( LS_WB_reg_CSR_wen     )
);

monitor u_monitor(
    .clk                	( sys_clk             ),
    .LS_WB_reg_ls_valid 	( LS_WB_reg_ls_valid  ),
    .LS_WB_reg_ebreak   	( LS_WB_reg_ebreak    )
);

block_monitor u_block_monitor(
    .rs1                        ( rs1                        ),
    .rs2                        ( rs2                        ),
    .rs1_valid                  ( rs1_valid                  ),
    .rs2_valid                  ( rs2_valid                  ),
    .ID_EX_reg_rd               ( ID_EX_reg_rd               ),
    .ID_EX_reg_dest_wen         ( ID_EX_reg_dest_wen         ),
    .EX_LS_reg_rd               ( EX_LS_reg_rd               ),
    .EX_LS_reg_dest_wen         ( EX_LS_reg_dest_wen         ),
    .LS_WB_reg_rd               ( LS_WB_reg_rd               ),
    .LS_WB_reg_dest_wen         ( LS_WB_reg_dest_wen         ),
    .EX_MON_reg_Jump_flag       ( EX_MON_reg_Jump_flag       ),
    .IF_ID_reg_inst_valid 	    ( IF_ID_reg_inst_valid       ),
    .ID_EX_reg_decode_valid   	( ID_EX_reg_decode_valid     ),
    .EX_LS_reg_execute_valid   	( EX_LS_reg_execute_valid    ),
    .LS_WB_reg_ls_valid         ( LS_WB_reg_ls_valid         ),
    .EX_LS_reg_load_sign_flag  	( EX_LS_reg_load_sign[0]     ),
    .EX_LS_reg_store_sign_flag 	( EX_LS_reg_store_sign[0]    ),
    .LS_MON_ls_valid           	( LS_MON_ls_valid            ),
    .IF_reg_inst_enable        	( IF_reg_inst_enable         ),
    .ID_reg_decode_enable      	( ID_reg_decode_enable       ),
    .EX_reg_execute_enable     	( EX_reg_execute_enable      ),
    .LS_reg_load_store_enable  	( LS_reg_load_store_enable   ),
    .IF_reg_inst_flush          ( IF_reg_inst_flush          ),
    .ID_reg_decode_flush        ( ID_reg_decode_flush        ),
    .src1_bypass_flag           ( src1_bypass_flag           ),
    .src2_bypass_flag           ( src2_bypass_flag           ),
    .MON_ID_src_block_flag      ( MON_ID_src_block_flag      )
);


`ifndef HAS_AXI_BUS_ARBITER
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
    .ifu_raddr      	( ifu_raddr[DATA_LEN-1:2]   ),
    .ifu_rvalid     	( ifu_rvalid                ),
    .ifu_rready     	( ifu_rready                ),
    .ifu_rdata      	( ifu_rdata                 ),
    .ifu_rresp      	( ifu_rresp                 ),
    .icache_arvalid 	( icache_arvalid            ),
    .icache_arready 	( icache_arready            ),
    .icache_raddr   	( icache_raddr              ),
    .icache_rvalid  	( icache_rvalid             ),
    .icache_rready  	( icache_rready             ),
    .icache_rresp   	( icache_rresp              ),
    .icache_rdata   	( icache_rdata              )
);

assign ifu_addr_offset = ifu_raddr[1:0];

`else

assign icache_arvalid   = ifu_arvalid;
assign ifu_arready      = icache_arready;
assign icache_raddr     = ifu_raddr;
assign ifu_rvalid       = icache_rvalid;
assign icache_rready    = ifu_rready;
assign ifu_rdata        = icache_rdata;
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
