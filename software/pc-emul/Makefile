ROOT_DIR:=../..
include ../software.mk

#default baud and freq for pc emulation
BAUD=5000000
FREQ=100000000

#additional compiler flags
CFLAGS=-g -Os

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

#OBJECTS
OBJ += verilated.o

#TEST VECTOR
TEST_VECTOR_RSP:=$(SW_TEST_DIR)/SHA256ShortMsg.rsp
VALIDATION_LOG:=validation.log

#RULES

all: build run

ifeq ($(AUTOMATIC_TEST),1)
run: firmware.out
	./firmware.out $(TEST_LOG) > $(EMUL_LOG)
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars
	@if cmp --silent $(VALIDATION_LOG) $(EMUL_LOG); then echo "\n\nShortMessage Test PASSED\n\n"; else echo "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)
else
run: firmware.out
	./firmware.out $(TEST_LOG)
	mv versat_instance.v $(HW_DIR)/src/
	mv versat_defs.vh $(HW_DIR)/include/
endif

build: firmware.out

firmware.out: $(HDR) $(SRC) $(OBJ)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(OBJ) -lstdc++ -lm -lgcc -lc -lncurses

verilated.o:
	g++  -I.  -MMD -I/usr/local/share/verilator/include -I/usr/local/share/verilator/include/vltstd -DVL_PRINTF=printf \
	-DVM_COVERAGE=0 -DVM_SC=0 -DVM_TRACE=0 -Wno-sign-compare -Wno-uninitialized -Wno-unused-but-set-variable \
	-Wno-unused-parameter -Wno-unused-variable -Wno-shadow -c -o ./verilated.o /usr/local/share/verilator/include/verilated.cpp

#SHA256 test vectors
test_vectors.h:
	./$(SW_TEST_DIR)/gen_test_header.py $(TEST_VECTOR_RSP) > $@

clean:
	@rm -rf firmware.out periphs.h test.log test_vectors.h $(EMUL_LOG) $(VALIDATION_LOG)

.PHONY: all build run clean subs periphs_tmp.h firmware.out
