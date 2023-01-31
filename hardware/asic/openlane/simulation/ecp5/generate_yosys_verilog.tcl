set PROJECT_NAME system
set INCLUDE_PATH [lindex $argv 0]
set DEFINE [lindex $argv 1]
set VSRC [lindex $argv 2]
set VERILOG_OUTPUT [lindex $argv 3]

#debug purposes
puts "INCLUDE_PATH: $INCLUDE_PATH \n"
puts "DEFINE: $DEFINE \n"
puts "VSRC: $VSRC \n"

set DEFINE [string map {-D ""} $DEFINE] ;#remove "-D" from defines (yosys does not need it)
# set VSRC [lrange $VSRC 0 end-1] ;# we dont want to synthesize "system_tb.v"

puts "\n-> Synthesizing design...\n"
set yosys_script "$PROJECT_NAME.ys"
set yosys_script_handle [open $yosys_script "w"]
puts $yosys_script_handle "read -define USE_SPRAM $DEFINE \n"
puts $yosys_script_handle "verilog_defaults -add $INCLUDE_PATH \n"
puts $yosys_script_handle "read_verilog $VSRC \n"
puts $yosys_script_handle "synth_ecp5 -top $PROJECT_NAME -json $PROJECT_NAME.json \n"
puts $yosys_script_handle "write_verilog $VERILOG_OUTPUT \n"
close $yosys_script_handle
exec yosys -T $yosys_script -q -q -t -l "${PROJECT_NAME}_synthesis.log"
