#!/usr/bin/env python3

import argparse


def parse_arguments():
    parser = argparse.ArgumentParser(description="Generate ROM verilog switch case.")
    parser.add_argument("hexfile", help="ROM content in hexadecimal format")
    parser.add_argument(
        "outfile", default="boot_switch.vh", help="ROM verilog switch file"
    )
    return parser.parse_args()


def generate_verilog_switch(hexfile, outfile):
    f_hex = open(hexfile, "r")

    with open(outfile, "w") as fout:
        fout.write("\talways @(posedge clk) begin\n")
        fout.write("\t\tcase (addr)\n")
        addr = 0
        for line in f_hex:
            fout.write(f"\t\t\tDATA_W'd{addr}: r_data <= DATA_W'h{line.strip()};\n")
        fout.write("\t\tendcase\n")
        fout.write("\tend\n")

    f_hex.close()


if __name__ == "__main__":
    print("Generate ROM Script")
    args = parse_arguments()
    generate_verilog_switch(args.hexfile, args.outfile)
