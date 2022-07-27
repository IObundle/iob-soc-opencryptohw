#
# SYNTHESIS AND IMPLEMENTATION SCRIPT
#

#select top module and FPGA decive
set TOP iob_versat
set DEVICE [lindex $argv 3]

set INCLUDE [lindex $argv 0]
set DEFINE [lindex $argv 1]
set VSRC [lindex $argv 2]

#verilog sources
foreach file [split $VSRC \ ] {
    if {$file != ""} {
        read_verilog -sv $file
    }
}

set_property part $DEVICE [current_project]

read_xdc ./top_system.xdc

synth_design -include_dirs $INCLUDE -verilog_define $DEFINE -part $DEVICE -top $TOP

opt_design

place_design

route_design

report_utilization

report_timing

report_clocks

write_edif -force $TOP.edif
set TOP_STUB $TOP
append TOP_STUB "_stub"
write_verilog -force -mode synth_stub $TOP_STUB.v
