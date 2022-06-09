#!/usr/bin/env python3

import string
import sys
import parse

def write_hex_str_to_file(f, hex_str):
    """Write hex string to file
    Write hex string as binary data to file

    # Parameters:
    #   f: output file
    #   hex_str: string in hex format
    # Returns:
    #   no return arguments
    """
    f.write(bytes.fromhex(hex_str))

def write_d_in_file(f, lengths, msgs):
    """Write Data in binary file
    Data is written in binary format with the following structure:
    #Messages | length msg0 (bytes) | msg0 (bytes) | length msg1 | msg1 | ...
    ... | length msgN-1 | msgN-1

    # Parameters:
    #   f: output file
    #   lengths: lenghts of messages (in bits)
    #   msgs: messages (hex format)
    # Returns:
    #   no return arguments
    """
    integer_bytes = 4

    if len(lengths) != len(msgs):
        print("Missmatch between #message lengths and #messages")
        print("Using min value between message lengths and messages")

    # get min value
    num_messages = min(len(lengths), len(msgs))

    f.write(num_messages.to_bytes(integer_bytes, byteorder='little'))
    for l, m in zip(lengths, msgs):
        # write length in bytes
        f.write((int(l)//8).to_bytes(integer_bytes, byteorder='little'))
        # write message
        write_hex_str_to_file(f, m)

def write_d_out_file(f, msg_digests):
    """Write Data in binary file
    Data is written in binary format with the following structure:
    msg_digest0 | msg_digest1 | ... | msg_digestsN-1

    # Parameters:
    #   f: output file
    #   msg_digests: list of message digests (hex format)
    # Returns:
    #   no return arguments
    """

    for md in msg_digests:
        # write message
        write_hex_str_to_file(f, md)

def main():
    try:
        REQUEST_file = sys.argv[1]
    except:
        print("Usage: gen_test_data.py [REQUEST_file]")
        print("REQUEST_file: file with test dataset")
        print("Outputs: data_in.bin data_out.bin")
        print("data_in.bin: input data in binary")
        print("data_out.bin: output data in binary")
        exit()

    # Strings to parse
    len_line = "Len ="
    msg_line = "Msg ="
    md_line = "MD ="

    # Variables
    cnt_msg = 0
    msg_arrays = []
    msg_lens = []
    msg_digests = []

    # Open output files
    d_in_fname = parse.parse("{}.rsp", REQUEST_file)[0] + "_d_in.bin"
    d_out_fname = parse.parse("{}.rsp", REQUEST_file)[0] + "_d_out.bin"
    try:
        f_din = open(d_in_fname, 'wb')
        f_dout = open(d_out_fname, 'wb')
    except:
        print("Failed to open output files")
        sys.exit()

    # Parse input file
    with open(REQUEST_file, 'r') as req_file:
        for line in req_file:
            if len_line in line:
                msg_len = line.split(" ")[-1].strip()
                msg_lens.append(msg_len)
            elif msg_line in line:
                msg = line.split(" ")[-1].strip()
                msg_arrays.append(msg)
            elif md_line in line:
                msg_md = line.split(" ")[-1].strip()
                msg_digests.append(msg_md)
                
    # Generate output files
    write_d_in_file(f_din, msg_lens, msg_arrays)
    write_d_out_file(f_dout, msg_digests)

    # Close output files
    f_din.close()
    f_dout.close()
            


if __name__ == "__main__":
    main()
