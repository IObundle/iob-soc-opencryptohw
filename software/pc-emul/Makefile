ROOT_DIR:=../..

GLOBAL_CFLAGS:=-g -m32 -Wall #-fsanitize=address

include ../software.mk

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

HDR+=$(FIRM_DIR)/unitConfiguration.hpp

#SOURCES
SRC+= $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c
SRC+= $(FIRM_DIR)/automaticTests.cpp
SRC+= $(FIRM_DIR)/unitConfiguration.cpp
SRC+= $(FIRM_DIR)/versatSHA.cpp

INCLUDE+=$(incdir)./build

SOC_OUT_BIN:=soc-out.bin

#units to verilate
CUSTOM_VERILATE_UNIT = xunitF xunitM
HDR+=$(foreach obj,$(CUSTOM_VERILATE_UNIT),$(BUILD_DIR)/V$(obj).h)
UNIT_VERILOG+=$(foreach unit,$(CUSTOM_VERILATE_UNIT),$(HW_DIR)/src/units/$(unit).v)

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

#UNITS TO VERILATE
VERILATE_UNIT := xunitF xunitM
UNIT_VERILOG += $(foreach unit,$(VERILATE_UNIT),$(HW_DIR)/src/units/$(unit).v)

UNIT_HDR+=$(foreach obj,$(VERILATE_UNIT),./build/V$(obj).h)
OBJ+=./build/firmware.o

#CONSOLE
CONSOLE_CMD=$(ROOT_DIR)/software/console/eth_console -L

#RULES

build: gen_data fw_emul
	mkdir -p debug
	mkdir -p src
	mkdir -p trace_out

fw_emul: $(HDR) $(SRC) $(UNIT_HDR) $(CPP_OBJ) $(OBJ)
	gcc -o $@ $(GLOBAL_CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) ./build/*.o -lstdc++ -lm -lgcc -lc #-lncurses
	
run: build
	@mkdir -p $(HW_DIR)/src/GeneratedUnits/
	@mkdir -p debug
	@mkdir -p src
	./fw_emul $(TEST_LOG)
	cp versat_instance.v $(HW_DIR)/src/
	cp versat_defs.vh $(HW_DIR)/include/
	cp versat_data.inc $(FIRM_DIR)/
	cp ./build/verilogWrapper.inc $(FIRM_DIR)/
	cp src/* $(HW_DIR)/src/GeneratedUnits/

clean:
	@rm -rf fw_emul periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) $(SOC_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG)
	@rm -f versat_data.inc versat_defs.vh versat_instance.v
	@make -C $(SW_DIR)/python clean
	@make -C $(SW_TEST_DIR) clean
	@rm -f -r ./build
	@rm -f -r ./debug
	@rm -f -r ./src
	@rm -f -r ./trace_out
	@rm -f -r $(HW_DIR)/src/GeneratedUnits

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

./build/%.o: $(FIRM_DIR)/%.cpp $(UNIT_OBJ)
	g++ $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $(FIRM_DIR)/$*.cpp -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/V%.h: $(HW_DIR)/src/units/%.v
	verilator -CFLAGS "$(VERILATE_FLAGS)" -I$(VERSAT_HW_DIR)/include --trace --cc -Mdir ./obj $<;
	cd ./obj && VM_DEFAULT_RULES=0 make -f V$*.mk;
	mkdir -p ./build; mv ./obj/*.o ./build/;
	mv ./obj/*.h ./build/
	rm -r -f ./obj

.PHONY: build run \
	test validate profile \
	output-versat \
	clean

