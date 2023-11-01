import "DPI-C" function void Log_mem_read(int unsigned addr);
import "DPI-C" function void Log_mem_wirte(int unsigned addr, int unsigned data,byte wmask);
import "DPI-C" function void pmem_read(
  input int unsigned raddr, output int unsigned rdata);
import "DPI-C" function void pmem_write(
  input int unsigned waddr, input int unsigned wdata, input byte wmask);
module sram#(parameter DATA_LEN=32, STORB_LEN=4 ,ADDR_LEN=32) (
    input                   clk,
    input                   rst_n,
//write addr channel 
    input                   awvalid,
    output                  awready,
    input   [ADDR_LEN-1:0]  waddr,
//write data channel
    input                   wvalid,
    output                  wready,
    input   [DATA_LEN-1:0]  wdata,
    input   [STORB_LEN-1:0] wstrob,
//write resp channel 
    output                  bvalid,
    input                   bready,
    output  [2:0]           bresp,
//read addr channel 
    input                   arvalid,
    output                  arready,
    input   [ADDR_LEN-1:0]  raddr,
//read data channel
    output                  rvalid,
    input                   rready,
    output  [DATA_LEN-1:0]  rdata,
    output  [2:0]           rresp
);
// //write fsm 
// // localparam AXI_WRITE_IDLE = 0;
// // localparam AXI_WRITE_ = ;
// reg [7:0] write_delay;
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         write_delay<=0;
//     end
// end

// //read fsm 
// reg [7:0] read_delay,read_cnt;
// reg arready_reg,rvalid_reg;
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         read_delay<=0;
//         read_cnt<=0;
//         arready_reg<=1'b0;
//         rvalid_reg<=1'b0;
//     end
//     else begin
//         if(rvalid&rready)begin
//             rvalid_reg<=1'b0;
//         end
//         if(arvalid&arready)begin
//             arready_reg<=1'b0;
//         end
//         else if(arvalid&(read_delay==0))begin
//             arready_reg<=1'b0;
//             rvalid_reg<=1'b0;
//             pmem_read(raddr,rdata);
//         end
//         else if(arvalid&)
//     end
// end
// assign rresp = 3'b000;
// assign arready = arready_reg;
// assign rvalid = rvalid_reg;

reg bvalid_reg,rvalid_reg;
assign bvalid = bvalid_reg;
//write
reg [DATA_LEN-1:0]  write_data_buf;
reg [ADDR_LEN-1:0]  write_addr_buf;
reg write_addr_buf_flag,write_data_buf_flag;
// reg [7:0] write_lsfr;
wire [ADDR_LEN-1:0] write_addr;
assign write_addr =(write_addr_buf_flag)?write_addr_buf:waddr;
wire [DATA_LEN-1:0] write_data;
assign write_data = (write_data_buf_flag)?write_data_buf:wdata;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        bvalid_reg<=1'b0;
        write_addr_buf<=0;
        write_data_buf<=0;
        // write_lsfr<=1;
        write_addr_buf_flag<=1'b0;
        write_data_buf_flag<=1'b0;
    end
    else begin
        if(awvalid&wvalid)begin
            if(~((bvalid)&(~bready)))begin
                // repeat({24'b0,write_lsfr})begin
                    // @(posedge clk);
                // end
                // write_lsfr<={(write_lsfr[4]^write_lsfr[3]^write_lsfr[2]^write_lsfr[0]),write_lsfr[7:1]};
                bvalid_reg<=1'b1;
                pmem_write(write_addr,write_data,{4'h0,wstrob});
                if(write_data_buf_flag)begin
                    write_data_buf<=wdata;
                end
                if(write_addr_buf_flag)begin
                    write_addr_buf<=waddr;
                end
            end
        end
        else if(awvalid)begin
            if(write_data_buf_flag)begin
                if(bvalid&(~bready))begin
                    write_addr_buf_flag<=1'b1;
                    write_addr_buf<=waddr;
                end
                else begin
                    bvalid_reg<=1'b1;
                    pmem_write(write_addr,write_data,{4'h0,wstrob});
                    Log_mem_wirte(write_addr,write_data,{4'h0,wstrob});
                    write_data_buf_flag<=1'b0;
                end
            end
            else begin
                if(bready)begin
                    bvalid_reg<=1'b0;
                end
                write_addr_buf_flag<=1'b1;
                write_addr_buf<=waddr;
            end
        end
        else if(wvalid)begin
            if(write_addr_buf_flag)begin
                if(bvalid&(~bready))begin
                    write_data_buf_flag<=1'b1;
                    write_data_buf<=rdata;
                end
                else begin
                    bvalid_reg<=1'b1;
                    pmem_write(write_addr,write_data,{4'h0,wstrob});
                    Log_mem_wirte(write_addr,write_data,{4'h0,wstrob});
                    write_addr_buf_flag<=1'b0;
                end
            end
            else begin
                if(bready)begin
                    bvalid_reg<=1'b0;
                end
                write_data_buf_flag<=1'b1;
                write_data_buf<=wdata;
            end
        end
        else if(bvalid&bready)begin
            bvalid_reg<=1'b0;
        end
    end
end
assign bresp    = 3'b000;
assign awready  = 1'b1;
assign wready   = 1'b1;
// reg [7:0] read_lsfr;
//read
assign rvalid = rvalid_reg;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        rvalid_reg<=1'b0;
        // read_lsfr<=1;
    end
    else begin
        if(rvalid&rready&arvalid)begin
            rvalid_reg<=1'b1;
            pmem_read(raddr,rdata);
            Log_mem_read(raddr);
        end
        else if(rvalid&(~rready)&arvalid)begin
            rvalid_reg<=1'b1; 
        end
        else if(arvalid)begin
            
            // repeat({24'h0,8'd30})begin
            //     @(posedge clk);
            // end
            // read_lsfr<={(read_lsfr[4]^read_lsfr[3]^read_lsfr[2]^read_lsfr[0]),read_lsfr[7:1]};
            rvalid_reg<=1'b1;
            pmem_read(raddr,rdata);
            Log_mem_read(raddr);
        end
        else if(rvalid&rready)begin
            rvalid_reg<=1'b0;
        end
    end
end
assign arready  = 1'b1;
assign rresp    = 3'b000;

endmodule //sram
