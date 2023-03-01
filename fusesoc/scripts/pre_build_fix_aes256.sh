#!/usr/bin/env bash
# Pre Build Fix for AES 256 Flow
# build bitstream in non-project mode

VIVADOPATH=/opt/Xilinx/Vivado/2020.2

source $VIVADOPATH/settings64.sh
vivado -nojournal -log vivado.log -mode batch -source fpga-AES256-fix.tcl
# cp top_system.bit iobundle_opencryptohw_0.0.1_0.bit
