module memory_load_rv32#(parameter DATA_LEN = 32)(
    input   [DATA_LEN-1:0]  pre_data,
    input                   is_byte,
    input                   is_half,
    input                   is_word,
    input                   is_sign,
    output  [DATA_LEN-1:0]  data
);

localparam FILLER_LEN_BYTE = DATA_LEN-8 ;
localparam FILLER_LEN_HALF = DATA_LEN-16;

wire [DATA_LEN-1:0] data_byte;
wire [DATA_LEN-1:0] data_half;
wire [DATA_LEN-1:0] data_word;
wire [DATA_LEN-1:0] data_signed_byte;
wire [DATA_LEN-1:0] data_signed_half;
wire [DATA_LEN-1:0] data_unsigned_byte;
wire [DATA_LEN-1:0] data_unsigned_half;

assign data_signed_byte = {{FILLER_LEN_BYTE{pre_data[7]} },pre_data[7:0] };
assign data_signed_half = {{FILLER_LEN_HALF{pre_data[15]}},pre_data[15:0]};

assign data_unsigned_byte = {{FILLER_LEN_BYTE{1'b0}},pre_data[7:0] };
assign data_unsigned_half = {{FILLER_LEN_HALF{1'b0}},pre_data[15:0]};

assign data_word = pre_data;
assign data_byte = (is_sign)?data_signed_byte:data_unsigned_byte;
assign data_half = (is_sign)?data_signed_half:data_unsigned_half;

assign data = (is_byte)?data_byte:((is_half)?data_half:((is_word)?data_word:32'h0));

endmodule //memory_load
