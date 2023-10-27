module FF_D_matrix#(parameter DATA_LEN=1,RST_DATA=0,ADDR_LEN=1)(
	input 			        clk,
	input 			        rst_n,
	input 			        wen,
    input  [ADDR_LEN-1:0]   addr,
	input  [DATA_LEN-1:0]	data_in,
	output [DATA_LEN-1:0]	data_out
);

localparam ADDR_MAX = 2**ADDR_LEN;

reg [DATA_LEN-1:0] data_out_matrix[0:ADDR_MAX-1];
// reg [DATA_LEN-1:0] data_out_reg;
integer i;

always @(posedge clk or negedge rst_n)begin
	if(!rst_n)begin
        for(i=0;i<ADDR_MAX;i=i+1)begin
		    data_out_matrix[i] <= RST_DATA;
        end
	end
	else if(wen)begin
		data_out_matrix[addr] <= data_in ;
	end
end

// always @(posedge clk) begin
//     data_out_reg<=data_out_matrix[addr];
// end

assign data_out=data_out_matrix[addr];

endmodule //FF_D_matrix
