#!/usr/bin/env bash
# Validate test results
# Usage: ./path/to/validate_hook.sh {SHA256,AES256}

# Variables
if [[ "$1" == "SHA256" ]]
then
    VALIDATION_OUT_BIN=SHA256ShortMsg_d_out.bin
elif [[ "$1" == "AES256" ]]
then
    VALIDATION_OUT_BIN=AESECB256_d_out.bin
else
    VALIDATION_OUT_BIN=""
fi

SOC_OUT_BIN=soc-out.bin

# SOC_OUT_BIN in sha256_test.py directory
SOC_OUT_BIN=$(find . -name $SOC_OUT_BIN | head -n 1)

# Validate test
if python3 validate_test.py $VALIDATION_OUT_BIN $SOC_OUT_BIN
then 
    printf "\n\nShortMessage Test PASSED\n\n"; 
else 
    printf "\n\nShortMessage Test FAILED\n\n"; 
    exit 1; 
fi;
