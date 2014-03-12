.PHONY: clean strip all re
SHELL=/bin/bash

docname= informe

all: $(docname).pdf

$(docname).pdf: $(docname).tex
	pdflatex $(docname).tex

clean:
	rm -f $(docname).{log,aux,pdf}

strip: $(docname).pdf
	rm -f $(docname).{log,aux}

re: clean all
