ROOT_DIR:=../..
include ../software.mk

#default baud and freq for pc emulation
BAUD=5000000
FREQ=100000000

#additional compiler flags
CFLAGS=-g -Os

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $(SUBMODULES_DIR)/$p/software/pc-emul/pc.mk))

#HEADERS
HDR+=periphs.h

#SOURCES
SRC+= $(FIRM_DIR)/firmware.c
SRC+= $(FIRM_DIR)/crypto/*.c
SRC+=$(UART_DIR)/software/printf.c

#RULES

all: build run
run: firmware.out
	./firmware.out $(TEST_LOG) > $(EMUL_LOG)
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars
	@if cmp --silent $(VALIDATION_LOG) $(EMUL_LOG); then echo "\n\nShortMessage Test PASSED\n\n"; else echo "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)
	mv versat_instance.v $(HW_DIR)/src/
	mv versat_defs.vh $(HW_DIR)/include/

build: firmware.out

firmware.out: $(HDR) $(SRC)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc -lncurses

clean:
	@rm -rf firmware.out periphs.h test.log

.PHONY: all build run clean subs periphs_tmp.h
