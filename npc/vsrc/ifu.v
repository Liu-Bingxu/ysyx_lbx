`include "./define.v"
module ifu#(parameter DATA_LEN=32) (
    input                   clk,
    input                   rst_n,
    input                   Jump_flag,
    input  [DATA_LEN-1:0]   Jump_PC,
    input  [DATA_LEN-1:0]   inst_in,
    output [DATA_LEN-1:0]   PC_S,
    output [DATA_LEN-1:0]   PC_D,
    output [DATA_LEN-1:0]   PC_now,
    output [DATA_LEN-1:0]   inst_fetch

    // input                   rvalid,
    // output                  rready,
    // input                   awready,      
    // output                  awvalid,        

);

reg  [DATA_LEN-1:0] PC;
reg [DATA_LEN-1:0] inst_reg;

assign PC_S = (PC + 32'h4);
assign PC_D = (Jump_flag) ? Jump_PC : PC_S;
assign PC_now = PC;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        PC <= 32'h7ffffffc;
    end
    else begin
        PC <= PC_D;
    end
end

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        inst_reg <= `NOP;
    end
    else begin
        inst_reg <= inst_in;
    end
end

assign inst_fetch = inst_reg;

endmodule //ifu
