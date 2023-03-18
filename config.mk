######################################################################
#
# IOb-SoC Configuration File
#
######################################################################

IOBSOC_NAME:=IOBSOCOPENCRYPTOHW

#
# PRIMARY PARAMETERS: CAN BE CHANGED BY USERS OR OVERRIDEN BY ENV VARS
#

#ALGORITHM (SHA256, AES256, MCELIECE)
ALGORITHM ?= MCELIECE
#TEST VECTOR
ifeq ($(ALGORITHM),SHA256)
TEST_VECTOR_RSP ?= $(ALGORITHM)/SHA256ShortMsg.rsp
HARDWARE_TEST = 2
endif
ifeq ($(ALGORITHM),AES256)
TEST_VECTOR_RSP ?= $(ALGORITHM)/AESECB256.rsp
HARDWARE_TEST = 10
endif
ifeq ($(ALGORITHM),MCELIECE)
# TEST_VECTOR_RSP ?= $(ALGORITHM)/kat_kem.rsp
TEST_VECTOR_RSP ?= $(ALGORITHM)/kat_kem_reduce.rsp
HARDWARE_TEST = 13
endif

#CPU ARCHITECTURE
DATA_W := 32
ADDR_W := 32

#FIRMWARE SIZE (LOG2)
FIRM_ADDR_W ?=20

#SRAM SIZE (LOG2)
SRAM_ADDR_W ?=12

#DDR
USE_DDR ?=1
RUN_EXTMEM ?=1

#DATA CACHE ADDRESS WIDTH (tag + index + offset)
DCACHE_ADDR_W:=24

#ROM SIZE (LOG2)
BOOTROM_ADDR_W:=12

#PRE-INIT MEMORY WITH PROGRAM AND DATA
INIT_MEM ?=1

#RMAC ADDRESS
RMAC_ADDR := 4437e6a6893b

#PERIPHERAL LIST
#must match respective submodule CORE_NAME in the core.mk file of the submodule
#PERIPHERALS:=UART
PERIPHERALS ?=UART TIMER VERSAT ETHERNET

#RISC-V HARD MULTIPLIER AND DIVIDER INSTRUCTIONS
USE_MUL_DIV ?=1

#RISC-V COMPRESSED INSTRUCTIONS
USE_COMPRESSED ?=1

#ROOT DIRECTORY ON REMOTE MACHINES
REMOTE_ROOT_DIR ?=sandbox/iob-soc-opencryptohw

#SIMULATION
#default simulator running locally or remotely
#check the respective Makefile in hardware/simulation/$(SIMULATOR) for specific settings
SIMULATOR ?=verilator

#BOARD
#default board running locally or remotely
#check the respective Makefile in hardware/fpga/$(BOARD) for specific settings
BOARD ?=AES-KU040-DB-G

#DOCUMENTATION
#default document to compile
DOC ?= pb

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
PICORV32_DIR=$(ROOT_DIR)/submodules/PICORV32
CACHE_DIR=$(ROOT_DIR)/submodules/CACHE
UART_DIR=$(ROOT_DIR)/submodules/UART
TIMER_DIR=$(ROOT_DIR)/submodules/TIMER
ETHERNET_DIR=$(ROOT_DIR)/submodules/ETHERNET
VERSAT_DIR=$(ROOT_DIR)/submodules/VERSAT
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
BOARD_DIR ?=$(shell find hardware -name $(BOARD))

#doc paths
DOC_DIR=$(ROOT_DIR)/document/$(DOC)

#define macros
DEFINE+=$(defmacro)DATA_W=$(DATA_W)
DEFINE+=$(defmacro)ADDR_W=$(ADDR_W)
DEFINE+=$(defmacro)BOOTROM_ADDR_W=$(BOOTROM_ADDR_W)
DEFINE+=$(defmacro)SRAM_ADDR_W=$(SRAM_ADDR_W)
DEFINE+=$(defmacro)FIRM_ADDR_W=$(FIRM_ADDR_W)
DEFINE+=$(defmacro)DCACHE_ADDR_W=$(DCACHE_ADDR_W)
DEFINE+=$(defmacro)N_SLAVES=$(N_SLAVES) #peripherals

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
N_SLAVES:=0
$(foreach p, $(PERIPHERALS), $(eval $p=$(N_SLAVES)) $(eval N_SLAVES:=$(shell expr $(N_SLAVES) \+ 1)))
$(foreach p, $(PERIPHERALS), $(eval DEFINE+=$(defmacro)$p=$($p)))

N_SLAVES_W = $(shell echo "import math; print(math.ceil(math.log($(N_SLAVES),2)))"|python3 )
DEFINE+=$(defmacro)N_SLAVES_W=$(N_SLAVES_W)

ifneq ($(HARDWARE_TEST),)
DEFINE+=$(defmacro)HARDWARE_TEST=$(HARDWARE_TEST)
endif

#BOARD and FREQ
#default baud and system clock frequency
SIM_BAUD = 2500000
SIM_FREQ =50000000
#default baud and frequency if not given
BAUD ?=$(SIM_BAUD)
FREQ ?=$(SIM_FREQ)
#default board running locally or remotely
BOARD_DIR =$(shell find hardware -name $(BOARD))
#default baud and system clock freq for boards
BOARD_BAUD = 115200
#default board frequency
BOARD_FREQ ?=100000000
ifeq ($(BOARD), CYCLONEV-GT-DK)
BOARD_FREQ =50000000
endif



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
