`include "define.v"
module add_without_Cin #(parameter DATA_LEN=32)(
    input  [DATA_LEN-1:0]   OP_A,
    input  [DATA_LEN-1:0]   OP_B,
    output [DATA_LEN-1:0]   Sum
);

wire [DATA_LEN-1:0] a,b,s; 

assign a = OP_A;
assign b = OP_B;
assign Sum = s;

`ifdef ADD_USE_ADD_BASE

wire [DATA_LEN-2:0] c;
genvar i;

generate 
    for(i=0;i<DATA_LEN;i=i+1)begin:add
        if(i==0)begin
            assign c[0] = a[0]&b[0];
            assign s[0] = a[0]^b[0];
        end
        else if(i==DATA_LEN-1)begin
            assign s[i] = a[i] ^ b[i] ^c[i-1];
        end
        else begin
            add_base u_add_base(
                .a    	( a[i]          ),
                .b    	( b[i]          ),
                .c    	( c[i-1]        ),
                .Cout 	( c[i]          ),
                .s    	( s[i]          )
            );    
        end
    end
endgenerate

`else 

assign s = a + b;

`endif

endmodule //adadd_with_Cin
