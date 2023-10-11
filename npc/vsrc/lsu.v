module lsu #(parameter DATA_LEN=32,DATA_BIT_NUM=4)(
    input                       clk,
    input                       rst_n,
//interface with exu 
    output                      ls_valid,
    input                       ls_ready,
    input  [4:0]                load_sign,
    input  [3:0]                store_sign,
    output [DATA_LEN-1:0]       load_data,
//interface to GPR 
    output                      load_wen,
//interface with sram
    //the write addr channel 
    output                      awvalid,
    input                       awready,
    output [DATA_LEN-1:0]       waddr,
    input  [DATA_LEN-1:0]       store_addr,
    //the write data channel
    output                      wvalid,
    input                       wready,
    output [DATA_BIT_NUM-1:0]   wstrob,
    output [DATA_LEN-1:0]       wdata,
    input  [DATA_LEN-1:0]       store_data,
    //the write resp channel
    input                       bvalid,
    output                      bready,
    input   [2:0]               bresp, 
    //the read addr channel 
    output                      arvalid,
    input                       arready,
    output [DATA_LEN-1:0]       raddr,
    input  [DATA_LEN-1:0] 	    addr_load,
    //the read data channel 
    input                       rvalid,
    input  [2:0]                rresp,
    output                      rready,
    input  [DATA_LEN-1:0]       pre_data
);
                //load part 
