import "DPI-C" function void icache_access();
import "DPI-C" function void icache_hit();
module icache_rv32#(parameter WAY_NUM=4,SRAM_NUM=2,DATA_LEN=32)(
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
reg                     ifu_rvalid_reg;
reg  [31:0]             ifu_rdata_reg;
reg                     icache_arvalid_reg;
reg  [DATA_LEN-1:0]     icache_raddr_reg;
wire [127:0]            data;

reg                     bypass_flag;
reg                     icache_read_error;
wire [127:0]            data_sel;

reg  [2:0]              icache_fsm_status;
reg  [1:0]              icache_read_cnt;

wire [WAY_ADDR_LEN-1:0] rand_way;
reg  [WAY_ADDR_LEN-1:0] hit_way;

genvar i;
generate 
    for(i=0;i<WAY_NUM;i=i+1)begin: cache_sram  
        icache_line #(.DATA_LEN(DATA_LEN),.SRAM_NUM(SRAM_NUM))u_icache_way(
            .clk          	( clk                       ),
            .rst_n        	( rst_n                     ),
            .tag_in       	( tag_in                    ),
            .addr_valid     ( ifu_addr_handshake_flag   ),
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

assign offset       = ifu_raddr[3:2];
assign A            = ifu_raddr[(3+ADDR_LEN):4];
assign target_tag   = ifu_raddr[31:(4+ADDR_LEN)];
assign D            = data;
assign tag_in       = target_tag;

FF_D_without_asyn_rst #(128) write_data_to_cache(clk,icache_data_handshake_flag,{icache_rdata,data[127:32]},data);

rand_lfsr_8_bit #(WAY_ADDR_LEN)u_rand_lfsr_8_bit_get_rand_way_num(
    .clk   	( clk           ),
    .rst_n 	( rst_n         ),
    .out   	( rand_way      )
);

//fsm logic 
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        icache_fsm_status<=ICACHE_IDLE;
        icache_read_cnt<=2'b0;
        ifu_rvalid_reg<=1'b0;
        WEN_reg<=1'b1;
        CEN_reg<={WAY_NUM{1'b1}};
        icache_arvalid_reg<=1'b0;
        bypass_flag<=1'b0;
        icache_read_error<=1'b0;
        ifu_arready_reg<=1'b1;
    end
    else begin
        case (icache_fsm_status)
            ICACHE_IDLE:begin
                if(ifu_addr_handshake_flag)begin
                    icache_fsm_status<=ICACHE_CMP_TAG;
                    ifu_arready_reg<=1'b0;
                    icache_access();
                end
            end
            ICACHE_CMP_TAG:begin
                if(res==0)begin
                    icache_fsm_status<=ICACHE_READ_ADDR;
                    icache_arvalid_reg<=1'b1;
                    icache_raddr_reg<={ifu_raddr[DATA_LEN-1:4],icache_read_cnt,2'h0};
                    icache_read_cnt<=icache_read_cnt+1'b1;
                end
                else begin
                    icache_fsm_status<=ICACHE_GET_DATA;
                    bypass_flag<=1'b0;
                    CEN_reg<=~res;
                    WEN_reg<=1'b1;
                    icache_hit();
                end
            end
            ICACHE_READ_ADDR:begin
                if(icache_addr_handshake_flag)begin
                    icache_fsm_status<=ICACHE_READ_DATA;
                    icache_arvalid_reg<=1'b0;
                end
            end
            ICACHE_READ_DATA:begin
                if(icache_data_handshake_flag&(icache_rresp==3'b000)&(~icache_read_error))begin
                    if(icache_read_cnt==2'b00)begin
                        icache_fsm_status<=ICACHE_GET_DATA;
                        bypass_flag<=1'b1;
                        WEN_reg<=1'b0;
                        CEN_reg[rand_way]<=1'b0;
                    end
                    else begin
                        icache_fsm_status<=ICACHE_READ_ADDR;
                        icache_arvalid_reg<=1'b1;
                        icache_read_cnt<=icache_read_cnt+1'b1;
                        icache_raddr_reg<={ifu_raddr[DATA_LEN-1:4],icache_read_cnt,2'h0};
                    end
                end
                else if(icache_data_handshake_flag)begin
                    if(icache_read_cnt==2'b00)begin
                        icache_fsm_status<=ICACHE_IDLE;
                        ifu_arready_reg<=1'b1;
                        icache_read_error<=1'b0;
                    end
                    else begin
                        icache_fsm_status<=ICACHE_READ_ADDR;
                        icache_arvalid_reg<=1'b1;
                        icache_read_cnt<=icache_read_cnt+1'b1;
                        icache_raddr_reg<={ifu_raddr[DATA_LEN-1:4],icache_read_cnt,2'h0};
                        icache_read_error<=1'b1;
                    end
                end
            end
            ICACHE_GET_DATA:begin
                if(ifu_data_handshake_flag)begin
                    icache_fsm_status<=ICACHE_IDLE;
                    ifu_arready_reg<=1'b1;
                    ifu_rvalid_reg<=1'b0;
                    bypass_flag<=1'b0;
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
        4'b0001: begin
            hit_way=0;
        end
        4'b0010:begin
            hit_way=1;
        end
        4'b0100:begin
            hit_way=2;
        end
        4'b1000:begin
            hit_way=3;
        end
        default: begin
            hit_way=0;
        end
    endcase
end

assign data_sel = (bypass_flag)?data:Q[hit_way];
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
assign ifu_addr_handshake_flag = ifu_arvalid&ifu_arready;
assign ifu_data_handshake_flag = ifu_rvalid &ifu_rready;
assign icache_arvalid=icache_arvalid_reg;
assign icache_raddr=icache_raddr_reg;
assign icache_addr_handshake_flag = icache_arvalid&icache_arready;
assign icache_data_handshake_flag = icache_rvalid &icache_rready;

assign BWEN = 0;
assign icache_rready = 1'b1;
assign ifu_rresp = 3'b000;

endmodule //icache_rv32
