ROOT_DIR:=../..
include $(ROOT_DIR)/config.mk

DOC=acceleration_plan

KEEP_FILES = $(DOC).tex 
KEEP_FILES+= intro.tex profile.tex proposal.tex 
KEEP_FILES+= plan.cls
KEEP_FILES+= $(DOC).pdf

NOCLEAN= -name "Makefile"
NOCLEAN+= $(foreach f, $(KEEP_FILES),-o -name "$f")

LIB_DOC_DIR=$(LIB_DIR)/document

all: $(DOC).pdf

view: $(DOC).pdf
	evince $< &

%.pdf: figures $(DOC)top.tex
	pdflatex -jobname $(DOC) $(DOC)top.tex
	pdflatex -jobname $(DOC) $(DOC)top.tex

figures:
	make -C $(LIB_DOC_DIR)/figures pb_figs

$(DOC)top.tex:
	echo "\def\TEX{$(LIB_DOC_DIR)}" > $@
	echo "\input{$(DOC).tex}" >> $@

clean:
	@find . -type f -not \( $(NOCLEAN) \) -delete
	@rm -rf $(DOC).pdf
	@make -C $(LIB_DOC_DIR)/figures clean

.PHONY: all clean figures
