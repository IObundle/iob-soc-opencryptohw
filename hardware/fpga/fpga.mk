BOARD=$(shell basename `pwd`)

LOAD_FILE=/tmp/$(BOARD).load

QUEUE_FILE=/tmp/$(BOARD).queue

TOOL=$(shell find $(HW_DIR)/fpga -name $(BOARD) | cut -d"/" -f7)

JOB=$(shell echo $(USER) `md5sum $(FPGA_OBJ)  | cut -d" " -f1`)

include $(ROOT_DIR)/hardware/hardware.mk

#SOURCES
VSRC+=./verilog/top_system.v

#TEST VECTOR
SOC_LOG=soc.log
ETH_LOG:=ethernet.log
FPGA_PROFILE_LOG:=fpga_profile.log

#OUTPUT BIN
SOC_OUT_BIN:=soc-out.bin

FORCE ?= 1
#console command 
CONSOLE_CMD=$(CONSOLE_DIR)/eth_console -s /dev/usb-uart
ifeq ($(INIT_MEM),0)
CONSOLE_CMD+=-f
endif


#RULES

#
# Use
#
FORCE ?= 1

run:
ifeq ($(NORUN),0)
ifeq ($(BOARD_SERVER),)
	cp $(FIRM_DIR)/firmware.bin .
	if [ ! -f $(LOAD_FILE) ]; then touch $(LOAD_FILE); chown $(USER):dialout $(LOAD_FILE); chmod 664 $(LOAD_FILE); fi;\
	bash -c "trap 'make queue-out' INT TERM KILL; make queue-in; if [ $(FORCE) = 1 -o \"`head -1 $(LOAD_FILE)`\" != \"$(JOB)\" ];\
	then ../prog.sh; echo $(JOB) > $(LOAD_FILE); fi; $(CONSOLE_CMD) $(TEST_LOG); make queue-out;"
else
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR) 
	bash -c "trap 'make queue-out-remote' INT TERM KILL; ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@ INIT_MEM=$(INIT_MEM) FORCE=$(FORCE) TEST_LOG=\"$(TEST_LOG)\"'"
	scp -r $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(ETH_LOG) . 2>/dev/null || :
	scp -r $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/software/python/*.bin . 2>/dev/null || :
ifneq ($(TEST_LOG),)
	scp $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(lastword $(TEST_LOG)) .
endif
endif
endif

build: $(FPGA_OBJ)

ifeq ($(INIT_MEM),1)
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex firmware.hex
else
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex
endif
ifeq ($(NORUN),0)
ifeq ($(FPGA_SERVER),)
	@rm -f $(FPGA_LOG)
	../build.sh "$(INCLUDE)" "$(DEFINE)" "$(VSRC)" "$(DEVICE)"
	make post-build
else 
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@ INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM)'
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_OBJ) .
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_LOG) .
endif
endif



#
# Board access queue
#
queue-in:
	if [ ! -f $(QUEUE_FILE) ]; then touch $(QUEUE_FILE); chown $(USER):dialout $(QUEUE_FILE); chmod 664 $(QUEUE_FILE); fi;\
	if [ "`head -1 $(QUEUE_FILE)`" != "$(JOB)" ]; then echo $(JOB) >> $(QUEUE_FILE); fi;\
	bash -c "trap 'make queue-out; exit' INT TERM KILL; make queue-wait"

queue-wait:
	while [ "`head -1 $(QUEUE_FILE)`" != "$(JOB)" ]; do echo "Job queued for board access. Queue length: `wc -l $(QUEUE_FILE) | cut -d" " -f1`"; sleep 10s; done

queue-out:
	sed '/$(JOB)/d' $(QUEUE_FILE) > queue; cat queue > $(QUEUE_FILE); rm queue

queue-out-remote:
ifeq ($(BOARD_SERVER),)
	@if [ "`ps aux | grep $(USER) | grep console | grep python3 | grep -v grep`" ]; then \
	kill -9 $$(ps aux | grep $(USER) | grep console | grep python3 | grep -v grep | awk '{print $$2}'); fi
	@if [ "`ps aux | grep $(USER) | grep sha256_test | grep python3 | grep -v grep`" ]; then \
	kill -9 $$(ps aux | grep $(USER) | grep sha256_test | grep python3 | grep -v grep | awk '{print $$2}'); fi
	make queue-out
else
	ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@'
endif

#
# Testing
#

test: clean-testlog test-shortmsg

test-shortmsg: run-shortmsg test-validate

run-shortmsg:
	make -C $(ROOT_DIR) fpga-run INIT_MEM=0 USE_DDR=1 RUN_EXTMEM=0

test-validate: 
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

#
# Profiling
#
profile: clean-all profile1 
	@printf "\n=== PROFILE LOG ===\n"
	@cat $(FPGA_PROFILE_LOG)
	@printf "=== PROFILE LOG ===\n"

profile1:
	make -C $(ROOT_DIR) fpga-run INIT_MEM=0 USE_DDR=1 RUN_EXTMEM=0 PROFILE=1 TEST_LOG="> $(SOC_LOG)"
	@grep "PROFILE:" $(SOC_LOG) > $(FPGA_PROFILE_LOG)

#
# Clean
#

clean-all: hw-clean
	@rm -f $(FPGA_OBJ) $(FPGA_LOG) $(SOC_LOG) $(ETH_LOG)
	@make -C $(SW_TEST_DIR) clean
ifneq ($(FPGA_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) clean CLEANIP=$(CLEANIP)'
endif
ifneq ($(BOARD_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) clean'
endif

#clean test log only when board testing begins
clean-testlog:
	@rm -f test.log $(SOC_LOG) $(ETH_LOG) $(FPGA_PROFILE_LOG)
ifneq ($(BOARD_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@'
endif

.PHONY: run build \
	queue-in queue-out queue-out-remote \
	test test-shortmsg run-shortmsg test-validate \
	profile profile1 \
	clean-all clean-testlog
