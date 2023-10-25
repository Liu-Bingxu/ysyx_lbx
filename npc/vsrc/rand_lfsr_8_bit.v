module rand_lfsr_8_bit#(parameter USING_LEN=2)(
    input                   clk,
    input                   rst_n,
    output  [USING_LEN-1:0] out
);

wire [7:0] 	data_out;

wire feedback;

assign feedback = data_out[4]^data_out[3]^data_out[2]^data_out[0];

FF_D_without_wen #(8,1)lfsr_8bit(
    .clk      	( clk                       ),
    .rst_n    	( rst_n                     ),
    .data_in  	( {feedback,data_out[7:1]}  ),
    .data_out 	( data_out                  )
);

assign out = data_out[USING_LEN-1:0];

endmodule //rand_lfsr_8_bit
