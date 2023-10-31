module fifo#(parameter DATA_LEN=32,AddR_Width=6)(
    input clk,rstn,Wready,Rready,
    input [DATA_LEN-1:0] wdata,
    output empty,
    output [DATA_LEN-1:0] rdata
);

parameter Word_Depth = 2** AddR_Width;

reg [AddR_Width:0] wdata_poi,rdata_poi;
reg [DATA_LEN-1:0] fifo_sram[0:Word_Depth-1];
assign empty = wdata_poi==rdata_poi;
assign rdata=fifo_sram[rdata_poi[AddR_Width-1:0]];
always @(posedge clk or negedge rstn) begin
    if(!rstn)begin
        wdata_poi<={(AddR_Width+1){1'b0}};
        rdata_poi<={(AddR_Width+1){1'b0}};
    end
    else begin
        case ({Wready,Rready})
            2'b11:begin
                fifo_sram[wdata_poi[AddR_Width-1:0]]<=wdata;
                wdata_poi<=wdata_poi+1'b1;
                rdata_poi<=rdata_poi+1'b1;
            end
            2'b10:begin
                fifo_sram[wdata_poi[AddR_Width-1:0]]<=wdata;
                wdata_poi<=wdata_poi+1'b1;
            end
            2'b01:begin
                rdata_poi<=rdata_poi+1'b1;
            end
            default ;
        endcase
    end
end

endmodule //fifo
