#!/usr/bin/env python3

import sys
import os
import math

if __name__ == "__main__":
    try:
        valid_fname = sys.argv[1]
        eval_fname = sys.argv[2]
    except:
        print("Usage: valitate_test.py [valid_file] [eval_file] [block_size]")
        print("valid_file: file with validation/expected data")
        print("eval_file: file to evaluate")
        print("block_size: block size (in bytes) to evaluate at each time")
        exit()

    try:
        blk_size = int(sys.argv[3])
    except:
        blk_size = 32

    failed_validation = 0

    # Open files
    try:
        f_valid = open(valid_fname, 'rb')
        f_eval = open(eval_fname, 'rb')
    except:
        print("Failed to open files")
        sys.exit(1)

    # Min file size
    min_fsize = min(os.path.getsize(valid_fname), os.path.getsize(eval_fname))

    num_blks = math.ceil(min_fsize/blk_size)

    print("\n=== Message Digest Validation ===")
    print(f'Min File size: {min_fsize} bytes. {num_blks} blocks of {blk_size} bytes')

    for i in range(num_blks):
        valid_blk = f_valid.read(blk_size)
        eval_blk = f_eval.read(blk_size)
        if valid_blk != eval_blk:
            print(f'Blk {i} differ:')
            print(f'Valid: {valid_blk.hex()}')
            print(f'Eval: {eval_blk.hex()}\n')
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
