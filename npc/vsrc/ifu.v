`include "./define.v"
module ifu#(parameter DATA_LEN=32) (
    input                   clk,
    input                   rst_n,
//jump flag and jump dest 
    // input                   EX_LS_reg_execute_valid,
    // input                   EX_IF_reg_Jump_flag,
    input                   IF_reg_inst_flush,
    input  [DATA_LEN-1:0]   EX_IF_reg_Jump_PC,
    // output [DATA_LEN-1:0]   PC_D,
    // output [DATA_LEN-1:0]   PC_S,

//interface with axi sram
    //read addr channel
    output                  ifu_arvalid,
    input                   ifu_arready,
    output [DATA_LEN-1:0]   ifu_raddr,
    //read data channel
    input  [DATA_LEN-1:0]   ifu_rdata,
    input                   ifu_rvalid,
    input  [2:0]            ifu_rresp,
    output                  ifu_rready,        
//interface with idu
    output                  IF_ID_reg_inst_valid,  
    input                   IF_reg_inst_enable,
    output [DATA_LEN-1:0]   IF_ID_reg_inst,
//to idu help jump work
    output [DATA_LEN-1:0]   IF_ID_reg_PC
);

localparam AXI_ADDR_IDLE                = 2'b00;
localparam AXI_ADDR_WITE_arready        = 2'b01;
localparam AXI_ADDR_WITE_enable         = 2'b11;
// localparam AXI_ADDR_FULL_WITE_arready   = 2'b10;
// localparam AXI_DATA_WITE_rvalid  = 1'b0;
// localparam AXI_DATA_WITE_enable  = 1'b1;

reg [1:0] IFU_FSM_STATUS_ADDR;
// reg       IFU_FSM_STATUS_DATA;

// reg                  IF_IF2_reg_inst_valid;
// reg [DATA_LEN-1:0]   IF_IF2_reg_PC;

// wire addr_ready;
wire data_ready;
wire pc_full;
wire pc_empty;
wire inst_empty;
wire [31:0] inst_rdata;
wire [DATA_LEN:0] valid_pc_rdata;

wire  read_addr_handshake_flag;
wire  read_data_handshake_flag;
wire  read_data_handshake_flag_success;
// wire  read_data_handshake_flag_false;

assign read_addr_handshake_flag = ifu_arvalid&ifu_arready;
assign read_data_handshake_flag = ifu_rvalid&ifu_rready;
assign read_data_handshake_flag_success = read_data_handshake_flag&(ifu_rresp==3'h0);
// assign read_data_handshake_flag_false   = read_data_handshake_flag&(ifu_rresp!=3'h0);

// wire IF_reg_inst_addr_enable;
wire IF_reg_inst_data_enable;

// assign IF_reg_inst_addr_enable = IF_reg_inst_enable|(~pc_full);
assign IF_reg_inst_data_enable = IF_reg_inst_enable;

// wire  [DATA_LEN-1:0] PC_D;
// reg [DATA_LEN-1:0] PC;

//reg to wire
reg    ifu_arvalid_reg;
// reg    rready_reg;
// reg    IF_ID_reg_inst_valid_reg;   
// reg [DATA_LEN-1:0]  IF_ID_reg_PC_now_reg;
reg [DATA_LEN-1:0]  PC_to_sram_reg;
// reg [DATA_LEN-1:0] IF_ID_reg_inst_reg;
assign ifu_arvalid      = ifu_arvalid_reg;
// assign ifu_rready       = rready_reg;
// assign IF_ID_reg_inst_valid   = IF_ID_reg_inst_valid_reg;
// assign IF_ID_reg_PC_now       = IF_ID_reg_PC_now_reg;
// assign IF_ID_reg_inst_fetch   = IF_ID_reg_inst_reg;
assign ifu_raddr   = PC_to_sram_reg;

wire [DATA_LEN-1:0] PC_S;
assign PC_S = (PC_to_sram_reg + 32'h4);
// assign PC_now = PC;
// assign PC_D = (Jump_flag) ? Jump_PC : PC_S;
// assign PC_to_sram = (Jump_flag) ? Jump_PC : PC;

//test
// reg [7:0] ifu_lfsr;
//trst

// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         //init valid=0 is no problem, but
//         ifu_arvalid_reg<=1'b0;
//         // rready_reg<=1'b0;
//         // inst_valid_reg<=1'b0;
//         // inst_reg <= `NOP;
//         // PC<=`RST_PC;
//         // PC_now_reg<=`RST_PC;
//         PC_to_sram_reg<=`RST_PC;
//         IFU_FSM_STATUS<=AXI_IDLE;
//         // ifu_lfsr<=1;
//     end
//     else begin
//         case (IFU_FSM_STATUS)
//         //rst status, try to get the valid inst
//             AXI_IDLE:begin
//                 // if(inst_ready)begin
//                 //     IFU_FSM_STATUS<=AXI_WITE_ifu_arready;
//                 //     ifu_aifu_rvalid_reg<=1'b1;
//                 //     rready_reg<=1'b1;
//                 //     inst_valid_reg<=1'b0;
//                 //     PC_to_sram_reg <= (Jump_flag) ? Jump_PC : PC;
//                 //     // PC<=PC_D;
//                 // end
//                 // else begin
//                 //     IFU_FSM_STATUS<=AXI_IDLE;
//                 //     inst_valid_reg<=1'b0;
//                 // end
//                 IFU_FSM_STATUS<=AXI_WITE_arready;
//                 // repeat({24'h0,ifu_lfsr})begin
//                     // @(posedge clk);
//                 // end
//                 // ifu_lfsr<={(ifu_lfsr[4]^ifu_lfsr[3]^ifu_lfsr[2]^ifu_lfsr[0]),ifu_lfsr[7:1]};
//                 ifu_arvalid_reg<=1'b1;
//                 // rready_reg<=1'b1;
//                 // PC_to_sram_reg <= (EX_IF_reg_Jump_flag) ? EX_IF_reg_Jump_PC : PC_S;
//             end
//         //wiating sram revice the read addr
//             AXI_WITE_arready:begin
//                 //if sram get read addr and send data at one time & this read succes
//                 //entry the get data status and wait inst ready of idu
//                 // if(read_addr_handshake_flag&ifu_rvalid&(ifu_rresp==3'b000))begin
//                 //     IFU_FSM_STATUS<=AXI_GET_DATA;
//                 //     inst_reg<=inst_in;
//                 //     inst_valid_reg<=1'b1;
//                 //     // rready_reg<=1'b0;
//                 //     ifu_aifu_rvalid_reg<=1'b0;
//                 //     PC<=PC_S;
//                 //     PC_now_reg<=PC_to_sram;
//                 // end
//                 //if sram get read addr and send data at one time & this read false
//                 //sram will reback the false and exu will get the mtvec to jump_pc
//                 // else if(ifu_arready&ifu_rvalid&(ifu_rresp!=3'b000))begin
//                 //     IFU_FSM_STATUS<=AXI_WITE_ifu_arready;
//                 //     // rready_reg<=1'b0;
//                 //     PC_to_sram_reg <= (EX_IF_reg_Jump_flag) ? EX_IF_reg_Jump_PC : PC;
//                 // end
//                 //if the sram only get the read addr not send the read data
//                 //entry the wait ifu_rvalid to wait the read data
//                 // else if(ifu_arready)begin
//                 //     IFU_FSM_STATUS<=AXI_WITE_ifu_rvalid;
//                 //     ifu_aifu_rvalid_reg<=1'b0;
//                 //     PC<=PC_S;
//                 //     PC_now_reg<=PC_to_sram;
//                 // end
//                 if(read_addr_handshake_flag)begin
//                     IFU_FSM_STATUS<=AXI_WITE_rvalid;
//                     ifu_arvalid_reg<=1'b0;
//                     // PC<=PC_S;
//                     // PC_now_reg<=PC_to_sram;
//                 end
//             end
//         //wait the read data is valid
//             AXI_WITE_rvalid: begin
//                 //if the read data is valid when the resp is OK
//                 //entry the get data status to wait the inst ready of idu
//                 if(read_data_handshake_flag_success)begin
//                     IFU_FSM_STATUS<=AXI_GET_DATA;
//                     // inst_reg<=inst_in;
//                     // inst_valid_reg<=1'b1;
//                     // rready_reg<=1'b0;
//                 end
//                 //if the read data is valid when the resp is error
//                 //sram will reback the false and exu will get the mtvec to jump_pc
//                 else if(read_data_handshake_flag_false)begin
//                     IFU_FSM_STATUS<=AXI_WITE_arready;
//                     // rready_reg<=1'b0;
//                     ifu_arvalid_reg<=1'b1;
//                     // PC_to_sram_reg <= (EX_IF_reg_Jump_flag) ? EX_IF_reg_Jump_PC : PC_S;
//                 end
//             end
//         //wait inst ready of idu
//             AXI_GET_DATA:begin
//                 //if idu get the data succes then ifu will try to get new inst
//                 //else kepp wait
//                 if(IF_reg_inst_enable)begin
//                     // inst_valid_reg<=1'b0;
//                     // repeat({24'h0,ifu_lfsr})begin
//                         // @(posedge clk);
//                     // end
//                     // ifu_lfsr<={(ifu_lfsr[4]^ifu_lfsr[3]^ifu_lfsr[2]^ifu_lfsr[0]),ifu_lfsr[7:1]};
//                     IFU_FSM_STATUS<=AXI_WITE_arready;
//                     ifu_arvalid_reg<=1'b1;
//                     // rready_reg<=1'b1;
//                     PC_to_sram_reg <= (EX_IF_reg_Jump_flag) ? EX_IF_reg_Jump_PC : PC_S;
//                 end
//             end
//         //if is a illegal status try to rst 
//             default: begin
//                 ifu_arvalid_reg<=1'b0;
//                 // rready_reg<=1'b0;
//                 // inst_valid_reg<=1'b0;
//                 // inst_reg <= `NOP;
//                 // PC<=`RST_PC;
//                 // PC_now_reg<=`RST_PC;
//                 PC_to_sram_reg<=`RST_PC;
//                 IFU_FSM_STATUS<=AXI_IDLE;
//             end
//         endcase
//     end
// end

