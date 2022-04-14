SHELL:=/bin/bash

ROOT_DIR:=../..
include $(ROOT_DIR)/config.mk

ETH_INPUT_BIN = $(basename $(TEST_VECTOR_RSP))_d_in.bin
SOC_OUT_BIN?=soc-out.bin

run-eth-scripts: $(ETH_INPUT_BIN)
	mv $(SW_DIR)/firmware/$(ETH_INPUT_BIN) .
	$(eval RMAC := $(shell ethtool -P $(RMAC_INTERFACE) | awk '{print $$3}' | sed 's/://g'))
	source /opt/pyeth3/bin/activate; python3 ./sha256_test.py $(RMAC_INTERFACE) $(RMAC) ./$(ETH_INPUT_BIN) $(SOC_OUT_BIN); deactivate;

$(ETH_INPUT_BIN):
	$(eval ETH_INPUT_BIN_PATH = $(shell find $(ROOT_DIR) -name "$@"))
	cp $(ETH_INPUT_BIN_PATH) $@


kill-remote-eth:
	@$(eval ETH_PROC=sha256_test)
	@echo "INFO: Remote ethernet scripts will be killed"
	ssh $(ETH_USER)@$(ETH_SERVER) 'pkill -f $(ETH_PROC)'

clean:
	@rm -rf *.bin
