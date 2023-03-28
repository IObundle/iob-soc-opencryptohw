#!/usr/bin/env python3

import sys
import os
import math

MCELIECE348864_PUBLICKEYBYTES = 261120
MCELIECE348864_SECRETKEYBYTES = 6452

if __name__ == "__main__":
    valid_fname = sys.argv[1]
    try:
        valid_fname = sys.argv[1]
        eval_fname = sys.argv[2]
    except IndexError:
        print("Usage: valitate_test.py [valid_file] [eval_file] [block_size]")
        print("valid_file: file with validation/expected data")
        print("eval_file: file to evaluate")
        exit()

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

    pk_size = MCELIECE348864_PUBLICKEYBYTES
    sk_size = MCELIECE348864_SECRETKEYBYTES
    num_keypairs = math.ceil(min_fsize / (pk_size + sk_size))

    print("\n=== Ciphertext Validation ===")
    print(f"Min File size: {min_fsize} bytes. {num_keypairs} keypairs")

    for i in range(num_keypairs):
        # validate public key
        valid_pk = f_valid.read(pk_size)
        eval_pk = f_eval.read(pk_size)
        if valid_pk != eval_pk:
            print(f"Public keys {i} differ:")
            print(f"Valid: {valid_pk.hex()}")
            print(f"Eval: {eval_pk.hex()}\n")
            failed_validation = 1
        # validate secret key
        valid_sk = f_valid.read(sk_size)
        eval_sk = f_eval.read(sk_size)
        if valid_sk != eval_sk:
            print(f"Secret keys {i} differ:")
            print(f"Valid: {valid_sk.hex()}")
            print(f"Eval: {eval_sk.hex()}\n")
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
