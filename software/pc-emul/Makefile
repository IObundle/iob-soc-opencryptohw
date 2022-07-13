ROOT_DIR:=../..

include ../software.mk

BUILD_DIR=./build
DEBUG_DIR=./debug
VERSAT_PRG_SRC_DIR=./src

#additional compiler flags
CFLAGS=-Os -std=gnu99 -Wl,--strip-debug -g -m32

#DEFINE+=-DLONGLONG 
DEFINE+=$(defmacro)PC

SOC_LOG:=soc.log
ETH_LOG:=ethernet.log
EMUL_PROFILE_LOG:=emul_profile.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $($p_DIR)/software/pc-emul/pc-emul.mk))

#HEADERS
HDR+=periphs.h 

#INCLUDES
INCLUDE+=$(incdir)$(FIRM_DIR)/units

#SOURCES
SRC+= $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c

SOC_OUT_BIN:=soc-out.bin

#units to verilate
VERILATE_UNIT = xunitF xunitM
HDR+=$(foreach obj,$(VERILATE_UNIT),$(BUILD_DIR)/V$(obj).h)

#unit wrapper
CPP_OBJ+=$(BUILD_DIR)/unitWrapper.o

#firmware object
CPP_OBJ+=$(BUILD_DIR)/versat_sha.o

#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
INCLUDE+=$(incdir)$(FIRM_DIR)
endif

#CONSOLE
CONSOLE_CMD=$(ROOT_DIR)/software/console/eth_console -L

#RULES

build: gen_data fw_emul
	mkdir -p $(DEBUG_DIR)
	mkdir -p $(VERSAT_PRG_SRC_DIR)

fw_emul: $(HDR) $(SRC) $(CPP_OBJ) $(BUILD_DIR)/firmware.o
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(BUILD_DIR)/*o -lgcc -lc -lstdc++ -lm

run: build
	@rm -f soc2cnsl cnsl2soc
	$(CONSOLE_CMD) $(TEST_LOG) &
	./fw_emul $(TEST_LOG)
	@make clean-eth-socket

clean:
	@rm -rf fw_emul periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) $(SOC_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG) \
		$(BUILD_DIR) $(DEBUG_DIR) $(VERSAT_PRG_SRC_DIR) *.dot
	@rm -rf *.vh *.v *.inc output-versat
	@make -C $(SW_DIR)/python clean
	@make -C $(SW_TEST_DIR) clean
	@make clean-eth-socket

test: 
	make run
	make validate

validate:
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

profile: clean $(EMUL_PROFILE_LOG)
	@printf "\n=== PROFILE LOG ===\n"
	@cat $(EMUL_PROFILE_LOG)
	@printf "=== PROFILE LOG ===\n"

$(SOC_LOG): 
	make run PROFILE=1 TEST_LOG="> $(SOC_LOG)"

$(EMUL_PROFILE_LOG): $(SOC_LOG)
	@grep "PROFILE:" $< > $(EMUL_PROFILE_LOG)

$(BUILD_DIR)/V%.h: $(HW_DIR)/src/units/%.v
	verilator -CFLAGS "-g -m32 -std=c++11" -I$(VERSAT_HW_DIR)/include --trace --cc -Mdir $(BUILD_DIR) $<;
	cd $(BUILD_DIR) && VM_DEFAULT_RULES=0 make -f V$*.mk;

$(BUILD_DIR)/%.o: $(FIRM_DIR)/units/%.cpp 
	g++ -std=c++11 -g -c -o $@ -m32 $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I $(BUILD_DIR) 

$(BUILD_DIR)/%.o: $(FIRM_DIR)/%.cpp
	g++ -std=c++11 -g -c -o $@ -m32 $(DEFINE) $(INCLUDE) $< -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I $(BUILD_DIR) 

output-versat: $(HDR) $(CPP_OBJ) $(BUILD_DIR)/versat_source.o
	mkdir -p $(DEBUG_DIR)
	mkdir -p $(VERSAT_PRG_SRC_DIR)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(BUILD_DIR)/*o -lgcc -lc -lstdc++ -lm
	./$@
ifneq ($(OUTPUT_VERSAT_DST),)
	mv versat_instance.v $(OUTPUT_VERSAT_DST) 
	mv versat_defs.vh $(OUTPUT_VERSAT_DST) 
	mv versat_data.inc $(OUTPUT_VERSAT_DST) 
endif

.PHONY: build run \
	test validate profile \
	output-versat \
	clean
