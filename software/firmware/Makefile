ROOT_DIR=../..
include ../software.mk

#additional compiler flags
CFLAGS+=--specs=nano.specs -Wl,-Bstatic,-T,../template.lds,-Map,firmware.map,--strip-debug

#SUBMODULES

#cache
ifeq ($(USE_DDR),1)
include $(CACHE_DIR)/software/software.mk
endif

#peripherals (embedded)
$(foreach p, $(PERIPHERALS), $(eval include $(SUBMODULES_DIR)/$p/software/embedded/embedded.mk))

#HEADERS
HDR+=periphs.h test_vectors.h

#SOURCES
SRC+= firmware.S firmware.c $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c

#TEST VECTOR
TEST_VECTOR_RSP:=$(SW_TEST_DIR)/SHA256ShortMsg.rsp
VALIDATION_LOG:=validation.log

#RULES
run: firmware.elf

firmware.elf: ../template.lds $(HDR) $(SRC)
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(DEFINE) $(INCLUDE) $(SRC) -lgcc -lc -lnosys
	$(TOOLCHAIN_PREFIX)objcopy -O binary firmware.elf firmware.bin

#SHA256 test vectors
test_vectors.h:
	./$(SW_TEST_DIR)/gen_test_header.py $(TEST_VECTOR_RSP) > $@

clean: gen-clean 
	@rm -rf firmware.bin firmware.elf firmware.map *.hex periphs.h *sw_reg.h

.PHONY: clean
