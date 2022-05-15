`timescale 1ns / 1ps
`include "axi.vh"
`include "xversat.vh"
`include "xdefs.vh"

module M_Stage #(
      parameter ADDR_W = `ADDR_W,
      parameter DATA_W = `DATA_W,
      parameter AXI_ADDR_W = `AXI_ADDR_W
   )
   (

   input run,
   output done,

   input [DATA_W-1:0]              in0,
   input [DATA_W-1:0]              in1,
   input [DATA_W-1:0]              in2,
   input [DATA_W-1:0]              in3,
   output [DATA_W-1:0]             out0,
   input [31:0]     constant_0,
   input [31:0]     constant_1,
   input [31:0]     constant_2,
   input [31:0]     constant_3,
   input [31:0]     constant_4,
   input [31:0]     constant_5,
   input                           clk,
   input                           rst
   );

wire wor_ready;

wire [31:0] unitRdataFinal;
reg [31:0] stateRead;

// Memory access
wire [10:0] unitDone;

assign done = &unitDone;

wire [31:0] output_0_0 , output_1_0 , output_2_0 , output_3_0 , output_4_0 , output_5_0 , output_6_0 , output_7_0 , output_8_0 , output_9_0 , unused_10 ;

// Memory mapped
circuitInput circuitInput_0 (
      .in0(in0),
      .out0(output_0_0),
      .run(run),
      .done(unitDone[0]),
      .clk(clk),
      .rst(rst)
   );

circuitInput circuitInput_1 (
      .in0(in1),
      .out0(output_1_0),
      .run(run),
      .done(unitDone[1]),
      .clk(clk),
      .rst(rst)
   );

circuitInput circuitInput_2 (
      .in0(in2),
      .out0(output_2_0),
      .run(run),
      .done(unitDone[2]),
      .clk(clk),
      .rst(rst)
   );

circuitInput circuitInput_3 (
      .in0(in3),
      .out0(output_3_0),
      .run(run),
      .done(unitDone[3]),
      .clk(clk),
      .rst(rst)
   );

sigma sigma_4 (
      .out0(output_4_0),
      .in0(output_3_0),
      .constant_0(constant_0),
      .constant_1(constant_1),
      .constant_2(constant_2),
      .run(run),
      .done(unitDone[4]),
      .clk(clk),
      .rst(rst)
   );

sigma sigma_5 (
      .out0(output_5_0),
      .in0(output_1_0),
      .constant_0(constant_3),
      .constant_1(constant_4),
      .constant_2(constant_5),
      .run(run),
      .done(unitDone[5]),
      .clk(clk),
      .rst(rst)
   );

pipeline_register pipeline_register_6 (
      .out0(output_6_0),
      .in0(output_9_0),
      .run(run),
      .done(unitDone[6]),
      .clk(clk),
      .rst(rst)
   );

ADD ADD_7 (
      .out0(output_7_0),
      .in0(output_5_0),
      .in1(output_2_0),
      .run(run),
      .done(unitDone[7]),
      .clk(clk),
      .rst(rst)
   );

ADD ADD_8 (
      .out0(output_8_0),
      .in0(output_4_0),
      .in1(output_0_0),
      .run(run),
      .done(unitDone[8]),
      .clk(clk),
      .rst(rst)
   );

ADD ADD_9 (
      .out0(output_9_0),
      .in0(output_7_0),
      .in1(output_8_0),
      .run(run),
      .done(unitDone[9]),
      .clk(clk),
      .rst(rst)
   );

circuitOutput circuitOutput_10 (
      .out0(out0),
      .in0(output_6_0),
      .run(run),
      .done(unitDone[10]),
      .clk(clk),
      .rst(rst)
   );

endmodule
