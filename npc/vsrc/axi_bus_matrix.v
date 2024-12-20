module axi_bus_matrix#(parameter DATA_LEN=32,ADDR_LEN=32,STROB_LEN=4)(
    input                   clk,
    input                   rst_n,
//ifu interface    
    //read addr channel
    input                   ifu_arvalid,
    output                  ifu_arready,
    input  [ADDR_LEN-1:0]   ifu_raddr,
    //read data channel
    output                  ifu_rvalid,
    input                   ifu_rready,
    output [2:0]            ifu_rresp,
    output [DATA_LEN-1:0]   ifu_rdata,
//lsu interface
    //read addr channel
    input                   lsu_arvalid,
    output                  lsu_arready,
    input  [ADDR_LEN-1:0]   lsu_raddr,
    //read data channel
    output                  lsu_rvalid,
    input                   lsu_rready,
    output [2:0]            lsu_rresp,
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
    output [2:0]            lsu_bresp,
//sram interface
    //read addr channel
    output                  sram_arvalid,
    input                   sram_arready,
    output [ADDR_LEN-1:0]   sram_raddr,
    //read data channel
    input                   sram_rvalid,
    output                  sram_rready,
    input [2:0]             sram_rresp,
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
    input [2:0]             sram_bresp
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

/* verilator lint_off DECLFILENAME */
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
