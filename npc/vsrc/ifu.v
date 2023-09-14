module ifu#(parameter DATA_LEN=32) (
    input                   clk,
    input                   rst_n,
    input                   Jump_flag,
    input  [DATA_LEN-1:0]   Jump_PC,
    input  [DATA_LEN-1:0]   inst_in,
    output [DATA_LEN-1:0]   pc_out,
    output [DATA_LEN-1:0]   inst_fetch
);

reg  [DATA_LEN-1:0] PC;
wire [DATA_LEN-1:0] PC_next;

reg [DATA_LEN-1:0] inst_reg;

assign PC_next = (Jump_flag)?Jump_PC:(PC+32'h4);
assign pc_out = PC;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        PC<=32'h80000000;
    end
    else begin
        PC<=PC_next;
    end
end

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        inst_reg<=32'h0;
    end
    else begin
        inst_reg<=inst_in;
    end
end

assign inst_fetch = inst_reg;

endmodule //ifu
