#!/usr/bin/env python3

import sys
import parse

AES_BLK_SIZE = 128

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


def write_d_in_file(f, ptexts, keys, keysize=128):
    """Write Data in binary file
    Data is written in binary format with the following structure:
    #plaintext / key pairs
    | ptext0 (bytes)   | key0 (bytes)
    | ptext1 (bytes)   | key1 (bytes)
    ...
    | ptextN-1 (bytes) | keyN-1 (bytes)

    Note: assume all ptexts with 128bit length (1 AES block)

    # Parameters:
    #   f: output file
    #   ptexts: plaintexts (hex format)
    #   keys: keys (hex format)
    #   keysize: keysize in bits (optional)
    # Returns:
    #   no return arguments
    """
    integer_bytes = 4

    if len(ptexts) != len(keys):
        print("Missmatch between #plaintexts and #keys")
        print("Using min value between plaintexts and keys")

    # get min value
    num_texts = min(len(ptexts), len(keys))

    count = 0
    # print(f'num texts: {num_texts}')
    f.write(num_texts.to_bytes(integer_bytes, byteorder="little"))
    for ptext, key in zip(ptexts, keys):
        # write plaintext in bytes
        write_hex_str_to_file(f, ptext, AES_BLK_SIZE // 8)
        # write key in bytes
        write_hex_str_to_file(f, key, keysize // 8)
        # print(f"COUNT = {count}")
        # print(f"KEY = {key}")
        # print(f"PLAINTEXT = {ptext}")
        # print("CIPHERTEXT\n") 
        count = count + 1


def write_d_out_file(f, ciphertexts):
    """Write Data in binary file
    Data is written in binary format with the following structure:
    ciphertext0 | ciphertext1 | ... | ciphertextsN-1

    # Parameters:
    #   f: output file
    #   ciphertexts: list of ciphertexts (hex format)
    # Returns:
    #   no return arguments
    """

    for ctext in ciphertexts:
        # write ciphertext
        write_hex_str_to_file(f, ctext, AES_BLK_SIZE // 8)


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

    # keysize (AES128 = 128, AES256 = 256)
    keysize = 256

    # Strings to parse
    plaintext_line = "PLAINTEXT ="
    key_line = "KEY ="
    ciphertext_line = "CIPHERTEXT ="

    # Variables
    plaintext_arrays = []
    key_arrays = []
    ciphertext_arrays = []

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
            if plaintext_line in line:
                plaintext = line.split(" ")[-1].strip()
                plaintext_arrays.append(plaintext)
            elif ciphertext_line in line:
                ciphertext = line.split(" ")[-1].strip()
                ciphertext_arrays.append(ciphertext)
            elif key_line in line:
                key = line.split(" ")[-1].strip()
                key_arrays.append(key)

    # Generate output files
    write_d_in_file(f_din, plaintext_arrays, key_arrays, keysize)
    write_d_out_file(f_dout, ciphertext_arrays)

    # Close output files
    f_din.close()
    f_dout.close()


if __name__ == "__main__":
    main()
