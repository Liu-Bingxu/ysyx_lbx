`include "define.v"
module icache_line#(parameter DATA_LEN=32,TAG_LEN=DATA_LEN-10-$clog2(SRAM_NUM),SRAM_NUM=1,ADDR_LEN=6+$clog2(SRAM_NUM))(
    input                   clk,
    input                   rst_n,
//interface with cache to get the flag
    input  [TAG_LEN-1:0]    tag_in,
    input                   addr_valid,
    output                  valid,
    output [TAG_LEN-1:0]    tag,
//interface with cache_sram
    output [127:0]          Q,
    input                   CEN,
    input                   WEN,
    input  [127:0]          BWEN,
    input  [ADDR_LEN-1:0]   A,
    input  [127:0]          D
);

localparam CACHE_LINE_NUM = 64*SRAM_NUM;

wire [TAG_LEN-1:0]  cache_tag[0:CACHE_LINE_NUM-1];
wire                cache_valid[0:CACHE_LINE_NUM-1];
wire                cache_tag_wen[0:CACHE_LINE_NUM-1];
wire                cache_CEN[0:SRAM_NUM-1];
wire [127:0]        cache_Q[0:SRAM_NUM-1];

genvar i;
generate
    for(i=0;i<CACHE_LINE_NUM;i=i+1)begin: cacheline
        assign cache_tag_wen[i] = (A==i)&(~CEN)&(~WEN);
        FF_D #(1,0) ff_valid (clk,rst_n,cache_tag_wen[i],1'b1,cache_valid[i]);
        FF_D_without_asyn_rst #(TAG_LEN) ff_tag (clk,cache_tag_wen[i], tag_in,cache_tag[i]);
    end
endgenerate

// assign valid = cache_valid[A];
// assign tag = cache_tag[A];
FF_D #(1,0)       valid_out (clk,rst_n,addr_valid,cache_valid[A],valid);
FF_D #(TAG_LEN,0) tag_out   (clk,rst_n,addr_valid,cache_tag[A],tag);

genvar y;
generate
    for(y=0;y<SRAM_NUM;y=y+1)begin: cache_sram

        `ifdef CACHE_LINE_MULIT_SRAM

        assign cache_CEN[y] = CEN & (A[ADDR_LEN-1:6]==y);

        `else

        assign cache_CEN[y] = CEN;

        `endif

        S011HD1P_X32Y2D128_BW u_S011HD1P_X32Y2D128_BW(
            .Q    	( cache_Q[y]    ),
            .CLK  	( clk           ),
            .CEN  	( cache_CEN[y]  ),
            .WEN  	( WEN           ),
            .BWEN 	( BWEN          ),
            .A    	( A[5:0]        ),
            .D    	( D             )
        );
    end
endgenerate

`ifdef CACHE_LINE_MULIT_SRAM

assign Q = cache_Q[A[ADDR_LEN-1:6]] & {128{CEN}};

`else

assign Q = cache_Q[0] & {128{CEN}};

`endif

endmodule //icache_line
