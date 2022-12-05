ROOT_DIR=..

CORE=iobundle:opencryptohw:0.0.1

#
# VIRTUAL ENVIRONMENT SETUP
#
VENV=$(ROOT_DIR)/.venv
PYTHON_VENV=$(VENV)/bin/python3
FUSESOC=$(VENV)/bin/fusesoc

$(VENV): requirements.txt
	python3 -m venv $(VENV)
	$(PYTHON_VENV) -m pip install --upgrade pip
	$(PYTHON_VENV) -m pip install -r requirements.txt

setup: $(VENV)

sim-setup: $(VENV)
	fusesoc run --target=sim --setup $(CORE)

sim-build: $(VENV)
	fusesoc run --target=sim --setup --build $(CORE)

sim: $(VENV)
	fusesoc run --target=sim $(CORE)

fpga-setup: $(VENV)
	fusesoc run --target=fpga --setup $(CORE)
    
fpga-build: $(VENV)
	source /opt/Xilinx/Vivado/2020.2/settings64.sh; \
		fusesoc run --target=fpga --setup --build $(CORE)

fpga: $(VENV)
	source /opt/Xilinx/Vivado/2020.2/settings64.sh; \
		fusesoc run --target=fpga $(CORE)

clean:
	rm -rf build
