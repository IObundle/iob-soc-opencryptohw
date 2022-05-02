ROOT_DIR:=../..
include $(ROOT_DIR)/config.mk

DOC=acceleration_plan

KEEP_FILES = $(DOC).tex 
KEEP_FILES+= intro.tex sha256_algo.tex profile.tex proposal.tex
KEEP_FILES+= expected_results.tex conclusion.tex
KEEP_FILES+= refs.bib
KEEP_FILES+= plan.cls
KEEP_FILES+= $(DOC).pdf
KEEP_FILES+= profile.log

NOCLEAN= -name "Makefile"
NOCLEAN+= $(foreach f, $(KEEP_FILES),-o -name "$f")

LIB_DOC_DIR=$(LIB_DIR)/document

TEX_SRC=profile_tab.tex

all: $(DOC).pdf

view: $(DOC).pdf
	evince $< &

%.pdf: figures $(TEX_SRC) $(DOC)top.tex
	pdflatex -jobname $(DOC) $(DOC)top.tex
	if [ -f *.bib ]; then bibtex $(DOC); fi
	pdflatex -jobname $(DOC) $(DOC)top.tex
	pdflatex -jobname $(DOC) $(DOC)top.tex

figures:
	make -C $(LIB_DOC_DIR)/figures pb_figs
	make -C ./figures all

$(DOC)top.tex:
	echo "\def\TEX{$(LIB_DOC_DIR)}" > $@
	echo "\input{$(DOC).tex}" >> $@

profile_tab.tex: profile.log
	./$(SW_DIR)/python/profile_log2tex.py $< $@

profile.log:
ifeq ($(shell find $(HW_DIR) -name *profile.log),)
	make -C $(ROOT_DIR) fpga-run-profile
endif
	$(eval PROFILE_LOG_PATH = $(shell find $(HW_DIR) -name *profile.log))
	cp $(PROFILE_LOG_PATH) $@

clean:
	@find . -maxdepth 1 -type f -not \( $(NOCLEAN) \) -delete 
	@make -C $(LIB_DOC_DIR)/figures clean
	@make -C ./figures clean

.PHONY: all clean figures
