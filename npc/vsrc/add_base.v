module add_base (
    input a,
    input b,
    input c,
    output Cout,
    output s
);

assign s = a ^ b ^ c;
assign Cout = ((a & b) | ((a ^ b) & c));

endmodule //add_base

