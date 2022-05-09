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
make pc-emul-run
```
This target performs the Short Message Test for Byte-Oriented `sha256()` 
implementations from the 
[NIST Cryptograpphic Algorithm Validation Program](https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program/secure-hashing).

The test vectors are a set of 65 messages from 0 to 64 byte length. The 
implementation program only receives the messages and outputs the corresponding
message digests (MD). An external script compares the implementation output with
the expected MD from the test vectors.

The implementation output can be checked manually in 
`software/pc-emul/soc.log` and `software/pc-emul/host.log`

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
- Python 3.6+

* * *
# RISCV Emulation
The iob-soc-sha system can be emulated using a verilog simulator like icarus 
with:
```Make
# Test with all supported simulators
make test-sim
# Test with a specific simulator
make sim-test SIMULATOR=icarus
make sim-test SIMULATOR=verilator
```

### Clean environment
To clean the workspace after the RISCV emulation:
```
make test-sim-clean
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
- Verilog simulator, for example: 
    - [icarus verilog](https://github.com/steveicarus/iverilog)  
    - [verilator](https://github.com/verilator/verilator)

# FPGA Execution
The system can be tested on FPGA with:
```
make test-fpga
```

The results can be manually checked in 
`hardware/fpga/<tool>/<board>/test.log_parsed.log`, where `<tool>` is the 
tool used for synthesis and `<board>` is the board directory name.

The system has been tested with the `AES-KU040-DB-G` board from Xilinx. In that
case the results can be found in: `hardware/fpga/vivado/AES-KU040-DB-G`. 

### Clean environment
To clean the workspace after the FPGA execution:
```
make test-fpga-clean
```

### Requirements/Setup
FPGA execution requires:
- Supported FPGA board
- Setup environment for FPGA execution
    - Add the executable paths and license servers in `$HOME/.bashrc`:
    ```
    export VIVADOPATH=/path/to/vivado
    ...
    export LM_LICENSE_FILE=port@licenseserver.myorg.com;lic_or_dat_file
    ```
    - Follow [IOb-soc's README](https://github.com/IObundle/iob-soc#readme) for
    more installation details.

# Profiling
The system can be profiled using a 
[Timer core](https://www.github.com/IObundle/iob-timer.git), a software 
controlled counter.

The `pc-emul` version simulates the counter behaviour by calling the C standard
`<timer.h>` library.

The profiling is available for `pc-emul` using the following command:
```
make pc-emul-profile
```
The `pc-emul-profile` target outputs an `emul_profile.log` file with the profiling 
information.

For `fpga` profiling run the following command:
```
make fpga-run-profile
```
The `fpga-run-profile` target outputs a `fpga_profile.log` file with the 
profiling information.

# Ethernet
The system supports ethernet communication using the 
[IOb-Eth core](https://github.com/IObundle/iob-eth).

Check [IO-Eth's README](https://github.com/IObundle/iob-eth#readme) for setup 
instructions and further details.
