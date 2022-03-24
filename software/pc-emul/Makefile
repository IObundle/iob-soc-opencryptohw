ROOT_DIR:=../..
include ../software.mk

#default baud and freq for pc emulation
BAUD=5000000
FREQ=100000000

#additional compiler flags
CFLAGS=-Os -std=gnu99 -Wl,--strip-debug

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

SOC_LOG:=soc_log.log
HOST_LOG:=host.log
EMUL_PROFILE_LOG:=emul_profile.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $($p_DIR)/software/pc-emul/pc.mk))

#HEADERS
HDR+=periphs.h 

#SOURCES
SRC+= $(FIRM_DIR)/firmware.c $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c

SOC_OUT_BIN:=soc-out.bin

#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
INCLUDE+=$(incdir)$(FIRM_DIR)
endif

#RULES

all: clean gen_data build
	make -j2 run-parallel

build: firmware.out

run: firmware.out
	./firmware.out $(TEST_LOG)

firmware.out: $(HDR) $(SRC)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc

test: 
	make all
	make validate

run-parallel: run-soc-int run-python-int

run-soc-int:
	make run > $(SOC_LOG)

run-python-int:
	make run-python > $(HOST_LOG)

run-python:
	PC=1 python3 $(ROOT_DIR)/software/python/sha256_test.py enp1 112233445566 ./SHA256ShortMsg_d_in.bin ./$(SOC_OUT_BIN);

validate:
	cp $(SOC_OUT_BIN) $(SW_TEST_DIR)/
	make -C $(SW_TEST_DIR) validate SOC_OUT_BIN=$(SOC_OUT_BIN) TEST_VECTOR_RSP=$(TEST_VECTOR_RSP)

profile: $(EMUL_PROFILE_LOG)
	@printf "\n=== PROFILE LOG ===\n"
	@cat $<
	@printf "=== PROFILE LOG ===\n"

$(SOC_LOG): 
	make all PROFILE=1

$(EMUL_PROFILE_LOG): $(SOC_LOG)
	@grep "PROFILE:" $< > $(EMUL_PROFILE_LOG)

clean:
	make -C $(SW_TEST_DIR) clean
	@rm -rf firmware.out periphs.h test.log *swreg.h \
		test.log $(EMUL_LOG) $(EMUL_PROFILE_LOG) \
		*d_in.bin soc.log host.log $(SOC_OUT_BIN)

.PHONY: all build run run-parallel run-soc-int run-python-int run-python \
	validate profile 
