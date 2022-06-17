PACKAGE=units
XUNITM=xunitM
XUNITF=xunitF
RTL_DIR=rtl
SCALA_DIR=src/main/scala/$(PACKAGE)

VSRC=$(RTL_DIR)/$(XUNITM).v
VSRC+=$(RTL_DIR)/$(XUNITF).v

all: $(VSRC)

$(RTL_DIR)/%.v: $(SCALA_DIR)/%.scala
	mkdir -p rtl
	sbt "runMain $(PACKAGE).$(basename $(@F))"

clean:
	@sbt clean
	@rm -rf target project/ rtl

.PHONY: all clean

