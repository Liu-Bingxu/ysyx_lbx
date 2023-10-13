module axi_arbiter#(parameter ARBITARTE_NUM=2)(
    input                       clk,
    input                       rst_n,
    input   [ARBITARTE_NUM-1:0] avalid,
    input   [ARBITARTE_NUM-1:0] valid,
    input   [ARBITARTE_NUM-1:0] ready,
    output  [ARBITARTE_NUM-1:0] sel
// //read channel
//     input   [ARBITARTE_NUM-1:0] arvalid,
//     input   [ARBITARTE_NUM-1:0] rvalid,
//     input   [ARBITARTE_NUM-1:0] rready,
//     output  [ARBITARTE_NUM-1:0] read_sel,
// //write channel
//     input   [ARBITARTE_NUM-1:0] awvalid,
//     input   [ARBITARTE_NUM-1:0] bvalid,
//     input   [ARBITARTE_NUM-1:0] bready,
//     output  [ARBITARTE_NUM-1:0] write_sel
);

// wire [ARBITARTE_NUM-1:0] read_sel_cascade;
// wire [ARBITARTE_NUM-2:0] read_sel_cascade_mask;
// genvar i;
// generate for(i = 0 ; i < ARBITARTE_NUM ; i = i + 1 )begin: read_sel_cascade_module
//     if(i==0)begin
//         assign read_sel_cascade[i] = arvalid[i];
//         assign read_sel_cascade_mask[i] = (~arvalid[i]);
//     end
//     else if(i==(ARBITARTE_NUM-1))begin
//         assign read_sel_cascade[i] = arvalid[i]&read_sel_cascade_mask[i-1];
//     end
//     else begin
//         assign read_sel_cascade[i] = arvalid[i]&read_sel_cascade_mask[i-1];
//         assign read_sel_cascade_mask[i] = (~arvalid[i])&read_sel_cascade_mask[i];
//     end
// end
// endgenerate
// wire [ARBITARTE_NUM-1:0]    read_result;
// assign read_result = rvalid&rready;
// reg [ARBITARTE_NUM-1:0] read_sel_reg;
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         read_sel_reg<=0;
//     end
//     else begin
//         if(read_sel!=0)begin
//             if((read_sel&read_result)!=0)begin
//                 read_sel_reg<=read_sel_cascade;
//             end
//         end
//         else begin
//             if(arvalid!=0)begin
//                 read_sel_reg<=read_sel_cascade;
//             end
//         end
//     end
// end
// assign read_sel = read_sel_reg;

// wire [ARBITARTE_NUM-1:0] write_sel_cascade;
// wire [ARBITARTE_NUM-2:0] write_sel_cascade_mask;
// genvar x;
// generate for(x = 0 ; x < ARBITARTE_NUM ; x = x + 1 )begin: write_sel_cascade_module
//     if(x==0)begin
//         assign write_sel_cascade[x] = awvalid[x];
//         assign write_sel_cascade_mask[x] = (~awvalid[x]);
//     end
//     else if(x==(ARBITARTE_NUM-1))begin
//         assign write_sel_cascade[x] = awvalid[x]&write_sel_cascade_mask[x-1];
//     end
//     else begin
//         assign write_sel_cascade[x] = awvalid[x]&write_sel_cascade_mask[x-1];
//         assign write_sel_cascade_mask[x] = (~awvalid[x])&write_sel_cascade_mask[x];
//     end
// end
// endgenerate
// wire [ARBITARTE_NUM-1:0]    write_result;
// assign write_result = bvalid&bready;
// reg [ARBITARTE_NUM-1:0] write_sel_reg;
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         write_sel_reg<=0;
//     end
//     else begin
//         if(write_sel!=0)begin
//             if((write_sel&write_result)!=0)begin
//                 write_sel_reg<=write_sel_cascade;
//             end
//         end
//         else begin
//             if(awvalid!=0)begin
//                 write_sel_reg<=write_sel_cascade;
//             end
//         end
//     end
// end
// assign write_sel = write_sel_reg;

wire [ARBITARTE_NUM-1:0] sel_cascade;
wire [ARBITARTE_NUM-2:0] sel_cascade_mask;
genvar i;
generate for(i = 0 ; i < ARBITARTE_NUM ; i = i + 1 )begin: sel_cascade_module
    if(i==0)begin
        assign sel_cascade[i] = avalid[i];
        assign sel_cascade_mask[i] = (~avalid[i]);
    end
    else if(i==(ARBITARTE_NUM-1))begin
        assign sel_cascade[i] = avalid[i]&sel_cascade_mask[i-1];
    end
    else begin
        assign sel_cascade[i] = avalid[i]&sel_cascade_mask[i-1];
        assign sel_cascade_mask[i] = (~avalid[i])&sel_cascade_mask[i];
    end
end
endgenerate
wire [ARBITARTE_NUM-1:0]    result;
assign result = valid&ready;
reg [ARBITARTE_NUM-1:0] sel_reg;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        sel_reg<=0;
    end
    else begin
        if(sel!=0)begin
            if((sel&result)!=0)begin
                sel_reg<=sel_cascade;
            end
        end
        else begin
            if(avalid!=0)begin
                sel_reg<=sel_cascade;
            end
        end
    end
end
assign sel = sel_reg;

endmodule //axi_arbiter
