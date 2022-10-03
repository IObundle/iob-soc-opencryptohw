`timescale 1ns / 1ps

`include "iob_lib.vh"
`include "system.vh"

//PHEADER

module system_top 
  (
   output                             trap,
   //tester uart
   input                              uart_valid,
   input [`iob_uart_swreg_ADDR_W-1:0] uart_addr,
   input [`DATA_W-1:0]                uart_wdata,
   input [3:0]                        uart_wstrb,
   output [`DATA_W-1:0]               uart_rdata,
   output                             uart_ready,
`include "iob_gen_if.vh"
   );
 
   localparam AXI_ID_W = 4;
   localparam AXI_ADDR_W=`DDR_ADDR_W;
   localparam AXI_DATA_W=`DATA_W;
 
   //PWIRES

   
   /////////////////////////////////////////////
   // TEST PROCEDURE
   //
   initial begin

`ifdef VCD
      $dumpfile("system.vcd");
      $dumpvars();
`endif

   end
   
   //
   // INSTANTIATE COMPONENTS
   //

   //AXI wires for connecting system to memory

`ifdef USE_DDR
 `include "m_axi_wire.vh"
`endif

   //
   // UNIT UNDER TEST
   //
   system
     #(
       .AXI_ID_W(AXI_ID_W),
       .AXI_ADDR_W(AXI_ADDR_W),
       .AXI_DATA_W(AXI_DATA_W)
       )
   uut
     (
      //PORTS
`ifdef USE_DDR
 `include "m_axi_portmap.vh"
`endif               
      .clk (clk),
      .rst (rst),
      .trap (trap)
      );


   //instantiate the axi memory
`ifdef USE_DDR
   axi_ram 
     #(
 `ifdef DDR_INIT
       .FILE("firmware.hex"),
       .FILE_SIZE(`FW_SIZE),
 `endif
       .ID_WIDTH(AXI_ID_W),
       .DATA_WIDTH (`DATA_W),
       .ADDR_WIDTH (`DDR_ADDR_W)
       )
   ddr_model_mem
     (
 `include "s_axi_portmap.vh"
      .clk(clk),
      .rst(rst)
      );   
`endif

   
   //finish simulation on trap
   /* always @(posedge trap) begin
    #10 $display("Found CPU trap condition");
    $finish;
   end*/

   //sram monitor - use for debugging programs
   /*
    wire [`SRAM_ADDR_W-1:0] sram_daddr = uut.int_mem0.int_sram.d_addr;
    wire sram_dwstrb = |uut.int_mem0.int_sram.d_wstrb & uut.int_mem0.int_sram.d_valid;
    wire sram_drdstrb = !uut.int_mem0.int_sram.d_wstrb & uut.int_mem0.int_sram.d_valid;
    wire [`DATA_W-1:0] sram_dwdata = uut.int_mem0.int_sram.d_wdata;


    wire sram_iwstrb = |uut.int_mem0.int_sram.i_wstrb & uut.int_mem0.int_sram.i_valid;
    wire sram_irdstrb = !uut.int_mem0.int_sram.i_wstrb & uut.int_mem0.int_sram.i_valid;
    wire [`SRAM_ADDR_W-1:0] sram_iaddr = uut.int_mem0.int_sram.i_addr;
    wire [`DATA_W-1:0] sram_irdata = uut.int_mem0.int_sram.i_rdata;

    
    always @(posedge sram_dwstrb)
    if(sram_daddr == 13'h090d)  begin
    #10 $display("Found CPU memory condition at %f : %x : %x", $time, sram_daddr, sram_dwdata );
    //$finish;
      end
    */

	//Manually added testbench uart core. RS232 pins attached to the same pins
	//of the uut UART0 instance to communicate with it
   iob_uart uart_tb
     (
      .clk       (clk),
      .rst       (rst),
      
      .valid     (uart_valid),
      .address   (uart_addr),
      .wdata     (uart_wdata),
      .wstrb     (uart_wstrb),
      .rdata     (uart_rdata),
      .ready     (uart_ready),
      
      .txd       (UART0_rxd),
      .rxd       (UART0_txd),
      .rts       (UART0_cts),
      .cts       (UART0_rts)
      );

	//Manually added testbench ethernet core. Signals connected to ETHERNET0
	//instance of the system.
   //ethernet clock: 4x slower than system clock
   reg [1:0] eth_cnt = 2'b0;
   reg eth_clk;

   always @(posedge clk) begin
       eth_cnt <= eth_cnt + 1'b1;
       eth_clk <= eth_cnt[1];
   end

   // Ethernet Interface signals
   assign ETHERNET0_RX_CLK = eth_clk;
   assign ETHERNET0_TX_CLK = eth_clk;
   assign ETHERNET0_PLL_LOCKED = 1'b1;

//add core test module in testbench
iob_eth_tb_gen eth_tb(
      .clk      (clk),
      .reset    (rst),

      // This module acts like a loopback
      .RX_CLK(ETHERNET0_TX_CLK),
      .RX_DATA(ETHERNET0_TX_DATA),
      .RX_DV(ETHERNET0_TX_EN),

      // The wires are thus reversed
      .TX_CLK(ETHERNET0_RX_CLK),
      .TX_DATA(ETHERNET0_RX_DATA),
      .TX_EN(ETHERNET0_RX_DV)
);

endmodule
