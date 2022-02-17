ROOT_DIR:=.
include ./config.mk

.PHONY: pc-emul pc-emul-test pc-emul-clean\
	sim sim-test sim-clean\
	fpga-build fpga-run fpga-test fpga-clean\
	asic-synth asic-sim-post-synth asic-test asic-clean\
	clean\
	test-pc-emul test-pc-emul-clean\
	test-sim test-sim-clean\
	test-fpga test-fpga-clean\
	test-asic test-asic-clean\
	test-doc test-doc-clean\
	test test-clean\
	clean clean-all\
	debug

#
# SIMULATE RTL
#

sim:
	make -C $(SIM_DIR) all

sim-test:
	make -C $(SIM_DIR) test TEST_LOG="$(TEST_LOG)"

sim-clean:
	make -C $(SIM_DIR) clean clean-testlog

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
	make -C $(BOARD_DIR) all TEST_LOG="$(TEST_LOG)"

fpga-run-profile:
	make -C $(BOARD_DIR) profile TEST_LOG=">> test.log"

fpga-test:
	make -C $(BOARD_DIR) test

fpga-clean:
	make -C $(BOARD_DIR) clean clean-testlog

fpga-clean-all:
	make fpga-clean BOARD=AES-KU040-DB-G

#
# SYNTHESIZE AND SIMULATE ASIC
#

asic-synth:
	make -C $(ASIC_DIR) synth

asic-sim-post-synth:
	make -C $(ASIC_DIR) all TEST_LOG="$(TEST_LOG)"

asic-test:
	make -C $(ASIC_DIR) test

asic-clean:
	make -C $(ASIC_DIR) clean clean-testlog

#
# TEST ON SIMULATORS AND BOARDS
#
test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim: test-sim-clean
	make sim-test SIMULATOR=icarus TEST_LOG=">> test.log"

test-sim-clean:
	make sim-clean SIMULATOR=icarus

test-fpga: test-fpga-clean
	make fpga-test BOARD=AES-KU040-DB-G TEST_LOG=">> test.log"

test-fpga-clean:
	make fpga-clean BOARD=AES-KU040-DB-G TEST_LOG=">> test.log"

test-asic:
	make asic-test ASIC_NODE=umc130

test-asic-clean:
	make asic-clean ASIC_NODE=umc130

test: test-clean 
	make test-pc-emul 
	make pc-emul PROFILE=1
	make test-pc-emul-clean
	make test-sim 
	make test-sim-clean
	make test-fpga
	make fpga-run PROFILE=1
	make test-fpga-clean

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean


#generic clean
clean:
	make pc-emul-clean
	make sim-clean
	make fpga-clean
	make asic-clean

clean-all: test-clean
