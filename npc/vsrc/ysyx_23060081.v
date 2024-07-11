module ysyx_23060081 (
    input           clock,
    input           reset,
    input           io_interrupt,
    input           io_master_awready,
    output          io_master_awvalid,
    output [31:0]   io_master_awaddr,  
    output [3:0]    io_master_awid,    
    output [7:0]    io_master_awlen,   
    output [2:0]    io_master_awsize,  
    output [1:0]    io_master_awburst, 
    input           io_master_wready,  
    output          io_master_wvalid,  
    output [63:0]   io_master_wdata,   
    output [7:0]    io_master_wstrb,   
    output          io_master_wlast,   
    output          io_master_bready,
    input           io_master_bvalid,  
    input  [1:0]    io_master_bresp,   
    input  [3:0]    io_master_bid,     
    input           io_master_arready, 
    output          io_master_arvalid, 
    output [31:0]   io_master_araddr,  
    output [3:0]    io_master_arid,    
    output [7:0]    io_master_arlen,   
    output [2:0]    io_master_arsize,  
    output [1:0]    io_master_arburst, 
    output          io_master_rready,
    input           io_master_rvalid,  
    input  [1:0]    io_master_rresp,   
    input  [63:0]   io_master_rdata,   
    input           io_master_rlast,   
    input  [3:0]    io_master_rid,     
    output          io_slave_awready, 
    input           io_slave_awvalid, 
    input  [31:0]   io_slave_awaddr,  
    input  [3:0]    io_slave_awid,    
    input  [7:0]    io_slave_awlen,   
    input  [2:0]    io_slave_awsize,  
    input  [1:0]    io_slave_awburst, 
    output          io_slave_wready,  
    input           io_slave_wvalid,  
    input  [63:0]   io_slave_wdata,   
    input  [7:0]    io_slave_wstrb,   
    input           io_slave_wlast,   
    input           io_slave_bready,  
    output          io_slave_bvalid,  
    output [1:0]    io_slave_bresp,   
    output [3:0]    io_slave_bid,     
    output          io_slave_arready, 
    input           io_slave_arvalid, 
    input  [31:0]   io_slave_araddr,  
    input  [3:0]    io_slave_arid,    
    input  [7:0]    io_slave_arlen,   
    input  [2:0]    io_slave_arsize,  
    input  [1:0]    io_slave_arburst, 
    input           io_slave_rready,  
    output          io_slave_rvalid,  
    output [1:0]    io_slave_rresp,   
    output [63:0]   io_slave_rdata,   
    output          io_slave_rlast,   
    output [3:0]    io_slave_rid     
);

wire [63:0]           inst;
wire [63:0]           lsu_rdata_get;

wire                  clk;
wire                  rst_n;
wire        	      MXR;
wire        	      SUM;
wire        	      MPRV;
wire [1:0]  	      MPP;
wire [3:0]            satp_mode;
wire [15:0]           satp_asid;
wire [43:0]           satp_ppn;
wire                  ifu_arready;
wire                  ifu_arvalid;
wire [63:0]           ifu_araddr;
wire                  ifu_rvalid;
wire                  ifu_rready;
wire [1:0]            ifu_rresp;
wire [31:0]           ifu_rdata;
wire                  lsu_arvalid;
wire                  lsu_arready;
wire                  lsu_arlock;
wire  [2:0]           lsu_arsize;
wire  [63:0]          lsu_araddr;
wire                  lsu_rvalid;
wire                  lsu_rready;
wire [1:0]            lsu_rresp;
wire [63:0]           lsu_rdata;
wire                  lsu_awvalid;
wire                  lsu_awready;
wire                  lsu_awlock;
wire  [2:0]           lsu_awsize;
wire  [63:0]          lsu_awaddr;
wire                  lsu_wvalid;
wire                  lsu_wready;
wire [7:0]            lsu_wstrob;
wire [63:0]           lsu_wdata;
wire                  lsu_bvalid;
wire                  lsu_bready;
wire [1:0]            lsu_bresp;

