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
EMUL_LOG_PARSED:=emul_parsed.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $($p_DIR)/software/pc-emul/pc.mk))

#HEADERS
HDR+=periphs.h test_vectors.h

#SOURCES
SRC+= $(FIRM_DIR)/firmware.c $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c

#TEST VECTOR
TEST_VECTOR_RSP:=$(SW_TEST_DIR)/SHA256ShortMsg.rsp
VALIDATION_LOG:=validation.log

#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
endif

#RULES

all: build run parse

build: firmware.out

run: firmware.out
	./firmware.out $(TEST_LOG) > $(EMUL_LOG)

firmware.out: $(HDR) $(SRC)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc

test:
	make all TEST_LOG="> test.log"
	diff -q $(PC_DIR)/test.log $(PC_DIR)/test.expected

parse: $(EMUL_LOG_PARSED) $(VALIDATION_LOG)
	@if cmp --silent $(VALIDATION_LOG) $(EMUL_LOG_PARSED); then printf "\n\nShortMessage Test PASSED\n\n"; else printf "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)
	@rm -rf $(EMUL_LOG_PARSED)

$(EMUL_LOG_PARSED): $(EMUL_LOG)
	@grep -v "PROFILE:" $(EMUL_LOG) > $(EMUL_LOG_PARSED)


$(VALIDATION_LOG): $(TEST_VECTOR_RSP)
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars

#SHA256 test vectors
test_vectors.h:
	./$(SW_TEST_DIR)/gen_test_header.py $(TEST_VECTOR_RSP) > $@

clean:
	@rm -rf firmware.out periphs.h test.log *sw_reg.h \
		test.log test_vectors.h $(EMUL_LOG) $(EMUL_LOG_PARSED) $(VALIDATION_LOG) 

.PHONY: all build run clean subs periphs_tmp.h test
