#!/usr/bin/env bash
# Variables
AXI_GEN="./submodules/LIB/software/python/axi_gen.py"
# Generate axi ram portmap
$AXI_GEN axi_portmap 's_' 's_' 'm_'
# Generate system_tb.v and system_top.v
make -f hardware/simulation/simulation.mk system_top.v ROOT_DIR=.
make -f hardware/simulation/simulation.mk system_tb.v ROOT_DIR=.
