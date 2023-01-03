`timescale 1ns / 1ps

module iob_rom_sp
  #(
    parameter DATA_W = 8,
    parameter ADDR_W = 10,
    parameter HEXFILE = "none"
	)
   (
    input                    clk,
    input                    r_en,
    input [ADDR_W-1:0]       addr,
    output reg [DATA_W-1:0]  r_data
    );
   
    sky130_sram_4kbyte_1rw1r_32x1024_8 srom1(
        .clk0(clk),
        .csb0(1'b0),
        .web0(1'b1),
        .wmask0(4'b0000),
        .addr0(addr),
        .din0(),
        .dout0(r_data),

        .clk1(1'b0),
        .csb1(1'b0),
        .addr1(),
        .dout1()
    );
   
endmodule
