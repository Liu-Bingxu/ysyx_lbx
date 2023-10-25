`include "define.v"
module add_with_Cout #(parameter DATA_LEN=32)(
    input  [DATA_LEN-1:0]   OP_A,
    input  [DATA_LEN-1:0]   OP_B,
    input                   Cin,
    output [DATA_LEN-1:0]   Sum,
    output                  overflow,
    output                  Cout
);

wire [DATA_LEN-1:0] a,b,s; 

assign a = OP_A;
assign b = OP_B ^ {DATA_LEN{Cin}};
assign Sum = s;

`ifdef ADD_USE_ADD_BASE

wire [DATA_LEN-2:0] c;
genvar i;

generate 
    for(i=0;i<DATA_LEN;i=i+1)begin:add
        if(i==0)begin
            add_base u_add_base(
                .a    	( a[i]          ),
                .b    	( b[i]          ),
                .c    	( Cin           ),
                .Cout 	( c[i]          ),
                .s    	( s[i]          )
            );  
        end
        else if(i==DATA_LEN-1)begin
            add_base u_add_base(
                .a    	( a[i]          ),
                .b    	( b[i]          ),
                .c    	( c[i-1]        ),
                .Cout 	( Cout          ),
                .s    	( s[i]          )
            ); 
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
/* verilator lint_off WIDTHEXPAND */
assign {Cout,s} = a + b +Cin;

`endif

assign overflow = ((a[DATA_LEN-1]==b[DATA_LEN-1])&(a[DATA_LEN-1]!=Sum[DATA_LEN-1]));

endmodule //adadd_with_Cin

