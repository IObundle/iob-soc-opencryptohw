`ifdef SRAM_INIT
    localparam HEXFILE = "firmware";
`else
    localparam HEXFILE = "none";
`endif

    // sram data ports
    wire                      sram_enA;
    wire [`SRAM_ADDR_W-3:0]   sram_addrA;
    wire [`DATA_W/8-1:0]      sram_weA;
    wire [`DATA_W-1:0]        sram_dinA;
    wire [`DATA_W-1:0]        sram_doutA;

    // sram instruction ports
    wire                      sram_enB;
    wire [`SRAM_ADDR_W-3:0]   sram_addrB;
    wire [`DATA_W/8-1:0]      sram_weB;
    wire [`DATA_W-1:0]        sram_dinB;
    wire [`DATA_W-1:0]        sram_doutB;

    iob_ram_dp_be
     #(
       .HEXFILE(HEXFILE),
       .ADDR_W(`SRAM_ADDR_W-2),
       .DATA_W(`DATA_W)
       )
    main_mem_byte
     (
      .clk   (clk),

      // data port
      .enA   (sram_enA),
      .addrA (sram_addrA),
      .weA   (sram_weA),
      .dinA  (sram_dinA),
      .doutA (sram_doutA),

      // instruction port
      .enB   (sram_enB),
      .addrB (sram_addrB),
      .weB   (sram_weB),
      .dinB  (sram_dinB),
      .doutB (sram_doutB)
      );
