// True-Dual-Port BRAM with Byte-wide Write Enable
// Read-First mode 

`timescale 1 ns / 1 ps
`include "global_defines.vh"

module iob_ram_tdp_be
  #(
    parameter HEXFILE = "none",
    parameter ADDR_W = 10, // Addr Width in bits : 2*ADDR_W = RAM Depth
    parameter DATA_W = 32  // Data Width in bits
    )
   (
    // Port A
    input                clkA,
    input                enA,
    input [DATA_W/8-1:0] weA,
    input [ADDR_W-1:0]   addrA,
    input [DATA_W-1:0]   dinA,
    output [DATA_W-1:0]  doutA,

    // Port B
    input                clkB,
    input                enB,
    input [DATA_W/8-1:0] weB,
    input [ADDR_W-1:0]   addrB,
    input [DATA_W-1:0]   dinB,
    output [DATA_W-1 :0] doutB
    );

    sky130_sram_2kbyte_1rw1r_32x512_8 sram_tdp (
        .clk0(clkA),
        .csb0(!enA),
        .web0(!weA),
        .wmask0(4'b1111),
        .addr0(addrA),
        .din0(dinA),
        .dout0(doutA),
    
        .clk1(clkB),
        .csb1(!enB),
        .addr1(addrB),
        .dout1(doutB)
  );

endmodule
