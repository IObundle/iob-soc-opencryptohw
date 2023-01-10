`timescale 1ns / 1ps
`include "system.vh"

module sram
    (
    input                    clk,
    input                    rst,

    `include "sram_port.vh"

    // intruction bus
    input                    i_valid,
    input [`SRAM_ADDR_W-3:0] i_addr,
    input [`DATA_W-1:0]      i_wdata, //used for booting
    input [`DATA_W/8-1:0]    i_wstrb, //used for booting
    output [`DATA_W-1:0]     i_rdata,
    output reg               i_ready,

    // data bus
    input                    d_valid,
    input [`SRAM_ADDR_W-3:0] d_addr,
    input [`DATA_W-1:0]      d_wdata,
    input [`DATA_W/8-1:0]    d_wstrb,
    output [`DATA_W-1:0]     d_rdata,
    output reg               d_ready

    );

    // data port
    assign sram_enA = d_valid;
    assign sram_addrA = d_addr;
    assign sram_weA = d_wstrb;
    assign sram_dinA = d_wdata;
    assign d_rdata = sram_doutA;

    // instruction port
    assign sram_enB = i_valid;
    assign sram_addrB = i_addr;
    assign sram_weB = i_wstrb;
    assign sram_dinB = i_wdata;
    assign i_rdata = sram_doutB;

   // reply with ready 
   always @(posedge clk, posedge rst)
     if(rst) begin
	    d_ready <= 1'b0;
	    i_ready <= 1'b0;
     end else begin 
	    d_ready <= d_valid;
	    i_ready <= i_valid;
     end
endmodule
