ROOT_DIR=../../..

OPENLANE_DESIGN=system
OPENLANE_SRC_DIR=$(OPENLANE_DESIGN)/src

OPENLANE_TOOL_DIR ?= $(ROOT_DIR)/../OpenLane
OPENLANE_GIT_REPO = https://github.com/The-OpenROAD-Project/OpenLane.git
OPENLANE_GIT_TAG = 2022.12.16

include $(ROOT_DIR)/hardware/hardware.mk

VHDR_FILTER = $(filter-out %/inst_tb.vh %/pio.vh %/inst.vh, $(VHDR))
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

UNIQ_VSRC = $(call uniq, $(VSRC))
UNIQ_VHDR = $(call uniq, $(VHDR_FILTER))

all: run

setup: $(OPENLANE_TOOL_DIR) $(OPENLANE_SRC_DIR) gen-versat $(VSRC) $(VHDR)
	cp $(UNIQ_VSRC) $(OPENLANE_SRC_DIR)
	cp $(UNIQ_VHDR) $(OPENLANE_SRC_DIR)

$(OPENLANE_SRC_DIR):
	mkdir -p $(OPENLANE_SRC_DIR)

gen-versat:
	make -C $(PC_DIR) gen-versat

run: setup copy-to-openlane
	make -C $(OPENLANE_TOOL_DIR) quick_run QUICK_RUN_DESIGN=$(OPENLANE_DESIGN)

copy-to-openlane: 
	cp -r $(OPENLANE_DESIGN) $(OPENLANE_TOOL_DIR)/designs

$(OPENLANE_TOOL_DIR):
	@echo "Please setup OpenLane tool in $(OPENLANE_TOOL_DIR)"
	cd $(ROOT_DIR)/..; git clone --depth 1 $(OPENLANE_GIT_REPO) --branch $(OPENLANE_GIT_TAG)
	cd $(OPENLANE_TOOL_DIR); make

test: setup
	tree .

clean:
	rm -rf $(OPENLANE_SRC_DIR)
	make -C $(PC_DIR) clean
	rm -rf *.v *.vh
	make -C $(OPENLANE_TOOL_DIR) clean_runs
