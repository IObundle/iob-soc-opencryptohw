ROOT_DIR:=../..
include $(ROOT_DIR)/config.mk

DOC=acceleration_plan

NOCLEAN= -name "Makefile" -o -name "$(DOC).pdf" -o -name "$(DOC).tex"
NOCLEAN+= -o -name "$(DOC).cls"

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
