    // sram data ports
    output                      sram_enA,
    output [`SRAM_ADDR_W-3:0]   sram_addrA,
    output [`DATA_W/8-1:0]      sram_weA,
    output [`DATA_W-1:0]        sram_dinA,
    input [`DATA_W-1:0]         sram_doutA,

    // sram instruction ports
    output sram_enB,
    output [`SRAM_ADDR_W-3:0]   sram_addrB,
    output [`DATA_W/8-1:0]      sram_weB,
    output [`DATA_W-1:0]        sram_dinB,
    input [`DATA_W-1:0]         sram_doutB,
