#
# pc-emul/makefile
#
# This makefile overrides some pc-emul/Makefile targets
# copy this file to $(PC_DIR)
#
# Based on the GNU Make manual example:
# https://www.gnu.org/software/make/manual/html_node/Overriding-Makefiles.html
#
# See also:
# https://www.gnu.org/software/make/manual/html_node/Makefile-Names.html
#

ROOT_DIR:=../..
include ../software.mk

SOC_LOG:=soc.log
HOST_LOG:=host.log

SOC_OUT_BIN:=soc-out.bin

#
# Override original targets
#
all: clean build 
	make -j2 run-parallel

clean:
	@rm -rf $(SOC_LOG) $(HOST_LOG)
	@$(MAKE) -f Makefile $@
	
#
# Added targets
#
run-parallel: run-int run-eth-int
	

run-int:
	@$(MAKE) -f Makefile run > $(SOC_LOG)

run-eth-int:
	@$(MAKE) pc-eth > $(HOST_LOG)

pc-eth:
	PC=1 python3 $(ROOT_DIR)/software/python/sha256_test.py enp1 112233445566 ./SHA256ShortMsg_d_in.bin ./$(SOC_OUT_BIN);
#
# Run target from original makefile
#
%: force
	@$(MAKE) -f Makefile $@

force: ;
