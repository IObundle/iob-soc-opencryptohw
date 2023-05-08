include $(ROOT_DIR)/hardware/hardware.mk
include $(LIB_DIR)/hardware/iob_reset_sync/hardware.mk

BAUD=$(BOARD_BAUD)
FREQ=$(BOARD_FREQ)

TOOL=$(shell find $(HW_DIR)/fpga -name $(BOARD) | cut -d"/" -f7)


#SOURCES
VSRC+=./verilog/top_system.v

ifeq ($(RUN_EXTMEM),1)
INIT_MEM=0
endif

#console command
CONSOLE_CMD=$(CONSOLE_DIR)/eth_console -s /dev/usb-uart
ifeq ($(INIT_MEM),0)
CONSOLE_CMD+=-f
endif

# Input/Output
SOC_IN_BIN=soc-in.bin
TEST_IN_BIN=$(SW_TEST_DIR)/$(basename $(TEST_VECTOR_RSP))_d_in.bin
SOC_OUT_BIN:=soc-out.bin
ETH_LOG=ethernet.log

# Board grab
BOARD_GRAB_CMD=$(SW_DIR)/python/board_client.py grab 600
FPGA_PROG=../prog.sh

#RULES

#
# Use
#

FORCE ?= 1

run: $(FPGA_OBJ) $(SOC_IN_BIN)
ifeq ($(NORUN),0)
ifeq ($(BOARD_SERVER),)
	cp $(FIRM_DIR)/firmware.bin .
	$(BOARD_GRAB_CMD) -p '$(FPGA_PROG)' -c '$(CONSOLE_CMD) $(TEST_LOG)'
	# cp $(SW_DIR)/python/$(SOC_OUT_BIN) .
else
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude-from=$(ROOT_DIR)/.rsync_exclude $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh -t $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@ ALGORITHM=$(ALGORITHM) INIT_MEM=$(INIT_MEM) FORCE=$(FORCE) TEST_LOG=\"$(TEST_LOG)\"'
ifneq ($(TEST_LOG),)
	scp $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/test.log .
endif
	scp $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(SOC_OUT_BIN) .
	scp $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/ethernet.log .
endif
endif

build: $(FPGA_OBJ)

#make the FPGA programming file either locally or remotely
ifeq ($(INIT_MEM),1)
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex firmware.hex
else ifeq ($(USE_DDR),1)
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex
else
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex
endif
ifeq ($(NORUN),0)
ifeq ($(FPGA_SERVER),)
	@rm -f $(FPGA_LOG)
	make local-build
else
	ssh $(FPGA_USER)@$(FPGA_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude-from=$(ROOT_DIR)/.rsync_exclude $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh -t $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR) fpga-build BOARD=$(BOARD) INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM) ALGORITHM=$(ALGORITHM)'
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_OBJ) .
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_LOG) .
endif
endif

build-versat: $(FPGA_VERSAT_OBJ)

$(FPGA_VERSAT_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR)
ifeq ($(FPGA_SERVER),)
	@rm -f $(FPGA_VERSAT_LOG)
	../build_versat.sh "$(INCLUDE)" "$(DEFINE)" "$(VSRC)" "$(DEVICE)"
	make post-build
else 
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude-from=$(ROOT_DIR)/.rsync_exclude $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh -t $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@ INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM)'
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_VERSAT_OBJ) .
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_VERSAT_LOG) .
endif

#
# Testing
#

test: clean-testlog test-shortmsg

test-shortmsg: run-shortmsg test-validate

run-shortmsg:
	make -C $(ROOT_DIR) fpga-run HARDWARE_TEST=$(HARDWARE_TEST)

test-validate:
	cp $(SOC_OUT_BIN) $(SW_TEST_DIR)
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

#
# Clean
#

clean-all: clean-local clean-remote

clean-local: hw-clean
	@rm -f $(FPGA_OBJ) $(FPGA_LOG) $(SOC_LOG) $(ETH_LOG)
	@rm -rf $(FPGA_VERSAT_OBJ) $(FPGA_VERSAT_LOG)
	@make -C $(SW_TEST_DIR) clean

clean-remote:
ifneq ($(FPGA_SERVER),)
	ssh $(FPGA_USER)@$(FPGA_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude-from=$(ROOT_DIR)/.rsync_exclude $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh -t $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) clean-local CLEANIP=$(CLEANIP) ALGORITHM=$(ALGORITHM)'
endif
ifneq ($(BOARD_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude-from=$(ROOT_DIR)/.rsync_exclude $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh -t $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) ALGORITHM=$(ALGORITHM) clean-local'
endif

#clean test log only when board testing begins
clean-testlog:
	@rm -f *.log
ifneq ($(BOARD_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude-from=$(ROOT_DIR)/.rsync_exclude $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh -t $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) ALGORITHM=$(ALGORITHM) $@'
endif

$(SOC_IN_BIN): $(TEST_IN_BIN)
	cp $< $@

$(TEST_IN_BIN):
	make -C $(SW_TEST_DIR) gen_test_data TEST_VECTOR_RSP=$(TEST_VECTOR_RSP) ALGORITHM=$(ALGORITHM)

debug:
	@echo $(VHDR)
	@echo $(VSRC)
	@echo $(INCLUDE)
	@echo $(DEFINE)


.PRECIOUS: $(FPGA_OBJ) test.log s_fw.bin

.PHONY: run build \
	test test-shortmsg run-shortmsg test-validate \
	clean-all clean-testlog clean-local clean-remote
