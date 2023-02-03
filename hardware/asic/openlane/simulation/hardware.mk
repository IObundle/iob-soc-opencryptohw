include $(ROOT_DIR)/config.mk

#add itself to MODULES list
HW_MODULES+=$(IOBSOC_NAME)

#
# ADD SUBMODULES HARDWARE
#

#include LIB modules
include $(LIB_DIR)/hardware/iob_merge/hardware.mk
include $(LIB_DIR)/hardware/iob_split/hardware.mk
include $(LIB_DIR)/hardware/iob_pulse_gen/hardware.mk
include $(LIB_DIR)/hardware/iob_edge_detect/hardware.mk

#include MEM modules
include $(MEM_DIR)/hardware/rom/iob_rom_sp/hardware.mk
include $(MEM_DIR)/hardware/ram/iob_ram_dp_be/hardware.mk

#CPU
include $(PICORV32_DIR)/hardware/hardware.mk

#CACHE
include $(CACHE_DIR)/hardware/hardware.mk

#UART
include $(UART_DIR)/hardware/hardware.mk

#TIMER
include $(TIMER_DIR)/hardware/hardware.mk
	
#ETHERNET
include $(ETHERNET_DIR)/hardware/hardware.mk

#VERSAT
include $(VERSAT_DIR)/hardware/hardware.mk

#AXI
include $(AXI_DIR)/hardware/axiinterconnect/hardware.mk

#HARDWARE PATHS
INC_DIR:=$(HW_DIR)/include
SRC_DIR:=$(HW_DIR)/src
SPINAL_DIR=$(HW_DIR)/src/spinalHDL
ifeq ($(SPINAL),1)
XUNIT_DIR:=$(SRC_DIR)/spinalHDL/rtl
else
XUNIT_DIR:=$(SRC_DIR)/units
endif
XUNITM_VSRC=$(XUNIT_DIR)/xunitM.v
XUNITF_VSRC=$(XUNIT_DIR)/xunitF.v

#DEFINES
DEFINE+=$(defmacro)DDR_DATA_W=$(DDR_DATA_W)
DEFINE+=$(defmacro)DDR_ADDR_W=$(DDR_ADDR_W)
DEFINE+=$(defmacro)AXI_ADDR_W=32

#INCLUDES
INCLUDE+=$(incdir). $(incdir)$(INC_DIR) $(incdir)$(LIB_DIR)/hardware/include

#HEADERS
VHDR+=$(INC_DIR)/system.vh $(LIB_DIR)/hardware/include/iob_intercon.vh
VHDR+=$(INC_DIR)/sram_port.vh $(INC_DIR)/sram_portmap.vh
VHDR+=$(INC_DIR)/bootrom_port.vh $(INC_DIR)/bootrom_portmap.vh
VHDR+=versat_defs.vh

#OpenLane PDK Sources
VSRC+=pdk/primitives.v
VSRC+=pdk/sky130_fd_sc_hd.v

#axi wires to connect cache to external memory in system top
VHDR+=m_axi_wire.vh
m_axi_wire.vh:
	$(LIB_DIR)/software/python/axi_gen.py axi_wire 'm_' 'm_' 'm_'

#SOURCES

VSRC+=$(OPENLANE_SIM_TYPE)/system.v

HEXPROGS=boot.hex firmware.hex

# make system.v with peripherals
system.v: $(SRC_DIR)/system_core.v
	cp $< $@
	$(foreach p, $(PERIPHERALS), $(eval HFILES=$(shell echo `ls $($p_DIR)/hardware/include/*.vh | grep -v pio | grep -v inst | grep -v swreg | grep -v port`)) \
	$(eval HFILES+=$(notdir $(filter %swreg_def.vh, $(VHDR)))) \
	$(if $(HFILES), $(foreach f, $(HFILES), sed -i '/PHEADER/a `include \"$(notdir $f)\"' $@;),)) # insert header files
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/pio.vh; then sed -i '/PIO/r $($p_DIR)/hardware/include/pio.vh' $@; fi;) #insert system IOs for peripheral
	$(foreach p, $(PERIPHERALS), if test -f $($p_DIR)/hardware/include/inst.vh; then sed -i '/endmodule/e cat $($p_DIR)/hardware/include/inst.vh' $@; fi;) # insert peripheral instances

# make and copy memory init files
boot.hex: $(BOOT_DIR)/boot.bin
	$(PYTHON_DIR)/makehex.py $< $(BOOTROM_ADDR_W) > $@

firmware.hex: $(FIRM_DIR)/firmware.bin
	$(PYTHON_DIR)/makehex.py $< $(FIRM_ADDR_W) > $@
	$(PYTHON_DIR)/hex_split.py firmware .

$(BOOT_DIR)/boot.bin $(FIRM_DIR)/firmware.bin:
	make -C $(ROOT_DIR) fw-build SIM=1

#clean general hardware files
hw-clean: gen-clean
	@rm -f *.v *.vh *.hex *.bin $(SRC_DIR)/system.v $(TB_DIR)/system_tb.v *.inc	$(SRC_DIR)/GeneratedUnits/*.v $(SRC_DIR)/versat_instance.v $(INC_DIR)/versat_defs.vh
	@make -C $(ROOT_DIR) pc-emul-clean

gen-spinal-sources: $(XUNITM_VSRC) $(XUNITF_VSRC)

$(XUNITM_VSRC) $(XUNITF_VSRC):
	make -C $(SPINAL_DIR) rtl/$(notdir $@)

versat_instance.v versat_defs.vh: $(PC_DIR)/$(@F)
	cp $(PC_DIR)/$(@F) $@

$(PC_DIR)/versat_instance.v $(PC_DIR)/versat_defs.vh:
	make -C $(ROOT_DIR) pc-emul-output-versat

pdk/%.v: $(ROOT_DIR)/submodules/OpenLane/pdks/sky130B/libs.ref/sky130_fd_sc_hd/verilog/%.v
	mkdir -p pdk
	cp $< $@

$(ROOT_DIR)/../OpenLane/pdks/sky130B/libs.ref/sky130_fd_sc_hd/verilog/%.v:
	make -C $(ROOT_DIR) openlane-setup

.PHONY: hw-clean
