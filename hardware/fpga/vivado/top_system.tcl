#
# SYNTHESIS AND IMPLEMENTATION SCRIPT
#

#select top module and FPGA decive
set TOP top_system
set INCLUDE [lindex $argv 0]
set DEFINE [lindex $argv 1]
set VSRC [lindex $argv 2]
set DEVICE [lindex $argv 3]

set USE_DDR [string last "USE_DDR" $DEFINE]

#verilog sources
foreach file [split $VSRC \ ] {
    if {$file != ""} {
        read_verilog -sv $file
    }
}

set_property part $DEVICE [current_project]
read_xdc ./top_system.xdc

if { $USE_DDR < 0 } {
    read_verilog verilog/clock_wizard.v
} else {
    source ../ddr_ip.tcl
}

file mkdir reports
file mkdir checkpoints

synth_design -include_dirs $INCLUDE -verilog_define $DEFINE -part $DEVICE -top $TOP -debug_log -verbose
report_utilization -hierarchical -file reports/synth_utilization.txt
write_checkpoint -force checkpoints/post_synth

#start_gui

opt_design -debug_log -verbose
report_timing_summary -file reports/opt_timing.txt -max_paths 3000

place_design
write_checkpoint -force checkpoints/post_place

route_design
write_checkpoint -force checkpoints/post_route

report_timing -file reports/timing.txt -max_paths 3000
report_clocks -file reports/clocks.txt
report_clock_interaction -file reports/clock_interaction.txt
report_cdc -details -file reports/cdc.txt
report_synchronizer_mtbf -file reports/synchronizer_mtbf.txt
report_utilization -hierarchical -file reports/utilization.txt

write_bitstream -force top_system.bit

write_verilog -force top_system.v
