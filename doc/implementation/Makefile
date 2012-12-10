DOCUMENT = implementation
# Assume the GitHub wiki cloned here:
PUB_DIR=../smecc.wiki/docs

# We have an index to compute
#MAKEINDEX=1

USE_PDFTEX=1


ifndef P4A_ROOT
  $(error The P4A_ROOT environment variable should point where the par4all git working copy is.)
endif

ifndef COURSE_ROOT
  $(error The COURSE_ROOT environment variable should point where the Ronan Keryell courses are (such as ~/cours).)
endif

TEX_ROOT=$(COURSE_ROOT)/TeX

#ifndef P4A_PRIV_ROOT
#  $(error The P4A_PRIV_ROOT environment variable should point where the par4all-priv git working copy is.)
#endif

# Add this to the TeX path:
INSERT_TEXINPUTS=::$(TEX_ROOT)//:$(TEX_ROOT)/../Images//:$(TEX_ROOT)/../presentations/2009/nVidia-GPU_Technology_Conference-2009:$(P4A_ROOT)/doc/figures//:$(COURSE_ROOT):../../..:../../../Multithread/2008-09-Astellia_3_jours
#APPEND_TEXINPUTS=$(TEX_ROOT)//:$(TEX_ROOT)/../Images//
#//////include $(TEX_ROOT)/Makefiles/beamer.mk

pub:
	# To publish temporary versions on my web page:
	mkdir -p $(PUB_DIR)
	cp -a $(DOCUMENT).pdf $(PUB_DIR)

sources:
	tar zcvf SME-C.tar.gz pragma_SMECY.tex pragma_SMECY.pdf SMECY_pragma_slides-HPC-expose.pdf examples/{*.c,*.h}

