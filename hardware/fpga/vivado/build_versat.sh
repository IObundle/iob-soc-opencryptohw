#!/bin/bash
set -e
source $VIVADOPATH/settings64.sh
vivado -nojournal -log versat.log -mode batch -source ../versat.tcl -tclargs "$1" "$2" "$3" "$4"
