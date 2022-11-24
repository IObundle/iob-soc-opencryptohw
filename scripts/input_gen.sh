#!/usr/bin/env bash
# Variables
TEST_DIR="software/test"
TEST_VECTOR_RSP=SHA256ShortMsg
# Generate soc-in.bin
make -C $TEST_DIR gen_test_data
# Copy to root dir
cp $TEST_DIR/*_d_in.bin soc-in.bin
cp $TEST_DIR/*_d_out.bin "$TEST_VECTOR_RSP"_d_out.bin
