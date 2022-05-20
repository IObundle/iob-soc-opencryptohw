ROOT_DIR:=../../..

defmacro:=-D
incdir:=-I

SIMULATOR=icarus
SIM_SERVER=$(IVSIM_SERVER)
SIM_USER=$(IVSIM_USER)
SIM_PROC=a.out

include ../simulation.mk

#simulator flags
VLOG = iverilog -W all -g2005-sv $(INCLUDE) $(DEFINE)

comp: $(SIM_PROC)

#simulation executable
$(SIM_PROC):
	$(VLOG) $(VSRC)

.PHONY: run clean debug

exec:
	./$(SIM_PROC)

clean: clean-remote
	@rm -f $(SIM_PROC)

# xunitM tests
ifeq ($(SPINAL),)
XUNITM_VSRC=$(HW_DIR)/src/units/xunitM.v
else
XUNITM_VSRC=$(HW_DIR)/src/spinalHDL/rtl/xunitM.v
endif
XUNITM_VSRC+=$(HW_DIR)/simulation/verilog_tb/test_xunitM_tb.v

xunitM:
	make -C $(SW_TEST_DIR) gen_xunitM_bin
	cp $(SW_TEST_DIR)/xunitM_*.bin .
	touch xversat.vh
	$(VLOG) $(XUNITM_VSRC)
	./$(SIM_PROC)

# xunitF tests
XUNITF_VSRC=$(HW_DIR)/src/units/xunitF.v
XUNITF_VSRC+=$(HW_DIR)/simulation/verilog_tb/test_xunitF_tb.v

xunitF:
	make -C $(SW_TEST_DIR) gen_xunitF_bin
	cp $(SW_TEST_DIR)/xunitF_*.bin .
	touch xversat.vh
	$(VLOG) $(XUNITF_VSRC)
	./$(SIM_PROC)

debug:
	@echo $(SUBMODULES)
	@echo $(MODULES)
	@echo $(MEM_MODULES)
	@echo $(AXIMEM_DIR)
	@echo $(INTERCON_DIR)
	@echo $(MEM_DIR)
	@echo $(CPU_DIR)
	@echo $(CACHE_DIR)
	@echo $(UART_DIR)
	@echo $(VHDR)
	@echo $(VSRC)
	@echo $(INCLUDE)
	@echo $(DEFINE)
	@echo $(MODULE)

.PHONY: comp exec clean debug xunitM
