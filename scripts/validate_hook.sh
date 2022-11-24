#!/usr/bin/env bash
# Variables
VALIDATION_OUT_BIN=SHA256ShortMsg_d_out.bin
SOC_OUT_BIN=soc-out.bin
if python validate_test.py $VALIDATION_OUT_BIN $SOC_OUT_BIN
then 
    printf "\n\nShortMessage Test PASSED\n\n"; 
else 
    printf "\n\nShortMessage Test FAILED\n\n"; 
    exit 1; 
fi;
