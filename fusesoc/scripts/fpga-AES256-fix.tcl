# Auto-generated project tcl file

# close previous project
close_project -delete

# remove previous project vivado files
file delete -force -- .Xil
file delete -force -- reports
file delete -force -- iobundle_opencryptohw_0.0.1_0.cache
file delete -force -- iobundle_opencryptohw_0.0.1_0.hw
file delete -force -- iobundle_opencryptohw_0.0.1_0.ip_user_files
file delete -force -- iobundle_opencryptohw_0.0.1_0.xpr
file delete -force -- ip
file delete -force {*}[glob -nocomplain vivado*]

create_project iobundle_opencryptohw_0.0.1_0 -force

read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/iob_merge/iob_merge.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/iob_split/iob_split.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/iob_pulse_gen/iob_pulse_gen.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/iob_edge_detect/iob_edge_detect.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/rom/iob_rom_sp/iob_rom_sp.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_dp/iob_ram_dp.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_dp_be/iob_ram_dp_be.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/PICORV32/hardware/src/picorv32.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/PICORV32/hardware/src/iob_picorv32.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/regfile/iob_regfile_sp/iob_regfile_sp.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_2p_asym/iob_ram_2p_asym.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/fifo/iob_fifo_sync/iob_fifo_sync.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_2p/iob_ram_2p.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_sp/iob_ram_sp.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/back-end-axi.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/back-end-native.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/cache-control.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/cache-memory.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/front-end.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/iob-cache-axi.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/iob_cache.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/onehot-to-bin-encoder.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/read-channel-axi.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/read-channel-native.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/replacement-policy.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/write-channel-axi.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/src/write-channel-native.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/iob_reg/iob_reg.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/UART/hardware/src/uart_core.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/UART/hardware/src/iob_uart.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/TIMER/hardware/src/iob_timer.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/TIMER/hardware/src/timer.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/src/iob_eth_crc.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/src/iob_eth_rx.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/src/iob_eth_tb_gen.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/src/iob_eth_tx.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/src/iob_eth.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/src/mem_burst.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_tdp/iob_ram_tdp.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/MEM/hardware/ram/iob_ram_tdp_be/iob_ram_tdp_be.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/submodules/MEM/hardware/ram/tdp_ram/iob_tdp_ram.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/submodules/MEM/hardware/ram/2p_ram/iob_2p_ram.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/submodules/MEM/hardware/ram/dp_ram/iob_dp_ram.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/submodules/MEM/hardware/fifo/sfifo/iob_sync_fifo.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/submodules/MEM/hardware/fifo/bin_counter.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/AXIBoundary.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/AxiDelay.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Buffer.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Const.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/ext_addrgen.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/FixedBuffer.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/iob_versat.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/LookupTable.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/MemoryReader.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/MemoryWriter.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Mem.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Merge.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Muladd.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Mul.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Mux2.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/MyAddressGen.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/PipelineRegister.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/Reg.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/SwapEndian.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/VRead.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/VWrite.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/xaddrgen2.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/xaddrgen.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/xalulite.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/src/xmux4.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/src/ext_mem.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/src/boot_ctr.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/src/int_mem.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/src/sram.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/src/units/xunitF.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/src/units/xunitM.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/hardware/fpga/vivado/AES-KU040-DB-G/verilog/top_system.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/iob_reset_sync/iob_reset_sync.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-system_gen_hw_aes256_0/system.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/versat_instance.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/AddRoundKey.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/AES.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/CH.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/Comb_F_Stage.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ComplexAdder.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ComplexMultiplier.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ComplexShareConfig.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/Constants.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ConvolutionStage.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/Convolution.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/DoRow.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/FourthLineKey.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/FirstLineKey.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/F_Stage.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/F.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/KeySchedule256.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/KeySchedule.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/MainRound.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/Maj.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/MatrixMultiplication.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/MatrixMultiplicationVread.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/MixColumns.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/MixProduct.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/M_Stage.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/M.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/OnlyInputToOutput.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ReadWriteAES.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/SBox.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/SemiComplexAdder.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ShaSingleState.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ShaState.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/SHA.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/ShiftRows.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/sigma_stage.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/sigma.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/Sigma.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/SimpleAdder.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/SimpleShareConfig.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/StaticConst.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/StaticMuladd.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/StaticMux.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/StringHasher.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/T1.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/T2.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/T_Stage.v}
read_verilog -sv {../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul/src/VReadToVWrite.v}

set_property part xcku040-fbva676-1-c [current_project]

read_xdc {../src/iobundle_opencryptohw_0.0.1_0/hardware/fpga/vivado/AES-KU040-DB-G/top_system.xdc}
source {../src/iobundle_opencryptohw_0.0.1_0/hardware/fpga/vivado/ddr_ip.tcl}

set_property include_dirs [list ../src/iobundle_opencryptohw_0.0.1_0/submodules/CACHE/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/submodules/UART/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/submodules/LIB/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/submodules/TIMER/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/submodules/ETHERNET/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/submodules/AXI/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/submodules/VERSAT/hardware/include ../src/iobundle_opencryptohw_0.0.1_0/hardware/include ../src/iobundle_opencryptohw_0.0.1-cache_if_gen_0 ../src/iobundle_opencryptohw_0.0.1-uart_mkregs_gen_hw_0 ../src/iobundle_opencryptohw_0.0.1-timer_mkregs_gen_hw_0 ../src/iobundle_opencryptohw_0.0.1-ethernet_mkregs_gen_hw_0 ../src/iobundle_opencryptohw_0.0.1-versat_if_gen_0 ../src/iobundle_opencryptohw_0.0.1-system_gen_hw_aes256_0 ../src/iobundle_opencryptohw_0.0.1-versat_gen_aes256_0/software/pc-emul .] [get_filesets sources_1]

set_property verilog_define {RUN_EXTMEM=1 USE_DDR=1 INIT_MEM=0 DATA_W=32 ADDR_W=32 BOOTROM_ADDR_W=12 SRAM_ADDR_W=12 FIRM_ADDR_W=18 DCACHE_ADDR_W=24 N_SLAVES=4 E=31 P=30 B=29 UART=0 TIMER=1 VERSAT=2 ETHERNET=3 N_SLAVES_W=2 HARDWARE_TEST=10 USE_MUL_DIV=1 USE_COMPRESSED=1 DDR_DATA_W=32 DDR_ADDR_W=30 AXI_ADDR_W=32 } [get_filesets sources_1]

set_property top top_system [current_fileset]
# set_property source_mgmt_mode None [current_project]

file mkdir reports

synth_design -debug_log -verbose
report_utilization -hierarchical -file reports/synth_utilization.txt

opt_design -debug_log -verbose
report_timing_summary -file reports/opt_timing.txt -max_paths 3000

place_design

route_design

report_timing -file reports/timing.txt -max_paths 3000
report_clocks -file reports/clocks.txt
report_clock_interaction -file reports/clock_interaction.txt
report_cdc -details -file reports/cdc.txt
report_synchronizer_mtbf -file reports/synchronizer_mtbf.txt
report_utilization -hierarchical -file reports/utilization.txt

write_bitstream -force iobundle_opencryptohw_0.0.1_0.bit
