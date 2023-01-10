   wire bootrom_r_en;
   wire [`BOOTROM_ADDR_W-2-1:0] bootrom_addr;
   wire [`DATA_W-1:0] bootrom_r_data;

   //
   //INSTANTIATE ROM
   //
   iob_rom_sp
     #(
       .DATA_W(`DATA_W),
       .ADDR_W(`BOOTROM_ADDR_W-2),
       .HEXFILE("boot.hex")
       )
   sp_rom0 
     (
      .clk(clk),
      .r_en(bootrom_r_en),
      .addr(bootrom_addr),
      .r_data(bootrom_r_data)
      );
