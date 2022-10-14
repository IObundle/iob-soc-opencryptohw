export SHELL = /bin/bash

ROOT_DIR:=.
include ./config.mk

# TESTER PORTMAP
tester-portmap:
	make -C submodules/TESTER/ portmap

#
# BUILD EMBEDDED SOFTWARE
#

fw-build:
	make -C $(FIRM_DIR) build-all

fw-clean:
	make -C $(FIRM_DIR) clean-all

fw-debug:
	make -C $(FIRM_DIR) debug

#
# EMULATE ON PC
#

#default baud and system clock frequency
SIM_BAUD ?= 2500000
SIM_FREQ ?=50000000
pc-emul-build:
	make fw-build BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ)
	make -C $(PC_DIR) BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ)

pc-emul-run: pc-emul-build
	make -C $(PC_DIR) run BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ)

pc-emul-clean: fw-clean
	make -C $(PC_DIR) clean

pc-emul-test: pc-emul-clean
	make -C $(PC_DIR) test BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ)

pc-emul-profile:
	make fw-build BAUD=5000000 FREQ=$(SIM_FREQ) PROFILE=1
	make -C $(PC_DIR) profile BAUD=5000000 FREQ=$(SIM_FREQ)

#
# SIMULATE RTL
#

sim-build:
	make fw-build BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ) SIM=1
	make -C $(SIM_DIR) build BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ)

sim-run: sim-build
	make -C $(SIM_DIR) run

sim-clean: fw-clean
	make -C $(SIM_DIR) clean

sim-test:
	make -C $(SIM_DIR) test

sim-debug:
	make -C $(SIM_DIR) debug

sim-versat-fus:
	make -C $(SIM_DIR) xunitM BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ) SIMULATOR=icarus
	make -C $(SIM_DIR) xunitF BAUD=$(SIM_BAUD) FREQ=$(SIM_FREQ) SIMULATOR=icarus

tester-sim-build:
	make -C submodules/TESTER sim-build

tester-sim-run:
	make -C submodules/TESTER sim-run

tester-sim-test: tester-sim-build
	make tester-sim-run | tee /dev/tty | grep "Test complete! Output and expected files are equal!" >/dev/null

#
# BUILD, LOAD AND RUN ON FPGA BOARD
#

#default baud and system clock freq for boards
BOARD_BAUD ?= 115200
#default board frequency
ifeq ($(BOARD), CYCLONEV-GT-DK)
BOARD_FREQ ?=50000000
else
BOARD_FREQ ?=100000000
endif

fpga-build:
	make fw-build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)
	make -C $(BOARD_DIR) build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ) SHELL=$(SHELL)

fpga-run: fpga-build
	make -C $(BOARD_DIR) run TEST_LOG="$(TEST_LOG)"

fpga-clean: fw-clean
	make -C $(BOARD_DIR) clean

fpga-veryclean:
	make -C $(BOARD_DIR) veryclean

fpga-debug:
	make -C $(BOARD_DIR) debug
 
fpga-run-profile:
	make fw-build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ) PROFILE=1
	make -C $(BOARD_DIR) profile BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)

fpga-test:
	make -C $(BOARD_DIR) test

tester-fpga-build:
	make -C submodules/TESTER fpga-build

tester-fpga-run:
	make -C submodules/TESTER fpga-run

tester-fpga-test: tester-fpga-build
	make tester-fpga-run | tee /dev/tty | grep "Test complete! Output and expected files are equal!" >/dev/null
#
# COMPILE DOCUMENTS
#
doc-accel-plan:
	make -C $(DOC_DIR) accel-plan

doc-accel-plan-clean:
	make -C $(DOC_DIR) accel-plan-clean

doc-clean:
	make -C $(DOC_DIR) clean

#
# CLEAN
#

clean: pc-emul-clean sim-clean fpga-clean doc-clean python-cache-clean
	make -C submodules/TESTER clean

#
# TEST ALL PLATFORMS
#
test-versat-fus:
	make sim-versat-fus SIMULATOR=icarus

test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim: test-sim-clean
	make sim-test SIMULATOR=verilator
	make sim-test SIMULATOR=icarus

test-sim-clean:
	make sim-clean SIMULATOR=verilator
	make sim-clean SIMULATOR=icarus

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
	make test-versat-fus
	make test-sim-clean
	make test-versat-fus SPINAL=1
	make test-sim-clean

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean


clean-all: test-clean

python-cache-clean:
	find . -name "*__pycache__" -exec rm -rf {} \; -prune

.PHONY: fw-build fw-clean fw-debug\
	pc-emul-build pc-emul-run pc-emul-test pc-emul-clean pc-emul-profile \
	sim-build sim-run sim-clean sim-test sim-debug sim-versat-fus \
	tester-sim-build tester-sim-run tester-sim-test \
	fpga-build fpga-run fpga-run-profile fpga-test fpga-debug fpga-clean \
	tester-fpga-build tester-fpga-run tester-fpga-test \
	doc-accel-plan doc-accel-plan-clean doc-clean \
	test-versat-fus \
	test-pc-emul test-pc-emul-clean \
	test-sim test-sim-clean \
	test-fpga test-fpga-clean \
	test test-clean \
	tester-portmap \
	clean clean-all
