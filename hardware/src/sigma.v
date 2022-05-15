`timescale 1ns / 1ps
`include "axi.vh"
`include "xversat.vh"
`include "xdefs.vh"

module sigma #(
      parameter ADDR_W = `ADDR_W,
      parameter DATA_W = `DATA_W,
      parameter AXI_ADDR_W = `AXI_ADDR_W
   )
   (

   input run,
   output done,

   input [DATA_W-1:0]              in0,
   output [DATA_W-1:0]             out0,
   input [31:0]     constant_0,
   input [31:0]     constant_1,
   input [31:0]     constant_2,
   input                           clk,
   input                           rst
   );

wire wor_ready;

wire [31:0] unitRdataFinal;
reg [31:0] stateRead;

// Memory access
wire [9:0] unitDone;

assign done = &unitDone;

wire [31:0] output_0_0 , output_1_0 , output_2_0 , output_3_0 , output_4_0 , output_5_0 , output_6_0 , output_7_0 , output_8_0 , unused_9 ;

// Memory mapped
circuitInput circuitInput_0 (
      .in0(in0),
      .out0(output_0_0),
      .run(run),
      .done(unitDone[0]),
      .clk(clk),
      .rst(rst)
   );

xconst xconst_1 (
      .out0(output_1_0),
      .constant_0(constant_0),
      .run(run),
      .done(unitDone[1]),
      .clk(clk),
      .rst(rst)
   );

xconst xconst_2 (
      .out0(output_2_0),
      .constant_0(constant_1),
      .run(run),
      .done(unitDone[2]),
      .clk(clk),
      .rst(rst)
   );

xconst xconst_3 (
      .out0(output_3_0),
      .constant_0(constant_2),
      .run(run),
      .done(unitDone[3]),
      .clk(clk),
      .rst(rst)
   );

RHR RHR_4 (
      .out0(output_4_0),
      .in0(output_0_0),
      .in1(output_1_0),
      .run(run),
      .done(unitDone[4]),
      .clk(clk),
      .rst(rst)
   );

RHR RHR_5 (
      .out0(output_5_0),
      .in0(output_0_0),
      .in1(output_2_0),
      .run(run),
      .done(unitDone[5]),
      .clk(clk),
      .rst(rst)
   );

SHR SHR_6 (
      .out0(output_6_0),
      .in0(output_0_0),
      .in1(output_3_0),
      .run(run),
      .done(unitDone[6]),
      .clk(clk),
      .rst(rst)
   );

XOR XOR_7 (
      .out0(output_7_0),
      .in0(output_4_0),
      .in1(output_5_0),
      .run(run),
      .done(unitDone[7]),
      .clk(clk),
      .rst(rst)
   );

XOR XOR_8 (
      .out0(output_8_0),
      .in0(output_7_0),
      .in1(output_6_0),
      .run(run),
      .done(unitDone[8]),
      .clk(clk),
      .rst(rst)
   );

circuitOutput circuitOutput_9 (
      .out0(out0),
      .in0(output_8_0),
      .run(run),
      .done(unitDone[9]),
      .clk(clk),
      .rst(rst)
   );

endmodule