// reg addr_ready_reg;
// reg data_ready_reg;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        IFU_FSM_STATUS_ADDR<=AXI_ADDR_IDLE;
        ifu_arvalid_reg<=1'b0;
        PC_to_sram_reg<=`RST_PC;
        // addr_ready_reg<=1'b0;
    end
    else begin
        case (IFU_FSM_STATUS_ADDR)
            AXI_ADDR_IDLE: begin
                // if(IF_reg_inst_addr_enable)begin
                //     IFU_FSM_STATUS_ADDR<=AXI_WITE_arready;
                //     ifu_arvalid_reg<=1'b1;
                //     IF_IF2_reg_inst_valid<=1'b1;
                //     IF_IF2_reg_PC<=PC_to_sram_reg;
                //     PC_to_sram_reg <= (EX_IF_reg_Jump_flag) ? EX_IF_reg_Jump_PC : PC_S;
                // end
                IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                ifu_arvalid_reg<=1'b1;
                // addr_ready_reg<=1'b1;
            end
            AXI_ADDR_WITE_arready:begin
                // if(read_addr_handshake_flag&IF_reg_inst_flush&(~pc_full))begin
                //     IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                //     PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                //     // addr_ready_reg<=1'b1;
                // end
                // else if(read_addr_handshake_flag&IF_reg_inst_flush)begin
                //     if(data_ready)begin
                //         IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                //         PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                //     end
                //     else begin
                //         IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_enable;
                //         ifu_arvalid_reg<=1'b0;
                //         PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                //     end
                // end
                // else if(IF_reg_inst_flush)begin
                //     IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                //     PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                //     // addr_ready_reg<=1'b0;
                // end
                if(IF_reg_inst_flush)begin
                    IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                    PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                    // addr_ready_reg<=1'b0;
                end
                else if(read_addr_handshake_flag&(~pc_full))begin
                    IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                    PC_to_sram_reg <= PC_S;
                    // addr_ready_reg<=1'b1;
                end
                else if(read_addr_handshake_flag)begin
                    if(data_ready)begin
                        IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                        PC_to_sram_reg <= PC_S;
                    end
                    else begin
                        IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_enable;
                        ifu_arvalid_reg<=1'b0;
                        PC_to_sram_reg <= PC_S;
                    end
                end
                // else if(IF_reg_inst_addr_enable)begin
                //     IF_IF2_reg_inst_valid<=1'b0;
                // end
            end
            AXI_ADDR_WITE_enable:begin
                // if(data_ready&IF_reg_inst_flush)begin
                //     IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                //     ifu_arvalid_reg<=1'b1;
                //     PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                //     // PC_to_sram_reg <= PC_S;
                //     // addr_ready_reg<=1'b1;
                //     // IF_IF2_reg_inst_valid<=1'b1;
                //     // IF_IF2_reg_PC<=PC_to_sram_reg;
                //     // PC_to_sram_reg <= (EX_IF_reg_Jump_flag) ? EX_IF_reg_Jump_PC : PC_S;
                // end
                if(data_ready)begin
                    IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                    ifu_arvalid_reg<=1'b1;
                end
                else if(IF_reg_inst_flush)begin
                    IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
                    PC_to_sram_reg <= EX_IF_reg_Jump_PC;
                    ifu_arvalid_reg<=1'b1;
                end
            end
            // AXI_ADDR_FULL_WITE_arready:begin
            //     if(read_addr_handshake_flag&IF_reg_inst_flush&(~pc_full))begin
            //         IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
            //         PC_to_sram_reg <= EX_IF_reg_Jump_PC;
            //         // addr_ready_reg<=1'b1;
            //     end
            //     else if(read_addr_handshake_flag&IF_reg_inst_flush)begin
            //         if(data_ready)begin
            //             IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
            //             PC_to_sram_reg <= EX_IF_reg_Jump_PC;
            //         end
            //         else begin
            //             IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_enable;
            //             ifu_arvalid_reg<=1'b0;
            //             PC_to_sram_reg <= EX_IF_reg_Jump_PC;
            //         end
            //     end
            //     else if(IF_reg_inst_flush)begin
            //         IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
            //         PC_to_sram_reg <= EX_IF_reg_Jump_PC;
            //         // addr_ready_reg<=1'b0;
            //     end
            //     else if(read_addr_handshake_flag&(~pc_full))begin
            //         IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
            //         PC_to_sram_reg <= PC_S;
            //         // addr_ready_reg<=1'b1;
            //     end
            //     else if(read_addr_handshake_flag)begin
            //         if(data_ready)begin
            //             IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_arready;
            //             PC_to_sram_reg <= PC_S;
            //         end
            //         else begin
            //             IFU_FSM_STATUS_ADDR<=AXI_ADDR_WITE_enable;
            //             ifu_arvalid_reg<=1'b0;
            //             PC_to_sram_reg <= PC_S;
            //         end
            //     end
            //     // if(read_addr_handshake_flag&)
            //     // IFU_FSM_STATUS_ADDR<=AXI_ADDR_IDLE;
            //     // ifu_arvalid_reg<=1'b0;
            //     // PC_to_sram_reg<=`RST_PC;
            // end
            default:begin
                IFU_FSM_STATUS_ADDR<=AXI_ADDR_IDLE;
                ifu_arvalid_reg<=1'b0;
                PC_to_sram_reg<=`RST_PC;
            end
        endcase
    end
end

// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_rvalid;
//         // IF_ID_reg_inst_valid_reg<=1'b0;
//     end
//     else begin
//         case (IFU_FSM_STATUS_DATA)
//             AXI_DATA_WITE_rvalid: begin
//                 if(read_data_handshake_flag_success&IF_reg_inst_flush)begin
//                     IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_rvalid;
//                     // IF_ID_reg_inst_valid_reg<=1'b0;
//                 end
//                 // else if(IF_reg_inst_flush)begin
//                 //     IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_rvalid;
//                 //     IF_ID_reg_inst_valid_reg<=1'b0;
//                 // end
//                 else if(read_data_handshake_flag_success&IF_reg_inst_data_enable)begin
//                     IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_rvalid;
//                     // IF_ID_reg_inst_valid_reg<=IF_IF2_reg_inst_valid;
//                     // IF_ID_reg_PC_reg<=IF_IF2_reg_PC;
//                     // IF_ID_reg_inst_reg<=ifu_rdata;
//                 end
//                 else if(read_data_handshake_flag_success)begin
//                     IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_enable;
//                     // IF_ID_reg_inst_reg<=ifu_rdata;
//                 end
//                 else if(IF_reg_inst_data_enable)begin
//                     IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_rvalid;
//                     // IF_ID_reg_inst_valid_reg<=1'b0;
//                 end
//             end
//             AXI_DATA_WITE_enable :begin
//                 if(IF_reg_inst_data_enable)begin
//                     IFU_FSM_STATUS_DATA<=AXI_DATA_WITE_rvalid;
//                     // IF_ID_reg_inst_valid_reg<=IF_IF2_reg_inst_valid;
//                     // IF_ID_reg_PC_reg<=IF_IF2_reg_PC;
//                 end
//             end
//         endcase
//     end
// end

reg                 IF_ID_reg_inst_valid_reg;
reg [31:0]          IF_ID_reg_inst_reg;
reg [DATA_LEN-1:0]  IF_ID_reg_PC_reg;

always @(posedge clk or negedge rst_n) begin
    if(!rst_n)begin
        IF_ID_reg_inst_valid_reg<=1'b0;
        // data_ready_reg<=1'b0;
    end
    else begin
        if(IF_reg_inst_flush)begin
            IF_ID_reg_inst_valid_reg<=1'b0;
            // data_ready_reg<=1'b0;
        end
        else if(IF_reg_inst_data_enable)begin    
            if(inst_empty|pc_empty)begin
                IF_ID_reg_inst_valid_reg<=1'b0;
                // data_ready_reg<=1'b0;
            end
            else begin
                IF_ID_reg_inst_valid_reg<=valid_pc_rdata[DATA_LEN];
                IF_ID_reg_PC_reg<=valid_pc_rdata[DATA_LEN-1:0];
                IF_ID_reg_inst_reg<=inst_rdata;
                // data_ready_reg<=1'b1;
            end
        end
        // else begin
        //     data_ready_reg<=1'b0;
        // end
    end
end

assign IF_ID_reg_inst_valid = IF_ID_reg_inst_valid_reg;
assign IF_ID_reg_inst = IF_ID_reg_inst_reg;
assign IF_ID_reg_PC = IF_ID_reg_PC_reg;

// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         PC <= 32'h80000000;
//     end
//     else begin
//         PC <= PC_D;
//     end
// end

// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         inst_reg <= `NOP;
//     end
//     else begin
//         inst_reg <= inst_in;
//     end
// end

