#!/usr/bin/python3

import string
import sys
import re
from sys import argv

def format_msg_to_header(msg, count):
    msg_repr = []
    # split into pairs of chars
    byte_list = re.findall('..', msg)

    # generate static char array for C header
    formated_msg = "unsigned char msg_" + str(count) + "[] = { "
    first = True
    # append each by to array
    for byte_str in byte_list:
        if first:
            first = False
        else:
            formated_msg += ", "
        formated_msg += r"0x" + byte_str

    formated_msg += " };"

    return formated_msg

def msg_array(count):
    formated_msg = "unsigned char *msg_array[] = { "
    first = True
    for i in range(count):
        if first:
            first = False
        else:
            formated_msg += ", "
        formated_msg += "msg_" + str(i)

    formated_msg += " };"
    return formated_msg
        
def msg_len_array(msg_lens):
    formated_len_array = "int msg_len[] = { "
    first = True
    for msg_len in msg_lens:
        if first:
            first = False
        else:
            formated_len_array += ", "
        formated_len_array += msg_len

    formated_len_array += " };"
    

    return formated_len_array


def main():
    try:
        REQUEST_file = argv[1]
    except:
        sus.exit("Usage: gen_test_header.py [REQUEST file] > [header file]")

    msg_substring = "Msg ="
    len_substring = "Len ="

    count = 0
    msg_arrays = []
    msg_lens = []
    # Parse REQUEST file - generate char array for each message
    with open(REQUEST_file, 'r') as req_file:
        for line in req_file:
            if msg_substring in line:
                msg = line.split(" ")[-1].strip()
                # print(format_msg_to_header(msg, count))
                msg_arrays.append(format_msg_to_header(msg, count))
                count = count + 1
            if len_substring in line:
                msg_len = line.split(" ")[-1].strip()
                msg_len_bytes = str(int(msg_len)//8)
                msg_lens.append(msg_len_bytes)
    

    print("#define NUM_MSGS (" + str(count) + ")")
    for msg in msg_arrays:
        print(msg)
    # Output array of char arrays
    print(msg_array(count))

    # Output array of msg lens (in bytes)
    print(msg_len_array(msg_lens))

if __name__ == "__main__":
    main()
