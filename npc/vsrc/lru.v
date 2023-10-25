module lru#(parameter ADDR_LEN=2)(
    input                   clk,
    input                   using_flag,
    input  [ADDR_LEN-1:0]   using_addr,
    output [ADDR_LEN-1:0]   addr
);

wire charge_flag;

assign charge_flag = (using_addr==addr)&using_flag;

endmodule //lru
