`timescale 1ns / 1ps
`include "system.vh"

module top_system(
	          input         c0_sys_clk_clk_p, 
                  input         c0_sys_clk_clk_n, 
	          input         reset,

	          //uart
	          output        uart_txd,
	          input         uart_rxd,

`ifdef USE_DDR
                  output        c0_ddr4_act_n,
                  output [16:0] c0_ddr4_adr,
                  output [1:0]  c0_ddr4_ba,
                  output [0:0]  c0_ddr4_bg,
                  output [0:0]  c0_ddr4_cke,
                  output [0:0]  c0_ddr4_odt,
                  output [0:0]  c0_ddr4_cs_n,
                  output [0:0]  c0_ddr4_ck_t,
                  output [0:0]  c0_ddr4_ck_c,
                  output        c0_ddr4_reset_n,
                  inout [3:0]   c0_ddr4_dm_dbi_n,
                  inout [31:0]  c0_ddr4_dq,
                  inout [3:0]   c0_ddr4_dqs_c,
                  inout [3:0]   c0_ddr4_dqs_t, 
`endif                  
		  output        trap
		  );

`ifdef USE_DDR
   //
   // AXI INTERCONNECT
   //
                         
   // SYSTEM/SLAVE SIDE
   // address write
   wire [0:0]			sys_awid;
   wire [`DDR_ADDR_W-1:0]       sys_awaddr;
   wire [7:0] 			sys_awlen;
   wire [2:0] 			sys_awsize;
   wire [1:0] 			sys_awburst;
   wire 			sys_awlock;
   wire [3:0]			sys_awcache;
   wire [2:0] 			sys_awprot;
   wire [3:0] 			sys_awqos;
   wire 			sys_awvalid;
   wire 			sys_awready;
   //write
   wire [31:0]                  sys_wdata;
   wire [3:0] 			sys_wstrb;
   wire 			sys_wlast;
   wire 			sys_wvalid;
   wire 			sys_wready;
   //write response
   wire [0:0]			sys_bid;
   wire [1:0] 			sys_bresp;
   wire 			sys_bvalid;
   wire 			sys_bready;
   //address read
   wire [0:0]			sys_arid;
   wire [`DDR_ADDR_W-1:0]       sys_araddr;
   wire [7:0] 			sys_arlen;
   wire [2:0] 			sys_arsize;
   wire [1:0] 			sys_arburst;
   wire 			sys_arlock;
   wire [3:0] 			sys_arcache;
   wire [2:0] 			sys_arprot;
   wire [3:0] 			sys_arqos;
   wire 			sys_arvalid;
   wire 			sys_arready;
   //read
   wire [0:0]			sys_rid;
   wire [`DATA_W-1:0]           sys_rdata;   
   wire [1:0]                   sys_rresp;   
   wire 			sys_rlast;
   wire 			sys_rvalid;
   wire 			sys_rready;

   // DDR/MASTER SIDE
   //Write address
   wire [3:0] 			ddr_awid;
   wire [`DDR_ADDR_W-1:0]       ddr_awaddr;
   wire [7:0] 			ddr_awlen;
   wire [2:0] 			ddr_awsize;
   wire [1:0] 			ddr_awburst;
   wire 			ddr_awlock;
   wire [3:0] 			ddr_awcache;
   wire [2:0] 			ddr_awprot;
   wire [3:0] 			ddr_awqos;
   wire 			ddr_awvalid;
   wire 			ddr_awready;
   //Write data
   wire [31:0] 			ddr_wdata;
   wire [3:0] 			ddr_wstrb;
   wire 			ddr_wlast;
   wire 			ddr_wvalid;
   wire 			ddr_wready;
   //Write response
   wire [3:0]                   ddr_bid;
   wire [1:0] 			ddr_bresp;
   wire 			ddr_bvalid;
   wire 			ddr_bready;
   //Read address
   wire [3:0] 			ddr_arid;
   wire [`DDR_ADDR_W-1:0]       ddr_araddr;
   wire [7:0] 			ddr_arlen;
   wire [2:0] 			ddr_arsize;
   wire [1:0] 			ddr_arburst;
   wire 			ddr_arlock;
   wire [3:0] 			ddr_arcache;
   wire [2:0] 			ddr_arprot;
   wire [3:0] 			ddr_arqos;
   wire 			ddr_arvalid;
   wire 			ddr_arready;
   //Read data
   wire [3:0]			ddr_rid;
   wire [31:0] 			ddr_rdata;
   wire [1:0] 			ddr_rresp;
   wire 			ddr_rlast;
   wire 			ddr_rvalid;
   wire 			ddr_rready;
