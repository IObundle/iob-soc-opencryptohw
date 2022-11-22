#!/usr/bin/env bash
# Variables
TEST_DIR="software/test"
# Generate soc-in.bin
make -C $TEST_DIR gen_test_data
# Copy to root dir
cp $TEST_DIR/*_d_in.bin soc-in.bin
