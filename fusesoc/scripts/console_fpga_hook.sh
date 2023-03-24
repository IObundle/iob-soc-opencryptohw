#!/usr/bin/env bash

PYTHON_DIR=software/python

# release function
release () {
    python3 $PYTHON_DIR/board_client.py release $USER
}

# Run console
python3 ./software/console/eth_console -s /dev/usb-uart -f
# Release board
release
