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
