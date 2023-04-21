#!/usr/bin/env bash

PYTHON_DIR=software/python

# grab board + skip programing + console command
python3 ./software/python/board_client.py grab 600 -p 'echo prog.sh' -c 'python3 ./software/console/eth_console -s /dev/usb-uart -f'
