ROOT_DIR:=.
include ./config.mk

.PHONY: sim sim-test sim-clean\
	pc-emul pc-emul-test pc-emul-clean pc-emul-profile\
	fpga-build fpga-build-all fpga-run fpga-run-profile fpga-test fpga-clean \
	fpga-clean-all fpga-eth fpga-eth-clean\
	test-pc-emul test-pc-emul-clean\
	test-sim test-sim-clean\
	test-fpga test-fpga-clean\
	test test-clean\
	clean clean-all

#
# SIMULATE RTL
#

sim:
	make -C $(SIM_DIR) all

sim-test:
	make -C $(SIM_DIR) test

sim-clean:
	make -C $(SIM_DIR) clean-all

#
# EMULATE ON PC
#

pc-emul:
	make -C $(PC_DIR) all

pc-emul-test:
	make -C $(PC_DIR) test

pc-emul-clean:
	make -C $(PC_DIR) clean

pc-emul-profile:
	make -C $(PC_DIR) profile

#
# BUILD, LOAD AND RUN ON FPGA BOARD
#

fpga-build:
	make -C $(BOARD_DIR) build

fpga-build-all:
	make fpga-build BOARD=AES-KU040-DB-G

fpga-run:
	make -C $(BOARD_DIR) all 

fpga-run-profile:
	make -C $(BOARD_DIR) profile

fpga-test:
	make -C $(BOARD_DIR) test

fpga-clean:
	make -C $(BOARD_DIR) clean-all

fpga-clean-all:
	make fpga-clean BOARD=AES-KU040-DB-G

fpga-eth:
	make -C $(SW_DIR)/python fpga-eth SOC_OUT_BIN=$(SOC_OUT_BIN)

fpga-eth-clean:
	make -C $(SW_DIR)/python clean

#
# TEST ON SIMULATORS AND BOARDS
#

test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim: test-sim-clean
	make sim-test SIMULATOR=icarus 
	make sim-test SIMULATOR=verilator 

test-sim-clean:
	make sim-clean SIMULATOR=icarus
	make sim-clean SIMULATOR=verilator

test-fpga: test-fpga-clean
	make fpga-test BOARD=AES-KU040-DB-G

test-fpga-clean:
	make fpga-clean BOARD=AES-KU040-DB-G

test: test-clean 
	make test-pc-emul 
	make pc-emul-profile
	make test-pc-emul-clean
	make test-sim
	make test-sim-clean
	make test-fpga
	make fpga-run-profile
	make test-fpga-clean

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean

#generic clean
clean: pc-emul-clean sim-clean fpga-clean 

clean-all: test-clean
