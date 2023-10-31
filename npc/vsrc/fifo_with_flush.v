module fifo_with_flush#(parameter DATA_LEN=32,AddR_Width=6,FLUSH_DATA=0)(
    input clk,rstn,Wready,Rready,flush,
    input [DATA_LEN-1:0] wdata,
    output full,empty,
    output [DATA_LEN-1:0] rdata
);

parameter Word_Depth = 2** AddR_Width;
integer i;

reg [AddR_Width:0] wdata_poi,rdata_poi;
reg [DATA_LEN-1:0] fifo_sram[0:Word_Depth-1];
assign rdata=fifo_sram[rdata_poi[AddR_Width-1:0]];

// wire [Word_Depth-1:0]   wen;
// wire [Word_Depth-1:0]   flush_flag;

wire [AddR_Width:0] wdata_poi_full;
assign wdata_poi_full = wdata_poi + 1'b1;

assign full = (wdata_poi_full[AddR_Width]!=rdata_poi[AddR_Width])&(wdata_poi_full[AddR_Width-1:0]==rdata_poi[AddR_Width-1:0]);
assign empty = wdata_poi==rdata_poi;

// generate

// for(i=0;i<Word_Depth;i=i+1)begin:fifo_sram_gen

//     assign wen[i] = (wdata_poi[AddR_Width-1:0]==i)&Wready;
//     assign flush_flag[i] = flush&(~((wdata_poi[AddR_Width-1:0]==i)&Wready));

//     FF_D_with_syn_rst_without_asyn #(DATA_LEN,FLUSH_DATA)u_FF_D(
//         .clk      	( clk           ),
//         .syn_rst    ( flush_flag[i] ),
//         .wen      	( wen[i]        ),
//         .data_in  	( wdata         ),
//         .data_out 	( fifo_sram[i]  )
//     );
    
// end

// endgenerate

always @(posedge clk or negedge rstn) begin
    if(!rstn)begin
        wdata_poi<={(AddR_Width+1){1'b0}};
        rdata_poi<={(AddR_Width+1){1'b0}};
    end
    else begin
        if(flush)begin
            for(i=0;i<Word_Depth;i=i+1)begin
                fifo_sram[i]<=FLUSH_DATA;
            end
            case ({Wready,Rready})
                2'b11:begin
                    wdata_poi<=wdata_poi+1'b1;
                    rdata_poi<=rdata_poi+1'b1;
                end
                2'b10:begin
                    wdata_poi<=wdata_poi+1'b1;
                end
                2'b01:begin
                    rdata_poi<=rdata_poi+1'b1;
                end
                default ;
            endcase
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
end

endmodule //fifo_with_flush
