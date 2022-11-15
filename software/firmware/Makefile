ROOT_DIR=../..
include ../software.mk

#additional linker flag
LFLAGS+=-Wl,-Map,firmware.map

#cache
ifeq ($(USE_DDR),1)
include $(CACHE_DIR)/software/software.mk
endif

#uart
include $(UART_DIR)/software/embedded/embedded.mk

#timer
include $(TIMER_DIR)/software/embedded/embedded.mk

#versat
include $(VERSAT_DIR)/software/embedded/embedded.mk

#ila
include $(ILA_DIR)/software/embedded/embedded.mk

#HEADERS
HDR+=periphs.h

#SOURCES
SRC+= firmware.S firmware.cpp $(UART_DIR)/software/printf.c
SRC+= $(FIRM_DIR)/crypto/*.c
SRC+= $(FIRM_DIR)/automaticTests.cpp
SRC+= $(FIRM_DIR)/unitConfiguration.cpp
SRC+= $(FIRM_DIR)/versatSHA.cpp
SRC+= $(FIRM_DIR)/ila.c

#PROFILE
ifeq ($(PROFILE),1)
HDR+=$(FIRM_DIR)/profile.h
SRC+= $(FIRM_DIR)/profile.c
DEFINE+=$(defmacro)PROFILE
endif

#SIMULATION
ifeq ($(SIM),1)
DEFINE+=$(defmacro)SIM
endif

#RULES

build: firmware.elf

firmware.elf: ../template.lds $(HDR) $(SRC) gen_data
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(LLIBS) -lgcc -lc -lnosys -fno-exceptions
	$(TOOLCHAIN_PREFIX)objcopy -O binary firmware.elf firmware.bin

clean:
	@rm -rf firmware.bin firmware.elf firmware.map *.hex periphs.h UARTsw_reg.h TIMERsw_reg.h *swreg.h \
		*swreg_emb.c \
		*d_in.bin *d_out.bin \
		verilogWrapper.inc versat_data.inc

.PHONY: build clean
