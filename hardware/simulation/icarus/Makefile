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

exec:
	./$(SIM_PROC)

clean: clean-remote
	@rm -f $(SIM_PROC)

xunitM: $(XUNITM_VSRC)
	make -C $(SW_TEST_DIR) gen_xunitM_bin
	cp $(SW_TEST_DIR)/xunitM_*.bin .
	touch xversat.vh
	$(VLOG) $(XUNITM_VSRC)
	./$(SIM_PROC)

xunitF: $(XUNITF_VSRC)
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

.PHONY: comp exec clean debug xunitM xunitF
