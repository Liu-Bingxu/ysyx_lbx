`include "./define.v"
`ifdef DPI_C
import "DPI-C" function void halt(byte code);

// `ifndef HAS_AXI_BUS

// import "DPI-C" function void Log_mem_read(int unsigned addr);
// import "DPI-C" function void Log_mem_wirte(int unsigned addr, int unsigned data,byte wmask);
// import "DPI-C" function void pmem_read(
//   input int unsigned raddr, output int unsigned rdata);
// import "DPI-C" function void pmem_write(
//   input int unsigned waddr, input int unsigned wdata, input byte wmask);

// `endif

module monitor(
    input                   clk,
    // input                   is_load,
    // input  [3:0]            store_sign,
    // input  [DATA_LEN-1:0]   store_addr,
    // input  [DATA_LEN-1:0]   store_data,
    // input  [DATA_LEN-1:0] 	addr_load,
    // input  [DATA_LEN-1:0]   PC_out,
    // output [DATA_LEN-1:0]   inst_in,
    // output [DATA_LEN-1:0]   pre_data,
    // input                   ebreak,
    input                  LS_WB_reg_ls_valid,
    input                  LS_WB_reg_ebreak
);
// wire [DATA_LEN-1:0] pre_move_data_0;
// wire [DATA_LEN-1:0] pre_move_data_1;
// wire [DATA_LEN-1:0] pre_move_data_2;
// wire [DATA_LEN-1:0] pre_move_data_3;
// reg  [DATA_LEN-1:0] pre_data_reg;
// wire [1:0] load_mask;
// assign load_mask = addr_load[1:0];

// wire [1:0] store_mask;
// assign store_mask = store_addr[1:0];

always @(posedge clk ) begin
    if(LS_WB_reg_ls_valid&LS_WB_reg_ebreak)halt(0);
end

// always @(negedge clk) begin
//     pmem_read(PC_out,inst_in);
//     // $display("PC read");
//     // Log_mem_read(PC_out);
// end

// always @(clk,addr_load) begin
//     pmem_read(addr_load,pre_move_data_0);
// end

// always @(negedge clk ) begin
//     if(is_load)begin
//         // $display("error");
//         pmem_read(addr_load,pre_move_data_0);
//         Log_mem_read(addr_load); 
//     end
// end

// always @(negedge clk) begin
//     if(store_sign[0])begin
//         if(store_sign[1]==1)begin
//             pmem_write(store_addr,store_data,1<<store_mask);
//             Log_mem_wirte(store_addr,store_data,1<<store_mask);
//         end
//         else if(store_sign[2]==1)begin
//            pmem_write(store_addr,store_data,3<<store_mask); 
//            Log_mem_wirte(store_addr,store_data,3<<store_mask);
//         end
//         else if(store_sign[3]==1)begin
//             pmem_write(store_addr,store_data,'hf);
//             Log_mem_wirte(store_addr,store_data,'hf);
//         end
//         else begin
//             halt(-1,0);
//         end
//     end
// end

// assign pre_move_data_1 = { 8'h0,pre_move_data_0[DATA_LEN-1:8] };
// assign pre_move_data_2 = {16'h0,pre_move_data_0[DATA_LEN-1:16]};
// assign pre_move_data_3 = {24'h0,pre_move_data_0[DATA_LEN-1:24]};

// always @(*) begin
//     if(load_mask==2'b00)begin
//         pre_data_reg=pre_move_data_0;
//     end
//     else if(load_mask==2'b01)begin
//         pre_data_reg=pre_move_data_1;
//     end
//     else if(load_mask==2'b10)begin
//         pre_data_reg=pre_move_data_2;
//     end
//     else begin
//         pre_data_reg=pre_move_data_3;
//     end
// end

// assign pre_data = pre_data_reg;

endmodule //monitor

`endif