wire                  sram_arvalid;
wire                  sram_arready;
wire [2:0]            sram_arsize;
wire [63:0]  	      sram_raddr;
wire                  sram_rvalid;
wire                  sram_rready;
wire [1:0]            sram_rresp;
wire [63:0]           sram_rdata;
wire                  sram_awvalid;
wire                  sram_awready;
wire [63:0]  	      sram_waddr;
wire                  sram_wvalid;
wire                  sram_wready;
wire [7:0] 	          sram_wstrob;
wire [63:0]  	      sram_wdata;
wire                  sram_bvalid;
wire                  sram_bready;
wire [1:0]            sram_bresp;

core_top #(
    .MHARTID 	( 0             ),
    .RST_PC     ( 64'h2000_0000 )
)u_core_top
(
    .clk         	( clk          ),
    .rst_n       	( rst_n        ),
    .stip_asyn   	( 1'b0         ),
    .seip_asyn   	( 1'b0         ),
    .ssip_asyn   	( 1'b0         ),
    .mtip_asyn   	( 1'b0         ),
    .meip_asyn   	( io_interrupt ),
    .msip_asyn   	( 1'b0         ),
    .MXR         	( MXR          ),
    .SUM         	( SUM          ),
    .MPRV        	( MPRV         ),
    .MPP         	( MPP          ),
    .satp_mode      ( satp_mode    ),
    .satp_asid      ( satp_asid    ),
    .satp_ppn       ( satp_ppn     ),
    .ifu_arready 	( ifu_arready  ),
    .ifu_arvalid 	( ifu_arvalid  ),
    .ifu_araddr  	( ifu_araddr   ),
    .ifu_rvalid  	( ifu_rvalid   ),
    .ifu_rready  	( ifu_rready   ),
    .ifu_rresp   	( ifu_rresp    ),
    .ifu_rdata   	( ifu_rdata    ),
    .lsu_arvalid 	( lsu_arvalid  ),
    .lsu_arready 	( lsu_arready  ),
    .lsu_arlock  	( lsu_arlock   ),
    .lsu_arsize  	( lsu_arsize   ),
    .lsu_araddr  	( lsu_araddr   ),
    .lsu_rvalid  	( lsu_rvalid   ),
    .lsu_rready  	( lsu_rready   ),
    .lsu_rresp   	( lsu_rresp    ),
    .lsu_rdata   	( lsu_rdata    ),
    .lsu_awvalid 	( lsu_awvalid  ),
    .lsu_awready 	( lsu_awready  ),
    .lsu_awlock  	( lsu_awlock   ),
    .lsu_awsize  	( lsu_awsize   ),
    .lsu_awaddr  	( lsu_awaddr   ),
    .lsu_wvalid  	( lsu_wvalid   ),
    .lsu_wready  	( lsu_wready   ),
    .lsu_wstrob  	( lsu_wstrob   ),
    .lsu_wdata   	( lsu_wdata    ),
    .lsu_bvalid  	( lsu_bvalid   ),
    .lsu_bready  	( lsu_bready   ),
    .lsu_bresp   	( lsu_bresp    )
);

axi_bus_matrix#(64,64,8) u_axi_bus_matrix(
    .clk          	( clk               ),
    .rst_n        	( rst_n             ),
    .ifu_arvalid  	( ifu_arvalid       ),
    .ifu_arready  	( ifu_arready       ),
    .ifu_arsize     ( 3'h2              ),
    .ifu_raddr    	( ifu_araddr        ),
    .ifu_rvalid   	( ifu_rvalid        ),
    .ifu_rready   	( ifu_rready        ),
    .ifu_rresp    	( ifu_rresp         ),
    .ifu_rdata    	( inst              ),
    .lsu_arvalid  	( lsu_arvalid       ),
    .lsu_arready  	( lsu_arready       ),
    .lsu_arsize     ( lsu_arsize        ),
    .lsu_raddr    	( lsu_araddr        ),
    .lsu_rvalid   	( lsu_rvalid        ),
    .lsu_rready   	( lsu_rready        ),
    .lsu_rresp    	( lsu_rresp         ),
    .lsu_rdata    	( lsu_rdata_get     ),
    .lsu_awvalid  	( lsu_awvalid       ),
    .lsu_awready  	( lsu_awready       ),
    .lsu_waddr    	( lsu_awaddr        ),
    .lsu_wvalid   	( lsu_wvalid        ),
    .lsu_wready   	( lsu_wready        ),
    .lsu_strob    	( lsu_wstrob        ),
    .lsu_wdata    	( lsu_wdata         ),
    .lsu_bvalid   	( lsu_bvalid        ),
    .lsu_bready   	( lsu_bready        ),
    .lsu_bresp    	( lsu_bresp         ),
    .sram_arvalid 	( sram_arvalid      ),
    .sram_arready 	( sram_arready      ),
    .sram_arsize    ( sram_arsize       ),
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

assign clk                      = clock;
assign rst_n                    = ~reset;

assign sram_awready             = io_master_awready;        
assign sram_wready              = io_master_wready;  
assign sram_bvalid              = io_master_bvalid;  
assign sram_bresp               = io_master_bresp;   
assign sram_arready             = io_master_arready; 
assign sram_rvalid              = io_master_rvalid;  
assign sram_rresp               = io_master_rresp;   
assign sram_rdata               = io_master_rdata;   

assign io_master_awvalid        = sram_awvalid;
assign io_master_awaddr         = sram_waddr[31:0];
assign io_master_awid           = 4'h0;
assign io_master_awlen          = 8'h0;
assign io_master_awsize         = lsu_awsize;
assign io_master_awburst        = 2'h0;
assign io_master_wvalid         = sram_wvalid;
assign io_master_wdata          = sram_wdata;
assign io_master_wstrb          = sram_wstrob;
assign io_master_wlast          = 1'b0;
assign io_master_bready         = sram_bready;
assign io_master_arvalid        = sram_arvalid;
assign io_master_araddr         = sram_raddr[31:0];
assign io_master_arid           = 4'h1;
assign io_master_arlen          = 8'h0;
assign io_master_arsize         = sram_arsize;
assign io_master_arburst        = 2'h0; 
assign io_master_rready         = sram_rready;

assign io_slave_awready     = 1'b0; 
assign io_slave_wready      = 1'b0;
assign io_slave_bvalid      = 1'b0;
assign io_slave_bresp       = 2'h0;
assign io_slave_bid         = 4'h0;
assign io_slave_arready     = 1'b0;
assign io_slave_rvalid      = 1'b0;
assign io_slave_rresp       = 2'h0;
assign io_slave_rdata       = 64'h0;   
assign io_slave_rlast       = 1'b0;   
assign io_slave_rid         = 4'h0;

assign ifu_rdata            = inst[31:0];
assign lsu_rdata            = (lsu_araddr[2]) ? {lsu_rdata_get[31:0], 32'h0} : lsu_rdata_get;

endmodule //ysyx_23060081

module axi_bus_matrix#(parameter DATA_LEN=32,ADDR_LEN=32,STROB_LEN=4)(
    input                   clk,
    input                   rst_n,
//ifu interface    
    //read addr channel
    input                   ifu_arvalid,
    output                  ifu_arready,
    input  [2:0]            ifu_arsize,
    input  [ADDR_LEN-1:0]   ifu_raddr,
    //read data channel
    output                  ifu_rvalid,
    input                   ifu_rready,
    output [1:0]            ifu_rresp,
    output [DATA_LEN-1:0]   ifu_rdata,
//lsu interface
    //read addr channel
    input                   lsu_arvalid,
    output                  lsu_arready,
    input  [2:0]            lsu_arsize,
    input  [ADDR_LEN-1:0]   lsu_raddr,
    //read data channel
    output                  lsu_rvalid,
    input                   lsu_rready,
    output [1:0]            lsu_rresp,
    output [DATA_LEN-1:0]   lsu_rdata,
    //write addr channel
    input                   lsu_awvalid,
    output                  lsu_awready,
    input  [ADDR_LEN-1:0]   lsu_waddr,
    //write data channel
    input                   lsu_wvalid,
    output                  lsu_wready,
    input [STROB_LEN-1:0]   lsu_strob,
    input [DATA_LEN-1:0]    lsu_wdata,
    //write resp channel
    output                  lsu_bvalid,
    input                   lsu_bready,
    output [1:0]            lsu_bresp,
//sram interface
    //read addr channel
    output                  sram_arvalid,
    input                   sram_arready,
    output [2:0]            sram_arsize,
    output [ADDR_LEN-1:0]   sram_raddr,
    //read data channel
    input                   sram_rvalid,
    output                  sram_rready,
    input [1:0]             sram_rresp,
    input [DATA_LEN-1:0]    sram_rdata,
    //write addr channel
    output                  sram_awvalid,
    input                   sram_awready,
    output  [ADDR_LEN-1:0]  sram_waddr,
    //write data channel
    output                  sram_wvalid,
    input                   sram_wready,
    output [STROB_LEN-1:0]  sram_strob,
    output [DATA_LEN-1:0]   sram_wdata,
    //write resp channel
    input                   sram_bvalid,
    output                  sram_bready,
    input [1:0]             sram_bresp
);

// read 
wire [1:0] 	read_sel;

axi_arbiter #(2)u_axi_read_arbiter(
    .clk    	( clk                       ),
    .rst_n  	( rst_n                     ),
    .avalid 	( {lsu_arvalid,ifu_arvalid} ),
    .valid  	( {lsu_rvalid,ifu_rvalid}   ),
    .ready  	( {lsu_rready,ifu_rready}   ),
    .sel    	( read_sel                  )
);

assign sram_arvalid = (read_sel==2'b01)?ifu_arvalid:((read_sel==2'b10)?lsu_arvalid:1'b0);
assign sram_rready = (read_sel==2'b01)?ifu_rready:lsu_rready;
assign sram_raddr = (read_sel==2'b01)?ifu_raddr:lsu_raddr;
assign sram_arsize = (read_sel==2'h01)?ifu_arsize:lsu_arsize;

assign ifu_arready = (read_sel==2'b01)?sram_arready:1'b0;
assign ifu_rvalid = (read_sel==2'b01)?sram_rvalid:1'b0;
assign ifu_rdata = sram_rdata;
assign ifu_rresp = sram_rresp;

assign lsu_arready = (read_sel==2'b10)?sram_arready:1'b0;
assign lsu_rvalid = (read_sel==2'b10)?sram_rvalid:1'b0;
assign lsu_rdata = sram_rdata;
assign lsu_rresp = sram_rresp;

//write
assign sram_awvalid = lsu_awvalid;
assign sram_waddr = lsu_waddr;
assign sram_wvalid = lsu_wvalid;
assign sram_wdata = lsu_wdata;
assign sram_strob = lsu_strob;
assign sram_bready = lsu_bready;

assign lsu_awready = sram_awready;
assign lsu_wready = sram_wready;
assign lsu_bvalid = sram_bvalid;
assign lsu_bresp = sram_bresp;

endmodule //axi_bus_matrix

module axi_arbiter#(parameter ARBITARTE_NUM=2)(
    input                       clk,
    input                       rst_n,
    input   [ARBITARTE_NUM-1:0] avalid,
    input   [ARBITARTE_NUM-1:0] valid,
    input   [ARBITARTE_NUM-1:0] ready,
    output  [ARBITARTE_NUM-1:0] sel
);


wire [ARBITARTE_NUM-1:0] sel_cascade;
wire [ARBITARTE_NUM-2:0] sel_cascade_mask;
genvar i;
generate for(i = 0 ; i < ARBITARTE_NUM ; i = i + 1 )begin: sel_cascade_module
    if(i==0)begin
        assign sel_cascade[i] = avalid[i];
        assign sel_cascade_mask[i] = (~avalid[i]);
    end
    else if(i==(ARBITARTE_NUM-1))begin
        assign sel_cascade[i] = avalid[i]&sel_cascade_mask[i-1];
    end
    else begin
        assign sel_cascade[i] = avalid[i]&sel_cascade_mask[i-1];
        assign sel_cascade_mask[i] = (~avalid[i])&sel_cascade_mask[i];
    end
end
endgenerate
wire [ARBITARTE_NUM-1:0]    result;
assign result = valid&ready;
reg [ARBITARTE_NUM-1:0] sel_reg;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        sel_reg<=0;
    end
    else begin
        if(sel!=0)begin
            if((sel&result)!=0)begin
                sel_reg<=sel_cascade;
            end
        end
        else begin
            if(avalid!=0)begin
                sel_reg<=sel_cascade;
            end
        end
    end
end
assign sel = sel_reg;

endmodule //axi_arbiter

