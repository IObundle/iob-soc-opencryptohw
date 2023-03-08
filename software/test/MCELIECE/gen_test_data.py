#!/usr/bin/env python3

import sys
import parse

MCELIECE_SEEDBYTES = 48
MCELIECE348864_PUBLICKEYBYTES = 261120
MCELIECE348864_SECRETKEYBYTES = 6452
# MCELIECE348864_SECRETKEYBYTES = 6492


def write_hex_str_to_file(f, hex_str, strlen=-1):
    """Write hex string to file
    Write hex string as binary data to file
    Pad bytes to next multiple of 4

    # Parameters:
    #   f: output file
    #   hex_str: string in hex format
    #   strlen: byte limit to write (-1 = all string by default) (optional)
    # Returns:
    #   no return arguments
    """
    if strlen > -1:
        hex_str = hex_str[: (strlen * 2)]
    byte_msg = bytes.fromhex(hex_str)
    padding = (4 - (len(byte_msg) % 4)) % 4
    padded_msg = byte_msg.ljust(len(byte_msg) + padding, b"\0")

    f.write(padded_msg)


def write_d_in_file(f, seeds):
    """Write Data in binary file
    Data is written in binary format with the following structure:
    #seed
    | seed0 (bytes)
    | seed1 (bytes)
    ...
    | seedN-1 (bytes)

    Note: assume all seeds with MCELIECE_SEEDBYTES 48

    # Parameters:
    #   f: output file
    #   seeds: public keys (hex format)
    # Returns:
    #   no return arguments
    """
    integer_bytes = 4

    # get number of seeds
    num_texts = len(seeds)

    # count = 0
    # print(f'num texts: {num_texts}')
    f.write(num_texts.to_bytes(integer_bytes, byteorder="little"))
    for seed in seeds:
        # write public key in bytes
        write_hex_str_to_file(f, seed, MCELIECE_SEEDBYTES)
        # print(f"count = {count}")
        # print(f"seed = {seed}")
        # count = count + 1


def write_d_out_file(f, pkeys, skeys):
    """Write Data in binary file
    Data is written in binary format with the following structure:
    pk0     | sk0
    pk1     | sk1
    ...     | ...
    pkN-1   | skN-1

    # Parameters:
    #   f: output file
    #   pkeys: list of public keys (hex format)
    #   skeys: list of secret keys (hex format)
    # Returns:
    #   no return arguments
    """

    if len(pkeys) != len(skeys):
        print("Missmatch between #public keys and #secret keys")
        print("Using min value between public and secret and keys")

    count = 0
    for pk, sk in zip(pkeys, skeys):
        # # write public key
        write_hex_str_to_file(f, pk, MCELIECE348864_PUBLICKEYBYTES)
        # # write secret key
        write_hex_str_to_file(f, sk, MCELIECE348864_SECRETKEYBYTES)
        # print(f"count = {count}")
        # print(f"pk = {pk}")
        # print(f"sk = {sk}")
        # count = count + 1


def main():
    try:
        REQUEST_file = sys.argv[1]
    except IndexError:
        print("Usage: gen_test_data.py [REQUEST_file]")
        print("REQUEST_file: file with test dataset")
        print("Outputs: data_in.bin data_out.bin")
        print("data_in.bin: input data in binary")
        print("data_out.bin: output data in binary")
        exit()

    # Strings to parse
    seed_line = "seed ="
    pk_line = "pk ="
    sk_line = "sk ="

    # Variables
    seed_arrays = []
    pk_arrays = []
    sk_arrays = []

    # Open output files
    d_in_fname = parse.parse("{}.rsp", REQUEST_file)[0] + "_d_in.bin"
    d_out_fname = parse.parse("{}.rsp", REQUEST_file)[0] + "_d_out.bin"
    try:
        f_din = open(d_in_fname, "wb")
        f_dout = open(d_out_fname, "wb")
    except OSError:
        print("Failed to open output files")
        sys.exit()

    # Parse input file
    with open(REQUEST_file, "r") as req_file:
        for line in req_file:
            if seed_line in line:
                seed = line.split(" ")[-1].strip()
                seed_arrays.append(seed)
            elif pk_line in line:
                pk = line.split(" ")[-1].strip()
                pk_arrays.append(pk)
            elif sk_line in line:
                sk = line.split(" ")[-1].strip()
                sk_arrays.append(sk)

    # Generate output files
    write_d_in_file(f_din, seed_arrays)
    write_d_out_file(f_dout, pk_arrays, sk_arrays)

    # Close output files
    f_din.close()
    f_dout.close()


if __name__ == "__main__":
    main()
