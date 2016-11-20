
texfname = manual

simple: $(texfname).tex
	pdflatex $(texfname).tex

all: $(texfname).tex
	pdflatex $(texfname).tex
	bibtex $(texfname).aux
	pdflatex $(texfname).tex
	pdflatex $(texfname).tex
	cp $(texfname).pdf ../.

view: $(texfname).pdf
	evince $(texfname).pdf

clean:
	rm -rf *~ $(texfname).aux $(texfname).bbl $(texfname).blg \
	$(texfname).dvi $(texfname).out $(texfname).log \
	$(texfname).lof $(texfname).lot $(texfname).toc

