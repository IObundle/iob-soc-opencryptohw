#!/usr/bin/env bash
# Variables
FIRM_DIR="software/firmware"
BOOT_DIR="software/bootloader"
PYTHON_DIR="submodules/LIB/software/python"
BOOTROM_ADDR_W=12
FIRM_ADDR_W=17
# Build versat verilog sources
make pc-emul-gen-versat
# Build simulation firmware and bootloader
make fw-build BAUD=115200 FREQ=100000000
# Copy firmware binary to top level
cp -u $FIRM_DIR/firmware.bin .
# Create firmware.hex from bin
$PYTHON_DIR/makehex.py $FIRM_DIR/firmware.bin $FIRM_ADDR_W > firmware.hex
# Create boot.hex from bin
$PYTHON_DIR/makehex.py $BOOT_DIR/boot.bin $BOOTROM_ADDR_W > boot.hex
