SHELL:=/bin/bash

ROOT_DIR:=../..
include $(ROOT_DIR)/config.mk


run-eth-scripts:
	$(eval ETH_INPUT_BIN = $(basename $(TEST_VECTOR_RSP))_d_in.bin)
	mv $(SW_DIR)/firmware/$(ETH_INPUT_BIN) .
	$(eval RMAC := $(shell ethtool -P $(RMAC_INTERFACE) | awk '{print $$3}' | sed 's/://g'))
	source /opt/pyeth3/bin/activate; python3 ./sha256_test.py $(RMAC_INTERFACE) $(RMAC) ./$(ETH_INPUT_BIN) ./soc-out.bin; deactivate;

fpga-eth:
ifeq ($(ETH_SERVER),)
	make run-eth-scripts
else
	ssh $(ETH_USER)@$(ETH_SERVER) "if [ ! -d $(REMOTE_ROOT_DIR) ]; then mkdir -p $(REMOTE_ROOT_DIR); fi"
	rsync -avz --delete --force --exclude .git $(ROOT_DIR) $(ETH_USER)@$(ETH_SERVER):$(REMOTE_ROOT_DIR)
	bash -c "trap 'make kill-remote-eth' INT TERM KILL; ssh $(ETH_USER)@$(ETH_SERVER) 'cd $(REMOTE_ROOT_DIR); make fpga-eth'"
	scp $(ETH_USER)@$(ETH_SERVER):$(REMOTE_ROOT_DIR)/software/python/soc-out.bin .
endif


kill-remote-eth:
	@$(eval ETH_PROC=sha256_test)
	@echo "INFO: Remote ethernet scripts will be killed"
	ssh $(ETH_USER)@$(ETH_SERVER) 'pkill -f $(ETH_PROC)'
