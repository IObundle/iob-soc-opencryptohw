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
EMUL_PROFILE_LOG:=emul_profile.log

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
INCLUDE+=$(incdir)$(FIRM_DIR)
endif

#RULES

all: clean build run

build: firmware.out

run: firmware.out
	./firmware.out $(TEST_LOG)

firmware.out: $(HDR) $(SRC)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc

test:
	make all TEST_LOG=">> $(EMUL_LOG)"
	make validate

validate: $(EMUL_LOG_PARSED) $(VALIDATION_LOG)
	@if cmp --silent $(VALIDATION_LOG) $(EMUL_LOG_PARSED); then printf "\n\nShortMessage Test PASSED\n\n"; else printf "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;
	@rm -rf $(VALIDATION_LOG)
	@rm -rf $(EMUL_LOG_PARSED)

profile: $(EMUL_PROFILE_LOG)
	@printf "\n=== PROFILE LOG ===\n"
	@cat $<
	@printf "=== PROFILE LOG ===\n"

$(EMUL_LOG): 
	make all TEST_LOG=">> $(EMUL_LOG)" PROFILE=1

$(EMUL_LOG_PARSED): $(EMUL_LOG)
	@grep -v "PROFILE:" $(EMUL_LOG) > $(EMUL_LOG_PARSED)

$(EMUL_PROFILE_LOG): $(EMUL_LOG)
	@grep "PROFILE:" $< > $(EMUL_PROFILE_LOG)

$(VALIDATION_LOG): $(TEST_VECTOR_RSP)
	@tail -n +6 $(TEST_VECTOR_RSP) > $(VALIDATION_LOG)
	@sed -i 's/\r//' $(VALIDATION_LOG) #remove carriage return chars

#SHA256 test vectors
test_vectors.h:
	./$(SW_TEST_DIR)/gen_test_header.py $(TEST_VECTOR_RSP) > $@

test_data_gen: $(TEST_VECTOR_RSP) read_bin
	./$(SW_TEST_DIR)/gen_test_data.py $(TEST_VECTOR_RSP)
	mv $(SW_TEST_DIR)/*d_in.bin .
	mv $(SW_TEST_DIR)/*d_out.bin .

read_bin: $(SW_TEST_DIR)/read_bin.c
	gcc $< -o $@

read_bin_run: read_bin
	$(eval RD_BIN_ARGS = $(basename $(notdir $(TEST_VECTOR_RSP)))_d_in.bin)
	$(eval RD_BIN_ARGS += $(basename $(notdir $(TEST_VECTOR_RSP)))_d_out.bin)
	./read_bin $(RD_BIN_ARGS) > read_bin.log

data_clean:
	@rm -rf *d_in.bin *d_out.bin
	@rm -rf read_bin read_bin.log

clean:
	@rm -rf firmware.out periphs.h test.log *sw_reg.h \
		test.log test_vectors.h $(EMUL_LOG) $(EMUL_LOG_PARSED) \
		$(EMUL_PROFILE_LOG) $(VALIDATION_LOG)

.PHONY: all build run clean subs periphs_tmp.h test
