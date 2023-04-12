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
    parameter AXI_ID_W=1,
    parameter AXI_ADDR_W=`ADDR_W,
    parameter AXI_DATA_W=`DATA_W
    )
  (
   //do not remove line below
   //PIO

   //CPU TRAP
   output trap,
          
   //Memory macros
   `include "sram_port.vh"
   `include "bootrom_port.vh"
   `include "versat_external_memory_port.vh"
          
`ifdef USE_DDR 
   //address write
   output [2*1-1:0]           m_axi_awid, 
   output [2*`DDR_ADDR_W-1:0] m_axi_awaddr,
   output [2*8-1:0]           m_axi_awlen,
   output [2*3-1:0]           m_axi_awsize,
   output [2*2-1:0]           m_axi_awburst,
   output [2*2-1:0]           m_axi_awlock,
   output [2*4-1:0]           m_axi_awcache,
   output [2*3-1:0]           m_axi_awprot,
   output [2*4-1:0]           m_axi_awqos,
   output [2*1-1:0]           m_axi_awvalid,
   input  [2*1-1:0]           m_axi_awready,

   //write
   output [2*`DATA_W-1:0]     m_axi_wdata,
   output [2*`DATA_W/8-1:0]   m_axi_wstrb,
   output [2*1-1:0]           m_axi_wlast,
   output [2*1-1:0]           m_axi_wvalid, 
   input  [2*1-1:0]           m_axi_wready,

   //write response
   input  [2*1-1:0]           m_axi_bid,
   input  [2*2-1:0]           m_axi_bresp,
   input  [2*1-1:0]           m_axi_bvalid,
   output [2*1-1:0]           m_axi_bready,
  
   //address read
   output [2*1-1:0]           m_axi_arid,
   output [2*`DDR_ADDR_W-1:0] m_axi_araddr, 
   output [2*8-1:0]           m_axi_arlen,
   output [2*3-1:0]           m_axi_arsize,
   output [2*2-1:0]           m_axi_arburst,
   output [2*2-1:0]           m_axi_arlock,
   output [2*4-1:0]           m_axi_arcache,
   output [2*3-1:0]           m_axi_arprot,
   output [2*4-1:0]           m_axi_arqos,
   output [2*1-1:0]           m_axi_arvalid, 
   input  [2*1-1:0]           m_axi_arready,

   //read
   input  [2*1-1:0]           m_axi_rid,
   input  [2*`DATA_W-1:0]     m_axi_rdata,
   input  [2*2-1:0]           m_axi_rresp,
   input  [2*1-1:0]           m_axi_rlast, 
   input  [2*1-1:0]           m_axi_rvalid, 
   output [2*1-1:0]           m_axi_rready,
`endif //  `ifdef USE_DDR
          
`include "iob_gen_if.vh"
   );
   
   //
   // SYSTEM RESET
   //

   wire   boot;
   wire   cpu_reset;
   wire   reset;

   assign reset = rst;
   
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
   iob_picorv32 cpu
       (
        .clk (clk),
        .rst (cpu_reset),
        .boot (boot),
        .trap (trap),
        
        //instruction bus
        .ibus_req (cpu_i_req),
        .ibus_resp (cpu_i_resp),
        
        //data bus
        .dbus_req(cpu_d_req),
        .dbus_resp(cpu_d_resp)

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

      `include "sram_portmap.vh"
      `include "bootrom_portmap.vh"

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
       .AXI_ADDR_W(`DDR_ADDR_W),
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
      //address write
      .m_axi_awid(m_axi_awid[0*1+:1]), 
      .m_axi_awaddr(m_axi_awaddr[0*`DDR_ADDR_W+:`DDR_ADDR_W]), 
      .m_axi_awlen(m_axi_awlen[0*8+:8]), 
      .m_axi_awsize(m_axi_awsize[0*3+:3]), 
      .m_axi_awburst(m_axi_awburst[0*2+:2]), 
      .m_axi_awlock(m_axi_awlock[0*2+:2]), 
      .m_axi_awcache(m_axi_awcache[0*4+:4]), 
      .m_axi_awprot(m_axi_awprot[0*3+:3]),
      .m_axi_awqos(m_axi_awqos[0*4+:4]), 
      .m_axi_awvalid(m_axi_awvalid[0*1+:1]), 
      .m_axi_awready(m_axi_awready[0*1+:1]), 
        //write
      .m_axi_wdata(m_axi_wdata[0*`DATA_W+:`DATA_W]), 
      .m_axi_wstrb(m_axi_wstrb[0*`DATA_W/8+:`DATA_W/8]), 
      .m_axi_wlast(m_axi_wlast[0*1+:1]), 
      .m_axi_wvalid(m_axi_wvalid[0*1+:1]), 
      .m_axi_wready(m_axi_wready[0*1+:1]), 
      //write response
      .m_axi_bid(m_axi_bid[0*1+:1]),
      .m_axi_bresp(m_axi_bresp[0*2+:2]), 
      .m_axi_bvalid(m_axi_bvalid[0*1+:1]), 
      .m_axi_bready(m_axi_bready[0*1+:1]), 
      //address read
      .m_axi_arid(m_axi_arid[0*1+:1]), 
      .m_axi_araddr(m_axi_araddr[0*`DDR_ADDR_W+:`DDR_ADDR_W]), 
      .m_axi_arlen(m_axi_arlen[0*8+:8]), 
      .m_axi_arsize(m_axi_arsize[0*3+:3]), 
      .m_axi_arburst(m_axi_arburst[0*2+:2]), 
      .m_axi_arlock(m_axi_arlock[0*2+:2]), 
      .m_axi_arcache(m_axi_arcache[0*4+:4]), 
      .m_axi_arprot(m_axi_arprot[0*3+:3]), 
      .m_axi_arqos(m_axi_arqos[0*4+:4]), 
      .m_axi_arvalid(m_axi_arvalid[0*1+:1]), 
      .m_axi_arready(m_axi_arready[0*1+:1]), 
      //read 
      .m_axi_rid(m_axi_rid[0*1+:1]),
      .m_axi_rdata(m_axi_rdata[0*`DATA_W+:`DATA_W]), 
      .m_axi_rresp(m_axi_rresp[0*2+:2]), 
      .m_axi_rlast(m_axi_rlast[0*1+:1]), 
      .m_axi_rvalid(m_axi_rvalid[0*1+:1]),  
      .m_axi_rready(m_axi_rready[0*1+:1]),

      .clk (clk),
      .rst (cpu_reset)
      );
`endif

   //peripheral instances are inserted here

endmodule
