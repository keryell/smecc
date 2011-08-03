SMECY_DIR			= /hpc/projects/smecy
RMFLAGS				= -f

all: report.pdf

%.pdf: %.tex
	@pdflatex $<
	@pdflatex $<
	bibtex $*
	@pdflatex $<
	@pdflatex $<

clean:
	@rm $(RMFLAGS) *.log *.out *.aux *.blg *.pdf *.toc *.bbl *~
	
backup: clean
	@mkdir ~/stage/codeBackup/`date +"%m%d%H%M"`documents/
	@cp -r * ~/stage/codeBackup/`date +"%m%d%H%M"`documents/
	
doc: doc.pdf
	@cp doc.pdf $(SMECY_DIR)/doc/smecc_doc.pdf
