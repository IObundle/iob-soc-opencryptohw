#!/usr/bin/env python3

import argparse
import os


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Include header file in all verilog files.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "-d", "--dir", default=".", help="Directory to search for verilog files."
    )
    parser.add_argument("-f", "--file", help="File to include in all verilog files.")
    return parser.parse_args()


def get_verilog_files(dir):
    file_list = os.listdir(dir)
    verilog_files = []

    for file in file_list:
        if file.endswith(".v"):
            verilog_files.append(f"{dir}/{file}")

    return verilog_files


def idx_contains(list, substr):
    # Get index of string that contains substring. -1 otherwise
    for s in list:
        if substr in s:
            return list.index(s)
    return -1


def include_header(header_file, verilog_files):
    include_str = f'`include "{header_file}"\n'
    for file in verilog_files:
        with open(file, "r+") as vfile:
            text = vfile.readlines()  # read all file text
            line = idx_contains(text, "timescale")
            # skip files without timescale or that already include header
            if line == -1 or include_str in text[line + 1]:
                continue
            text.insert(line + 1, include_str)  # insert include after timescale line
            vfile.seek(0)  # file pointer to begining of file
            vfile.writelines(text)  # write all file back


if __name__ == "__main__":
    args = parse_arguments()
    include_header(args.file, get_verilog_files(args.dir))
