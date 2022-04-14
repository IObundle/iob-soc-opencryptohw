#
# console/makefile
#
# This makefile overrides some console/Makefile targets
# copy this file to $(CONSOLE_DIR)
#
# Based on the GNU Make manual example:
# https://www.gnu.org/software/make/manual/html_node/Overriding-Makefiles.html
#
# See also:
# https://www.gnu.org/software/make/manual/html_node/Makefile-Names.html
#

ROOT_DIR=../..
include $(ROOT_DIR)/config.mk

SOC_LOG=soc.log
HOST_LOG:=host.log
SOC_OUT_BIN=soc-out.bin

#
# Override original targets
#
run: 
	make -j2 run-parallel

clean:
	@rm -rf $(SOC_LOG) $(HOST_LOG)
	@$(MAKE) -C $(SW_DIR)/python clean
	@$(MAKE) -f Makefile $@
	
#
# Added targets
#
run-parallel: run-int run-eth-int
	

run-int:
	@$(MAKE) -f Makefile run > $(SOC_LOG)

run-eth-int:
	@$(MAKE) -C $(SW_DIR)/python run-eth-scripts SOC_OUT_BIN=$(SOC_OUT_BIN) > $(HOST_LOG)

#
# Run target from original makefile
#
%: force
	@$(MAKE) -f Makefile $@

force: ;
