#!/usr/bin/env bash
# Variables
AXI_GEN="./submodules/LIB/software/python/axi_gen.py"
# Generate cache interfaces
$AXI_GEN axi_m_port 'm_' 'm_'
$AXI_GEN axi_portmap 'm_' 'm_' 'm_'
$AXI_GEN axi_m_write_port 'm_' 'm_'
$AXI_GEN axi_write_portmap 'm_' 'm_' 'm_'
$AXI_GEN axi_m_read_port 'm_' 'm_'
$AXI_GEN axi_read_portmap 'm_' 'm_' 'm_'
