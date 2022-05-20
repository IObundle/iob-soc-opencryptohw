PACKAGE=units
XUNITM=xunitM
RTL_DIR=rtl
SCALA_DIR=src/main/scala/$(PACKAGE)

VSRC=$(RTL_DIR)/$(XUNITM).v

all: $(VSRC)

$(RTL_DIR)/%.v: $(SCALA_DIR)/%.scala
	sbt "runMain $(PACKAGE).$(basename $(@F))"

clean:
	@sbt clean
	@rm -rf target project/

.PHONY: all clean

