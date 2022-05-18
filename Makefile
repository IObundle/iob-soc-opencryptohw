ROOT_DIR:=.
include ./config.mk

#
# BUILD EMBEDDED SOFTWARE
#

fw-build:
	make -C $(FIRM_DIR) build-all

fw-clean:
	make -C $(FIRM_DIR) clean-all

#
# EMULATE ON PC
#

pc-emul-build:
	make -C $(PC_DIR) build

pc-emul-run:
	make -C $(PC_DIR) run

pc-emul-test: pc-emul-clean
	make -C $(PC_DIR) test

pc-emul-clean: fw-clean
	make -C $(PC_DIR) clean

pc-emul-profile:
	make fw-build BAUD=5000000 PROFILE=1
	make -C $(PC_DIR) profile

#
# SIMULATE RTL
#

sim-build:
	make fw-build BAUD=5000000 SIM=1
	make -C $(SIM_DIR) build

sim-run: sim-build
	make -C $(SIM_DIR) run

sim-clean: fw-clean
	make -C $(SIM_DIR) clean

sim-test:
	make -C $(SIM_DIR) test

sim-versat-fus:
	make -C $(SIM_DIR) xunitM SIMULATOR=icarus
	make -C $(SIM_DIR) xunitF SIMULATOR=icarus

#
# BUILD, LOAD AND RUN ON FPGA BOARD
#

fpga-build:
	make fw-build
	make -C $(BOARD_DIR) build

fpga-run: fpga-build
	make -C $(BOARD_DIR) run TEST_LOG="$(TEST_LOG)"

fpga-run-profile:
	make fw-build PROFILE=1
	make -C $(BOARD_DIR) profile

fpga-test:
	make -C $(BOARD_DIR) test

fpga-clean: fw-clean
	make -C $(BOARD_DIR) clean-all

#
# GENERATE DOCUMENTATION
#

doc-accel-plan:
	make -C $(DOC_DIR) accel-plan

doc-accel-plan-clean:
	make -C $(DOC_DIR) accel-plan-clean

doc-clean:
	make -C $(DOC_DIR) clean

#
# TEST ON SIMULATORS AND BOARDS
#
test-versat-fus:
	make sim-versat-fus SIMULATOR=icarus

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
clean: pc-emul-clean sim-clean fpga-clean doc-clean

clean-all: test-clean

.PHONY: fw-build fw-clean \
	pc-emul-build pc-emul-run pc-emul-test pc-emul-clean pc-emul-profile \
	sim-build sim-run sim-clean sim-test sim-versat-fus \
	fpga-build fpga-run fpga-run-profile fpga-test fpga-clean \
	doc-accel-plan doc-accel-plan-clean doc-clean \
	test-versat-fus \
	test-pc-emul test-pc-emul-clean \
	test-sim test-sim-clean \
	test-fpga test-fpga-clean \
	test test-clean \
	clean clean-all
