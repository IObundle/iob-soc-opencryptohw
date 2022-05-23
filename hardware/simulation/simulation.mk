#DEFINES

#default baud and freq for simulation
BAUD ?=5000000
FREQ ?=100000000

#define for testbench
DEFINE+=$(defmacro)BAUD=$(BAUD)
DEFINE+=$(defmacro)FREQ=$(FREQ)

#ddr controller address width
DDR_ADDR_W=$(DCACHE_ADDR_W)

CONSOLE_CMD=$(CONSOLE_DIR)/console -L

#produce waveform dump
VCD ?=0

ifeq ($(VCD),1)
DEFINE+=$(defmacro)VCD
endif

include $(ROOT_DIR)/hardware/hardware.mk

ifeq ($(INIT_MEM),0)
CONSOLE_CMD+=-f
endif

FW_SIZE=$(shell if [ -f firmware.hex ]; then wc -l firmware.hex | awk '{print $$1}'; else echo "0"; fi)

DEFINE+=$(defmacro)FW_SIZE=$(FW_SIZE)
SIM=1
DEFINE+=$(defmacro)SIM=$(SIM)


#SOURCES

#verilog testbench
TB_DIR:=$(HW_DIR)/simulation/verilog_tb

#asic post-synthesis and post-pr sources
ifeq ($(ASIC),1)
ifeq ($(SYNTH),1)
VSRC=$(ASIC_DIR)/system_synth.v
endif
VSRC+=$(wildcard $(ASIC_DIR)/$(ASIC_MEM_FILES))
endif

#axi memory
include $(AXI_DIR)/hardware/axiram/hardware.mk

#TEST OUTPUT
SOC_OUT_BIN:=soc-out.bin

# Simulation images
SIM_IN_BIN:=sim_in.bin

VSRC+=system_top.v

#testbench
ifneq ($(SIMULATOR),verilator)
VSRC+=system_tb.v
endif

#RULES
build: $(VSRC) $(VHDR) $(HEXPROGS) $(SIM_IN_BIN)
ifeq ($(SIM_SERVER),)
	make comp
else
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR) sim-build SIMULATOR=$(SIMULATOR) INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM) VCD=$(VCD) TEST_LOG=\"$(TEST_LOG)\"'
endif

run:
ifeq ($(SIM_SERVER),)
	cp $(FIRM_DIR)/firmware.bin .
	@rm -f soc2cnsl cnsl2soc
	$(CONSOLE_CMD) $(TEST_LOG) &
	bash -c "trap 'make kill-sim' INT TERM KILL EXIT; make exec"
else
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	bash -c "trap 'make kill-remote-sim' INT TERM KILL; ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR) sim-run SIMULATOR=$(SIMULATOR) INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM) VCD=$(VCD) TEST_LOG=\"$(TEST_LOG)\"'"
ifneq ($(TEST_LOG),)
	scp $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/test.log $(SIM_DIR)
endif
ifeq ($(VCD),1)
	scp $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/*.vcd $(SIM_DIR)
endif
endif
ifeq ($(VCD),1)
	if [ ! `pgrep -u $(USER) gtkwave` ]; then gtkwave -a ../waves.gtkw system.vcd; fi &
endif

#
#EDIT TOP OR TB DEPENDING ON SIMULATOR
#

system_tb.v:
	cp $(TB_DIR)/system_core_tb.v $@
	$(if $(HFILES), $(foreach f, $(HFILES), sed -i '/PHEADER/a `include \"$f\"' $@;),) # insert header files

#create  simulation top module
system_top.v: $(TB_DIR)/system_top_core.v
	cp $< $@
	$(foreach p, $(PERIPHERALS), $(eval HFILES=$(shell echo `ls $($p_DIR)/hardware/include/*.vh | grep -v pio | grep -v inst | grep -v swreg`)) \
	$(eval HFILES+=$(notdir $(filter %swreg_def.vh, $(VHDR)))) \
	$(if $(HFILES), $(foreach f, $(HFILES), sed -i '/PHEADER/a `include \"$f\"' $@;),)) # insert header files
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed s/input/wire/ $($p_DIR)/hardware/include/pio.vh | sed s/output/wire/  | sed s/\,/\;/ > wires_tb.vh; sed -i '/PWIRES/r wires_tb.vh' $@; fi;) # declare and insert wire declarations
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed s/input// $($p_DIR)/hardware/include/pio.vh | sed s/output// | sed 's/\[.*\]//' | sed 's/\([A-Za-z].*\),/\.\1(\1),/' > ./ports.vh; sed -i '/PORTS/r ports.vh' $@; fi;) #insert and connect pins in uut instance
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/inst_tb.vh; then sed -i '/endmodule/e cat $($p_DIR)/hardware/include/inst_tb.vh' $@; fi;) # insert peripheral instances


#add peripheral testbench sources
VSRC+=$(foreach p, $(PERIPHERALS), $(shell if test -f $($p_DIR)/hardware/testbench/module_tb.sv; then echo $($p_DIR)/hardware/testbench/module_tb.sv; fi;)) 

kill-remote-sim:
	@echo "INFO: Remote simulator $(SIMULATOR) will be killed"
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'killall -q -u $(SIM_USER) -9 $(SIM_PROC); \
	make -C $(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR) kill-sim'
ifeq ($(VCD),1)
	scp $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/*.vcd $(SIM_DIR)
endif

kill-sim:
	@if [ "`ps aux | grep $(USER) | grep console | grep python3 | grep -v grep`" ]; then \
	kill -9 $$(ps aux | grep $(USER) | grep console | grep python3 | grep -v grep | awk '{print $$2}'); fi

test: clean-testlog test-shortmsg

test-shortmsg: sim-shortmsg validate

sim-shortmsg:
	make -C $(ROOT_DIR) sim-run INIT_MEM=1 USE_DDR=1 RUN_EXTMEM=0 

validate:
	cp $(SOC_OUT_BIN) $(SW_TEST_DIR)/
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

$(SIM_IN_BIN):
	$(eval TEST_VECTOR_RSP_BIN = $(basename $(TEST_VECTOR_RSP))_d_in.bin)
	$(eval TEST_VECTOR_RSP_PATH = $(shell find $(ROOT_DIR) -name "$(TEST_VECTOR_RSP_BIN)"))
	cp $(TEST_VECTOR_RSP_PATH) $(SIM_IN_BIN)

#clean target common to all simulators
clean-remote: hw-clean
	@rm -f soc2cnsl cnsl2soc
	@rm -f system.vcd *.log *.bin
	@make -C $(SW_TEST_DIR) clean
ifneq ($(SIM_SERVER),)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR) sim-clean SIMULATOR=$(SIMULATOR)'
endif

#clean test log only when sim testing begins
clean-testlog:
	@rm -f test.log
ifneq ($(SIM_SERVER),)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'rm -f $(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/test.log'
endif

.PRECIOUS: system.vcd test.log

.PHONY: build run \
	kill-remote-sim kill-sim \
	test test-shortmsg sim-shortmsg validate \
	clean-remote clean-testlog
