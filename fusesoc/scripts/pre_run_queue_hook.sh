#!/usr/bin/env bash

PYTHON_DIR=software/python

# release on INT TERM and KILL interrupts
# board_client.py installed in system from (iob-lib)
trap "board_client.py release exit" INT TERM KILL

# grab board
board_client.py grab 180
