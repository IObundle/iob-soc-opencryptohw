(*blackbox*)

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

endmodule
