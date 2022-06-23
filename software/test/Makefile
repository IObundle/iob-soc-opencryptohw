ROOT_DIR:=../..
include $(ROOT_DIR)/config.mk

TEST_VECTOR_RSP?=SHA256ShortMsg.rsp

# xunitM tests
XUNITM_SRC=$(SW_TEST_DIR)/gen_test_xunitM.c

# xunitF tests
XUNITF_SRC=$(SW_TEST_DIR)/gen_test_xunitF.c


gen_test_data: $(TEST_VECTOR_RSP)
	./gen_test_data.py $(TEST_VECTOR_RSP)

read_bin: $(SW_TEST_DIR)/read_bin.c
	gcc $< -o $@

read_bin_run: read_bin gen_test_data
	$(eval RD_BIN_ARGS = $(basename $(notdir $(TEST_VECTOR_RSP)))_d_in.bin)
	$(eval RD_BIN_ARGS += $(basename $(notdir $(TEST_VECTOR_RSP)))_d_out.bin)
	./read_bin $(RD_BIN_ARGS) > read_bin.log

validate: $(SOC_OUT_BIN)
	$(eval VALIDATION_OUT_BIN = $(basename $(TEST_VECTOR_RSP))_d_out.bin)
	@if ./$(SW_TEST_DIR)/validate_test.py $(VALIDATION_OUT_BIN) $(SOC_OUT_BIN); then printf "\n\nShortMessage Test PASSED\n\n"; else printf "\n\nShortMessage Test FAILED\n\n"; exit 1; fi;

$(SOC_OUT_BIN):
	$(eval SOC_OUT_BIN_PATH = $(shell find $(HW_DIR) -name "$@"))
	cp $(SOC_OUT_BIN_PATH) $@

gen_xunitM_bin: $(XUNITM_SRC)
	gcc $(XUNITM_SRC) -o $@
	./$@

gen_xunitF_bin: $(XUNITF_SRC)
	gcc $(XUNITF_SRC) -o $@
	./$@

clean:
	@rm -rf *.bin
	@rm -rf read_bin read_bin.log
	@rm -rf gen_xunitM_bin xunitM_*.bin 
	@rm -rf gen_xunitF_bin xunitF_*.bin 

.PHONY: clean gen_test_data read_bin read_bin_run \
	gen_xunitM_bin gen_xunitF_bin
