#!/usr/bin/env bash
# Console FPGA Hook
# Usage: ./path/to/console_fpga_hook.sh {SHA256,AES256,MCELIECE}

# grab board again, do not program, but run console program
if [[ "$1" == "AES256" ]]
then
python3 ./software/python/board_client.py grab 600 -p 'echo dummy program command' -c 'python3 ./submodules/LIB/software/python/console -s /dev/usb-uart -f'
else
python3 ./software/python/board_client.py grab 600 -p 'echo dummy program command' -c 'python3 ./software/console/eth_console -s /dev/usb-uart -f'
fi
