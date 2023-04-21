#!/usr/bin/env bash

# grab board again, do not program, but run console program
python3 ./software/python/board_client.py grab 600 -p 'echo dummy program command' -c 'python3 ./software/console/eth_console -s /dev/usb-uart -f'
