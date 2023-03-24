#!/usr/bin/env bash

PYTHON_DIR=software/python

# release function
release () {
    $PYTHON_DIR/board_client.py release $USER
}

# release on INT TERM and KILL interrupts
trap "release exit" INT TERM KILL

# grab board
while [[ $($PYTHON_DIR/board_client.py grab $USER | grep 'busy' --color=never) != "" ]]
do
    echo "Board is busy, waiting..."
    sleep 10;
done
