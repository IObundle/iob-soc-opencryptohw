#!/usr/bin/env bash

PYTHON_DIR=software/python

# Run console
python3 ./software/console/eth_console -s /dev/usb-uart -f
# Release board (script from iob-lib must be installed)
board_client.py release
