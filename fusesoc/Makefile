ROOT_DIR=..

CORE=iobundle:opencryptohw:0.0.1

FPGA_SERVER=$(VIVADO_SERVER)
FPGA_USER=$(VIVADO_USER)

BOARD_SERVER=$(KU40_SERVER)
BOARD_USER=$(KU40_USER)

REMOTE_ROOT_DIR=sandbox/iob-soc-opencryptohw

FUSESOC_BUILD_DIR=build/iobundle_opencryptohw_0.0.1
FPGA_BUILD_DIR=$(FUSESOC_BUILD_DIR)/fpga-vivado

#
# VIRTUAL ENVIRONMENT SETUP
#
VENV=$(ROOT_DIR)/.venv
PYTHON_VENV=$(VENV)/bin/python3
FUSESOC=$(VENV)/bin/fusesoc

setup $(VENV): requirements.txt
	python3 -m venv $(VENV)
	$(PYTHON_VENV) -m pip install --upgrade pip
	$(PYTHON_VENV) -m pip install -r requirements.txt

#
# SIMULATION
#
sim-setup: $(VENV)
	fusesoc run --target=sim --setup $(CORE)

sim-build: 
	# Note: fusesoc --build always runs --setup
	fusesoc run --target=sim --build $(CORE)

sim-run: sim-build
	fusesoc run --target=sim --run $(CORE)

#
# FPGA
#
fpga-setup: $(VENV)
	fusesoc run --target=fpga --setup $(CORE)

fpga-build $(FPGA_BUILD_DIR): $(VENV)
ifeq ($(FPGA_SERVER),)
	# Note: fusesoc --build always runs --setup
	./scripts/source_and_run.sh /opt/Xilinx/Vivado/2020.2/settings64.sh "fusesoc run --target=fpga --build $(CORE)"
else
	ssh $(FPGA_USER)@$(FPGA_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git --exclude .venv $(ROOT_DIR) $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(FPGA_USER)@$(FPGA_SERVER) 'make -C $(REMOTE_ROOT_DIR)/fusesoc fpga-build'
	rsync -avz --exclude .git --exclude .venv $(FPGA_USER)@$(FPGA_SERVER):$(REMOTE_ROOT_DIR)/fusesoc/build .
endif

fpga-run: $(VENV) $(FPGA_BUILD_DIR)
ifeq ($(BOARD_SERVER),)
	# Note: fusesoc --build always runs --setup
	./scripts/source_and_run.sh /opt/Xilinx/Vivado/2020.2/settings64.sh "fusesoc run --target=fpga --run $(CORE)"
else
	ssh $(BOARD_USER)@$(BOARD_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git --exclude .venv $(ROOT_DIR) $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)
	ssh $(BOARD_USER)@$(BOARD_SERVER) 'make -C $(REMOTE_ROOT_DIR)/fusesoc fpga-run'
	# rsync -avz --exclude .git --exclude .venv $(BOARD_USER)@$(BOARD_SERVER):$(REMOTE_ROOT_DIR)/fusesoc/build build
endif

#
# CLEAN
#
clean:
	rm -rf build

.PHONY: setup \
	sim-setup sim-build sim-run \
	fpga-setup fpga-build fpga-run \
	clean
