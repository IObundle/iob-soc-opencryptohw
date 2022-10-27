# Tester configuration file
# Use this file to set/override tester parameters and makefile targets
#
ifneq ($(INCLUDING_VARS),)
# MAKEFILE VARIABLES: PLACE BELOW VARIABLES USED BY TESTER
#

# Name of this unit under test
UUT_NAME=IOBSOCSHA

#FIRMWARE SIZE (LOG2)
FIRM_ADDR_W:=16

#SRAM SIZE (LOG2)
SRAM_ADDR_W:=16

#DDR
USE_DDR:=1
RUN_EXTMEM:=0

#DATA CACHE ADDRESS WIDTH (tag + index + offset)
DCACHE_ADDR_W:=24

#ROM SIZE (LOG2)
BOOTROM_ADDR_W:=12

#PRE-INIT MEMORY WITH PROGRAM AND DATA
INIT_MEM:=1

#SIMULATION
#default simulator running locally or remotely
#check the respective Makefile in TESTER/hardware/simulation/$(SIMULATOR) for specific settings
SIMULATOR ?=verilator

#BOARD
#default board running locally or remotely
#check the respective Makefile in TESTER/hardware/fpga/$(BOARD) for specific settings
BOARD ?=AES-KU040-DB-G

#Add Unit Under Test to Tester peripherals list
#this works even if UUT is not a "peripheral"
PERIPHERALS+=$(UUT_NAME)[\`ADDR_W,\`DATA_W,AXI_ID_W]
# Tester peripherals to add (besides the default ones in IOb-SoC-Tester)
PERIPHERALS+=UART
# Instance 0 of ETHERNET has default MAC address. Instance 1 has the same MAC address as the console (this way, the UUT always connects to the console's MAC address).
PERIPHERALS+=ETHERNET
PERIPHERALS+=ETHERNET[32,\`iob_eth_swreg_ADDR_W,48'h$(RMAC_ADDR)]
#Clock generator for internal ethernet interfaces
PERIPHERALS+=ETHCLOCKGEN

# Submodule paths for Tester peripherals (listed above)
ETHERNET_DIR=$($(UUT_NAME)_DIR)/submodules/ETHERNET
ETHCLOCKGEN_DIR=$($(UUT_NAME)_DIR)/submodules/ETHCLOCKGEN

#Root directory on remote machines
REMOTE_UUT_DIR ?=sandbox/iob-soc-sha

#Mac address of pc interface connected to ethernet peripheral
ifeq ($(BOARD),AES-KU040-DB-G) # Arroz eth if mac
RMAC_ADDR:=4437e6a6893b
else # Pudim eth if mac
RMAC_ADDR:=309c231e624a
endif
#Auto-set ethernet interface name based on MAC address
ETH_IF:=$(shell ip -br link | sed 's/://g' | grep $(RMAC_ADDR) | cut -d " " -f1)

#Configure Tester to use ethernet
USE_ETHERNET:=1
DEFINE+=$(defmacro)USE_ETHERNET=1

#Use UART to transfer files from/to console, as these transfers are not compatible with ETHERNET during simulation.
ifneq ($(ISSIMULATION),)
DEFINE+=$(defmacro)SIM=1
endif

#Extra tester target dependencies
#Run before building system
BUILD_DEPS+=$($(UUT_NAME)_DIR)/hardware/src/system.v
#Run before building system for simulation
SIM_DEPS+=set-simulation-variable $(SIM_DIR)/sim_in.bin $(SIM_DIR)/soc_out.bin
#Run before building system for fpga
FPGA_DEPS+=$(BOARD_DIR)/sim_in.bin $(BOARD_DIR)/soc_out.bin
#Run when cleaning tester
CLEAN_DEPS+=clean-top-module clean-files
#Run after finishing fpga run (useful to copy files from remote machines at the end of a run sequence)
FPGA_POST_RUN_DEPS+=

# Verification filename
TEST_VECTOR_RSP ?=SHA256ShortMsg.rsp

#
else
# MAKEFILE TARGETS: PLACE BELOW EXTRA TARGETS USED BY TESTER
#

#Target to build UUT topsystem
$($(UUT_NAME)_DIR)/hardware/src/system.v:
	make -C $($(UUT_NAME)_DIR)/hardware/src -f ../hardware.mk system.v ROOT_DIR=../..

clean-top-module:
	rm -f $($(UUT_NAME)_DIR)/hardware/src/system.v

#Target to build UUT bootloader and firmware
$($(UUT_NAME)_DIR)/software/firmware/boot.hex $($(UUT_NAME)_DIR)/software/firmware/firmware.hex:
	make -C $($(UUT_NAME)_DIR)/software/firmware build-all BAUD=$(BAUD) FREQ=$(FREQ) BOARD=$(BOARD)
	make -C $($(UUT_NAME)_DIR)/software/firmware -f ../../hardware/hardware.mk boot.hex firmware.hex ROOT_DIR=../..

#Targets to generate and copy sim_in.bin, soc_out.bin
$($(UUT_NAME)_DIR)/software/test/$(basename $(TEST_VECTOR_RSP))_d_in.bin $($(UUT_NAME)_DIR)/software/test/$(basename $(TEST_VECTOR_RSP))_d_out.bin:
	make -C $($(UUT_NAME)_DIR)/software/test gen_test_data TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

%/sim_in.bin: $($(UUT_NAME)_DIR)/software/test/$(basename $(TEST_VECTOR_RSP))_d_in.bin
	ln -sr $< $@

%/soc_out.bin: $($(UUT_NAME)_DIR)/software/test/$(basename $(TEST_VECTOR_RSP))_d_out.bin
	ln -sr $< $@

#Cleanup targets
clean-files:
	rm -f $(SIM_DIR)/sim_in.bin $(SIM_DIR)/soc_out.bin
	rm -f $(BOARD_DIR)/sim_in.bin $(BOARD_DIR)/soc_out.bin

#Set ISSIMULATION variable
set-simulation-variable:
	$(eval export ISSIMULATION=1)

.PHONY: clean-top-module set-simulation-variable
endif
