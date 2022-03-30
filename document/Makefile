ROOT_DIR:=..
include $(ROOT_DIR)/config.mk

.PHONY: accel-plan accel-plan-clean \
	clean

accel-plan:
	make -C ./acceleration_plan all

accel-plan-clean:
	make -C ./acceleration_plan clean

clean: accel-plan-clean

