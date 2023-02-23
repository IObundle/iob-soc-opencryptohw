#!/usr/bin/env bash
# Generate Firmware for simulation
# Usage: ./path/to/firmware_sim_gen.sh ALGORITHM={SHA256,AES256}

# Variables
FIRM_DIR="software/firmware"
BOOT_DIR="software/bootloader"
PYTHON_DIR="submodules/LIB/software/python"
BOOTROM_ADDR_W=12
FIRM_ADDR_W=18
# Build versat verilog sources
make pc-emul-gen-versat $1
# Build simulation firmware and bootloader
make fw-build SIM=1 $1
# Copy firmware binary to top level
cp -u $FIRM_DIR/firmware.bin .
# Create firmware.hex from bin
$PYTHON_DIR/makehex.py $FIRM_DIR/firmware.bin $FIRM_ADDR_W > firmware.hex
# Create boot.hex from bin
$PYTHON_DIR/makehex.py $BOOT_DIR/boot.bin $BOOTROM_ADDR_W > boot.hex
