#!/usr/bin/env bash
# Variables
AXI_GEN="./submodules/LIB/software/python/axi_gen.py"
# Generate versat axi interfaces
$AXI_GEN axi_m_port 'm_versat_' 'm_'
$AXI_GEN axi_m_write_port 'm_versat_' 'm_'
$AXI_GEN axi_m_read_port 'm_versat_' 'm_'
$AXI_GEN axi_write_portmap 'm_versat_' 'm_' 'm_'
$AXI_GEN axi_read_portmap 'm_versat_' 'm_' 'm_'
