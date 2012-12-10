#SMECY_DIR			= /hpc/projects/smecy
#RMFLAGS				= -f

PUB = doc.pdf
# Assume the GitHub wiki cloned here:
PUB_DIR=../smecc.wiki/docs

all: doc.pdf

%.pdf: %.tex
	@pdflatex $<
	@pdflatex $<
	bibtex $*
	@pdflatex $<
	@pdflatex $<

clean:
	@rm $(RMFLAGS) *.log *.out *.aux *.blg *.pdf *.toc *.bbl *~

#backup: clean
#	@mkdir ~/stage/codeBackup/`date +"%m%d%H%M"`documents/
#	@cp -r * ~/stage/codeBackup/`date +"%m%d%H%M"`documents/

doc: doc.pdf
	@cp doc.pdf $(SMECY_DIR)/doc/smecc_doc.pdf

pub:
	# To publish temporary versions on my web page:
	mkdir -p $(PUB_DIR)
	cp -a $(PUB) $(PUB_DIR)