//temp
assign rready = 1'b1;
//temp
//sign decode 
wire is_load_byte;
wire is_load_half;
wire is_load_word;
wire is_load_sign;
wire is_load;
assign is_load = load_sign[0];
assign is_load_sign = (~load_sign[1]);
assign is_load_byte = load_sign[2];
assign is_load_half = load_sign[3];
assign is_load_word = load_sign[4];
//load mask decode 
wire [1:0] load_mask;
assign load_mask = addr_load[1:0];
//get offest load data 
wire [DATA_LEN-1:0] pre_move_data_0;
wire [DATA_LEN-1:0] pre_move_data_1;
wire [DATA_LEN-1:0] pre_move_data_2;
wire [DATA_LEN-1:0] pre_move_data_3;
assign pre_move_data_0 = pre_data;
assign pre_move_data_1 = { 8'h0,pre_move_data_0[DATA_LEN-1:8] };
assign pre_move_data_2 = {16'h0,pre_move_data_0[DATA_LEN-1:16]};
assign pre_move_data_3 = {24'h0,pre_move_data_0[DATA_LEN-1:24]};
reg  [DATA_LEN-1:0] pre_data_reg;
always @(*) begin
    if(load_mask==2'b00)begin
        pre_data_reg=pre_move_data_0;
    end
    else if(load_mask==2'b01)begin
        pre_data_reg=pre_move_data_1;
    end
    else if(load_mask==2'b10)begin
        pre_data_reg=pre_move_data_2;
    end
    else begin
        pre_data_reg=pre_move_data_3;
    end
end
//get the truly load data 
memory_load_rv32 #(DATA_LEN)u_memory_load_rv32(
    .pre_data 	( pre_data_reg  ),
    .is_byte  	( is_load_byte  ),
    .is_half  	( is_load_half  ),
    .is_word    ( is_load_word  ),
    .is_sign  	( is_load_sign  ),
    .data     	( load_data     )
);

//AXI load FSM
localparam AXI_LOAD_IDLE            = 0;
localparam AXI_LOAD_WAIT_ARREADY    = 1;
localparam AXI_LOAD_WAIT_RVALID     = 2;
localparam AXI_LOAD_WAIT_LS_READY   = 3;
reg arvalid_reg;
reg load_vaild_reg;
reg load_wen_reg;
reg [1:0]   axi_load_fsm;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        arvalid_reg<=1'b0;
        load_vaild_reg<=1'b0;
        load_wen_reg<=1'b0;
        axi_load_fsm<=AXI_LOAD_IDLE;
    end
    else begin
        case (axi_load_fsm)
            AXI_LOAD_IDLE: begin
                if(is_load)begin
                    arvalid_reg<=1'b1;
                    axi_load_fsm<=AXI_LOAD_WAIT_ARREADY;
                end
            end
            AXI_LOAD_WAIT_ARREADY:begin
                if(arready&rvalid&rready&(rresp==3'b000))begin
                    load_vaild_reg<=1'b1;
                    arvalid_reg<=1'b0;
                    load_wen_reg<=1'b1;
                    axi_load_fsm<=AXI_LOAD_WAIT_LS_READY;
                end
                else if(arready&rvalid&rready&(rresp!=3'b000))begin
                    arvalid_reg<=1'b0;
                    axi_load_fsm<=AXI_LOAD_IDLE;
                end
                else if(arready)begin
                    arvalid_reg<=1'b0;
                    axi_load_fsm<=AXI_LOAD_WAIT_RVALID;
                end
            end
            AXI_LOAD_WAIT_RVALID:begin
                if(rvalid&rready&(rresp==3'b000))begin
                    load_vaild_reg<=1'b1;
                    load_wen_reg<=1'b1;
                    axi_load_fsm<=AXI_LOAD_WAIT_LS_READY;
                end
                else if(rvalid&rready&(rresp!=3'b000))begin
                    axi_load_fsm<=AXI_LOAD_IDLE;
                end
            end
            AXI_LOAD_WAIT_LS_READY:begin
                if(ls_ready)begin
                    load_vaild_reg<=1'b0;
                    load_wen_reg<=1'b0;
                    axi_load_fsm<=AXI_LOAD_IDLE;
                end
                else begin
                    load_wen_reg<=1'b0;
                end
            end
            default: begin
                arvalid_reg<=1'b0;
                load_vaild_reg<=1'b0;
                load_wen_reg<=1'b0;
                axi_load_fsm<=AXI_LOAD_IDLE;
            end
        endcase
    end
end
assign arvalid  = arvalid_reg;
assign  raddr   = addr_load;
assign load_wen = load_wen_reg;

                    //store part 
//sign decode 
wire is_store;
wire is_store_byte;
wire is_store_half;
wire is_store_word;
assign is_store         = store_sign[0];
assign is_store_byte    = store_sign[1];
assign is_store_half    = store_sign[2];
assign is_store_word    = store_sign[3];
//store_mask decode 
wire [1:0] store_mask;
assign store_mask = store_addr[1:0];
//get wstrb by control sign 
reg [3:0] byte_wstrob,half_wstrob,word_wstrob;
always @(*) begin
    case (store_mask)
        2'b00: byte_wstrob=4'b0001;
        2'b01: byte_wstrob=4'b0010;
        2'b10: byte_wstrob=4'b0100;
        2'b11: byte_wstrob=4'b1000;
        default: byte_wstrob=4'b0000;
    endcase
end
always @(*) begin
    case (store_mask)
        2'b00: half_wstrob=4'b0011;
        2'b10: half_wstrob=4'b1100;
        default: half_wstrob=4'b0000;
    endcase
end
always @(*) begin
    case (store_mask)
        2'b00: word_wstrob=4'b1111;
        default: word_wstrob=4'b0000;
    endcase
end
assign wstrob = (is_store_byte)?byte_wstrob:((is_store_half)?half_wstrob:((is_store_word)?word_wstrob:{DATA_BIT_NUM{1'b0}}));

//AXI store FSM 
localparam AXI_STORE_IDLE                   = 0;
localparam AXI_STORE_WITE_AEREADY_WREADY    = 1;
localparam AXI_STORE_WAIT_AWREADY           = 2;
localparam AXI_STORE_WAIT_WREADY            = 3;
localparam AXI_STORE_WAIT_BVALID            = 4;
localparam AXI_STORE_WAIT_LS_READY          = 5;
reg store_valid_reg;
reg awvalid_reg;
reg wvalid_reg;
reg [2:0] axi_store_fsm;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        store_valid_reg<=1'b0;
        awvalid_reg<=1'b0;
        wvalid_reg<=1'b0;
        axi_store_fsm<=AXI_STORE_IDLE;
    end
    else begin
        case (axi_store_fsm)
            AXI_STORE_IDLE:begin
                if(is_store)begin
                    awvalid_reg<=1'b1;
                    wvalid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WITE_AEREADY_WREADY;
                end
            end 
            AXI_STORE_WITE_AEREADY_WREADY:begin
                if(awready&wready&bvalid&bready&(bresp==3'b000))begin
                    awvalid_reg<=1'b0;
                    wvalid_reg<=1'b0;
                    store_valid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                end
                else if(awready&wready&bvalid&bready&(bresp!=3'b000))begin
                    awvalid_reg<=1'b0;
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
                else if(awready&wready)begin
                    awvalid_reg<=1'b0;
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_BVALID;
                end
                else if(awready)begin
                    awvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_WREADY;
                end
                else if(wready)begin
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_AWREADY;
                end
            end
            AXI_STORE_WAIT_AWREADY:begin
                if(awready&bvalid&bready&(bresp==3'b000))begin
                    awvalid_reg<=1'b0;
                    store_valid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                end
                else if(awready&bvalid&bready&(bresp!=3'b000))begin
                    awvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
                else if(awready)begin
                    awvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_BVALID;
                end
            end
            AXI_STORE_WAIT_WREADY:begin
                if(wready&bvalid&bready&(bresp==3'b000))begin
                    wvalid_reg<=1'b0;
                    store_valid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                end
                else if(wready&bvalid&bready&(bresp!=3'b000))begin
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
                else if(wready)begin
                    wvalid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_WAIT_BVALID;
                end
            end
            AXI_STORE_WAIT_BVALID:begin
                if(bvalid&bready&(bresp==3'b000))begin
                    store_valid_reg<=1'b1;
                    axi_store_fsm<=AXI_STORE_WAIT_LS_READY;
                end
                else if(bvalid&bready&(bresp!=3'b000))begin
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
            end
            AXI_STORE_WAIT_LS_READY:begin
                if(ls_ready)begin
                    store_valid_reg<=1'b0;
                    axi_store_fsm<=AXI_STORE_IDLE;
                end
            end
            default: begin
                store_valid_reg<=1'b0;
                awvalid_reg<=1'b0;
                wvalid_reg<=1'b0;
                axi_store_fsm<=AXI_STORE_IDLE;
            end
        endcase
    end
end
assign awvalid  = awvalid_reg;
assign waddr    = store_addr;
assign wvalid   = wvalid_reg;
assign wdata    = store_data;
assign bready   = 1'b1;

//last
assign ls_valid = load_vaild_reg|store_valid_reg;

endmodule //lsu
