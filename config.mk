######################################################################
#
# IOb-SoC Configuration File
#
######################################################################

IOBSOC_NAME:=IOBSOCSHA

TOP_MODULE:=system

#
# PRIMARY PARAMETERS: CAN BE CHANGED BY USERS OR OVERRIDEN BY ENV VARS
#

#TEST VECTOR
TEST_VECTOR_RSP ?=SHA256ShortMsg.rsp

#CPU ARCHITECTURE
DATA_W := 32
ADDR_W := 32

#FIRMWARE SIZE (LOG2)
FIRM_ADDR_W ?=16

#SRAM SIZE (LOG2)
SRAM_ADDR_W ?=16

#DDR
USE_DDR ?=1
RUN_EXTMEM ?=0

#DATA CACHE ADDRESS WIDTH (tag + index + offset)
DCACHE_ADDR_W:=24

#ROM SIZE (LOG2)
BOOTROM_ADDR_W:=12

#PRE-INIT MEMORY WITH PROGRAM AND DATA
INIT_MEM ?=1

#ETHERNET RMAC_ADDR
ifeq ($(BOARD),AES-KU040-DB-G) # Arroz eth if mac
RMAC_ADDR:=4437e6a6893b
else # Pudim eth if mac
RMAC_ADDR:=309c231e624b
endif

#PERIPHERAL LIST
#list with corename of peripherals to be attached to peripheral bus.
#to include multiple instances, write the corename of the peripheral multiple times.
#to pass verilog parameters to each instance, type the parameters inside parenthesis.
#Example: 'PERIPHERALS ?=UART[1,\"textparam\"] UART UART' will create 3 UART instances, 
#         the first one will be instantiated with verilog parameters 1 and "textparam", 
#         the second and third will use default parameters.
PERIPHERALS ?=UART TIMER ETHERNET

#RISC-V HARD MULTIPLIER AND DIVIDER INSTRUCTIONS
USE_MUL_DIV ?=1

#RISC-V COMPRESSED INSTRUCTIONS
USE_COMPRESSED ?=1

#ROOT DIRECTORY ON REMOTE MACHINES
REMOTE_ROOT_DIR ?=sandbox/iob-soc-sha

#SIMULATION
#default simulator running locally or remotely
#check the respective Makefile in hardware/simulation/$(SIMULATOR) for specific settings
SIMULATOR ?=icarus

#BOARD
#default board running locally or remotely
#check the respective Makefile in hardware/fpga/$(BOARD) for specific settings
BOARD ?=AES-KU040-DB-G

#IOB LIBRARY
UART_HW_DIR:=$(UART_DIR)/hardware

####################################################################
# DERIVED FROM PRIMARY PARAMETERS: DO NOT CHANGE BELOW THIS POINT
####################################################################

ifeq ($(RUN_EXTMEM),1)
DEFINE+=$(defmacro)RUN_EXTMEM
USE_DDR=1
endif

ifeq ($(USE_DDR),1)
DEFINE+=$(defmacro)USE_DDR
endif

ifeq ($(INIT_MEM),1)
DEFINE+=$(defmacro)INIT_MEM
endif

#submodule paths
VEXRISCV_DIR=$(ROOT_DIR)/submodules/VEXRISCV
CACHE_DIR=$(ROOT_DIR)/submodules/CACHE
UART_DIR=$(ROOT_DIR)/submodules/UART
TIMER_DIR=$(ROOT_DIR)/submodules/TIMER
ETHERNET_DIR=$(ROOT_DIR)/submodules/ETHERNET
LIB_DIR=$(ROOT_DIR)/submodules/LIB
MEM_DIR=$(ROOT_DIR)/submodules/MEM
AXI_DIR=$(ROOT_DIR)/submodules/AXI

#sw paths
SW_DIR:=$(ROOT_DIR)/software
PC_DIR:=$(SW_DIR)/pc-emul
FIRM_DIR:=$(SW_DIR)/firmware
BOOT_DIR:=$(SW_DIR)/bootloader
CONSOLE_DIR:=$(SW_DIR)/console
SW_TEST_DIR:=$(SW_DIR)/test

#scripts paths
PYTHON_DIR=$(LIB_DIR)/software/python

#hw paths
HW_DIR=$(ROOT_DIR)/hardware
SIM_DIR=$(HW_DIR)/simulation/$(SIMULATOR)
BOARD_DIR ?=$(shell find $(ROOT_DIR)/hardware -name $(BOARD))

#doc paths
DOC_DIR=$(ROOT_DIR)/document

#macro to return all defined directories separated by newline
GET_DIRS= $(eval ROOT_DIR_TMP:=$(ROOT_DIR))\
          $(eval override ROOT_DIR=.)\
          $(foreach V,$(sort $(.VARIABLES)),\
          $(if $(filter $(addsuffix _DIR, $(shell $(SW_DIR)/python/submodule_utils.py remove_duplicates_and_params "$(PERIPHERALS)")), $(filter %_DIR, $V)),\
          $V=$($V);))\
          $(eval override ROOT_DIR:=$(ROOT_DIR_TMP))

#define macros
DEFINE+=$(defmacro)DATA_W=$(DATA_W)
DEFINE+=$(defmacro)ADDR_W=$(ADDR_W)
DEFINE+=$(defmacro)BOOTROM_ADDR_W=$(BOOTROM_ADDR_W)
DEFINE+=$(defmacro)SRAM_ADDR_W=$(SRAM_ADDR_W)
DEFINE+=$(defmacro)FIRM_ADDR_W=$(FIRM_ADDR_W)
DEFINE+=$(defmacro)DCACHE_ADDR_W=$(DCACHE_ADDR_W)
DEFINE+=$(defmacro)N_SLAVES=$(shell $(SW_DIR)/python/submodule_utils.py get_n_slaves "$(PERIPHERALS)") #peripherals
DEFINE+=$(defmacro)N_SLAVES_W=$(shell $(SW_DIR)/python/submodule_utils.py get_n_slaves_w "$(PERIPHERALS)")

#address selection bits
E:=31 #extra memory bit
P:=30 #periphs
B:=29 #boot controller

DEFINE+=$(defmacro)E=$E
DEFINE+=$(defmacro)P=$P
DEFINE+=$(defmacro)B=$B

#PERIPHERAL IDs
#assign a sequential ID to each peripheral
#the ID is used as an instance name index in the hardware and as a base address in the software
DEFINE+=$(shell $(SW_DIR)/python/submodule_utils.py get_defines "$(PERIPHERALS)" "$(defmacro)")

#RULES

#kill "console", the background running program seriving simulators,
#emulators and boards
CNSL_PID:=ps aux | grep $(USER) | grep console | grep python3 | grep -v grep
kill-cnsl:
	@if [ "`$(CNSL_PID)`" ]; then \
	kill -9 $$($(CNSL_PID) | awk '{print $$2}'); fi

gen-clean:
	@rm -f *# *~

.PHONY: gen-clean kill-cnsl
