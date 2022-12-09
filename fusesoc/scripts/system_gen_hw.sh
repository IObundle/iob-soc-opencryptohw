#!/usr/bin/env bash
# Variables
AXI_GEN="./submodules/LIB/software/python/axi_gen.py"
# Generate system interface
$AXI_GEN axi_wire 'm_' 'm_' 'm_'
# Generate system.v
make -f hardware/hardware.mk system.v ROOT_DIR=.
