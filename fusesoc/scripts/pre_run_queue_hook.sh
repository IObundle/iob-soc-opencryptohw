#!/usr/bin/env bash

BOARD=AES-KU040-DB-G
LOAD_FILE=/tmp/$BOARD.load
QUEUE_FILE=/tmp/$BOARD.queue

FPGA_OBJ=$(ls *.bit)
JOB=$(echo $USER `md5sum $FPGA_OBJ | cut -d" " -f1`)

# Queue in function
queue_in () {
	if [ ! -f $QUEUE_FILE ] 
    then 
        touch $QUEUE_FILE 
        chown $USER:dialout $QUEUE_FILE 
        chmod 664 $QUEUE_FILE 
    fi
	if [ "`head -1 $QUEUE_FILE`" != "$JOB" ] 
    then 
        echo $JOB >> $QUEUE_FILE 
    fi
    # queue_out on INT TERM and KILL interrupts
    trap "queue_out exit" INT TERM KILL
    queue_wait
}

# Queue wait function
queue_wait () {
	while [ "`head -1 $QUEUE_FILE`" != "$JOB" ] 
    do 
        echo "Job queued for board access. Queue length: `wc -l $QUEUE_FILE | cut -d" " -f1`" 
        sleep 10s
    done
}

# Queue out function
queue_out () {
	sed '/$JOB/d' $QUEUE_FILE > queue 
    cat queue > $QUEUE_FILE 
    rm queue
}

# Check for LOAD_FILE
if [ ! -f $LOAD_FILE ] 
then 
    touch $LOAD_FILE 
    chown $USER:dialout $LOAD_FILE 
    chmod 664 $LOAD_FILE 
fi
queue_in

