module add #(parameter DATA_LEN=32)(
    input  [DATA_LEN-1:0]   OP_A,
    input  [DATA_LEN-1:0]   OP_B,
    input                   Cin,
    output [DATA_LEN-1:0]   Sum
    // output                  Cout
);

wire [DATA_LEN-1:0] B_COM;
assign B_COM = (OP_B^{DATA_LEN{Cin}});

assign Sum = B_COM+OP_A+{31'h0,Cin};

endmodule //add
