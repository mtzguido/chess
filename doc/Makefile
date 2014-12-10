.PHONY: clean strip all re
SHELL=/bin/bash

docname= informe

all: $(docname).pdf

$(docname).pdf: $(docname).tex bib.bib
	latex $(docname).tex
	bibtex $(docname)
	latex $(docname).tex
	latex $(docname).tex
	dvipdf $(docname).dvi $@

clean:
	rm -f $(docname).{log,aux,pdf,dvi}

strip: $(docname).pdf
	rm -f $(docname).{log,aux}

re: clean all
