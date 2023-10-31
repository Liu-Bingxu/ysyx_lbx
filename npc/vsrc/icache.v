`include "define.v"
import "DPI-C" function void icache_access();
import "DPI-C" function void icache_hit();
module icache#(parameter WAY_NUM=4,SRAM_NUM=2,DATA_LEN=32)(
    input                   clk,
    input                   rst_n,
//interface with ifu
    //channel one : pc input
    input                   ifu_arvalid,
    output                  ifu_arready,
    input   [DATA_LEN-1:2]  ifu_raddr,
    //channel two : inst ouput
    output                  ifu_rvalid,
    input                   ifu_rready,
    output  [31:0]          ifu_rdata,
    output  [2:0]           ifu_rresp,
//interface with sram
    //channel one : read addr
    output                  icache_arvalid,
    input                   icache_arready,
    output  [DATA_LEN-1:0]  icache_raddr,
    //channel two : read data
    input                   icache_rvalid,
    output                  icache_rready,
    input   [2:0]           icache_rresp,
    input   [DATA_LEN-1:0]  icache_rdata
);

localparam TAG_LEN = DATA_LEN - 10 - $clog2(SRAM_NUM);
localparam ADDR_LEN = 6 + $clog2(SRAM_NUM);
localparam WAY_ADDR_LEN = $clog2(WAY_NUM);
localparam STEP = 2-DATA_LEN/32;

localparam ICACHE_IDLE      = 3'b000;
localparam ICACHE_CMP_TAG   = 3'b001;
localparam ICACHE_READ_ADDR = 3'b011;
localparam ICACHE_READ_DATA = 3'b111;
localparam ICACHE_GET_DATA  = 3'b010;

wire [127:0]      	    Q[0:WAY_NUM-1];
wire [TAG_LEN-1:0]      tag[0:WAY_NUM-1];
wire [TAG_LEN-1:0]      tag_in;
wire [127:0]      	    BWEN;
wire [127:0]      	    D;
wire [ADDR_LEN-1:0]     A;
wire                    WEN;
wire [WAY_NUM-1:0]      CEN;
wire [WAY_NUM-1:0]      valid;

wire [3:2]              offset;
wire [TAG_LEN-1:0]      target_tag;

wire  [WAY_NUM-1:0]     tag_res;
wire  [WAY_NUM-1:0]     res;

wire                    ifu_addr_handshake_flag;
wire                    ifu_data_handshake_flag;
wire                    icache_addr_handshake_flag;
wire                    icache_data_handshake_flag;

reg   [WAY_NUM-1:0]     CEN_reg;
reg                     WEN_reg;
reg                     ifu_arready_reg;
reg   [DATA_LEN-1:2]    ifu_raddr_reg;
reg                     ifu_rvalid_reg;
reg  [31:0]             ifu_rdata_reg;
reg  [2:0]              ifu_rresp_reg;
reg                     icache_arvalid_reg;
reg  [DATA_LEN-1:0]     icache_raddr_reg;
wire [127:0]            data;

reg                     bypass_flag;
reg                     icache_read_error;
wire [127:0]            data_sel;

reg  [2:0]              icache_fsm_status;
reg  [STEP:0]           icache_read_cnt;

wire [WAY_ADDR_LEN-1:0] rand_way;
reg  [WAY_ADDR_LEN-1:0] hit_way;
reg  [WAY_ADDR_LEN-1:0] hit_way_reg;

genvar i;
generate 
    for(i=0;i<WAY_NUM;i=i+1)begin: cache_sram  
        icache_way #(.DATA_LEN(DATA_LEN),.SRAM_NUM(SRAM_NUM))u_icache_way(
            .clk          	( clk                       ),
            .rst_n        	( rst_n                     ),
            .tag_in       	( tag_in                    ),
            // .addr_valid     ( ifu_addr_handshake_flag   ),
            .valid        	( valid[i]                  ),
            .tag          	( tag[i]                    ),
            .Q            	( Q[i]                      ),
            .CEN          	( CEN[i]                    ),
            .WEN          	( WEN                       ),
            .BWEN         	( BWEN                      ),
            .A            	( A                         ),
            .D            	( D                         )
        );
        assign tag_res[i] = (tag[i]==target_tag);
        assign res[i] = tag_res[i]&valid[i];
    end
endgenerate

assign offset       = ifu_raddr_reg[3:2];
assign A            = ifu_raddr_reg[(3+ADDR_LEN):4];
assign target_tag   = ifu_raddr_reg[DATA_LEN-1:(4+ADDR_LEN)];
assign D            = data;
assign tag_in       = target_tag;

FF_D_without_asyn_rst #(128) write_data_to_cache(clk,icache_data_handshake_flag,{icache_rdata,data[127:DATA_LEN]},data);

rand_lfsr_8_bit #(WAY_ADDR_LEN)u_rand_lfsr_8_bit_get_rand_way_num(
    .clk   	( clk           ),
    .rst_n 	( rst_n         ),
    .out   	( rand_way      )
);

// always @(*) begin
//     ifu_raddr_reg=ifu_raddr;
// end

//fsm logic 
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        icache_fsm_status<=ICACHE_IDLE;
        icache_read_cnt<={(STEP+1){1'b0}};
        ifu_rvalid_reg<=1'b0;
        WEN_reg<=1'b1;
        CEN_reg<={WAY_NUM{1'b1}};
        icache_arvalid_reg<=1'b0;
        bypass_flag<=1'b0;
        icache_read_error<=1'b0;
        ifu_arready_reg<=1'b1;
        // ifu_rresp_reg<=3'h0;
    end
    else begin
        case (icache_fsm_status)
            ICACHE_IDLE:begin
                if(ifu_addr_handshake_flag)begin
                    icache_fsm_status<=ICACHE_CMP_TAG;
                    ifu_raddr_reg<=ifu_raddr;
                    ifu_arready_reg<=1'b0;
                    icache_access();
                end
            end
            ICACHE_CMP_TAG:begin
                if(res==0)begin
                    icache_fsm_status<=ICACHE_READ_ADDR;
                    icache_arvalid_reg<=1'b1;
                    icache_raddr_reg<={ifu_raddr_reg[DATA_LEN-1:4],icache_read_cnt,{(DATA_LEN/32+1){1'b0}}};
                    icache_read_cnt<=icache_read_cnt+1'b1;
                end
                else begin
                    icache_fsm_status<=ICACHE_GET_DATA;
                    ifu_rresp_reg<=3'h0;
                    bypass_flag<=1'b0;
                    CEN_reg<=~res;
                    WEN_reg<=1'b1;
                    hit_way_reg<=hit_way;
                    icache_hit();
                end
                ifu_arready_reg<=1'b0;
            end
            ICACHE_READ_ADDR:begin
                if(icache_addr_handshake_flag)begin
                    icache_fsm_status<=ICACHE_READ_DATA;
                    icache_arvalid_reg<=1'b0;
                end
            end
            ICACHE_READ_DATA:begin
                if(icache_data_handshake_flag&(icache_rresp==3'b000)&(~icache_read_error))begin
                    if(icache_read_cnt=={(STEP+1){1'b0}})begin
                        icache_fsm_status<=ICACHE_GET_DATA;
                        ifu_rresp_reg<=3'h0;
                        bypass_flag<=1'b1;
                        WEN_reg<=1'b0;
                        CEN_reg[rand_way]<=1'b0;
                    end
                    else begin
                        icache_fsm_status<=ICACHE_READ_ADDR;
                        icache_arvalid_reg<=1'b1;
                        icache_read_cnt<=icache_read_cnt+1'b1;
                        icache_raddr_reg<={ifu_raddr_reg[DATA_LEN-1:4],icache_read_cnt,{(DATA_LEN/32+1){1'b0}}};
                    end
                end
                else if(icache_data_handshake_flag)begin
                    if(icache_read_cnt=={(STEP+1){1'b0}})begin
                        icache_fsm_status<=ICACHE_GET_DATA;
                        ifu_rresp_reg<=3'h2;
                        ifu_arready_reg<=1'b1;
                        icache_read_error<=1'b0;
                    end
                    else begin
                        icache_fsm_status<=ICACHE_READ_ADDR;
                        icache_arvalid_reg<=1'b1;
                        icache_read_cnt<=icache_read_cnt+1'b1;
                        icache_raddr_reg<={ifu_raddr_reg[DATA_LEN-1:4],icache_read_cnt,{(DATA_LEN/32+1){1'b0}}};
                        icache_read_error<=1'b1;
                    end
                end
            end
            ICACHE_GET_DATA:begin
                // if(ifu_data_handshake_flag&ifu_arvalid)begin
                //     icache_fsm_status<=ICACHE_CMP_TAG;
                //     ifu_raddr_reg<=ifu_raddr;
                //     ifu_arready_reg<=1'b1;
                //     ifu_rvalid_reg<=1'b0;
                //     icache_access();
                // end
                if(ifu_data_handshake_flag)begin
                    icache_fsm_status<=ICACHE_IDLE;
                    ifu_arready_reg<=1'b1;
                    ifu_rvalid_reg<=1'b0;
                end
                else begin
                    CEN_reg<={WAY_NUM{1'b1}};
                    ifu_rvalid_reg<=1'b1;
                end
            end
            default:begin
                icache_fsm_status<=ICACHE_IDLE;
            end
        endcase
    end
end

//change with the way number 
always @(*) begin
    case (res)
        4'b0001:hit_way=0;
        4'b0010:hit_way=1;
        4'b0100:hit_way=2;
        4'b1000:hit_way=3;
        // 8'h01:hit_way=0;
        // 8'h02:hit_way=1;
        // 8'h04:hit_way=2;
        // 8'h08:hit_way=3;
        // 8'h10:hit_way=4;
        // 8'h20:hit_way=5;
        // 8'h40:hit_way=6;
        // 8'h80:hit_way=7;
        default: begin
            hit_way=0;
        end
    endcase
end

assign data_sel = (bypass_flag)?data:Q[hit_way_reg];
always @(*) begin
    case (offset)
        2'b00: begin
            ifu_rdata_reg=data_sel[31:0];
        end
        2'b01:begin
            ifu_rdata_reg=data_sel[63:32];
        end
        2'b10:begin
            ifu_rdata_reg=data_sel[95:64];
        end
        2'b11:begin
            ifu_rdata_reg=data_sel[127:96];
        end
        // default: 
    endcase
end

assign WEN = WEN_reg;
assign CEN = CEN_reg;
assign ifu_arready = ifu_arready_reg;
assign ifu_rvalid = ifu_rvalid_reg;
assign ifu_rdata = ifu_rdata_reg;
assign ifu_rresp = ifu_rresp_reg;
assign ifu_addr_handshake_flag = ifu_arvalid&ifu_arready;
assign ifu_data_handshake_flag = ifu_rvalid &ifu_rready;
assign icache_arvalid=icache_arvalid_reg;
assign icache_raddr=icache_raddr_reg;
assign icache_addr_handshake_flag = icache_arvalid&icache_arready;
assign icache_data_handshake_flag = icache_rvalid &icache_rready;

assign BWEN = 0;
assign icache_rready = 1'b1;

endmodule //icache

/* verilator lint_off DECLFILENAME */
module icache_way#(parameter DATA_LEN=32,TAG_LEN=DATA_LEN-10-$clog2(SRAM_NUM),SRAM_NUM=1,ADDR_LEN=6+$clog2(SRAM_NUM))(
    input                   clk,
    input                   rst_n,
//interface with cache to get the flag
    input  [TAG_LEN-1:0]    tag_in,
    // input                   addr_valid,
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

// localparam CACHE_LINE_NUM = 64*SRAM_NUM;

// wire [TAG_LEN-1:0]  cache_tag[0:CACHE_LINE_NUM-1];
// wire                cache_valid[0:CACHE_LINE_NUM-1];
// wire                cache_tag_wen[0:CACHE_LINE_NUM-1];
wire                cache_tag_wen;
wire                cache_CEN[0:SRAM_NUM-1];
wire [127:0]        cache_Q[0:SRAM_NUM-1];

// wire [TAG_LEN:0]    cache_flag[0:CACHE_LINE_NUM-1];
wire [TAG_LEN:0]    cache_flag;

// genvar i;
// generate
//     for(i=0;i<CACHE_LINE_NUM;i=i+1)begin: cacheline
//         assign cache_tag_wen[i] = (A==i)&(~CEN)&(~WEN);
//         // FF_D #(1,0) ff_valid (clk,rst_n,cache_tag_wen[i],1'b1,cache_valid[i]);
//         // FF_D_without_asyn_rst #(TAG_LEN) ff_tag (clk,cache_tag_wen[i], tag_in,cache_tag[i]);
//         FF_D #(TAG_LEN+1,0) ff_flag (clk,rst_n,cache_tag_wen[i],{1'b1,tag_in},cache_flag[i]);
//         // assign cache_tag[i] = cache_flag[i][TAG_LEN-1:0];
//         // assign cache_valid[i] = cache_flag[i][TAG_LEN];
//     end
// endgenerate
assign cache_tag_wen = (~CEN)&(~WEN);
FF_D_matrix #(TAG_LEN+1,0,ADDR_LEN) ff_flag_matrix (clk,rst_n,cache_tag_wen,A,{1'b1,tag_in},cache_flag);
// assign valid = cache_valid[A];
// assign tag = cache_tag[A];
// FF_D #(1,0)       valid_out (clk,rst_n,addr_valid,cache_valid[A],valid);
// FF_D #(TAG_LEN,0) tag_out   (clk,rst_n,addr_valid,cache_tag[A],tag);
// FF_D #(1,0)       valid_out (clk,rst_n,addr_valid,cache_flag[TAG_LEN],valid);
// FF_D #(TAG_LEN,0) tag_out   (clk,rst_n,addr_valid,cache_flag[TAG_LEN-1:0],tag);
assign valid = cache_flag[TAG_LEN];
assign tag = cache_flag[TAG_LEN-1:0];

genvar y;
generate
    for(y=0;y<SRAM_NUM;y=y+1)begin: cache_sram

        `ifdef ICACHE_LINE_MULIT_SRAM

        assign cache_CEN[y] = CEN | (~(A[ADDR_LEN-1:6]==y));

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

`ifdef ICACHE_LINE_MULIT_SRAM

assign Q = cache_Q[A[ADDR_LEN-1:6]];

`else

assign Q = cache_Q[0];

`endif

endmodule //icache_way

