// module shift_base #(parameter DATA_LEN = 32, SHIFT_NUM = 1)(
//     input                       LR,
//     input                       AL,
//     input                       shift_en,
//     input  [DATA_LEN-1:0]       data_in,
//     output [DATA_LEN-1:0]       data_out
// );

// localparam OVER_LEN = DATA_LEN - SHIFT_NUM;

// reg [DATA_LEN-1:0] data_out_reg;

// always @(*) begin
//     if(!LR)begin
//         if(AL)begin
//             if(shift_en)data_out_reg={{SHIFT_NUM{data_in[DATA_LEN-1]}},data_in[DATA_LEN-1:SHIFT_NUM]};
//             else data_out_reg=data_in;
//         end
//         else begin
//             if(shift_en)data_out_reg={{SHIFT_NUM{1'b0}},data_in[DATA_LEN-1:SHIFT_NUM]};
//             else data_out_reg=data_in;
//         end
//     end
//     else begin
//         if(shift_en)data_out_reg={data_in[OVER_LEN-1:0],{SHIFT_NUM{1'b0}}};
//         else data_out_reg=data_in;
//     end
// end

// assign data_out = data_out_reg;

// endmodule //shift_base
