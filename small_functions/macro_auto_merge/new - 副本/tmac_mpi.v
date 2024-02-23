//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Company      :   Hangzhou Chenxiao Technologies Co, Ltd.
// Project      :   CXT3301
// File Name    :   tmac_mpi.v
// Author       :   ASIC man
// E-Mail       :   asic@chenxiaotech.com
// Description  :   
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// $Log: TMAC_MPI.v $
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Including files
//~~~~~~~~~~~~~~~~~~~~~~~~~~~

`include "defines.v"

module TMAC_MPI
(
    input               pkt_ind1,
    input               pkt_ind2,
    input               rdi_buf0_wen,
    input               rdi_buf1_wen,
    input               rdi_buf2_wen,
    input       [7:0]   rdi_buf0_waddr,
    input       [7:0]   rdi_buf1_waddr,
    input       [7:0]   rdi_buf2_waddr,
    input               rdi_buf0_wdata,
    input               rdi_buf1_wdata,
    input               rdi_buf2_wdata,
    input       [4:0]   head_buf0_raddr,
    input       [4:0]   head_buf1_raddr,
    input       [4:0]   head_buf2_raddr,
    input               bm_fifo0_overflow,
    input               bm_fifo0_underflow,
    input               bm_fifo1_overflow,
    input               bm_fifo1_underflow,
    input               bm_fifo2_overflow,
    input               bm_fifo2_underflow,
    input               tmac_fifo0_overflow,
    input               tmac_fifo1_overflow,
    input               tmac_fifo2_overflow,
    input               tmac_fifo0_underflow,
    input               tmac_fifo1_underflow,
    input               tmac_fifo2_underflow,
    // OUTPUTS
    output  reg [15:0]  cpu_dout,
    output              cpu_int,
    output  reg [3:0]   ipg_th,
    output              pkt_com0,
    output              pkt_com1,
    output              pkt_com2,
    output              rdi_en0,
    output      [15:0]  head_buf0_rdata,
    output      [31:0]  label_buf0_rdata,
    output              rtp_en0,
    output      [9:0]   tpkt_length0,
    output              rdi_en1,
    output      [15:0]  head_buf1_rdata,
    output      [31:0]  label_buf1_rdata,
    output              rtp_en1,
    output      [9:0]   tpkt_length1,
    output              rdi_en2,
    output      [15:0]  head_buf2_rdata,
    output      [31:0]  label_buf2_rdata,
    output              rtp_en2,
    output      [9:0]   tpkt_length2
);


//------------------------------------------------------------------------------
// internal signals
//------------------------------------------------------------------------------
wire            cs0;
wire            cs1;
wire            cs2;
wire            cs3;
wire            cs4;
wire            cs5;
wire            cs6;
wire            cs7;
wire            cs8;
wire            cs9;

WRAP_TDP_RAM_256x1_D        U_PKT_COM2
(
    //  INPUTS
    .rsta                   ( rst ),
    .rstb                   ( rst ),
    .clka                   ( clk100m ),
    .clkb                   ( clk125m ),
    .ena                    ( 1'b1 ),
    .enb                    ( 1'b1 ),
    .wea                    ( pkt_com2_wen ),
    .web                    ( 1'b0 ),
    .addra                  ( cpu_addr[7:0] ),
    .addrb                  ( length_buf2_raddr[7:0] ),
    .dina                   ( cpu_din[0] ),
    .dinb                   ( 1'b0 ),
    //  OUTPUTS
    .douta                  ( pkt_com2_dout ),
    .doutb                  ( pkt_com2_rdata )
);


endmodule
