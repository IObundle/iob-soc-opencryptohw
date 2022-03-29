#!/usr/bin/env python3

"""
parse_profile.py
Python script to parte profile logs
"""

import sys
import parse
import inspect

def parse_input(input_fname):
    parsed_lines = []
    with open(input_fname) as fin:
        for line in fin:
            # Global time pattern
            parse_tmp = parse.parse("PROFILE: {} time: {}us {}\n", line)
            if parse_tmp is None:
                # Function time pattern
                parse_tmp = parse.parse("{}PROFILE: {} time: {}us ({}%)\n", line)

            if parse_tmp is None:
                continue

            parsed_lines.append(parse_tmp)

    return parsed_lines

def get_color(num_tabs):
    color_dict = {
            0: "\\rowcolor{dark-blue}\n",
            1: "\\rowcolor{dark-blue}\n",
            2: "\\rowcolor{mid-blue}\n",
            3: "\\rowcolor{light-blue}\n",
            4: ""
            }
    return color_dict[num_tabs]

def process_input(parsed_lines):
    table = []
    for line in parsed_lines:
        # check for \t in first result
        num_tabs = line[0].count('\t')
        if num_tabs == 0:
            # Global time format
            col0 = f"\hspace{{{num_tabs*10}pt}}" + line[0]
            col1 = line[1]
            col2 = "100"
        else:
            # Function time format
            col0 = f"\hspace{{{num_tabs*10}pt}}" + line[1]
            col1 = line[2]
            col2 = line[3]

        table_line = get_color(num_tabs) + col0 + "\t&" + col1 + "\t&" + col2 + "\t\\\\ \hline"

        table.append(table_line)

    return table

def write_table(output_fname, table):
    with open(output_fname, 'w') as fout:
        for line in table:
            fout.write(line)

if __name__ == "__main__":
    print("\nParse Profile Script\n")

    # Check input arguments
    if len(sys.argv) != 3:
        print(f'Usage: ./{sys.argv[0]} [input_profile.log] [output_table.tex]')
        quit()
    else:
        input_fname = sys.argv[1]
        output_fname = sys.argv[2]

    table_lines = process_input(parse_input(input_fname))

    write_table(output_fname, table_lines)

