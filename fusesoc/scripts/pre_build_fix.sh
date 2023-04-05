#!/usr/bin/env bash
# Pre Build Fix for FuseSoC FPGA Flows
# Replace vivado tcl script
# Usage: ./path/to/pre_build_fix.sh {AES256,MCELIECE}

cp fpga-$1-fix.tcl iobundle_opencryptohw_0.0.1_0_run.tcl
