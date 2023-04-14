#!/usr/bin/env python3

"""
sha256_test.py
Python script to send input data and receive output data from a single SHA256 
test
"""

# Import Ethernet package
import sys
sys.path.append('../../')
from submodules.ETHERNET.software.python.ethBase import CreateSocket, SyncAckFirst, SyncAckLast
from submodules.ETHERNET.software.python.ethSendVariableData import SendVariableFile
from submodules.ETHERNET.software.python.ethRcvVariableData import RcvVariableFile

if __name__ == "__main__":
    print("\nSHA 256 Single Test\n")

    # Check input arguments
    if len(sys.argv) != 5:
        print(f'Usage: ./{sys.argv[0]} [RMAC_INTERFACE] [RMAC] [input.bin] [output.bin]')
        quit()
    else:
        send_file = sys.argv[3]
        rcv_file = sys.argv[4]

    # Send Variable Data File
    print("\nStarting file transmission...")

    socket = CreateSocket()

    SyncAckFirst(socket)
    SendVariableFile(socket,send_file)

    # # Receive Variable Data File
    # print("\nStarting file reception...")
    #
    # SyncAckLast(socket)
    # RcvVariableFile(socket,rcv_file)

    # Close Socket
    socket.close()
