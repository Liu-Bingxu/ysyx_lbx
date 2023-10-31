module alu#(parameter DATA_LEN = 32) (
    input  [DATA_LEN-1:0]   NUM_A,
    input  [DATA_LEN-1:0]   NUM_B,
    input                   OP,
    input                   LR,
    input                   AL,
    input                   is_or,
    input                   is_xor,
    input                   is_and,
    input                   is_cmp,
    input                   is_sign,
    input                   is_shift,
    output                  out_eq,
    output                  out_ge,
    output                  out_lt,
    output                  out_neq,
    output                  out_ltu,
    output                  out_geu,
    output [DATA_LEN-1:0]   res
);

localparam SHAMT_LEN = $clog2(DATA_LEN);
localparam FILLER_LEN = DATA_LEN-1;

// arithmetic
wire [DATA_LEN-1:0] 	Sum;
wire                	Cout;
wire                    overflow;

//logic
wire [DATA_LEN-1:0]     res_and;
wire [DATA_LEN-1:0]     res_xor;
wire [DATA_LEN-1:0]     res_or;

//shift
wire [DATA_LEN-1:0]     res_shift;

//comparison
wire [DATA_LEN-1:0]     res_lt;
wire [DATA_LEN-1:0]     res_ltu;
wire [DATA_LEN-1:0]     res_cmp;
wire                    res_ge;   
wire                    res_eq;      
wire                    res_neq;   
wire                    res_geu;   

reg [DATA_LEN-1:0]      res_reg;

add_with_Cout #(DATA_LEN)add_sub(
    .OP_A 	    ( NUM_A     ),
    .OP_B 	    ( NUM_B     ),
    .Cin  	    ( OP        ),
    .Sum  	    ( Sum       ),
    .overflow   ( overflow  ),
    .Cout 	    ( Cout      )
);

assign res_and = NUM_A & NUM_B;
assign res_xor = NUM_A ^ NUM_B;
assign res_or = NUM_A  | NUM_B;

buck_shift #(DATA_LEN,SHAMT_LEN)u_buck_shift(
    .LR       	( LR                    ),
    .AL       	( AL                    ),
    .shamt    	( NUM_B[SHAMT_LEN-1:0]  ),
    .data_in  	( NUM_A                 ),
    .data_out 	( res_shift             )
);

assign res_lt  = {{FILLER_LEN{1'b0}},((overflow)?((NUM_A[DATA_LEN-1])&(~NUM_B[DATA_LEN-1])):Sum[DATA_LEN-1])};
assign res_ltu = {{FILLER_LEN{1'b0}},(~Cout)};
assign res_cmp = (is_sign)?res_lt:res_ltu;

assign res_eq  = (Sum==0)?1'b1:1'b0;
assign res_neq = (~res_eq);

assign res_ge  = ((overflow)?((~NUM_A[DATA_LEN-1])&(NUM_B[DATA_LEN-1])):(~Sum[DATA_LEN-1]));
assign res_geu = Cout;

assign out_eq  = res_eq;
assign out_ge  = res_ge;
assign out_lt  = ((overflow)?((NUM_A[DATA_LEN-1])&(~NUM_B[DATA_LEN-1])):Sum[DATA_LEN-1]);
assign out_ltu = (~Cout);
assign out_neq = res_neq;
assign out_geu = res_geu;

always @(*) begin
    case ({is_or,is_and,is_xor,is_cmp,is_shift})
        5'b10000: res_reg=res_or;
        5'b01000: res_reg=res_and;
        5'b00100: res_reg=res_xor;
        5'b00010: res_reg=res_cmp;
        5'b00001: res_reg=res_shift;
        default: res_reg=Sum;
    endcase
end

assign res = res_reg;

endmodule //alu

/* verilator lint_off DECLFILENAME */
module buck_shift#(parameter DATA_LEN = 32, SHAMT_LEN =5 ) (
    input                       LR,
    input                       AL,
    input  [SHAMT_LEN-1:0]      shamt,
    input  [DATA_LEN-1:0]       data_in,
    output [DATA_LEN-1:0]       data_out
);

wire [DATA_LEN-1:0] temp[SHAMT_LEN-2:0];

genvar i;
generate for(i = 0 ; i < SHAMT_LEN; i = i + 1) begin :shift
    if(i==0)begin
        shift_base #(DATA_LEN,2**i)u_shift_base(
            .LR       	( LR        ),
            .AL       	( AL        ),
            .shift_en 	( shamt[i]  ),
            .data_in  	( data_in   ),
            .data_out 	( temp[i]   )
        ); 
    end
    else if(i==SHAMT_LEN-1)begin
       shift_base #(DATA_LEN,2**i)u_shift_base(
        .LR       	( LR        ),
        .AL       	( AL        ),
        .shift_en 	( shamt[i]  ),
        .data_in  	( temp[i-1] ),
        .data_out 	( data_out  )
       );
    end
    else begin
        shift_base #(DATA_LEN,2**i)u_shift_base(
            .LR       	( LR        ),
            .AL       	( AL        ),
            .shift_en 	( shamt[i]  ),
            .data_in  	( temp[i-1] ),
            .data_out 	( temp[i]   )
        );
    end
end
endgenerate

endmodule //buck_shift

module shift_base #(parameter DATA_LEN = 32, SHIFT_NUM = 1)(
    input                       LR,
    input                       AL,
    input                       shift_en,
    input  [DATA_LEN-1:0]       data_in,
    output [DATA_LEN-1:0]       data_out
);

localparam OVER_LEN = DATA_LEN - SHIFT_NUM;

reg [DATA_LEN-1:0] data_out_reg;

always @(*) begin
    if(!LR)begin
        if(AL)begin
            if(shift_en)data_out_reg={{SHIFT_NUM{data_in[DATA_LEN-1]}},data_in[DATA_LEN-1:SHIFT_NUM]};
            else data_out_reg=data_in;
        end
        else begin
            if(shift_en)data_out_reg={{SHIFT_NUM{1'b0}},data_in[DATA_LEN-1:SHIFT_NUM]};
            else data_out_reg=data_in;
        end
    end
    else begin
        if(shift_en)data_out_reg={data_in[OVER_LEN-1:0],{SHIFT_NUM{1'b0}}};
        else data_out_reg=data_in;
    end
end

assign data_out = data_out_reg;

endmodule //shift_base

