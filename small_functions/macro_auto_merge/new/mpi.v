//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Company      :   Hangzhou Chenxiao Technologies Co, Ltd.
// Project      :
// File Name    :   MPI.v
// Author       :
// E-Mail       :
// Description  :   CPU 接口处理模块;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// $Log: $
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Including files
//~~~~~~~~~~~~~~~~~~~~~~~~~~~

`include "defines.v"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Module list
//~~~~~~~~~~~~~~~~~~~~~~~~~~~
module MPI
(
	input				rst,
    input				clk100m,

    input		[15:0]	cpu_din_0,  //用不到的赋值0
    input		[15:0]	cpu_din_1,
    input       [15:0]  cpu_din_2,
    input       [15:0]  cpu_din_3,
    input       [15:0]  cpu_din_4,
    input       [15:0]  cpu_din_5,
    input       [15:0]  cpu_din_6,
    output              ts_dir,
    output              fesw,
    output              e1sw,
    output              fesw_sel,
    output              e1sw_sel
);

//--------------------------------------------------------------------------------------------------
// Internal signals
//--------------------------------------------------------------------------------------------------
wire 			cpu_cs_global;
wire   [15:0]   cpu_din_global;
wire   [15:0]   lock_word;
wire            cpu_wen_lock;


//--------------------------------------------------------------------------------------------------
// Parameters
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Coding text
//--------------------------------------------------------------------------------------------------

MPI_CORE    			U_MPI_CORE
(
	.rst				( rst ),
    .clk100m			( clk100m ),
    //	mpi
    .mpi_addr			( mpi_a[24:0] ),
    .mpi_din			( mpi_din[15:0] ),
    .mpi_dout			( mpi_dout[15:0] ),
    .mpi_cs				( mpi_cs ),
    .mpi_rdwr			( mpi_wr ),
    .mpi_en				( mpi_en ),

    .cpu_din_0			( cpu_din_0[15:0] ),
    .cpu_din_1			( cpu_din_1[15:0] ),
    .cpu_din_2			( cpu_din_2[15:0] ),

    .cpu_cs_f			( cpu_cs_f ),
    .cpu_cs_global		( cpu_cs_global )
);


MPI_REG					    U_MPI_REG
(
    .rst				    ( rst ),
    .clk100m			    ( clk100m ),

	.cpu_cs				    ( cpu_cs_global ),
    .cpu_wen			    ( cpu_wen ),
    .cpu_ren			    ( cpu_ren ),

    .e1sw                   ( e1sw ),
    .fesw_sel               ( fesw_sel ),
    .e1sw_sel               ( e1sw_sel )

);
//~~~~~~~~~~~~~~~~~~~~~~~~~~
// Tasks
//~~~~~~~~~~~~~~~~~~~~~~~~~~


//~~~~~~~~~~~~~~~~~~~~~~~~~~
// Functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~

endmodule	//	MPI