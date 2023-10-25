module FF_D_with_syn_rst#(parameter DATA_LEN=1,RST_DATA=0)(
    input 			        clk,
	input 			        rst_n,
    input                   syn_rst,
	input 			        wen,
	input  [DATA_LEN-1:0]	data_in,
	output [DATA_LEN-1:0]	data_out
);

reg [DATA_LEN-1:0] data_out_reg;

always @(posedge clk or negedge rst_n)begin
	if(!rst_n)begin
		data_out_reg <= RST_DATA;
	end
    else if(syn_rst)begin
        data_out_reg <= RST_DATA;
    end
	else if(wen)begin
		data_out_reg <= data_in ;
	end
end

assign data_out=data_out_reg;

endmodule //FF_D_with_syn_rst
