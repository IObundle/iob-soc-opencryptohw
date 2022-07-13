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

#ethernet
include $(ETHERNET_DIR)/software/embedded/embedded.mk

#versat
include $(VERSAT_DIR)/software/embedded/embedded.mk


#HEADERS
HDR+=periphs.h
HDR+=versat_data.inc

#INCLUDES
INCLUDE+=$(incdir)$(FIRM_DIR)/units

#SOURCES
SRC+=firmware.S 
SRC+=$(UART_DIR)/software/printf.c
SRC+=$(FIRM_DIR)/crypto/*.c

#firmware object
CPP_SRC+=versat_sha.cpp
CPP_SRC+=firmware.cpp

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
	$(TOOLCHAIN_PREFIX)gcc -o $@ $(CFLAGS) $(LFLAGS) $(DEFINE) $(INCLUDE) $(SRC) $(CPP_SRC) $(LLIBS) -fno-exceptions
	$(TOOLCHAIN_PREFIX)objcopy -O binary firmware.elf firmware.bin

versat_data.inc:
	@$(eval CURRENT_DIR=$(shell pwd))
	@make -C $(ROOT_DIR) pc-emul-output-versat OUTPUT_VERSAT_DST=$(CURRENT_DIR)

clean:
	@rm -rf firmware.bin firmware.elf firmware.map *.hex periphs.h *swreg.h \
		*swreg_emb.c \
		*d_in.bin *d_out.bin \
		eth_frame_struct.h \
		versat_data.inc versat_defs.vh versat_instance.v

.PHONY: build clean
