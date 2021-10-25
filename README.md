# iob-soc-sha
SoC to run the program in software with or without acceleration using VERSAT2.0

# Setup
Clone the repository and the submodules with:
```
git clone --recursive git@github.com:IObundle/iob-soc-sha.git
```
or using the url:
```
git clone --recursive https://github.com/IObundle/iob-soc-sha.git
```
* * *
# PC Emulation
The iob-soc-sha system can build and run an environment for PC with:
```
make pc-emul
```
This target performs the Short Message Test for Byte-Oriented `sha256()` 
implementations from the 
[NIST Cryptograpphic Algorithm Validation Program](https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program/secure-hashing).

The test vectors are a set of 65 messages from 0 to 64 byte length. The 
implementation program only receives the messages and outputs the corresponding
message digests (MD). An external script compares the implementation output with
the expected MD from the test vectors.

The implementation output can be checked manually in 
`software/pc-emul/emul.log`.

### Clean environment
To clean the workspace after PC emulation:
```
make pc-emul-clean
```
### Requirements
PC emulation program requires:
- Git
- Make
- gcc
- Python 2.7

* * *
# RISCV Emulation
The iob-soc-sha system can be emulated using a verilog simulator like icarus 
with:
```
make test-simulator
```
- The `INIT_MEM=1` option skips the process of executing the bootloader and 
loading the firmware program into memory via UART.
- The `USE_DDR=0` option configures the system to only use internal memory.
- The `TEST_LOG="> test.log"` option indicates the log file for the simulation
output. This file is used to validate the simulation results, like the `pc-emul`
test.

The simulation output can be checked manually in 
`hardware/simulation/icarus/test.log_parsed.log`

### Clean environment
To clean the workspace after the RISCV emulation:
```
make sim-clean TEST_LOG=test.log
```

### Requirements/Setup
RISCV emulation requires:
- PC Emulation requirements
- RISCV toolchain
    - Add the RISCV toolchain to you `PATH` variable in `$HOME/.bashrc`:
    ```
    export RISCV=/path/to/riscv/bin
    export PATH=$RISCV:$PATH
    ```
- Verilog simulator (for example icarus verilog)
