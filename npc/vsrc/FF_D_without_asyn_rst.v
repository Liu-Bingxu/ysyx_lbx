module FF_D_without_asyn_rst#(parameter DATA_LEN=1)(
    input 			        clk,
	input 			        wen,
	input  [DATA_LEN-1:0]	data_in,
	output [DATA_LEN-1:0]	data_out
);

reg [DATA_LEN-1:0] data_out_reg;

always @(posedge clk)begin
    if(wen)begin
		data_out_reg <= data_in;
	end
end

assign data_out=data_out_reg;

endmodule //FF_D_without_asyn_rst
