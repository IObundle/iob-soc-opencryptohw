ROOT_DIR:=../..

GLOBAL_CFLAGS:=-std=c++17 -m32 -Wall -g -DVERSAT_DEBUG #-fsanitize=address

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
SRC+= $(FIRM_DIR)/automaticTests.cpp
SRC+= $(FIRM_DIR)/unitConfiguration.cpp
SRC+= $(FIRM_DIR)/versatSHA.cpp
SRC+= $(FIRM_DIR)/versatAES.cpp

ifeq ($(ALGORITHM),SHA256)
SRC+= $(FIRM_DIR)/crypto/*.c
INCLUDE+=$(incdir)$(FIRM_DIR)/$(ALGORITHM)
SRC+= $(FIRM_DIR)/$(ALGORITHM)/fullSHATests.cpp
endif

ifeq ($(ALGORITHM),AES256)
SRC+= $(FIRM_DIR)/crypto/*.c
INCLUDE+=$(incdir)$(FIRM_DIR)/$(ALGORITHM)
SRC+= $(FIRM_DIR)/$(ALGORITHM)/fullAESTests.cpp
endif

ifeq ($(ALGORITHM),MCELIECE)
SRC+= $(FIRM_DIR)/versatMCELIECE.cpp
INCLUDE+=$(incdir)$(FIRM_DIR)/$(ALGORITHM)
SRC+= $(FIRM_DIR)/$(ALGORITHM)/fullMCELIECETests.cpp
SRC+= $(FIRM_DIR)/$(ALGORITHM)/operations.cpp
SRC+= $(FIRM_DIR)/$(ALGORITHM)/versat_pk_gen.cpp
SRC+= $(wildcard $(FIRM_DIR)/$(ALGORITHM)/*.c)
INCLUDE+=$(incdir)$(FIRM_DIR)/$(ALGORITHM)/common
SRC+= $(wildcard $(FIRM_DIR)/$(ALGORITHM)/common/*.c)
endif

INCLUDE+=$(incdir)./build

SOC_IN_BIN=soc-in.bin
TEST_IN_BIN=$(SW_TEST_DIR)/$(basename $(TEST_VECTOR_RSP))_d_in.bin
SOC_OUT_BIN:=soc-out.bin

ifneq ($(GENERATE_ONLY),)
DEFINE+=$(defmacro)GENERATE_ONLY
endif
#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
endif
INCLUDE+=$(incdir)$(FIRM_DIR)

#UNITS TO VERILATE
VERILATE_UNIT := xunitF xunitM
UNIT_VERILOG += $(foreach unit,$(VERILATE_UNIT),$(HW_DIR)/src/units/$(unit).v)

UNIT_HDR+=$(foreach obj,$(VERILATE_UNIT),./build/V$(obj).h)
OBJ+=./build/firmware.o

FW_EMUL=fw_emul

#CONSOLE
ifneq ($(GENERATE_ONLY),)
CONSOLE_CMD=$(LIB_DIR)/software/python/console -L
else
CONSOLE_CMD=$(ROOT_DIR)/software/console/eth_console -L
endif

#PC EMUL OUTPUTS
VERSAT_GEN=versat_instance.v
VERSAT_GEN+=versat_data.inc
VERSAT_GEN+=versat_defs.vh
VERSAT_GEN+=$(wildcard src/*)

#RULES

build: $(FW_EMUL)
	mkdir -p debug
	mkdir -p src
	mkdir -p trace_out

$(FW_EMUL): $(HDR) $(SRC) $(UNIT_HDR) $(CPP_OBJ) $(OBJ)
	gcc -o $@ $(GLOBAL_CFLAGS) $(DEFINE) $(INCLUDE) -I$(VERILATOR_INCLUDE) $(SRC) ./build/*.o -lstdc++ -lm -lgcc -lc -lpthread #-lncurses
	
run: build $(SOC_IN_BIN)
	@mkdir -p $(HW_DIR)/src/GeneratedUnits/
	@mkdir -p debug
	@mkdir -p src
	$(CONSOLE_CMD) $(TEST_LOG) &
	./$(FW_EMUL) $(TEST_LOG)

clean: clean-eth-socket
	@rm -rf $(FW_EMUL) periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) $(SOC_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG) *.bin
	@rm -f versat_data.inc versat_defs.vh versat_instance.v \
		versat_external_memory_inst.vh versat_external_memory_port.vh \
		versat_external_memory_portmap.vh
	@make -C $(SW_DIR)/python clean
	@make -C $(SW_TEST_DIR) clean
	@rm -f $(FIRM_DIR)/*.inc
	@rm -f $(HW_DIR)/src/versat_instance.v
	@rm -f $(HW_DIR)/include/versat_defs.vh
	@rm -f -r ./build
	@rm -f -r ./debug
	@rm -f -r ./src
	@rm -f -r ./trace_out
	@rm -f -r $(HW_DIR)/src/GeneratedUnits
	@rm -f -r *.dot

test: clean
	make run HARDWARE_TEST=$(HARDWARE_TEST)
	make validate

validate:
	cp $(SOC_OUT_BIN) $(SW_TEST_DIR)
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP) 

gen-versat: $(VERSAT_GEN)
	cp -u versat_instance.v $(HW_DIR)/src/
	cp -u versat_defs.vh $(HW_DIR)/include/
	cp -u versat_external_memory_inst.vh $(HW_DIR)/include/
	cp -u versat_external_memory_port.vh $(HW_DIR)/include/
	cp -u versat_external_memory_portmap.vh $(HW_DIR)/include/
	cp -u versat_data.inc $(FIRM_DIR)/
	cp -u ./build/verilogWrapper.inc $(FIRM_DIR)/
	cp -u ./build/basicWrapper.inc $(FIRM_DIR)/
	cp -u src/* $(HW_DIR)/src/GeneratedUnits/

$(VERSAT_GEN):
	make run GENERATE_ONLY=1

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

./build/%.o: $(FIRM_DIR)/$(ALGORITHM)/%.cpp $(UNIT_OBJ)
	g++ $(GLOBAL_CFLAGS) -c -o ./build/$*.o $(DEFINE) $(INCLUDE) $(FIRM_DIR)/$(ALGORITHM)/$*.cpp -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/V%.h: $(HW_DIR)/src/units/%.v
	verilator -CFLAGS "$(VERILATE_FLAGS)" -I$(VERSAT_HW_DIR)/include --trace --cc -Mdir ./obj $<;
	cd ./obj && VM_DEFAULT_RULES=0 make -f V$*.mk;
	mkdir -p ./build; mv ./obj/*.o ./build/;
	mv ./obj/*.h ./build/
	rm -r -f ./obj

$(SOC_IN_BIN): $(TEST_IN_BIN)
	mv $< $@

$(TEST_IN_BIN):
	make -C $(SW_TEST_DIR) gen_test_data TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

.PHONY: run \
	test validate profile \
	gen-versat \
	clean

