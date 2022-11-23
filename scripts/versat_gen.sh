#!/usr/bin/env bash
# Variables
PC_DIR="software/pc-emul"
# Build versat verilog sources
make pc-emul-gen-versat
# Copy versat sources to top level
cp -u $PC_DIR/versat_instance.v .
cp -u $PC_DIR/versat_defs.vh .
cp -u $PC_DIR/src/* .
