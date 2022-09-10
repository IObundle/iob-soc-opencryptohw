`timescale 1ns/1ps
`include "iob_lib.vh"

module iob_ethclockgen 
  # (
     )

  (

	//PLL_LOCKED outputs
   `IOB_OUTPUT(PLL_LOCKED_0, 1),
   `IOB_OUTPUT(PLL_LOCKED_1, 1),
	//Clock outputs
   `IOB_OUTPUT(TX_CLK_0, 1),
   `IOB_OUTPUT(RX_CLK_0, 1),
   `IOB_OUTPUT(TX_CLK_1, 1),
   `IOB_OUTPUT(RX_CLK_1, 1),

`include "iob_gen_if.vh"
   );

	//Set PLL_LOCKED signal
	assign PLL_LOCKED_0 = 1'b1;
	assign PLL_LOCKED_1 = 1'b1;

	//ethernet clock: 4x slower than system clock
	reg [1:0] eth_cnt = 2'b0;
	reg eth_clk;

	always @(posedge clk) begin
		 eth_cnt <= eth_cnt + 1'b1;
		 eth_clk <= eth_cnt[1];
	end

	//Set clock outputs
	assign TX_CLK_0 = eth_clk;
	assign RX_CLK_0 = eth_clk;
	assign TX_CLK_1 = eth_clk;
	assign RX_CLK_1 = eth_clk;
   
endmodule