//always can get the inst data from sram
assign ifu_rready       = 1'b1;

// FF_D #(1,0) u_inst_valid	(clk,rst_n,IF_reg_inst_enable, read_data_handshake_flag_success,IF_ID_reg_inst_valid );
// reg IF_ID_reg_inst_valid_reg;
// always @(posedge clk or negedge rst_n) begin
//     if(!rst_n)begin
//         IF_ID_reg_inst_valid_reg<=1'b0;
//     end
//     else if(read_data_handshake_flag_success&IF_reg_inst_enable)begin
//         IF_ID_reg_inst_valid_reg<=1'b1;
//     end
//     else if(IF_reg_inst_enable)begin
//         IF_ID_reg_inst_valid_reg<=1'b0;
//     end
// end
// assign IF_ID_reg_inst_valid = IF_ID_reg_inst_valid_reg;
// FF_D_without_asyn_rst #(DATA_LEN) u_inst_fetch	(clk,read_data_handshake_flag_success&IF_reg_inst_enable, ifu_rdata,IF_ID_reg_inst   );
// FF_D_without_asyn_rst #(DATA_LEN) u_PC_now	    (clk,read_data_handshake_flag_success&IF_reg_inst_enable, PC_to_sram_reg,IF_ID_reg_PC);

fifo_with_flush #(DATA_LEN+1,2)valid_pc_fifo_with_flush(
    .clk    	( clk                       ),
    .rstn   	( rst_n                     ),
    .Wready 	( read_addr_handshake_flag  ),
    .Rready 	( data_ready                ),
    .flush  	( IF_reg_inst_flush         ),
    .wdata  	( {1'b1,PC_to_sram_reg}     ),
    .full       ( pc_full                   ),
    .empty      ( pc_empty                  ),
    .rdata  	( valid_pc_rdata            )
);

// fifo_with_flush #(1,2,0)valid_fifo_with_flush(
//     .clk    	( clk               ),
//     .rstn   	( rst_n             ),
//     .Wready 	( addr_ready        ),
//     .Rready 	( data_ready        ),
//     .flush  	( IF_reg_inst_flush ),
//     .wdata  	( 1'b1              ),
//     .rdata  	( valid_rdata       )
// );

// fifo #(DATA_LEN,2)pc_fifo(
//     .clk    	( clk           ),
//     .rstn   	( rst_n         ),
//     .Wready 	( addr_ready    ),
//     .Rready 	( data_ready    ),
//     .wdata  	( PC_to_sram_reg),
//     .full       ( pc_full       ),
//     .empty      ( pc_empty      ),
//     .rdata  	( pc_rdata      )
// );  
wire inst_fifo_wready;
fifo #(32,2)inst_fifo(
    .clk    	( clk               ),
    .rstn   	( rst_n             ),
    .Wready 	( inst_fifo_wready  ),
    .Rready 	( data_ready        ),
    .flush  	( IF_reg_inst_flush ),
    .wdata  	( ifu_rdata         ),
    .empty      ( inst_empty        ),
    .rdata  	( inst_rdata        )
);  
assign inst_fifo_wready = read_data_handshake_flag_success&(~pc_empty);
// assign addr_ready = addr_ready_reg;
// assign data_ready = data_ready_reg;
assign data_ready = (~IF_reg_inst_flush)&IF_reg_inst_data_enable&(~inst_empty)&(~pc_empty);

endmodule //ifu
