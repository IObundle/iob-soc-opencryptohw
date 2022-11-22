#!/usr/bin/env bash
# Variables
ROOT_DIR=".."
PC_DIR="$ROOT_DIR/software/pc-emul"
# Build versat verilog sources
make -C $ROOT_DIR/ pc-emul-gen-versat
# Copy pc-emul binary to top level
cp -u $PC_DIR/versat_instance.v .
cp -u $PC_DIR/versat_defs.vh .
cp -u $PC_DIR/src/* .
