`include "./define.v"
module ifu#(parameter DATA_LEN=32) (
    input                   clk,
    input                   rst_n,
//jump flag and jump dest 
    input                   Jump_flag,
    input  [DATA_LEN-1:0]   Jump_PC,
    // output [DATA_LEN-1:0]   PC_D,
    // output [DATA_LEN-1:0]   PC_S,

//interface with axi sram
    //read addr channel
    output                  arvalid,
    input                   arready,
    output [DATA_LEN-1:0]   PC_to_sram,
    //read data channel
    input  [DATA_LEN-1:0]   inst_in,
    input                   rvalid,
    input  [2:0]            rresp,
    output                  rready,        
//interface with idu
    output                  inst_valid,  
    input                   inst_ready,
    output [DATA_LEN-1:0]   inst_fetch,
//to idu help jump work
    output [DATA_LEN-1:0]   PC_now
);

localparam AXI_IDLE         = 2'b00;
localparam AXI_WITE_ARREADY = 2'b01;
localparam AXI_WITE_RVALID  = 2'b11;
localparam AXI_GET_DATA     = 2'b10;

reg [1:0]   IFU_FSM_STATUS;

// wire  [DATA_LEN-1:0] PC_D;
reg [DATA_LEN-1:0] PC;

//reg to wire
reg    arvalid_reg;
// reg    rready_reg;
reg    inst_valid_reg;   
reg [DATA_LEN-1:0]  PC_now_reg;
reg [DATA_LEN-1:0]  PC_to_sram_reg;  
reg [DATA_LEN-1:0] inst_reg;
assign arvalid      = arvalid_reg;
// assign rready       = rready_reg;
assign inst_valid   = inst_valid_reg;
assign PC_now       = PC_now_reg;
assign PC_to_sram   = PC_to_sram_reg;
assign inst_fetch   = inst_reg;

wire [DATA_LEN-1:0] PC_S;
assign PC_S = (PC_to_sram + 32'h4);
// assign PC_now = PC;
// assign PC_D = (Jump_flag) ? Jump_PC : PC_S;
// assign PC_to_sram = (Jump_flag) ? Jump_PC : PC;

//test
reg [7:0] ifu_lfsr;
//trst

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        //init valid=0 is no problem, but
        arvalid_reg<=1'b0;
        // rready_reg<=1'b0;
        inst_valid_reg<=1'b0;
        inst_reg <= `NOP;
        PC<=`RST_PC;
        PC_now_reg<=`RST_PC;
        PC_to_sram_reg<=`RST_PC;
        IFU_FSM_STATUS<=AXI_IDLE;


        ifu_lfsr<=1;


    end
    else begin
        case (IFU_FSM_STATUS)
        //rst status, try to get the valid inst
            AXI_IDLE:begin
                // if(inst_ready)begin
                //     IFU_FSM_STATUS<=AXI_WITE_ARREADY;
                //     arvalid_reg<=1'b1;
                //     rready_reg<=1'b1;
                //     inst_valid_reg<=1'b0;
                //     PC_to_sram_reg <= (Jump_flag) ? Jump_PC : PC;
                //     // PC<=PC_D;
                // end
                // else begin
                //     IFU_FSM_STATUS<=AXI_IDLE;
                //     inst_valid_reg<=1'b0;
                // end
                IFU_FSM_STATUS<=AXI_WITE_ARREADY;
                repeat({24'h0,ifu_lfsr})begin
                    @(posedge clk);
                end
                ifu_lfsr<={(ifu_lfsr[4]^ifu_lfsr[3]^ifu_lfsr[2]^ifu_lfsr[0]),ifu_lfsr[7:1]};
                arvalid_reg<=1'b1;
                // rready_reg<=1'b1;
                PC_to_sram_reg <= (Jump_flag) ? Jump_PC : PC;
            end
        //wiating sram revice the read addr
            AXI_WITE_ARREADY:begin
                //if sram get read addr and send data at one time & this read succes
                //entry the get data status and wait inst ready of idu
                if(arready&rvalid&(rresp==3'b000))begin
                    IFU_FSM_STATUS<=AXI_GET_DATA;
                    inst_reg<=inst_in;
                    inst_valid_reg<=1'b1;
                    // rready_reg<=1'b0;
                    arvalid_reg<=1'b0;
                    PC<=PC_S;
                    PC_now_reg<=PC_to_sram;
                end
                //if sram get read addr and send data at one time & this read false
                //sram will reback the false and exu will get the mtvec to jump_pc
                else if(arready&rvalid&(rresp!=3'b000))begin
                    IFU_FSM_STATUS<=AXI_WITE_ARREADY;
                    // rready_reg<=1'b0;
                    PC_to_sram_reg <= (Jump_flag) ? Jump_PC : PC;
                end
                //if the sram only get the read addr not send the read data
                //entry the wait rvalid to wait the read data
                else if(arready)begin
                    IFU_FSM_STATUS<=AXI_WITE_RVALID;
                    arvalid_reg<=1'b0;
                    PC<=PC_S;
                    PC_now_reg<=PC_to_sram;
                end
            end
        //wait the read data is valid
            AXI_WITE_RVALID: begin
                //if the read data is valid when the resp is OK
                //entry the get data status to wait the inst ready of idu
                if(rvalid&(rresp==3'b000))begin
                    IFU_FSM_STATUS<=AXI_GET_DATA;
                    inst_reg<=inst_in;
                    inst_valid_reg<=1'b1;
                    // rready_reg<=1'b0;
                end
                //if the read data is valid when the resp is error
                //sram will reback the false and exu will get the mtvec to jump_pc
                else if(rvalid&(rresp!=3'b000))begin
                    IFU_FSM_STATUS<=AXI_WITE_ARREADY;
                    // rready_reg<=1'b0;
                    arvalid_reg<=1'b1;
                    PC_to_sram_reg <= (Jump_flag) ? Jump_PC : PC;
                end
            end
        //wait inst ready of idu
            AXI_GET_DATA:begin
                //if idu get the data succes then ifu will try to get new inst
                //else kepp wait
                if(inst_ready)begin
                    inst_valid_reg<=1'b0;
                    repeat({24'h0,ifu_lfsr})begin
                        @(posedge clk);
                    end
                    ifu_lfsr<={(ifu_lfsr[4]^ifu_lfsr[3]^ifu_lfsr[2]^ifu_lfsr[0]),ifu_lfsr[7:1]};
                    IFU_FSM_STATUS<=AXI_WITE_ARREADY;
                    arvalid_reg<=1'b1;
                    // rready_reg<=1'b1;
                    PC_to_sram_reg <= (Jump_flag) ? Jump_PC : PC;
                end
            end
        //if is a illegal status try to rst 
            default: begin
                arvalid_reg<=1'b0;
                // rready_reg<=1'b0;
                inst_valid_reg<=1'b0;
                inst_reg <= `NOP;
                PC<=`RST_PC;
                PC_now_reg<=`RST_PC;
                PC_to_sram_reg<=`RST_PC;
                IFU_FSM_STATUS<=AXI_IDLE;
            end
        endcase
    end
end

// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         PC <= 32'h80000000;
//     end
//     else begin
//         PC <= PC_D;
//     end
// end

// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         inst_reg <= `NOP;
//     end
//     else begin
//         inst_reg <= inst_in;
//     end
// end

//always can get the inst data from sram
assign rready       = 1'b1;

endmodule //ifu
