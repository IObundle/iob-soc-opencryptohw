TEST_VECTOR_RSP?=SHA256ShortMsg.rsp

gen_test_data: $(TEST_VECTOR_RSP)
	./gen_test_data.py $(TEST_VECTOR_RSP)

read_bin: $(SW_TEST_DIR)/read_bin.c
	gcc $< -o $@

read_bin_run: read_bin gen_test_data
	$(eval RD_BIN_ARGS = $(basename $(notdir $(TEST_VECTOR_RSP)))_d_in.bin)
	$(eval RD_BIN_ARGS += $(basename $(notdir $(TEST_VECTOR_RSP)))_d_out.bin)
	./read_bin $(RD_BIN_ARGS) > read_bin.log

clean:
	@rm -rf *d_in.bin *d_out.bin
	@rm -rf read_bin read_bin.log

.PHONY: clean gen_test_data read_bin read_bin_run
