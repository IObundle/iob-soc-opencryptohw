#!/usr/bin/env bash
# Variables
FIRM_DIR="software/firmware"
BOOT_DIR="software/bootloader"
PYTHON_DIR="submodules/LIB/software/python"
BOOTROM_ADDR_W=12
# Build versat verilog sources
make pc-emul-gen-versat
# Build simulation firmware and bootloader
make fw-build SIM=1
# Copy firmware binary to top level
cp -u $FIRM_DIR/firmware.bin .
# Create boot.hex from bin
$PYTHON_DIR/makehex.py $BOOT_DIR/boot.bin $BOOTROM_ADDR_W > boot.hex
