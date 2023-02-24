#!/usr/bin/env python3

import sys
import os
import math

if __name__ == "__main__":
    valid_fname = sys.argv[1]
    try:
        valid_fname = sys.argv[1]
        eval_fname = sys.argv[2]
    except IndexError:
        print("Usage: valitate_test.py [valid_file] [eval_file] [block_size]")
        print("valid_file: file with validation/expected data")
        print("eval_file: file to evaluate")
        print("block_size: block size (in bytes) to evaluate at each time")
        exit()

    try:
        ctext_size = int(sys.argv[3])
    except IndexError:
        ctext_size = 16

    failed_validation = 0

    # Open files
    try:
        f_valid = open(valid_fname, "rb")
        f_eval = open(eval_fname, "rb")
    except OSError:
        print("Failed to open files")
        sys.exit(1)

    # Min file size
    min_fsize = min(os.path.getsize(valid_fname), os.path.getsize(eval_fname))

    num_ctexts = math.ceil(min_fsize / ctext_size)

    print("\n=== Ciphertext Validation ===")
    print(
        f"Min File size: {min_fsize} bytes. {num_ctexts} ciphertexts of {ctext_size} bytes"
    )

    for i in range(num_ctexts):
        valid_ctext = f_valid.read(ctext_size)
        eval_ctext = f_eval.read(ctext_size)
        if valid_ctext != eval_ctext:
            print(f"Ciphertexts {i} differ:")
            print(f"Valid: {valid_ctext.hex()}")
            print(f"Eval: {eval_ctext.hex()}\n")
            failed_validation = 1

    # Close output files
    f_valid.close()
    f_eval.close()

    if failed_validation == 0:
        print("SUCCESS: eval and validation file match")
    else:
        print("FAILURE: eval and validation file differ")
    print("=================================\n")

    sys.exit(failed_validation)
