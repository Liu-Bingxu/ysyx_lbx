module FF_D_without_wen#(parameter DATA_LEN=32,RST_DATA=0)(
	input 			        clk,
	input 			        rst_n,
	input  [DATA_LEN-1:0]	data_in,
	output [DATA_LEN-1:0]	data_out
);

reg [DATA_LEN-1:0] data_reg;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        data_reg<=RST_DATA;
    end    
    else begin
        data_reg<=data_in;
    end
end

assign data_out = data_reg;

endmodule //FF_D_without_wen
