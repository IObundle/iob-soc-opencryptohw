SHELL:=/bin/bash

TOP_MODULE=iob_ethclockgen

#PATHS
LIB_DIR ?=$(ETHCLOCKGEN_DIR)/submodules/LIB
ETHCLOCKGEN_SRC_DIR:=$(ETHCLOCKGEN_DIR)/hardware/src

# VERSION
VERSION ?=V0.1
$(TOP_MODULE)_version.txt:
	echo $(VERSION) > version.txt

ethclockgen-gen-clean:
	@rm -rf *# *~ version.txt

.PHONY: default ethclockgen-gen-clean
