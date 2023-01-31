ROOT_DIR:=../../../../..

defmacro:=-D
incdir:=-I

SIMULATOR=yosys-icarus
SIM_SERVER=$(IVSIM_SERVER)
SIM_USER=$(IVSIM_USER)
SIM_PROC=a.out

VSRC+=$(wildcard src/*v)

DEFINE+=$(defmacro)YOSYS_SIM=1

#simulator flags
VLOG = iverilog -Wall -g2012 $(INCLUDE) $(DEFINE)

#ECP5 source files
# ECP5_SRC = /usr/share/yosys/ecp5
ECP5_SRC = $(HOME)/opt/oss-cad-suite/share/yosys/ecp5

#generated verilog from yosys
GENERATED_VERILOG = SYNTH.v

#compile into netlist
comp: $(SIM_PROC)

$(SIM_PROC): $(GENERATED_VERILOG)
	$(VLOG) system_tb.v $(GENERATED_VERILOG) $(ECP5_SRC)/cells_bb.v -o $(SIM_PROC) -I $(ECP5_SRC) -l$(ECP5_SRC)/cells_sim.v

$(GENERATED_VERILOG): verilog-sources
	tclsh generate_yosys_verilog.tcl "$(INCLUDE)" "$(DEFINE)" "$(VSRC)" "$(GENERATED_VERILOG)"

verilog-sources:
	mkdir -p src
	make -C $(ROOT_DIR) openlane-setup
	cp ../../system/src/*.v src/
	cp ../../system/src/*.vh src/

exec:
	./$(SIM_PROC)

clean: 
	@rm -f $(SIM_PROC) *.log *.ys *.json *.blif

very-clean: clean
	@rm -rf src
	@make -C $(ROOT_DIR) openlane-clean

.PHONY: comp exec clean debug very-clean verilog-sources
