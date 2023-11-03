`include "./define.v"
`ifdef DPI_C
import "DPI-C" function void dcache_access();
import "DPI-C" function void dcache_hit();
import "DPI-C" function void mmio_access();
`endif
module dcache#(parameter WAY_NUM=4,SRAM_NUM=2,DATA_LEN=32,DATA_BIT_NUM=4,ADDR_LSB=DATA_LEN/32+1)(
    input                           clk,
    input                           rst_n,
//interface with lsu
    //channel one: read addr
    input                           lsu_arvalid,
    output                          lsu_arready,
    input   [DATA_LEN-1:0]          lsu_raddr,
    //channel two: read data
    output                          lsu_rvalid,
    input                           lsu_rready,
    output  [DATA_LEN-1:0]          lsu_rdata,
    output  [2:0]                   lsu_rresp,
    //channel three: write addr
    input                           lsu_awvalid,
    output                          lsu_awready,
    input   [DATA_LEN-1:0]          lsu_waddr,
    //channel four: write data
    input                           lsu_wvalid,
    output                          lsu_wready,
    input   [DATA_BIT_NUM-1:0]      lsu_wstrob,
    input   [DATA_LEN-1:0]          lsu_wdata,
    //channel five: write resp
    output                          lsu_bvalid,
    input                           lsu_bready,
    output  [2:0]                   lsu_bresp,
//interface with sram
    //channel one: read addr
    output                          dcache_arvalid,
    input                           dcache_arready,
    output  [DATA_LEN-1:0]          dcache_raddr,
    //channel two: read data
    input                           dcache_rvalid,
    output                          dcache_rready,
    input   [2:0]                   dcache_rresp,
    input   [DATA_LEN-1:0]          dcache_rdata,
    //channel three: write addr
    output                          dcache_awvalid,
    input                           dcache_awready,
    output  [DATA_LEN-1:0]          dcache_waddr,
    //channel four: write data
    output                          dcache_wvalid,
    input                           dcache_wready,
    output  [DATA_BIT_NUM-1:0]      dcache_wstrob,
    output  [DATA_LEN-1:0]          dcache_wdata,
    //channel five: write resp
    input                           dcache_bvalid,
    output                          dcache_bready,
    input   [2:0]                   dcache_bresp
);

localparam TAG_LEN = DATA_LEN - 10 - $clog2(SRAM_NUM);
localparam ADDR_LEN = 6 + $clog2(SRAM_NUM);
localparam WAY_ADDR_LEN = $clog2(WAY_NUM);
localparam STEP = 2-DATA_LEN/32;

localparam DCACHE_IDLE                      = 4'b0000;
localparam DCACHE_CMP_TAG                   = 4'b0001;
localparam DCACHE_READ_ADDR                 = 4'b0011;
localparam DCACHE_READ_DATA                 = 4'b0111;
localparam DCACHE_WRITE_WAIT_ADDR_DATA      = 4'b1111;
localparam DCACHE_WRITE_WAIT_ADDR           = 4'b1101;
localparam DCACHE_WRITE_WAIT_DATA           = 4'b1011;
localparam DCACHE_WRITE_WAIT_BACK           = 4'b1001;
localparam DCACHE_WRITE_DATA                = 4'b1000;
localparam DCACHE_GET_DATA                  = 4'b0101;
localparam DCACHE_MMIO_READ_ADDR            = 4'b0010;
localparam DCACHE_MMIO_READ_DATA            = 4'b1010;
localparam DCACHE_MMIO_WRITE_WAIT_ADDR_DATA = 4'b0100;
localparam DCACHE_MMIO_WRITE_WAIT_ADDR      = 4'b0110;
localparam DCACHE_MMIO_WRITE_WAIT_DATA      = 4'b1100;
localparam DCACHE_MMIO_WRITE_WAIT_BACK      = 4'b1110;

wire [127:0]      	    Q[0:WAY_NUM-1];
wire [TAG_LEN-1:0]      tag[0:WAY_NUM-1];
wire [TAG_LEN-1:0]      tag_in;
wire [127:0]      	    BWEN;
wire [127:0]      	    D;
wire [ADDR_LEN-1:0]     A;
wire                    WEN;
wire                    dirty_flag;
wire [WAY_NUM-1:0]      CEN;
wire [WAY_NUM-1:0]      valid;
wire [WAY_NUM-1:0]      dirty;

wire [3:2]              offset;
wire [TAG_LEN-1:0]      target_tag;

wire  [WAY_NUM-1:0]     tag_res;
wire  [WAY_NUM-1:0]     res;

wire                    lsu_read_addr_handshake_flag;
wire                    lsu_read_data_handshake_flag;
wire                    lsu_write_addr_handshake_flag;
wire                    lsu_write_data_handshake_flag;
wire                    lsu_write_back_handshake_flag;
wire                    dcache_read_addr_handshake_flag;
wire                    dcache_read_data_handshake_flag;
wire                    dcache_write_addr_handshake_flag;
wire                    dcache_write_data_handshake_flag;
wire                    dcache_write_back_handshake_flag;

reg  [127:0]            D_reg;
reg  [127:0]      	    BWEN_reg;
reg  [WAY_NUM-1:0]      CEN_reg;
reg                     WEN_reg;

reg                     mmio_access_flag_reg;

reg                     lsu_arready_reg;
reg                     lsu_rvalid_reg;
reg  [31:0]             lsu_rdata_reg;
reg  [2:0]              lsu_rresp_reg;
reg                     lsu_awready_reg;
reg                     lsu_wready_reg;
reg                     lsu_bvalid_reg;
reg  [2:0]              lsu_bresp_reg;

reg                     dcache_arvalid_reg;
reg  [DATA_LEN-1:0]     dcache_raddr_reg;
// reg                     dcache_rready_reg;
reg                     dcache_awvalid_reg;
reg  [DATA_LEN-1:0]     dcache_waddr_reg;
reg                     dcache_wvalid_reg;
// reg  [DATA_BIT_NUM-1:0] dcache_wstrob_reg;
reg  [DATA_LEN-1:0]     dcache_wdata_reg;
// reg                     dcache_bready_reg;

reg                     dirty_flag_reg;
wire [127:0]            data;
reg  [127:0]            data_reg;
reg  [DATA_LEN-1:0]     write_data_reg;

wire [DATA_LEN-1:ADDR_LSB]     addr;
reg  [DATA_LEN-1:ADDR_LSB]     addr_reg;
reg                     bypass_flag;
// reg                     data_bypass_flag;
reg  [DATA_LEN-1:0]     wdata;
wire [127:0]            bypass_data;
reg                     dcache_read_error;
reg                     dcache_write_error;
wire [127:0]            data_sel;

reg  [3:0]              dcache_fsm_status;
reg  [STEP:0]           dcache_cnt;

wire [WAY_ADDR_LEN-1:0] rand_way;
reg  [WAY_ADDR_LEN-1:0] rand_way_reg;
reg  [WAY_ADDR_LEN-1:0] hit_way;
reg  [WAY_ADDR_LEN-1:0] hit_way_reg;

reg                     lsu_write_flag;

reg  [127:0]            bwen;

localparam MMIO_SPACE_START                 = 32'hA0000000;
localparam MMIO_SPACE_END                   = 32'hA8000000;

wire                    mmio_access_flag;
assign mmio_access_flag = ({addr,{ADDR_LSB{1'b0}}}>=MMIO_SPACE_START)&({addr,{ADDR_LSB{1'b0}}}<=MMIO_SPACE_END);

genvar i;
generate 
    for(i=0;i<WAY_NUM;i=i+1)begin: cache_sram  
        dcache_way #(.DATA_LEN(DATA_LEN),.SRAM_NUM(SRAM_NUM))u_dcache_way(
            .clk          	( clk           ),
            .rst_n        	( rst_n         ),
            .tag_in       	( tag_in        ),
            .dirty_flag     ( dirty_flag    ),
            .dirty          ( dirty[i]      ),
            .valid        	( valid[i]      ),
            .tag          	( tag[i]        ),
            .Q            	( Q[i]          ),
            .CEN          	( CEN[i]        ),
            .WEN          	( WEN           ),
            .BWEN         	( BWEN          ),
            .A            	( A             ),
            .D            	( D             )
        );
        assign tag_res[i] = (tag[i]==target_tag);
        assign res[i] = tag_res[i]&valid[i];
    end
endgenerate

assign offset       = addr_reg[3:2];
assign A            = addr_reg[(3+ADDR_LEN):4];
assign target_tag   = addr_reg[31:(4+ADDR_LEN)];
assign D            = D_reg;
assign tag_in       = target_tag;
assign bypass_data  = {(128/DATA_LEN){wdata}};

FF_D_without_asyn_rst #(128) write_data_to_cache(clk,dcache_read_data_handshake_flag,{dcache_rdata,data[127:DATA_LEN]},data);

rand_lfsr_8_bit #(WAY_ADDR_LEN)u_rand_lfsr_8_bit_get_rand_way_num(
    .clk   	( clk           ),
    .rst_n 	( rst_n         ),
    .out   	( rand_way      )
);
// wire [127:0] D_reg_2;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        dcache_fsm_status<=DCACHE_IDLE;

        lsu_arready_reg<=1'b1;
        lsu_rvalid_reg<=1'b0;
        lsu_awready_reg<=1'b1;
        lsu_wready_reg<=1'b1;
        lsu_bvalid_reg<=1'b0;

        dcache_awvalid_reg<=1'b0;
        dcache_wvalid_reg<=1'b0;
        dcache_arvalid_reg<=1'b0;

        lsu_write_flag<=1'b0;

        WEN_reg<=1'b1;

        dcache_read_error<=1'b0;
        dcache_write_error<=1'b0;

        CEN_reg<={WAY_NUM{1'b1}};

        mmio_access_flag_reg<=1'b0;
    end
    else begin
        case (dcache_fsm_status)
            DCACHE_IDLE: begin
                if(lsu_read_addr_handshake_flag|(lsu_write_addr_handshake_flag&lsu_write_data_handshake_flag))begin
                    if(mmio_access_flag)begin
                        if(lsu_write_addr_handshake_flag&lsu_write_data_handshake_flag)begin
                            dcache_fsm_status<=DCACHE_MMIO_WRITE_WAIT_ADDR_DATA;
                            dcache_awvalid_reg<=1'b1;
                            dcache_wvalid_reg<=1'b1;
                            dcache_waddr_reg<=lsu_waddr;
                            dcache_wdata_reg<=lsu_wdata;
                        end
                        else if(lsu_read_addr_handshake_flag)begin
                            dcache_fsm_status<=DCACHE_MMIO_READ_ADDR;
                            dcache_arvalid_reg<=1'b1;
                            mmio_access_flag_reg<=1'b1;
                            dcache_raddr_reg<=lsu_raddr;
                        end
                        `ifdef DPI_C
                        mmio_access();
                        `endif
                    end
                    else begin
                        dcache_fsm_status<=DCACHE_CMP_TAG;
                        `ifdef DPI_C
                        dcache_access();
                        `endif
                    end
                    addr_reg<=addr;
                    lsu_arready_reg<=1'b0;
                    lsu_awready_reg<=1'b0;
                    lsu_wready_reg<=1'b0;
                end
                if(lsu_write_addr_handshake_flag&lsu_write_data_handshake_flag)begin
                    wdata<=lsu_wdata;
                    lsu_write_flag<=1'b1;
                end
                else if(lsu_read_addr_handshake_flag)begin
                    lsu_write_flag<=1'b0;
                end
            end
            DCACHE_CMP_TAG:begin
                if(res==0)begin
                    //temp
                    // if(lsu_write_flag)begin
                    //     dcache_fsm_status<=DCACHE_IDLE;
                    //     lsu_arready_reg<=1'b1;
                    //     lsu_awready_reg<=1'b1;
                    //     lsu_wready_reg<=1'b1;
                    // end
                    // else begin
                    //     dcache_fsm_status<=DCACHE_READ_ADDR;
                    //     dcache_arvalid_reg<=1'b1;
                    //     dcache_raddr_reg<={addr_reg[DATA_LEN-1:4],dcache_cnt,{(DATA_LEN/32+1){1'b0}}};
                    //     dcache_cnt<=dcache_cnt+1'b1;
                    //     rand_way_reg<=rand_way;
                    //     WEN_reg<=1'b1;
                    //     CEN_reg[rand_way]<=1'b0;
                    // end
                    //temp
                    dcache_fsm_status<=DCACHE_READ_ADDR;
                    dcache_arvalid_reg<=1'b1;
                    dcache_raddr_reg<={addr_reg[DATA_LEN-1:4],dcache_cnt,{(DATA_LEN/32+1){1'b0}}};
                    dcache_cnt<=dcache_cnt+1'b1;
                    rand_way_reg<=rand_way;
                    WEN_reg<=1'b1;
                    CEN_reg[rand_way]<=1'b0;
                end
                else begin
                    if(lsu_write_flag==0)begin
                        dcache_fsm_status<=DCACHE_GET_DATA;
                        lsu_rresp_reg<=3'h0;
                        bypass_flag<=1'b0;
                        WEN_reg<=1'b1;
                        hit_way_reg<=hit_way;
                    end
                    else begin
                        // dcache_fsm_status<=DCACHE_WRITE_DATA;
                        // WEN_reg<=1'b0;
                        // BWEN_reg<={128{1'b1}};
                        // dirty_flag_reg<=1'b0;
                        // valid_falg<=1'b0;
                        dcache_fsm_status<=DCACHE_WRITE_DATA;
                        lsu_bresp_reg<=3'h0;
                        WEN_reg<=1'b0;
                        dirty_flag_reg<=1'b1;
                        D_reg<=bypass_data;
                        BWEN_reg<=~bwen;
                    end
                    CEN_reg<=~res;
                    `ifdef DPI_C
                    dcache_hit();
                    `endif
                end
            end
            DCACHE_READ_ADDR:begin
                if(dcache_read_addr_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_READ_DATA;
                    dcache_arvalid_reg<=1'b0;
                end
            end
            DCACHE_READ_DATA:begin
                if(dcache_read_data_handshake_flag&(dcache_rresp==3'b000)&(~dcache_read_error))begin
                    if(dcache_cnt=={(STEP+1){1'b0}})begin
                        if(dirty[rand_way_reg])begin
                            dcache_fsm_status<=DCACHE_WRITE_WAIT_ADDR_DATA;
                            dcache_awvalid_reg<=1'b1;
                            dcache_wvalid_reg<=1'b1;
                            data_reg<=Q[rand_way_reg];
                            dcache_waddr_reg<={tag[rand_way_reg],A[ADDR_LEN-1:0],dcache_cnt,{(3-STEP){1'b0}}};
                            dcache_wdata_reg<=Q[rand_way_reg][DATA_LEN-1:0];
                            dcache_cnt<=dcache_cnt+1'b1;
                        end
                        else begin
                            if(lsu_write_flag)begin
                                dcache_fsm_status<=DCACHE_WRITE_DATA;
                                lsu_bresp_reg<=3'h0;
                                WEN_reg<=1'b0;
                                dirty_flag_reg<=1'b1;
                                D_reg<=(bypass_data&bwen)|({dcache_rdata,data[127:DATA_LEN]}&(~bwen));
                                BWEN_reg<={128{1'b0}};
                            end
                            else begin
                                dcache_fsm_status<=DCACHE_GET_DATA;
                                lsu_rresp_reg<=3'h0;
                                bypass_flag<=1'b1;
                                WEN_reg<=1'b0;
                                dirty_flag_reg<=1'b0;
                                D_reg<={dcache_rdata,data[127:DATA_LEN]};
                                BWEN_reg<={128{1'b0}};
                            end
                        end
                    end
                    else begin
                        dcache_fsm_status<=DCACHE_READ_ADDR;
                        dcache_arvalid_reg<=1'b1;
                        dcache_cnt<=dcache_cnt+1'b1;
                        dcache_raddr_reg<={addr_reg[DATA_LEN-1:4],dcache_cnt,{(DATA_LEN/32+1){1'b0}}};
                    end
                end
                else if(dcache_read_data_handshake_flag)begin
                    if(dcache_cnt=={(STEP+1){1'b0}})begin
                        if(lsu_write_flag)begin
                            dcache_fsm_status<=DCACHE_WRITE_DATA;
                            lsu_bresp_reg<=3'h2;
                            dcache_read_error<=1'b0;
                        end
                        else begin
                            dcache_fsm_status<=DCACHE_GET_DATA;
                            lsu_rresp_reg<=3'h2;
                            dcache_read_error<=1'b0;
                        end
                    end
                    else begin
                        dcache_fsm_status<=DCACHE_READ_ADDR;
                        dcache_arvalid_reg<=1'b1;
                        dcache_cnt<=dcache_cnt+1'b1;
                        dcache_raddr_reg<={addr_reg[DATA_LEN-1:4],dcache_cnt,{(DATA_LEN/32+1){1'b0}}};
                        dcache_read_error<=1'b1;
                    end
                end
            end
            DCACHE_WRITE_WAIT_ADDR_DATA:begin
                if(dcache_write_addr_handshake_flag&dcache_write_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_WRITE_WAIT_BACK;
                    dcache_awvalid_reg<=1'b0;
                    dcache_wvalid_reg<=1'b0;
                end
                else if(dcache_write_addr_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_WRITE_WAIT_DATA;
                    dcache_awvalid_reg<=1'b0;
                end
                else if(dcache_write_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_WRITE_WAIT_ADDR;
                    dcache_wvalid_reg<=1'b0;
                end
            end
            DCACHE_WRITE_WAIT_DATA:begin
                if(dcache_write_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_WRITE_WAIT_BACK;
                    dcache_wvalid_reg<=1'b0;
                end
            end
            DCACHE_WRITE_WAIT_ADDR:begin
                if(dcache_write_addr_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_WRITE_WAIT_BACK;
                    dcache_awvalid_reg<=1'b0;
                end
            end
            DCACHE_WRITE_WAIT_BACK:begin
                if(dcache_write_back_handshake_flag&(dcache_bresp==3'h0)&(~dcache_write_error))begin
                    if(dcache_cnt=={(STEP+1){1'b0}})begin
                        if(lsu_write_flag)begin
                            dcache_fsm_status<=DCACHE_WRITE_DATA;
                            lsu_bresp_reg<=3'h0;
                            WEN_reg<=1'b0;
                            dirty_flag_reg<=1'b1;
                            D_reg<=(bypass_data&bwen)|(data&(~bwen));
                            BWEN_reg<={128{1'b0}};
                        end
                        else begin
                            dcache_fsm_status<=DCACHE_GET_DATA;
                            lsu_rresp_reg<=3'h0;
                            bypass_flag<=1'b1;
                            WEN_reg<=1'b0;
                            dirty_flag_reg<=1'b0;
                            D_reg<=data;
                            BWEN_reg<={128{1'b0}};
                        end
                    end
                    else begin
                        dcache_fsm_status<=DCACHE_WRITE_WAIT_ADDR_DATA;
                        dcache_awvalid_reg<=1'b1;
                        dcache_wvalid_reg<=1'b1;
                        dcache_waddr_reg<={tag[rand_way_reg],A[ADDR_LEN-1:0],dcache_cnt,{(3-STEP){1'b0}}};
                        dcache_wdata_reg<=write_data_reg;
                        dcache_cnt<=dcache_cnt+1'b1;
                    end
                end
                else if(dcache_write_back_handshake_flag)begin
                    if(dcache_cnt=={(STEP+1){1'b0}})begin
                        if(lsu_write_flag)begin
                            dcache_fsm_status<=DCACHE_WRITE_DATA;
                            lsu_bresp_reg<=3'h2;
                            dcache_write_error<=1'b0;
                        end
                        else begin
                            dcache_fsm_status<=DCACHE_GET_DATA;
                            lsu_rresp_reg<=3'h2;
                            dcache_write_error<=1'b0;
                        end
                    end
                    else begin
                        dcache_fsm_status<=DCACHE_WRITE_WAIT_ADDR_DATA;
                        dcache_awvalid_reg<=1'b1;
                        dcache_wvalid_reg<=1'b1;
                        dcache_waddr_reg<={tag[rand_way_reg],A[ADDR_LEN-1:0],dcache_cnt,{(3-STEP){1'b0}}};
                        dcache_wdata_reg<=write_data_reg;
                        dcache_cnt<=dcache_cnt+1'b1;
                        dcache_write_error<=1'b1;
                    end
                end
            end
            DCACHE_GET_DATA:begin
                if(lsu_read_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_IDLE;
                    mmio_access_flag_reg<=1'b0;
                    lsu_arready_reg<=1'b1;
                    lsu_awready_reg<=1'b1;
                    lsu_wready_reg<=1'b1;
                    lsu_rvalid_reg<=1'b0;
                end
                else begin
                    CEN_reg<={WAY_NUM{1'b1}};
                    lsu_rvalid_reg<=1'b1;
                end
            end
            DCACHE_WRITE_DATA:begin
                if(lsu_write_back_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_IDLE;
                    lsu_arready_reg<=1'b1;
                    lsu_awready_reg<=1'b1;
                    lsu_wready_reg<=1'b1;
                    lsu_bvalid_reg<=1'b0;
                end
                else begin
                    CEN_reg<={WAY_NUM{1'b1}};
                    lsu_bvalid_reg<=1'b1;
                end
            end
            DCACHE_MMIO_READ_ADDR:begin
                if(dcache_read_addr_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_MMIO_READ_DATA;
                    dcache_arvalid_reg<=1'b0;
                end
            end
            DCACHE_MMIO_READ_DATA:begin
                if(dcache_read_data_handshake_flag&(dcache_rresp==3'h0))begin
                    dcache_fsm_status<=DCACHE_GET_DATA;
                    lsu_rresp_reg<=3'h0;
                end
                else if(dcache_read_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_GET_DATA;
                    lsu_rresp_reg<=3'h2;
                end
            end
            DCACHE_MMIO_WRITE_WAIT_ADDR_DATA:begin
                if(dcache_write_addr_handshake_flag&dcache_write_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_MMIO_WRITE_WAIT_BACK;
                    dcache_awvalid_reg<=1'b0;
                    dcache_wvalid_reg<=1'b0;
                end
                else if(dcache_write_addr_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_MMIO_WRITE_WAIT_DATA;
                    dcache_awvalid_reg<=1'b0;
                end
                else if(dcache_write_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_MMIO_WRITE_WAIT_ADDR;
                    dcache_wvalid_reg<=1'b0;
                end
            end
            DCACHE_MMIO_WRITE_WAIT_DATA:begin
                if(dcache_write_data_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_MMIO_WRITE_WAIT_BACK;
                    dcache_wvalid_reg<=1'b0;
                end
            end
            DCACHE_MMIO_WRITE_WAIT_ADDR:begin
                if(dcache_write_addr_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_MMIO_WRITE_WAIT_BACK;
                    dcache_awvalid_reg<=1'b0;
                end
            end
            DCACHE_MMIO_WRITE_WAIT_BACK:begin
                if(dcache_write_back_handshake_flag&(dcache_bresp==3'h0))begin
                    dcache_fsm_status<=DCACHE_WRITE_DATA;
                    lsu_bresp_reg<=3'h0;
                end
                else if(dcache_write_back_handshake_flag)begin
                    dcache_fsm_status<=DCACHE_WRITE_DATA;
                    lsu_bresp_reg<=3'h2;
                end
            end
            default:begin
                dcache_fsm_status<=DCACHE_IDLE;

                lsu_arready_reg<=1'b1;
                lsu_rvalid_reg<=1'b0;
                lsu_awready_reg<=1'b1;
                lsu_wready_reg<=1'b1;
                lsu_bvalid_reg<=1'b0;

                dcache_awvalid_reg<=1'b0;
                dcache_wvalid_reg<=1'b0;
                dcache_arvalid_reg<=1'b0;

                lsu_write_flag<=1'b0;

                WEN_reg<=1'b1;

                dcache_read_error<=1'b0;
                dcache_write_error<=1'b0;

                CEN_reg<={WAY_NUM{1'b1}};
            end
        endcase
    end
end

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

always @(*) begin
    case (dcache_cnt)
        2'b00: write_data_reg=data_reg[31:0];
        2'b01: write_data_reg=data_reg[63:32];
        2'b10: write_data_reg=data_reg[95:64];
        2'b11: write_data_reg=data_reg[127:96];
        // default: 
    endcase
end

always @(*) begin
    case (offset)
        2'b00:bwen={96'h0,{8{lsu_wstrob[3]}},{8{lsu_wstrob[2]}},{8{lsu_wstrob[1]}},{8{lsu_wstrob[0]}}};
        2'b01:bwen={64'h0,{8{lsu_wstrob[3]}},{8{lsu_wstrob[2]}},{8{lsu_wstrob[1]}},{8{lsu_wstrob[0]}},32'h0};
        2'b10:bwen={32'h0,{8{lsu_wstrob[3]}},{8{lsu_wstrob[2]}},{8{lsu_wstrob[1]}},{8{lsu_wstrob[0]}},64'h0};
        2'b11:bwen={{8{lsu_wstrob[3]}},{8{lsu_wstrob[2]}},{8{lsu_wstrob[1]}},{8{lsu_wstrob[0]}},96'h0};
    endcase
end

assign data_sel = (bypass_flag)?data:Q[hit_way_reg];
always @(*) begin
    case (offset)
        2'b00: begin
            lsu_rdata_reg=data_sel[31:0];
        end
        2'b01:begin
            lsu_rdata_reg=data_sel[63:32];
        end
        2'b10:begin
            lsu_rdata_reg=data_sel[95:64];
        end
        2'b11:begin
            lsu_rdata_reg=data_sel[127:96];
        end
        // default: 
    endcase
end

assign addr = (lsu_read_addr_handshake_flag)?lsu_raddr[DATA_LEN-1:ADDR_LSB]:lsu_waddr[DATA_LEN-1:ADDR_LSB];

assign dirty_flag = dirty_flag_reg;
assign BWEN = BWEN_reg;
assign WEN = WEN_reg;
assign CEN = CEN_reg;

assign lsu_read_addr_handshake_flag     = lsu_arvalid & lsu_arready;
assign lsu_read_data_handshake_flag     = lsu_rvalid & lsu_rready;
assign lsu_write_addr_handshake_flag    = lsu_awvalid & lsu_awready;
assign lsu_write_data_handshake_flag    = lsu_wvalid & lsu_wready;
assign lsu_write_back_handshake_flag    = lsu_bvalid & lsu_bready;
assign dcache_read_addr_handshake_flag  = dcache_arvalid & dcache_arready;
assign dcache_read_data_handshake_flag  = dcache_rvalid & dcache_rready;
assign dcache_write_addr_handshake_flag = dcache_awvalid & dcache_awready;
assign dcache_write_data_handshake_flag = dcache_wvalid & dcache_wready;
assign dcache_write_back_handshake_flag = dcache_bvalid & dcache_bready;

assign lsu_arready = lsu_arready_reg;
assign lsu_rvalid = lsu_rvalid_reg;
assign lsu_rdata = (mmio_access_flag_reg)?data[127:128-DATA_LEN]:lsu_rdata_reg;
assign lsu_rresp = lsu_rresp_reg;
assign lsu_awready = lsu_awready_reg;
assign lsu_wready = lsu_wready_reg;
assign lsu_bvalid = lsu_bvalid_reg;
assign lsu_bresp = lsu_bresp_reg;

assign dcache_arvalid = dcache_arvalid_reg;
assign dcache_raddr = dcache_raddr_reg;
// assign dcache_rready = dcache_rready_reg;
assign dcache_rready = 1'b1;
assign dcache_awvalid = dcache_awvalid_reg;
assign dcache_waddr = dcache_waddr_reg;
assign dcache_wvalid = dcache_wvalid_reg;
assign dcache_wdata = dcache_wdata_reg;
// assign dcache_wstrob = dcache_wstrob_reg;
assign dcache_wstrob = {DATA_BIT_NUM{1'b1}};
// assign dcache_bready = dcache_bready_reg;
assign dcache_bready = 1'b1;

assign dcache_wstrob = {DATA_BIT_NUM{1'b1}};

endmodule //dcache

/* verilator lint_off DECLFILENAME */
module dcache_way#(parameter DATA_LEN=32,TAG_LEN=DATA_LEN-10-$clog2(SRAM_NUM),SRAM_NUM=1,ADDR_LEN=6+$clog2(SRAM_NUM))(
    input                   clk,
    input                   rst_n,
//interface with cache to get the flag
    input  [TAG_LEN-1:0]    tag_in,
    // input                   tag_in_valid,
    // input                   valid_flag,
    input                   dirty_flag,
    output                  dirty,
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
// wire                cache_dirty[0:CACHE_LINE_NUM-1];
// wire                cache_valid[0:CACHE_LINE_NUM-1];
// wire                cache_tag_wen[0:CACHE_LINE_NUM-1];
wire [TAG_LEN+1:0]  cache_flag;
wire                cache_tag_wen;
wire                cache_CEN[0:SRAM_NUM-1];
wire [127:0]        cache_Q[0:SRAM_NUM-1];

// genvar i;
// generate
//     for(i=0;i<CACHE_LINE_NUM;i=i+1)begin: cacheline
//         FF_D #(1,0) ff_dirty (clk,rst_n,(cache_tag_wen[i]),,cache_dirty[i] );
//         FF_D #(1,0) ff_valid (clk,rst_n,cache_tag_wen[i],1'b1,cache_valid[i]);
//         FF_D_without_asyn_rst #(TAG_LEN) ff_tag (clk,cache_tag_wen[i], tag_in,cache_tag[i]);
//     end
// endgenerate

assign cache_tag_wen = (~CEN)&(~WEN);
FF_D_matrix #(TAG_LEN+2,0,ADDR_LEN) ff_flag_matrix (clk,rst_n,cache_tag_wen,A,{dirty_flag,1'b1,tag_in},cache_flag);
assign dirty = cache_flag[TAG_LEN+1];
assign valid = cache_flag[TAG_LEN];
assign tag   = cache_flag[TAG_LEN-1:0];

genvar y;
generate
    for(y=0;y<SRAM_NUM;y=y+1)begin: cache_sram

        `ifdef DCACHE_LINE_MULIT_SRAM

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

`ifdef DCACHE_LINE_MULIT_SRAM

assign Q = cache_Q[A[ADDR_LEN-1:6]];

`else

assign Q = cache_Q[0];

`endif

endmodule //dcache_way

