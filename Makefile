ROOT_DIR=.
include $(ROOT_DIR)/config.mk

#
# FUSESOC TARGETS
#
FUSESOC_DIR=fusesoc
fusesoc-setup:
	make -C $(FUSESOC_DIR) setup ALGORITHM=$(ALGORITHM)

fusesoc-sim-setup:
	make -C $(FUSESOC_DIR) sim-setup ALGORITHM=$(ALGORITHM)

fusesoc-sim-build:
	make -C $(FUSESOC_DIR) sim-build ALGORITHM=$(ALGORITHM)
		
fusesoc-sim-run:
	make -C $(FUSESOC_DIR) sim-run ALGORITHM=$(ALGORITHM)

fusesoc-fpga-setup:
	make -C $(FUSESOC_DIR) fpga-setup ALGORITHM=$(ALGORITHM)

fusesoc-fpga-build:
	make -C $(FUSESOC_DIR) fpga-build ALGORITHM=$(ALGORITHM)
		
fusesoc-fpga-run: fusesoc-fpga-build
	make -C $(FUSESOC_DIR) fpga-run ALGORITHM=$(ALGORITHM)

fusesoc-clean:
	make -C $(FUSESOC_DIR) clean ALGORITHM=$(ALGORITHM)

#
# OPENLANE TARGETS
#
OPENLANE_FLOW_DIR=$(HW_DIR)/asic/openlane
openlane-setup:
	make -C $(OPENLANE_FLOW_DIR) setup
openlane-run:
	make -C $(OPENLANE_FLOW_DIR) run
openlane-clean:
	make -C $(OPENLANE_FLOW_DIR) clean
openlane-post-synth-sim:
	make -C $(OPENLANE_FLOW_DIR)/simulation test OPENLANE_SIM_TYPE=post-synth
openlane-post-layout-sim:
	make -C $(OPENLANE_FLOW_DIR)/simulation test OPENLANE_SIM_TYPE=post-layout
openlane-sim-clean:
	make -C $(OPENLANE_FLOW_DIR)/simulation clean

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
# GENERATE SPINALHDL VERILOG SOURCES
#
gen-spinal-sources:
	make -C $(HW_DIR) gen-spinal-sources

#
# EMULATE ON PC
#

pc-emul-build:
	make -C $(PC_DIR) build

pc-emul-run: pc-emul-build
	make -C $(PC_DIR) run

pc-emul-clean: fw-clean
	make -C $(PC_DIR) clean

pc-emul-test: pc-emul-clean
	make -C $(PC_DIR) test

pc-emul-gen-versat:
	make -C $(PC_DIR) gen-versat

#
# SIMULATE RTL
#

sim-build:
	make -C $(PC_DIR) gen-versat
	make fw-build SIM=1
	make -C $(SIM_DIR) build

sim-run: sim-build
	make -C $(SIM_DIR) run

sim-clean: fw-clean
	make -C $(SIM_DIR) clean

sim-test:
	make -C $(SIM_DIR) test SPINAL=$(SPINAL)

sim-versat-fus:
	make -C $(SIM_DIR) xunitM SIMULATOR=icarus
	make -C $(SIM_DIR) xunitF SIMULATOR=icarus

sim-debug:
	make -C $(SIM_DIR) debug

#
# BUILD, LOAD AND RUN ON FPGA BOARD
#

fpga-build:
	make -C $(PC_DIR) gen-versat
	make fw-build BAUD=$(BOARD_BAUD) FREQ=$(BOARD_FREQ)
	make -C $(BOARD_DIR) build

fpga-run: fpga-build
	make -C $(BOARD_DIR) run TEST_LOG="$(TEST_LOG)"

fpga-clean: fw-clean
	make -C $(BOARD_DIR) clean

fpga-veryclean:
	make -C $(BOARD_DIR) veryclean

fpga-debug:
	make -C $(BOARD_DIR) debug

fpga-test:
	make -C $(BOARD_DIR) test

fpga-build-versat: pc-emul-gen-versat
	make -C $(BOARD_DIR) build-versat

#
# COMPILE DOCUMENTS
#

doc-build:
	make -C $(DOC_DIR) $(DOC).pdf

doc-clean:
	make -C $(DOC_DIR) clean

doc-test:
	make -C $(DOC_DIR) test

#
# TEST ALL PLATFORMS
#

test-pc-emul: pc-emul-test

test-pc-emul-clean: pc-emul-clean

test-sim:
	make sim-test SIMULATOR=verilator

test-sim-clean:
	make sim-clean SIMULATOR=verilator

test-fpga:
	make fpga-test BOARD=AES-KU040-DB-G

test-fpga-versat: test-fpga-clean
	make fpga-build-versat BOARD=AES-KU040-DB-G

test-fpga-clean:
	make fpga-clean BOARD=AES-KU040-DB-G

test-doc:
	make fpga-clean BOARD=CYCLONEV-GT-DK
	make fpga-clean BOARD=AES-KU040-DB-G
	make fpga-build BOARD=CYCLONEV-GT-DK
	make fpga-build BOARD=AES-KU040-DB-G
	make doc-test DOC=pb
	make doc-test DOC=presentation

test-doc-clean:
	make doc-clean DOC=pb
	make doc-clean DOC=presentation

test: test-clean test-pc-emul test-sim test-fpga test-doc

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean test-doc-clean

debug:
	@echo $(UART_DIR)
	@echo $(CACHE_DIR)

test: test-clean 
	make test-pc-emul 
	make pc-emul-profile
	make test-pc-emul-clean
	make pc-emul-gen-versat
	make test-pc-emul-clean
	make test-sim
	make test-sim-clean
	make test-fpga-versat
	make test-fpga
	make fpga-run-profile
	make test-fpga-clean
	make test-versat-fus
	make test-sim-clean
	make test-versat-fus SPINAL=1
	make test-sim-clean

test-clean: test-pc-emul-clean test-sim-clean test-fpga-clean

#generic clean
clean: pc-emul-clean sim-clean fpga-clean doc-clean

clean-all: test-clean

.PHONY: fusesoc-setup \
	fusesoc-sim-setup fusesoc-sim-build fusesoc-sim-run \
	fusesoc-fpga-setup fusesoc-fpga-build fusesoc-fpga-run \
	fw-build fw-clean fw-debug \
	gen-spinal-sources \
	pc-emul-build pc-emul-run pc-emul-clean pc-emul-test pc-emul-gen-versat \
	sim-build sim-run sim-clean sim-test sim-versat-fus sim-debug \
	fpga-build fpga-run fpga-clean fpga-veryclean fpga-debug \
	fpga-test fpga-build-versat \
	doc-build doc-clean doc-test \
	test-pc-emul test-pc-emul-clean \
	test-sim test-sim-clean \
	test-fpga test-fpga-clean test-fpga-versat \
	test-doc test-doc-clean \
	debug \
	test test-clean \
	clean clean-all



