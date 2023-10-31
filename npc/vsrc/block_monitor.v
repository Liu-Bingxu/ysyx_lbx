module block_monitor (
//sign to judge whether block
    //to block by RAW
    input [4:0] rs1,
    input [4:0] rs2,
    input [4:0] ID_EX_reg_rd,
    input       ID_EX_reg_dest_wen,
    input [4:0] EX_LS_reg_rd,
    input       EX_LS_reg_dest_wen,
    input [4:0] LS_WB_reg_rd,
    input       LS_WB_reg_dest_wen,
    input       rs1_valid,
    input       rs2_valid,
    //to block by jump
    input       EX_MON_reg_Jump_flag,
    //to block by load-store
    input       IF_ID_reg_inst_valid,
    input       ID_EX_reg_decode_valid,
    input       EX_LS_reg_execute_valid,
    input       LS_WB_reg_ls_valid,
    input       EX_LS_reg_load_sign_flag,
    input       EX_LS_reg_store_sign_flag,
    input       LS_MON_ls_valid,
//block control sign
    output      IF_reg_inst_enable,
    output      ID_reg_decode_enable,
    output      EX_reg_execute_enable,
    output      LS_reg_load_store_enable,
    output      IF_reg_inst_flush,
    output      ID_reg_decode_flush,
    output      src1_bypass_flag,
    output      src2_bypass_flag,
    output      MON_ID_src_block_flag
);

wire load_store_flag;
assign load_store_flag = EX_LS_reg_execute_valid&(EX_LS_reg_load_sign_flag|EX_LS_reg_store_sign_flag);

wire block_flag;
assign block_flag = (~load_store_flag)|LS_MON_ls_valid;

wire src1_block_flag;
wire src2_block_flag;

assign src1_block_flag = ((rs1==ID_EX_reg_rd)&ID_EX_reg_decode_valid&ID_EX_reg_dest_wen)|((rs1==EX_LS_reg_rd)&EX_LS_reg_execute_valid&EX_LS_reg_dest_wen);
assign src2_block_flag = ((rs2==ID_EX_reg_rd)&ID_EX_reg_decode_valid&ID_EX_reg_dest_wen)|((rs2==EX_LS_reg_rd)&EX_LS_reg_execute_valid&EX_LS_reg_dest_wen);

assign src1_bypass_flag = (rs1==LS_WB_reg_rd)&LS_WB_reg_ls_valid&LS_WB_reg_dest_wen;
assign src2_bypass_flag = (rs2==LS_WB_reg_rd)&LS_WB_reg_ls_valid&LS_WB_reg_dest_wen;

assign MON_ID_src_block_flag = (src1_block_flag&rs1_valid)|(src2_block_flag&rs2_valid);

assign EX_reg_execute_enable = block_flag;
assign ID_reg_decode_enable = (EX_reg_execute_enable|(~ID_EX_reg_decode_valid))&(~MON_ID_src_block_flag);
assign IF_reg_inst_enable = ID_reg_decode_enable|(~IF_ID_reg_inst_valid);

assign IF_reg_inst_flush = EX_LS_reg_execute_valid&EX_MON_reg_Jump_flag;
assign ID_reg_decode_flush = EX_LS_reg_execute_valid&EX_MON_reg_Jump_flag;

assign LS_reg_load_store_enable = 1'b1;

endmodule //block_monitor
