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
HOST_LOG:=host.log
FPGA_PROFILE_LOG:=fpga_profile.log

#OUTPUT BIN
SOC_OUT_BIN:=soc-out.bin

FORCE ?= 1

#RULES

#
# Use
#
all: build run
	

run:
ifeq ($(NORUN),0)
ifeq ($(BOARD_SERVER),)
	if [ ! -f $(LOAD_FILE) ]; then touch $(LOAD_FILE); chown $(USER):dialout $(LOAD_FILE); chmod 664 $(LOAD_FILE); fi;\
	bash -c "trap 'make queue-out' INT TERM KILL; make queue-in; if [ $(FORCE) = 1 -o \"`head -1 $(LOAD_FILE)`\" != \"$(JOB)\" ];\
	then ../prog.sh; echo $(JOB) > $(LOAD_FILE); fi; rm -f $(CONSOLE_DIR)/test.log; make -C $(CONSOLE_DIR) run; make queue-out;\
	if [ -f $(CONSOLE_DIR)/$(lastword $(TEST_LOG)) ]; then cat $(CONSOLE_DIR)/$(lastword $(TEST_LOG)) $(TEST_LOG); fi;\
	rm -f $(CONSOLE_DIR)/$(lastword $(TEST_LOG));\
	if ls $(CONSOLE_DIR)/*.log > /dev/null 2>&1; then cp $(CONSOLE_DIR)/*.log .; fi"
else
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR) 
	bash -c "trap 'make queue-out-remote' INT TERM KILL; ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@ INIT_MEM=$(INIT_MEM) FORCE=$(FORCE) TEST_LOG=\"$(TEST_LOG)\"'"
	scp -r $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/*.log . 2>/dev/null || :
	scp -r $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/software/python/*.bin . 2>/dev/null || :
endif
endif

build: sw $(FPGA_OBJ)

ifeq ($(INIT_MEM),1)
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex firmware.hex
else
$(FPGA_OBJ): $(wildcard *.sdc) $(VSRC) $(VHDR) boot.hex
endif
ifeq ($(NORUN),0)
ifeq ($(FPGA_SERVER),)
	../build.sh "$(INCLUDE)" "$(DEFINE)" "$(VSRC)" "$(DEVICE)"
	make post-build
else 
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@ INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM)'
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_OBJ) $(FPGA_OBJ)
	scp $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD)/$(FPGA_LOG) $(FPGA_LOG) 
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
	make queue-out
	@if [ "`ps aux | grep $(USER) | grep console | grep python3 | grep -v grep`" ]; then \
	kill -9 $$(ps aux | grep $(USER) | grep console | grep python3 | grep -v grep | awk '{print $$2}'); fi
else
	ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@'
endif

#
# Testing
#

test: clean-testlog test-shortmsg

test-shortmsg: run-shortmsg test-validate

run-shortmsg:
	make all INIT_MEM=1 USE_DDR=0 RUN_EXTMEM=0

test-validate: 
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

#
# Profiling
#
profile: clean-all $(FPGA_PROFILE_LOG)
	@printf "\n=== PROFILE LOG ===\n"
	@cat $(FPGA_PROFILE_LOG)
	@printf "=== PROFILE LOG ===\n"

$(FPGA_PROFILE_LOG): $(SOC_LOG)
	@grep "PROFILE:" $< > $@

$(SOC_LOG):
	make all PROFILE=1

#
# Clean
#

clean-remote: hw-clean
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
	@rm -f test.log $(FPGA_PROFILE_LOG) 
	@make -C $(SW_TEST_DIR) clean
ifneq ($(FPGA_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@'
endif
ifneq ($(BOARD_SERVER),)
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/fpga/$(TOOL)/$(BOARD) $@'
endif

clean-all: clean-testlog clean
	@rm -f $(FPGA_OBJ) $(FPGA_LOG)
	@rm -f $(SOC_LOG) $(HOST_LOG)

.PRECIOUS: $(FPGA_OBJ) $(FPGA_PROFILE_LOG)

.PHONY: all run build \
	queue-in queue-out queue-wait queue-out-remote \
	test test-shortmsg run-shortmsg parse-log\
	profile\
	clean-remote clean-testlog clean-all
