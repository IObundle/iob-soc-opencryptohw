#!/usr/bin/env bash

CORE=iobundle:opencryptohw:0.0.1
case "$1" in
    "sim-setup")
    echo "fusesoc run --target=sim --setup $CORE"
    ;;
    "sim-build")
    echo "fusesoc run --target=sim --setup --build $CORE"
    ;;
    "sim")
    echo "fusesoc run --target=sim $CORE"
    ;;
    "fpga-setup")
    echo "fusesoc run --target=fpga --setup $CORE"
    ;;
    "fpga-build")
    echo "fusesoc run --target=fpga --setup --build $CORE"
    ;;
    "fpga")
    echo "fusesoc run --target=fpga $CORE"
    ;;
    *)
    echo "Supported arguments: "
    echo "          sim-setup sim-build sim"
    echo "          fpga-setup fpga-build fpga"
    exit 1
    ;;
esac

