SHELL := /bin/bash

ROOT_DIR:=../../../../..

defmacro:=-D
incdir:=-I

SIMULATOR=verilator
SIM_SERVER=$(VSIM_SERVER)
SIM_USER=$(VSIM_USER)
SIM_PROC=Vsystem_top

HDR=iob_uart_swreg.h

include post-synth.mk

# remove space between -I and directory for verilator
INCLUDE_VERI=$(subst -I ,-I,$(INCLUDE))

#simulator flags
VLOG = iverilog -W all -g2005-sv $(INCLUDE) $(DEFINE) -s system_top

comp: $(SIM_PROC)

#simulation executable
$(SIM_PROC):
	$(VLOG) $(VSRC)

exec:
	./$(SIM_PROC)

clean: clean-remote
	@rm -f $(SIM_PROC)

.PHONY: comp exec clean
