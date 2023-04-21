#!/usr/bin/env bash

# grab board + skip programing
# use this only to wait for board to be idle
python3 ./software/python/board_client.py grab 600 -p 'echo dummy program command' -c 'echo dummy console command'
