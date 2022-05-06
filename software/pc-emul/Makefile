ROOT_DIR:=../..

include ../software.mk

#additional compiler flags
CFLAGS=-Os -std=gnu99 -Wl,--strip-debug

#DEFINE+=-DLONGLONG 
DEFINE+=-DPC

SOC_LOG:=soc.log
ETH_LOG:=ethernet.log
EMUL_PROFILE_LOG:=emul_profile.log

#SUBMODULES

#peripherals (pc)
$(foreach p, $(PERIPHERALS), $(eval include $($p_DIR)/software/pc-emul/pc-emul.mk))

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

#CONSOLE
CONSOLE_CMD=$(ROOT_DIR)/software/console/eth_console -L

#RULES

build: gen_data fw_emul

fw_emul: $(HDR) $(SRC)
	gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc

run: build
	@rm -f soc2cnsl cnsl2soc
	$(CONSOLE_CMD) $(TEST_LOG) &
	./fw_emul $(TEST_LOG)

clean:
	@rm -rf fw_emul periphs.h *swreg*.h *swreg*.c test.log soc2cnsl cnsl2soc \
		$(ETH_LOG) eth_frame_struct.h $(EMUL_PROFILE_LOG)
	@make -C $(SW_DIR)/python clean
	@make -C $(SW_TEST_DIR) clean

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
	make run PROFILE=1

$(EMUL_PROFILE_LOG): $(SOC_LOG)
	@grep "PROFILE:" $< > $(EMUL_PROFILE_LOG)

.PHONY: build run \
	test validate profile \
	clean
