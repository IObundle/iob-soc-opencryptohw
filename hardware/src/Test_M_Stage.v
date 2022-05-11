`timescale 1ns / 1ps
`include "axi.vh"
`include "xversat.vh"
`include "xdefs.vh"

module Test_M_Stage #(
      parameter ADDR_W = `ADDR_W,
      parameter DATA_W = `DATA_W,
      parameter AXI_ADDR_W = `AXI_ADDR_W
   )
   (

   input run,
   output done,

   input [31:0]     constant_0,
   input [31:0]     constant_1,
   input [31:0]     constant_2,
   input [31:0]     constant_3,
   input [31:0]     constant_4,
   input [31:0]     constant_5,
   output [31:0]    currentValue_0,
   output [31:0]    currentValue_1,
   output [31:0]    currentValue_2,
   output [31:0]    currentValue_3,
   output [31:0]    currentValue_4,
   input [31:0]                    delay0,
   input [31:0]                    delay1,
   input [31:0]                    delay2,
   input [31:0]                    delay3,
   input [31:0]                    delay4,
   input                           valid,
   input [ADDR_W-1:0]              addr,
   input [DATA_W/8-1:0]            wstrb,
   input [DATA_W-1:0]              wdata,
   output                          ready,
   output reg [DATA_W-1:0]         rdata,
   input                           clk,
   input                           rst
   );

wire wor_ready;

wire [31:0] unitRdataFinal;
reg [31:0] stateRead;

// Memory access
wire we = (|wstrb);
assign rdata = unitRdataFinal;
assign ready = wor_ready;
assign wor_ready = (|unitReady);
reg [4:0] memoryMappedEnable;
wire[4:0] unitReady;
wire [31:0] unitRData[4:0];

wire [31:0] rdata_0 , rdata_1 , rdata_2 , rdata_3 , rdata_4 ;

assign unitRdataFinal = (unitRData[0] |unitRData[1] |unitRData[2] |unitRData[3] |unitRData[4] );
wire [5:0] unitDone;

assign done = &unitDone;

wire [31:0] output_0_0, output_1_0, output_2_0, output_3_0, output_5_0;

// Memory mapped
always @*
begin
   memoryMappedEnable = {5{1'b0}};
   if(valid)
   begin
   if(addr[2:0] == 0)
         memoryMappedEnable[0] = 1'b1;
   if(addr[2:0] == 1)
         memoryMappedEnable[1] = 1'b1;
   if(addr[2:0] == 2)
         memoryMappedEnable[2] = 1'b1;
   if(addr[2:0] == 3)
         memoryMappedEnable[3] = 1'b1;
   if(addr[2:0] == 4)
         memoryMappedEnable[4] = 1'b1;
   end
end
xreg xreg_0 (
      .out0(output_0_0),
      .delay0(delay0),
      .currentValue(currentValue_0),
      .valid(memoryMappedEnable[0]),
      .wstrb(wstrb),
      .addr(addr[1:0]),
      .rdata(unitRData[0]),
      .ready(unitReady[0]),
      .wdata(wdata),
      .run(run),
      .done(unitDone[0]),
      .clk(clk),
      .rst(rst)
   );

xreg xreg_1 (
      .out0(output_1_0),
      .delay0(delay1),
      .currentValue(currentValue_1),
      .valid(memoryMappedEnable[1]),
      .wstrb(wstrb),
      .addr(addr[1:0]),
      .rdata(unitRData[1]),
      .ready(unitReady[1]),
      .wdata(wdata),
      .run(run),
      .done(unitDone[1]),
      .clk(clk),
      .rst(rst)
   );

xreg xreg_2 (
      .out0(output_2_0),
      .delay0(delay2),
      .currentValue(currentValue_2),
      .valid(memoryMappedEnable[2]),
      .wstrb(wstrb),
      .addr(addr[1:0]),
      .rdata(unitRData[2]),
      .ready(unitReady[2]),
      .wdata(wdata),
      .run(run),
      .done(unitDone[2]),
      .clk(clk),
      .rst(rst)
   );

xreg xreg_3 (
      .out0(output_3_0),
      .delay0(delay3),
      .currentValue(currentValue_3),
      .valid(memoryMappedEnable[3]),
      .wstrb(wstrb),
      .addr(addr[1:0]),
      .rdata(unitRData[3]),
      .ready(unitReady[3]),
      .wdata(wdata),
      .run(run),
      .done(unitDone[3]),
      .clk(clk),
      .rst(rst)
   );

xreg xreg_4 (
      .in0(output_5_0),
      .delay0(delay4),
      .currentValue(currentValue_4),
      .valid(memoryMappedEnable[4]),
      .wstrb(wstrb),
      .addr(addr[1:0]),
      .rdata(unitRData[4]),
      .ready(unitReady[4]),
      .wdata(wdata),
      .run(run),
      .done(unitDone[4]),
      .clk(clk),
      .rst(rst)
   );

M_Stage M_Stage_5 (
      .out0(output_5_0),
      .in0(output_0_0),
      .in1(output_1_0),
      .in2(output_2_0),
      .in3(output_3_0),
      .constant_0(constant_0),
      .constant_1(constant_1),
      .constant_2(constant_2),
      .constant_3(constant_3),
      .constant_4(constant_4),
      .constant_5(constant_5),
      .run(run),
      .done(unitDone[5]),
      .clk(clk),
      .rst(rst)
   );

endmodule
