ROOT_DIR:=../..
include ../software.mk

#default baud and freq for pc emulation
BAUD=5000000
FREQ=100000000

#additional compiler flags
CFLAGS=-Os -std=gnu99 -Wl,--strip-debug

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

EMUL_LOG:=emul.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $(SUBMODULES_DIR)/$p/software/pc-emul/pc.mk))

#HEADERS
HDR+=periphs.h test_vectors.h

#SOURCES
SRC+= $(FIRM_DIR)/firmware.c $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c

#TEST VECTOR
TEST_VECTOR_RSP:=$(SW_TEST_DIR)/SHA256ShortMsg.rsp
VALIDATION_LOG:=validation.log

#RULES

all: build run
run: firmware.out
	./firmware.out $(TEST_LOG) > $(EMUL_LOG)
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars
	@if cmp --silent $(VALIDATION_LOG) $(EMUL_LOG); then echo "\n\nShortMessage Test PASSED\n\n"; else echo "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)

build: firmware.out

firmware.out: $(HDR) $(SRC)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc

#SHA256 test vectors
test_vectors.h:
	./$(SW_TEST_DIR)/gen_test_header.py $(TEST_VECTOR_RSP) > $@

clean:
	@rm -rf firmware.out periphs.h test.log test_vectors.h $(EMUL_LOG) $(VALIDATION_LOG)

.PHONY: all build run clean subs periphs_tmp.h
