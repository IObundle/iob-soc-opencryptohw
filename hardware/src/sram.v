`timescale 1ns / 1ps
`include "system.vh"

module sram #(
              parameter FILE = "none"
         )
   (
    input                    clk,
    input                    rst,

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

   localparam file_suffix = {"3","2","1","0"};

   genvar                 i;
   generate
      for (i = 0; i < 4; i = i+1) begin : gen_main_mem_byte
         iob_tdp_ram
               #(
`ifdef SRAM_INIT
            .MEM_INIT_FILE({FILE, "_", file_suffix[8*(i+1)-1 -: 8], ".hex"}),
`endif
            .DATA_W(8),
                 .ADDR_W(`SRAM_ADDR_W-2))
         main_mem_byte 
               (
           .clkA             (clk),
           .clkB             (clk),
                //data 
           .enA            (d_valid),
           .weA            (d_wstrb[i]),
           .addrA          (d_addr),
           .dinA           (d_wdata[8*(i+1)-1 -: 8]),
           .doutA          (d_rdata[8*(i+1)-1 -: 8]),
                //instruction
           .enB            (i_valid),
           .weB            (i_wstrb[i]),
           .addrB          (i_addr),
           .dinB           (i_wdata[8*(i+1)-1 -: 8]),
           .doutB          (i_rdata[8*(i+1)-1 -: 8])
           );  
      end // block: gen_main_mem_byte
   endgenerate

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
