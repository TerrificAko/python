`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/05/17 14:06:20
// Design Name: 
// Module Name: dac_data
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module dac_data(
    input dac_axi_clk,
    input clk249M6,
    input RESETn,
    input s00_axis_tready,
    input s01_axis_tready,
    input s02_axis_tready,
    input s03_axis_tready,
    input s10_axis_tready,
    input s11_axis_tready,
    input s12_axis_tready,
    input s13_axis_tready,
    
    output   reg    [127:0]    dac_128point,
    output   reg               dac_128point_valid,


    output reg [127:0] dac_axis_tdata,
    output reg dac_axis_tvalid
    );
    
 wire    clk249m6;   
 reg    [3:0]   test_time;
 wire           axis_tready;

reg     [9:0]   cnt128;
wire    [127:0] dac_1m;
wire            dac_1m_valid;
wire    [127:0] dac_2m;
wire            dac_2m_valid;
 
 assign axis_tready = s00_axis_tready & s01_axis_tready & s02_axis_tready & s03_axis_tready 
                    & s10_axis_tready & s11_axis_tready & s12_axis_tready & s13_axis_tready;
 
 always@(posedge dac_axi_clk or negedge RESETn)begin
    if(!RESETn)
        test_time   <= 'd0;
    else if (axis_tready) 
        test_time   <=  test_time + 'd1;
    else
        ;
 end
 
always@(posedge dac_axi_clk or negedge RESETn)begin
    if(!RESETn)begin
        dac_axis_tdata  <=  'd0;
        dac_axis_tvalid <=  'd0;
    end
    else if(test_time[3:0] == 4'b1111) begin
        dac_axis_tdata  <=  {16'h7fff,16'h7fff,16'h7fff,16'h7fff,16'h7fff,16'h7fff,16'h7fff,16'h7fff};
        dac_axis_tvalid <=  'd1;
    end
    else if(test_time[3:0] == 4'b0111) begin
//        dac_axis_tdata  <=  {16'hffff,16'hffff,16'hffff,16'hffff,16'hffff,16'hffff,16'hffff,16'hffff};
        dac_axis_tdata  <=  {16'h8000,16'h8000,16'h8000,16'h8000,16'h8000,16'h8000,16'h8000,16'h8000};
        dac_axis_tvalid <=  'd1;
    end
    else begin
        dac_axis_tdata  <=  'd0;
        dac_axis_tvalid <=  'd0;
    end
end


always@(posedge dac_axi_clk or negedge RESETn)begin
    if(!RESETn)  
        cnt128   <=  'd0;
    else
        cnt128   <=  cnt128   +   'd1;
end


dac_1m                      u_dac_1m(
    .clk                    (dac_axi_clk),
    .rst_n                  (RESETn),
    .axis_tready            (axis_tready),
    .dac_data               (dac_1m),
    .dac_data_valid         (dac_1m_valid)
);
dac_2m                      u_dac_2m(
    .clk                    (dac_axi_clk),
    .rst_n                  (RESETn),
    .axis_tready            (axis_tready),
    .dac_data               (dac_2m),
    .dac_data_valid         (dac_2m_valid)
);
always@(*)begin
    if(cnt128[9:8] == 2'b00)begin
        dac_128point        =   dac_1m;
        dac_128point_valid  =   dac_1m_valid;
    end
    else if (cnt128[9:8] == 2'b10)begin
        dac_128point        =   dac_2m;
        dac_128point_valid  =   dac_2m_valid;
    end
    else begin
        dac_128point        =   'd0;
        dac_128point_valid  =   'd0;
    end
end
//assign  dac_128point        =   cnt128[6]   ?   dac_1m  :   dac_2m;
//assign  dac_128point_valid  =   cnt128[6]   ?   dac_1m_valid    :   dac_2m_valid;

        
        
 
endmodule
