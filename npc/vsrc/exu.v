module exu#(parameter DATA_LEN=32) (
    input  [DATA_LEN-1:0]   operand1,
    input  [DATA_LEN-1:0]   operand2,    
    input                   op,
    output                  dest_wen,
    output                  Jump_flag,
    output [DATA_LEN-1:0]   dest_data,
    output [DATA_LEN-1:0]   Jump_PC
);

// outports wire
wire [DATA_LEN-1:0] 	Sum;
// wire                    Cout;

add u_add(
    .OP_A 	( operand1  ),
    .OP_B 	( operand2  ),
    .Cin  	( op        ),
    .Sum  	( Sum       )
    // .Cout 	( Cout      )
);

assign dest_data = Sum;
//以下为临时代码
assign Jump_flag = 1'b0;
assign Jump_PC = 32'h0;
assign dest_wen = 1'b1;

endmodule //exu
