#DEFINES

#default baud and freq for simulation
BAUD ?=5000000
FREQ ?=100000000

#define for testbench
DEFINE+=$(defmacro)BAUD=$(BAUD)
DEFINE+=$(defmacro)FREQ=$(FREQ)

#ddr controller address width
DDR_ADDR_W=$(DCACHE_ADDR_W)

CONSOLE_CMD=$(ROOT_DIR)/software/console/console -L

#produce waveform dump
VCD ?=0

ifeq ($(VCD),1)
DEFINE+=$(defmacro)VCD
endif

include $(ROOT_DIR)/hardware/hardware.mk

ifeq ($(INIT_MEM),0)
CONSOLE_CMD+=-f
endif

FW_SIZE=$(shell wc -l firmware.hex | awk '{print $$1}')

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
IMAGES+=$(SIM_IN_BIN)

#testbench
ifeq ($(SIMULATOR),verilator)
VSRC+=system_top.v
else
VSRC+=system_tb.v
endif

#RULES
all: clean sw build sim

sim:
ifeq ($(SIM_SERVER),)
	@rm -f soc2cnsl cnsl2soc
	make $(SIM_PROC)
	$(CONSOLE_CMD) $(TEST_LOG) &
	bash -c "trap 'make kill-sim' INT TERM KILL EXIT; make run"
else
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	bash -c "trap 'make kill-remote-sim' INT TERM KILL; ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR) $@ INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM) VCD=$(VCD) ASIC=$(ASIC) SYNTH=$(SYNTH) ASIC_MEM_FILES=$(ASIC_MEM_FILES) LIBS=$(LIBS) TEST_LOG=\"$(TEST_LOG)\"'"
ifneq ($(TEST_LOG),)
	scp $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/test.log $(SIM_DIR)
endif
ifeq ($(VCD),1)
	scp $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/*.vcd $(SIM_DIR)
endif
endif
ifeq ($(VCD),1)
	if [ "`pgrep -u $(USER) gtkwave`" ]; then killall -q -9 gtkwave; fi
	gtkwave -a ../waves.gtkw system.vcd &
endif

build: $(VSRC) $(VHDR) $(IMAGES)

ifeq ($(SIMULATOR),verilator)
#create top system module
system_top.v: system_top_tmp.v
else
#create testbench
system_tb.v: system_tb_tmp.v
endif
	$(foreach p, $(PERIPHERALS), $(eval HFILES=$(shell echo `ls $($p_DIR)/hardware/include/*.vh | grep -v pio | grep -v inst | grep -v swreg`)) \
	$(eval HFILES+=$(shell echo `basename $($p_DIR)/hardware/include/*swreg.vh | sed 's/swreg/swreg_def/g'`)) \
	$(if $(HFILES), $(foreach f, $(HFILES), sed -i '/PHEADER/a `include \"$f\"' $@;),)) # insert header files
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed s/input/wire/ $($p_DIR)/hardware/include/pio.vh | sed s/output/wire/  | sed s/\,/\;/ > wires_tb.vh; sed -i '/PWIRES/r wires_tb.vh' $@; fi;) # declare and insert wire declarations
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed s/input// $($p_DIR)/hardware/include/pio.vh | sed s/output// | sed 's/\[.*\]//' | sed 's/\([A-Za-z].*\),/\.\1(\1),/' > ./ports.vh; sed -i '/PORTS/r ports.vh' $@; fi;) #insert and connect pins in uut instance
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/inst_tb.vh; then sed -i '/endmodule/e cat $($p_DIR)/hardware/include/inst_tb.vh' $@; fi;) # insert peripheral instances

system_tb_tmp.v: $(TB_DIR)/system_core_tb.v
	cp $< $@; cp $@ system_tb.v

system_top_tmp.v: $(TB_DIR)/system_top_core.v
	cp $< $@; cp $@ system_top.v

VSRC+=$(foreach p, $(PERIPHERALS), $(shell if test -f $($p_DIR)/hardware/testbench/module_tb.sv; then echo $($p_DIR)/hardware/testbench/module_tb.sv; fi;)) #add test cores to list of sources

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

test-shortmsg: sim-shortmsg validate

sim-shortmsg:
	make -C $(SIM_DIR) all INIT_MEM=1 USE_DDR=0 RUN_EXTMEM=0 

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
ifneq ($(SIM_SERVER),)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR) sim-clean SIMULATOR=$(SIMULATOR)'
endif

#clean test log only when sim testing begins
clean-testlog:
	@rm -f test.log
	make -C $(SW_TEST_DIR) clean
ifneq ($(SIM_SERVER),)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(SIM_SYNC_FLAGS) $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_SSH_FLAGS) $(SIM_USER)@$(SIM_SERVER) 'rm -f $(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/test.log'
endif

clean-all: clean-testlog clean

.PRECIOUS: system.vcd test.log

.PHONY: all sim build\
	kill-remote-sim kill-sim\
	test test-shortmsg sim-shortmsg validate \
	clean-remote clean-testlog clean-all
