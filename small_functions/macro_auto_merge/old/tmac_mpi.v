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

    .doutb                  ( pkt_com2_rdata )
);


endmodule
