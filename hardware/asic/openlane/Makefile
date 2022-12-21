ROOT_DIR=../../..

OPENLANE_DESIGN=system
OPENLANE_SRC_DIR=$(OPENLANE_DESIGN)/src

OPENLANE_TOOL_DIR ?= $(ROOT_DIR)/../OpenLane
OPENLANE_GIT_REPO = https://github.com/The-OpenROAD-Project/OpenLane.git
OPENLANE_GIT_TAG = 2022.12.16

#DDR controller address and data widths
DDR_ADDR_W=30
DDR_DATA_W=32

include $(ROOT_DIR)/hardware/hardware.mk

# Missing verilog headers
# in other flows (sim/fpga) these pointed to with -Include PATH flags
VHDR+=$(AXI_DIR)/hardware/include/axi.vh

# Add global defines
VHDR+=global_defines.vh

# Remove BlackBox modules
BLACKBOX_VSRC = $(addprefix %/,$(subst .bb.v,.v, $(notdir $(wildcard $(OPENLANE_DESIGN)/blackbox/*.bb.v))))
VSRC_FILTER = $(filter-out $(BLACKBOX_VSRC), $(VSRC))

# Remove unnecessary headers that cause conflict
VHDR_FILTER = $(filter-out %/inst_tb.vh %/pio.vh %/inst.vh, $(VHDR))

# Remove duplicated items from lists
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
UNIQ_VSRC = $(call uniq, $(VSRC_FILTER))
UNIQ_VHDR = $(call uniq, $(VHDR_FILTER))

all: run

setup: $(OPENLANE_TOOL_DIR) $(OPENLANE_SRC_DIR) gen-versat $(VSRC) $(VHDR) boot.hex
	cp $(UNIQ_VSRC) $(OPENLANE_SRC_DIR)
	cp $(UNIQ_VHDR) $(OPENLANE_SRC_DIR)
	cp boot.hex $(OPENLANE_SRC_DIR)
	./scripts/include_header.py -d $(OPENLANE_SRC_DIR) -f global_defines.vh
	make verilog-fixes

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

global_defines.vh: 
	./scripts/gen_defines.py -o $@ $(DEFINE)

verilog-fixes:
	sed -i '/$$display/d' $(OPENLANE_SRC_DIR)/axi_interconnect.v

$(BOOT_DIR)/boot.bin:
	make -C $(ROOT_DIR) fw-build

clean:
	rm -rf $(OPENLANE_SRC_DIR)
	make -C $(PC_DIR) clean
	make -C $(ROOT_DIR) fw-clean
	rm -rf *.v *.vh *.hex
	# make -C $(OPENLANE_TOOL_DIR) clean_runs
