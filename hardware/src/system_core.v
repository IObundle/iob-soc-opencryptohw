`timescale 1 ns / 1 ps
`include "system.vh"
`include "iob_lib.vh"
`include "iob_intercon.vh"

//do not remove line below
//PHEADER

module system
  #(
    parameter ADDR_W=`ADDR_W,
    parameter DATA_W=`DATA_W,
    parameter AXI_ID_W=0,
    parameter AXI_ADDR_W=`ADDR_W,
    parameter AXI_DATA_W=`DATA_W
    )
  (
   //do not remove line below
   //PIO

   //CPU TRAP
   output trap,
          
          
//Temporarily comment ifdef because its not yet compatible with tester python scripts
//`ifdef USE_DDR
 //AXI MASTER INTERFACE
 //`include "m_axi_m_port.vh" //Tester python scripts not compatible with this include
 `IOB_OUTPUT(m_axi_awid, AXI_ID_W), //Address write channel ID
 `IOB_OUTPUT(m_axi_awaddr, AXI_ADDR_W), //Address write channel address
 `IOB_OUTPUT(m_axi_awlen, 8), //Address write channel burst length
 `IOB_OUTPUT(m_axi_awsize, 3), //Address write channel burst size. This signal indicates the size of each transfer in the burst
 `IOB_OUTPUT(m_axi_awburst, 2), //Address write channel burst type
 `IOB_OUTPUT(m_axi_awlock, 2), //Address write channel lock type
 `IOB_OUTPUT(m_axi_awcache, 4), //Address write channel memory type. Transactions set with Normal Non-cacheable Modifiable and Bufferable (0011).
 `IOB_OUTPUT(m_axi_awprot, 3), //Address write channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
 `IOB_OUTPUT(m_axi_awqos, 4), //Address write channel quality of service
 `IOB_OUTPUT(m_axi_awvalid, 1), //Address write channel valid
 `IOB_INPUT(m_axi_awready, 1), //Address write channel ready
 `IOB_OUTPUT(m_axi_wdata, AXI_DATA_W), //Write channel data
 `IOB_OUTPUT(m_axi_wstrb, (AXI_DATA_W/8)), //Write channel write strobe
 `IOB_OUTPUT(m_axi_wlast, 1), //Write channel last word flag
 `IOB_OUTPUT(m_axi_wvalid, 1), //Write channel valid
 `IOB_INPUT(m_axi_wready, 1), //Write channel ready
 `IOB_INPUT(m_axi_bid, AXI_ID_W), //Write response channel ID
 `IOB_INPUT(m_axi_bresp, 2), //Write response channel response
 `IOB_INPUT(m_axi_bvalid, 1), //Write response channel valid
 `IOB_OUTPUT(m_axi_bready, 1), //Write response channel ready
 `IOB_OUTPUT(m_axi_arid, AXI_ID_W), //Address read channel ID
 `IOB_OUTPUT(m_axi_araddr, AXI_ADDR_W), //Address read channel address
 `IOB_OUTPUT(m_axi_arlen, 8), //Address read channel burst length
 `IOB_OUTPUT(m_axi_arsize, 3), //Address read channel burst size. This signal indicates the size of each transfer in the burst
 `IOB_OUTPUT(m_axi_arburst, 2), //Address read channel burst type
 `IOB_OUTPUT(m_axi_arlock, 2), //Address read channel lock type
 `IOB_OUTPUT(m_axi_arcache, 4), //Address read channel memory type. Transactions set with Normal Non-cacheable Modifiable and Bufferable (0011).
 `IOB_OUTPUT(m_axi_arprot, 3), //Address read channel protection type. Transactions set with Normal, Secure, and Data attributes (000).
 `IOB_OUTPUT(m_axi_arqos, 4), //Address read channel quality of service
 `IOB_OUTPUT(m_axi_arvalid, 1), //Address read channel valid
 `IOB_INPUT(m_axi_arready, 1), //Address read channel ready
 `IOB_INPUT(m_axi_rid, AXI_ID_W), //Read channel ID
 `IOB_INPUT(m_axi_rdata, AXI_DATA_W), //Read channel data
 `IOB_INPUT(m_axi_rresp, 2), //Read channel response
 `IOB_INPUT(m_axi_rlast, 1), //Read channel last word
 `IOB_INPUT(m_axi_rvalid, 1), //Read channel valid
 `IOB_OUTPUT(m_axi_rready, 1), //Read channel ready
//`endif //  `ifdef USE_DDR
          
`include "iob_gen_if.vh"
   );
   
   //
   // SYSTEM RESET
   //

   wire   boot;
   wire   cpu_reset;
   
   //
   //  CPU
   //

   // instruction bus
   wire [`REQ_W-1:0] cpu_i_req;
   wire [`RESP_W-1:0] cpu_i_resp;

   // data cat bus
   wire [`REQ_W-1:0]  cpu_d_req;
   wire [`RESP_W-1:0] cpu_d_resp;
   
   //instantiate the cpu
   iob_VexRiscv cpu
       (
        .clk (clk),
        .rst (cpu_reset),
        .boot (boot),
        .trap (trap),
        
        //instruction bus
        .ibus_req (cpu_i_req),
        .ibus_resp (cpu_i_resp),
        
        //data bus
        .dbus_req (cpu_d_req),
        .dbus_resp (cpu_d_resp),

        //interrupt
        .timerInterrupt(1'b0),
        .softwareInterrupt(1'b0)
        );


   //   
   // SPLIT CPU BUSES TO ACCESS INTERNAL OR EXTERNAL MEMORY
   //

   //internal memory instruction bus
   wire [`REQ_W-1:0]  int_mem_i_req;
   wire [`RESP_W-1:0] int_mem_i_resp;
   //external memory instruction bus
`ifdef RUN_EXTMEM
   wire [`REQ_W-1:0]  ext_mem_i_req;
   wire [`RESP_W-1:0] ext_mem_i_resp;
`endif

   // INSTRUCTION BUS
   iob_split
     #(
`ifdef RUN_EXTMEM
       .N_SLAVES(2),
`else
       .N_SLAVES(1),
`endif
       .P_SLAVES(`E_BIT)
       )
   ibus_split
     (
      .clk (clk),
      .rst (cpu_reset),
      // master interface
      .m_req (cpu_i_req),
      .m_resp (cpu_i_resp),
      
      // slaves interface
`ifdef RUN_EXTMEM
      .s_req ( {ext_mem_i_req, int_mem_i_req} ),
      .s_resp ( {ext_mem_i_resp, int_mem_i_resp} )
`else
      .s_req (int_mem_i_req),
      .s_resp ( int_mem_i_resp)
`endif
      );


   // DATA BUS

`ifdef USE_DDR
   //external memory data bus
   wire [`REQ_W-1:0]         ext_mem_d_req;
   wire [`RESP_W-1:0]        ext_mem_d_resp;
   //internal data bus
   wire [`REQ_W-1:0]         int_d_req;
   wire [`RESP_W-1:0]        int_d_resp;

   iob_split
     #(
       .N_SLAVES(2), //E,{P,I}
       .P_SLAVES(`E_BIT)
       )
   dbus_split
     (
      .clk    ( clk   ),
      .rst    ( cpu_reset ),

      // master interface
      .m_req  ( cpu_d_req  ),
      .m_resp ( cpu_d_resp ),

      // slaves interface
      .s_req  ( {ext_mem_d_req, int_d_req}   ),
      .s_resp ( {ext_mem_d_resp, int_d_resp} )
      );
