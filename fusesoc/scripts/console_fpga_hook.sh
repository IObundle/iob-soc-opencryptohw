#!/usr/bin/env bash

BOARD=AES-KU040-DB-G
LOAD_FILE=/tmp/$BOARD.load
QUEUE_FILE=/tmp/$BOARD.queue

FPGA_OBJ=$(ls *.bit)
JOB=$(echo $USER `md5sum $FPGA_OBJ  | cut -d" " -f1`)

# Queue out function
queue_out () {
	sed '/$JOB/d' $QUEUE_FILE > queue; 
    cat queue > $QUEUE_FILE; 
    rm queue
}

# Load file
echo $JOB > $LOAD_FILE;
# Run console
python3 ./software/console/eth_console -s /dev/usb-uart -f
# Remove from queue
queue_out