`endif


   //
   // CLOCK MANAGEMENT
   //

   //system clock
   wire 			sys_clk;
   
`ifdef USE_DDR
   wire                         ddr_aclk;
`else 
   clock_wizard #(
		  .OUTPUT_PER(10),
		  .INPUT_PER(4)
		  )
   clk_250_to_100_MHz(
		      .clk_in1_p(c0_sys_clk_clk_p),
		      .clk_in1_n(c0_sys_clk_clk_n),
		      .clk_out1(sys_clk)
		      );
`endif
   
   //ddr clock output from ddr ctrl 
 


   //   
   // RESET MANAGEMENT
   //

   //system reset
 
   wire                         sys_rst;

`ifdef USE_DDR
   wire                         init_calib_complete;
   wire                         sys_rstn;

   assign sys_rst  = ~sys_rstn;
`else
   reg [15:0] 			rst_cnt;
   reg                          sys_rst_int;
   
   always @(posedge sys_clk, posedge reset)
     if(reset) begin
        sys_rst_int <= 1'b0;
        rst_cnt <= 16'hFFFF;
     end else begin 
        if(rst_cnt != 16'h0)
          rst_cnt <= rst_cnt - 1'b1;
        sys_rst_int <= (rst_cnt != 16'h0);
     end

   assign sys_rst = sys_rst_int;
   
`endif

`ifdef USE_DDR
   //AXI DDR side reset (ddr_arst) : generated by MIG itself
   wire                         ddr_arstn;   
   wire                         ddr_ui_clk;
`endif
   

   //
   // DDR CONTROLLER
   //
                 
`ifdef USE_DDR   
   ddr4_0 ddr4_ram 
     (
      .sys_rst                (reset),
      .c0_sys_clk_p           (c0_sys_clk_clk_p),
      .c0_sys_clk_n           (c0_sys_clk_clk_n),

      .dbg_clk                (),
      .dbg_bus                (),
      
      //EXTERNAL SIDE
      .c0_ddr4_act_n          (c0_ddr4_act_n),
      .c0_ddr4_adr            (c0_ddr4_adr),
      .c0_ddr4_ba             (c0_ddr4_ba),
      .c0_ddr4_bg             (c0_ddr4_bg),
      .c0_ddr4_cke            (c0_ddr4_cke),
      .c0_ddr4_odt            (c0_ddr4_odt),
      .c0_ddr4_cs_n           (c0_ddr4_cs_n),
      .c0_ddr4_ck_t           (c0_ddr4_ck_t),
      .c0_ddr4_ck_c           (c0_ddr4_ck_c),
      .c0_ddr4_reset_n        (c0_ddr4_reset_n),
      .c0_ddr4_dm_dbi_n       (c0_ddr4_dm_dbi_n),
      .c0_ddr4_dq             (c0_ddr4_dq),
      .c0_ddr4_dqs_c          (c0_ddr4_dqs_c),
      .c0_ddr4_dqs_t          (c0_ddr4_dqs_t),
      .c0_init_calib_complete (init_calib_complete),
      
      //generated clocks and resets
      .c0_ddr4_ui_clk         (ddr_ui_clk),
      .c0_ddr4_ui_clk_sync_rst(ddr_ui_rst),
      .addn_ui_clkout1        (sys_clk),

      //USER AXI INTERFACE
      //address write 
      .c0_ddr4_aresetn        (ddr_arstn),
      .c0_ddr4_s_axi_awid     (ddr_awid),
      .c0_ddr4_s_axi_awaddr   (ddr_awaddr),
      .c0_ddr4_s_axi_awlen    (ddr_awlen),
      .c0_ddr4_s_axi_awsize   (ddr_awsize),
      .c0_ddr4_s_axi_awburst  (ddr_awburst),
      .c0_ddr4_s_axi_awlock   (ddr_awlock),
      .c0_ddr4_s_axi_awprot   (ddr_awprot),
      .c0_ddr4_s_axi_awcache  (ddr_awcache),
      .c0_ddr4_s_axi_awqos    (ddr_awqos),
      .c0_ddr4_s_axi_awvalid  (ddr_awvalid),
      .c0_ddr4_s_axi_awready  (ddr_awready),

      //write  
      .c0_ddr4_s_axi_wvalid   (ddr_wvalid),
      .c0_ddr4_s_axi_wready   (ddr_wready),
      .c0_ddr4_s_axi_wdata    (ddr_wdata),
      .c0_ddr4_s_axi_wstrb    (ddr_wstrb),
      .c0_ddr4_s_axi_wlast    (ddr_wlast),

      //write response
      .c0_ddr4_s_axi_bready   (ddr_bready),
      .c0_ddr4_s_axi_bid      (ddr_bid),
      .c0_ddr4_s_axi_bresp    (ddr_bresp),
      .c0_ddr4_s_axi_bvalid   (ddr_bvalid),

      //address read
      .c0_ddr4_s_axi_arid     (ddr_arid),
      .c0_ddr4_s_axi_araddr   (ddr_araddr),
      .c0_ddr4_s_axi_arlen    (ddr_arlen), 
      .c0_ddr4_s_axi_arsize   (ddr_arsize),    
      .c0_ddr4_s_axi_arburst  (ddr_arburst),
      .c0_ddr4_s_axi_arlock   (ddr_arlock),
      .c0_ddr4_s_axi_arcache  (ddr_arcache),
      .c0_ddr4_s_axi_arprot   (ddr_arprot),
      .c0_ddr4_s_axi_arqos    (ddr_arqos),
      .c0_ddr4_s_axi_arvalid  (ddr_arvalid),
      .c0_ddr4_s_axi_arready  (ddr_arready),
      
      //read   
      .c0_ddr4_s_axi_rready   (ddr_rready),
      .c0_ddr4_s_axi_rid      (ddr_rid),
      .c0_ddr4_s_axi_rdata    (ddr_rdata),
      .c0_ddr4_s_axi_rresp    (ddr_rresp),
      .c0_ddr4_s_axi_rlast    (ddr_rlast),
      .c0_ddr4_s_axi_rvalid   (ddr_rvalid)
      );   


   axi_interconnect_0 cache2ddr 
     (
      .INTERCONNECT_ACLK     (ddr_ui_clk),
      .INTERCONNECT_ARESETN  (~(ddr_ui_rst | ~init_calib_complete)),
      
      //
      // SYSTEM SIDE
      //
      .S00_AXI_ARESET_OUT_N (sys_rstn),
      .S00_AXI_ACLK         (sys_clk),
      
      //Write address
      .S00_AXI_AWID         (sys_awid),
      .S00_AXI_AWADDR       (sys_awaddr),
      .S00_AXI_AWLEN        (sys_awlen),
      .S00_AXI_AWSIZE       (sys_awsize),
      .S00_AXI_AWBURST      (sys_awburst),
      .S00_AXI_AWLOCK       (sys_awlock),
      .S00_AXI_AWCACHE      (sys_awcache),
      .S00_AXI_AWPROT       (sys_awprot),
      .S00_AXI_AWQOS        (sys_awqos),
      .S00_AXI_AWVALID      (sys_awvalid),
      .S00_AXI_AWREADY      (sys_awready),

      //Write data
      .S00_AXI_WDATA        (sys_wdata),
      .S00_AXI_WSTRB        (sys_wstrb),
      .S00_AXI_WLAST        (sys_wlast),
      .S00_AXI_WVALID       (sys_wvalid),
      .S00_AXI_WREADY       (sys_wready),
      
      //Write response
      .S00_AXI_BID           (sys_bid),
      .S00_AXI_BRESP         (sys_bresp),
      .S00_AXI_BVALID        (sys_bvalid),
      .S00_AXI_BREADY        (sys_bready),
      
      //Read address
      .S00_AXI_ARID         (sys_arid),
      .S00_AXI_ARADDR       (sys_araddr),
      .S00_AXI_ARLEN        (sys_arlen),
      .S00_AXI_ARSIZE       (sys_arsize),
      .S00_AXI_ARBURST      (sys_arburst),
      .S00_AXI_ARLOCK       (sys_arlock),
      .S00_AXI_ARCACHE      (sys_arcache),
      .S00_AXI_ARPROT       (sys_arprot),
      .S00_AXI_ARQOS        (sys_arqos),
      .S00_AXI_ARVALID      (sys_arvalid),
      .S00_AXI_ARREADY      (sys_arready),
      
      //Read data
      .S00_AXI_RID          (sys_rid),
      .S00_AXI_RDATA        (sys_rdata),
      .S00_AXI_RRESP        (sys_rresp),
      .S00_AXI_RLAST        (sys_rlast),
      .S00_AXI_RVALID       (sys_rvalid),
      .S00_AXI_RREADY       (sys_rready),
      //
      // DDR SIDE
      //

      .M00_AXI_ARESET_OUT_N  (ddr_arstn),
      .M00_AXI_ACLK          (ddr_ui_clk),
      
      //Write address
      .M00_AXI_AWID          (ddr_awid),
      .M00_AXI_AWADDR        (ddr_awaddr),
      .M00_AXI_AWLEN         (ddr_awlen),
      .M00_AXI_AWSIZE        (ddr_awsize),
      .M00_AXI_AWBURST       (ddr_awburst),
      .M00_AXI_AWLOCK        (ddr_awlock),
      .M00_AXI_AWCACHE       (ddr_awcache),
      .M00_AXI_AWPROT        (ddr_awprot),
      .M00_AXI_AWQOS         (ddr_awqos),
      .M00_AXI_AWVALID       (ddr_awvalid),
      .M00_AXI_AWREADY       (ddr_awready),
      
      //Write data
      .M00_AXI_WDATA         (ddr_wdata),
      .M00_AXI_WSTRB         (ddr_wstrb),
      .M00_AXI_WLAST         (ddr_wlast),
      .M00_AXI_WVALID        (ddr_wvalid),
      .M00_AXI_WREADY        (ddr_wready),
      
      //Write response
      .M00_AXI_BID           (ddr_bid),
      .M00_AXI_BRESP         (ddr_bresp),
      .M00_AXI_BVALID        (ddr_bvalid),
      .M00_AXI_BREADY        (ddr_bready),
      
      //Read address
      .M00_AXI_ARID         (ddr_arid),
      .M00_AXI_ARADDR       (ddr_araddr),
      .M00_AXI_ARLEN        (ddr_arlen),
      .M00_AXI_ARSIZE       (ddr_arsize),
      .M00_AXI_ARBURST      (ddr_arburst),
      .M00_AXI_ARLOCK       (ddr_arlock),
      .M00_AXI_ARCACHE      (ddr_arcache),
      .M00_AXI_ARPROT       (ddr_arprot),
      .M00_AXI_ARQOS        (ddr_arqos),
      .M00_AXI_ARVALID      (ddr_arvalid),
      .M00_AXI_ARREADY      (ddr_arready),
      
      //Read data
      .M00_AXI_RID          (ddr_rid),
      .M00_AXI_RDATA        (ddr_rdata),
      .M00_AXI_RRESP        (ddr_rresp),
      .M00_AXI_RLAST        (ddr_rlast),
      .M00_AXI_RVALID       (ddr_rvalid),
      .M00_AXI_RREADY       (ddr_rready)
      );
`endif

   //
   // SYSTEM
   //
   system system 
     (
      .clk           (sys_clk),
      .reset         (sys_rst),
      .trap          (trap),

`ifdef USE_DDR
      //address write
      .m_axi_awid    (sys_awid),
      .m_axi_awaddr  (sys_awaddr),
      .m_axi_awlen   (sys_awlen),
      .m_axi_awsize  (sys_awsize),
      .m_axi_awburst (sys_awburst),
      .m_axi_awlock  (sys_awlock),
      .m_axi_awcache (sys_awcache),
      .m_axi_awprot  (sys_awprot),
      .m_axi_awqos   (sys_awqos),
      .m_axi_awvalid (sys_awvalid),
      .m_axi_awready (sys_awready),

      //write  
      .m_axi_wdata   (sys_wdata),
      .m_axi_wstrb   (sys_wstrb),
      .m_axi_wlast   (sys_wlast),
      .m_axi_wvalid  (sys_wvalid),
      .m_axi_wready  (sys_wready),
      
      //write response
      //.m_axi_bid     (sys_bid),
      .m_axi_bresp   (sys_bresp),
      .m_axi_bvalid  (sys_bvalid),
      .m_axi_bready  (sys_bready),

      //address read
      .m_axi_arid    (sys_arid),
      .m_axi_araddr  (sys_araddr),
      .m_axi_arlen   (sys_arlen),
      .m_axi_arsize  (sys_arsize),
      .m_axi_arburst (sys_arburst),
      .m_axi_arlock  (sys_arlock),
      .m_axi_arcache (sys_arcache),
      .m_axi_arprot  (sys_arprot),
      .m_axi_arqos   (sys_arqos),
      .m_axi_arvalid (sys_arvalid),
      .m_axi_arready (sys_arready),

      //read   
      //.m_axi_rid     (sys_rid),
      .m_axi_rdata   (sys_rdata),
      .m_axi_rresp   (sys_rresp),
      .m_axi_rlast   (sys_rlast),
      .m_axi_rvalid  (sys_rvalid),
      .m_axi_rready  (sys_rready),	
`endif
      
      //UART
      .uart_txd      (uart_txd),
      .uart_rxd      (uart_rxd),
      .uart_rts      (),
      .uart_cts      (1'b1)
      );
   
endmodule
