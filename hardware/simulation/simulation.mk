#DEFINES

#default baud and freq for simulation
BAUD ?=5000000
FREQ ?=100000000

#define for testbench
DEFINE+=$(defmacro)BAUD=$(BAUD)
DEFINE+=$(defmacro)FREQ=$(FREQ)

#ddr controller address width
DDR_ADDR_W=$(FIRM_ADDR_W)

#produce waveform dump
VCD ?=0

ifeq ($(VCD),1)
DEFINE+=$(defmacro)VCD
endif

include $(ROOT_DIR)/hardware/hardware.mk

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
VSRC+=$(CACHE_DIR)/submodules/MEM/submodules/LIB/submodules/AXI/rtl/axi_ram.v
#testbench
VSRC+=system_tb.v

#TEST VECTOR
SIM_LOG:=$(lastword $(TEST_LOG))
SIM_PARSED_LOG:=$(SIM_LOG)_parsed.log
TEST_VECTOR_RSP:=$(SW_TEST_DIR)/SHA256ShortMsg.rsp
VALIDATION_LOG:=validation.log

#RULES
all: clean sw
ifeq ($(SIM_SERVER),)
	make run 
else
	ssh $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	bash -c "trap 'make kill-remote-sim' INT TERM KILL; ssh $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR) run INIT_MEM=$(INIT_MEM) USE_DDR=$(USE_DDR) RUN_EXTMEM=$(RUN_EXTMEM) VCD=$(VCD) ASIC=$(ASIC) SYNTH=$(SYNTH) ASIC_MEM_FILES=$(ASIC_MEM_FILES) LIBS=$(LIBS) TEST_LOG=\"$(TEST_LOG)\"'"
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


#create testbench
system_tb.v: system_tb_tmp.v
	$(foreach p, $(PERIPHERALS), $(shell sed -i '/PHEADER/a `include \"$p.vh\"' $@)) # insert peripheral header file
	$(foreach p, $(PERIPHERALS), $(shell sed -i '/PHEADER/a `include \"$(p)sw_reg_def.vh\"' $@)) # insert register address header file
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed s/input/wire/ $($p_DIR)/hardware/include/pio.vh | sed s/output/wire/  | sed s/\,/\;/ > wires_tb.vh; sed -i '/PWIRES/r wires_tb.vh' $@; fi;) # declare and insert wire declarations
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed s/input// $($p_DIR)/hardware/include/pio.vh | sed s/output// | sed 's/\[.*\]//' | sed 's/\([A-Za-z].*\),/\.\1(\1),/' > ./ports.vh; sed -i '/PORTS/r ports.vh' $@; fi;) #insert and connect pins in uut instance
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/inst_tb.vh; then sed -i '/endmodule/e cat $($p_DIR)/hardware/include/inst_tb.vh' $@; fi;) # insert peripheral instances

system_tb_tmp.v: $(TB_DIR)/system_core_tb.v
	cp $< $@; cp $@ system_tb.v

VSRC+=$(foreach p, $(PERIPHERALS), $(shell if test -f $($p_DIR)/hardware/testbench/module_tb.sv; then echo $($p_DIR)/hardware/testbench/module_tb.sv; fi;)) #add test cores to list of sources

kill-remote-sim:
	@echo "INFO: Remote simulator $(SIMULATOR) will be killed"
	ssh $(SIM_USER)@$(SIM_SERVER) 'killall -q -u $(SIM_USER) -9 $(SIM_PROC)'

test: clean-testlog test-shortmsg
	if cmp --silent $(VALIDATION_LOG) $(SIM_PARSED_LOG); then printf "\n\nShortMessage Test PASSED\n\n"; else printf "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)

test-shortmsg: sim-shortmsg parse-log 

sim-shortmsg:
	make -C $(SIM_DIR) all INIT_MEM=1 USE_DDR=0 RUN_EXTMEM=0 TEST_LOG="$(TEST_LOG)"

parse-log: $(SIM_LOG)
	sed -n -e '/\[L = /,$$p' $(SIM_LOG) | tac | sed -n -e '/MD =/,$$p' | tac > $(SIM_PARSED_LOG)
	echo "" >> $(SIM_PARSED_LOG) # add final newline
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars

#clean target common to all simulators
clean-remote: hw-clean 
	@rm -f system.vcd *.log
ifneq ($(SIM_SERVER),)
	ssh $(SIM_USER)@$(SIM_SERVER) 'if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi'
	rsync -avz --delete --exclude .git $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_USER)@$(SIM_SERVER) 'make -C $(REMOTE_ROOT_DIR) sim-clean SIMULATOR=$(SIMULATOR)'
endif

#clean test log only when sim testing begins
clean-testlog:
	@rm -f test.log
ifneq ($(SIM_SERVER),)
	ssh $(SIM_USER)@$(SIM_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(SIM_USER)@$(SIM_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(SIM_USER)@$(SIM_SERVER) 'rm -f $(REMOTE_ROOT_DIR)/hardware/simulation/$(SIMULATOR)/test.log'
endif

.PRECIOUS: system.vcd test.log

.PHONY: all \
	kill-remote-sim \
	test test-shortmsg sim-shortmsg parse-log \
	clean-remote clean-testlog
