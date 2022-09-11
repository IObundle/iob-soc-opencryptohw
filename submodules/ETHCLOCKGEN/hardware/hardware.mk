ifeq ($(filter ETHCLOCKGEN, $(HW_MODULES)),)

include $(ETHCLOCKGEN_DIR)/config.mk

#add itself to HW_MODULES list
HW_MODULES+=ETHCLOCKGEN

#import module
include $(LIB_DIR)/hardware/iob_reg/hardware.mk

#include files
VHDR+=$(LIB_DIR)/hardware/include/iob_lib.vh $(LIB_DIR)/hardware/include/iob_gen_if.vh

#hardware include dirs
INCLUDE+=$(incdir). $(incdir)$(LIB_DIR)/hardware/include

#sources
VSRC+=$(ETHCLOCKGEN_SRC_DIR)/iob_ethclockgen.v

ethclockgen-hw-clean: ethclockgen-gen-clean
	@rm -f *.v *.vh

.PHONY: ethclockgen-hw-clean

endif