`endif

   //
   // SPLIT INTERNAL MEMORY AND PERIPHERALS BUS
   //

   //internal memory data bus
   wire [`REQ_W-1:0]         int_mem_d_req;
   wire [`RESP_W-1:0]        int_mem_d_resp;
   //peripheral bus
   wire [`REQ_W-1:0]         pbus_req;
   wire [`RESP_W-1:0]        pbus_resp;

   iob_split
     #(
       .N_SLAVES(2), //P,I
       .P_SLAVES(`P_BIT)
       )
   int_dbus_split
     (
      .clk (clk),
      .rst (cpu_reset),

`ifdef USE_DDR
      // master interface
      .m_req  ( int_d_req  ),
      .m_resp ( int_d_resp ),
`else
      // master interface
      .m_req  ( cpu_d_req  ),
      .m_resp ( cpu_d_resp ),
`endif

      // slaves interface
      .s_req  ( {pbus_req, int_mem_d_req}   ),
      .s_resp ( {pbus_resp, int_mem_d_resp} )
      );


   //
   // SPLIT PERIPHERAL BUS
   //

   //slaves bus
   wire [`N_SLAVES*`REQ_W-1:0] slaves_req;
   wire [`N_SLAVES*`RESP_W-1:0] slaves_resp;

   iob_split
     #(
       .N_SLAVES(`N_SLAVES),
       .P_SLAVES(`P_BIT-1)
       )
   pbus_split
     (
      .clk (clk),
      .rst (cpu_reset),
      // master interface
      .m_req (pbus_req),
      .m_resp (pbus_resp),
      
      // slaves interface
      .s_req (slaves_req),
      .s_resp (slaves_resp)
      );


   //
   // INTERNAL SRAM MEMORY
   //

   int_mem int_mem0
     (
      .clk (clk),
      .rst (rst),
      .boot (boot),
      .cpu_reset (cpu_reset),

      // instruction bus
      .i_req (int_mem_i_req),
      .i_resp (int_mem_i_resp),

      //data bus
      .d_req (int_mem_d_req),
      .d_resp (int_mem_d_resp)
      );

`ifdef USE_DDR
   //
   // EXTERNAL DDR MEMORY
   //
   ext_mem
     #(
       .AXI_ID_W(AXI_ID_W),
       .AXI_ADDR_W(AXI_ADDR_W),
       .AXI_DATA_W(AXI_DATA_W)
       )
   ext_mem0
   (
 `ifdef RUN_EXTMEM
      // instruction bus
      .i_req ( {ext_mem_i_req[`valid(0)], ext_mem_i_req[`address(0, `FIRM_ADDR_W)-2], ext_mem_i_req[`write(0)]} ),
      .i_resp (ext_mem_i_resp),
 `endif
      //data bus
      .d_req ( {ext_mem_d_req[`valid(0)], ext_mem_d_req[`address(0, `DCACHE_ADDR_W+1)-2], ext_mem_d_req[`write(0)]} ),
      .d_resp (ext_mem_d_resp),

      //AXI INTERFACE 
`include "m_axi_portmap.vh"
      .clk (clk),
      .rst (cpu_reset)
      );
`endif

   //peripheral instances are inserted here

endmodule
