ROOT_DIR:=../..
include ../software.mk

#default baud and freq for pc emulation
BAUD=5000000
FREQ=100000000

#additional compiler flags
CFLAGS=-g -Os -m32 -std=c99

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

EMUL_LOG:=emul.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $(SUBMODULES_DIR)/$p/software/pc-emul/pc.mk))

#HEADERS
HDR+=periphs.h test_vectors.h

#SOURCES
SRC+= $(FIRM_DIR)/firmware.c
SRC+= $(FIRM_DIR)/crypto/*.c
SRC+=$(UART_DIR)/software/printf.c

#TEST VECTOR
TEST_VECTOR_RSP:=$(SW_TEST_DIR)/SHA256ShortMsg.rsp
VALIDATION_LOG:=validation.log

#UNITS TO VERILATE
VERILATE_UNIT = xunitF xunitM

UNIT_OBJ+=$(foreach obj,$(VERILATE_UNIT),./build/V$(obj)__ALLcls.o)
OBJ+=./build/unitWrapper.o

#RULES

all: build run

ifeq ($(AUTOMATIC_TEST),1)
run: firmware.out
	./firmware.out $(TEST_LOG) > $(EMUL_LOG)
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars
	@if cmp --silent $(VALIDATION_LOG) $(EMUL_LOG); then echo "\n\nShortMessage Test PASSED\n\n"; else echo "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)
	mv versat_instance.v $(HW_DIR)/src/
	mv versat_defs.vh $(HW_DIR)/include/
else
run: firmware.out
	./firmware.out $(TEST_LOG)
	mv versat_instance.v $(HW_DIR)/src/
	mv versat_defs.vh $(HW_DIR)/include/
endif

build: firmware.out

firmware.out: $(HDR) $(SRC) $(UNIT_OBJ) $(OBJ)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) ./build/*.o -lstdc++ -lm -lgcc -lc

./build/unitWrapper.o: $(FIRM_DIR)/unitWrapper.cpp $(UNIT_OBJ)
	g++ -std=c++11 -g -c -o ./build/unitWrapper.o -m32 $(FIRM_DIR)/unitWrapper.cpp -I $(VERSAT_SW_DIR) -I $(VERILATOR_INCLUDE) -I ./build/ 

./build/V%__ALLcls.o ./build/V%__ALLsup.o: $(HW_DIR)/src/units/%.v
	verilator -CFLAGS "-g -m32 -std=c++11" -I$(VERSAT_HW_DIR)/include --trace --cc -Mdir ./obj $<;
	cd ./obj && VM_DEFAULT_RULES=0 make -f V$*.mk;
	mkdir -p ./build; mv ./obj/*.o ./build/;
	mv ./obj/*.h ./build/
	rm -r -f ./obj
	
#SHA256 test vectors
test_vectors.h:
	./$(SW_TEST_DIR)/gen_test_header.py $(TEST_VECTOR_RSP) > $@

clean:
	@rm -rf firmware.out periphs.h test.log test_vectors.h $(EMUL_LOG) $(VALIDATION_LOG) build headers versat_instance_template.h UARTsw_reg.h

.PHONY: all build run clean subs periphs_tmp.h firmware.out
