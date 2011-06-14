DOCUMENT = SMECY_pragma_slides-HPC

# We have an index to compute
#MAKEINDEX=1

USE_PDFTEX=1

# OK, not really usable except by me right now... :-)
# point to where our local TeX stuff is installed:
TEX_ROOT=~/projets/Wild_Systems/hpc_project/doc/TeX

ifndef P4A_ROOT
  $(error The P4A_ROOT environment variable should point where the par4all git working copy is.)
endif

ifndef COURSE_ROOT
  $(error The COURSE_ROOT environment variable should point where the Ronan Keryell courses are (such as ~/cours).)
endif

#ifndef P4A_PRIV_ROOT
#  $(error The P4A_PRIV_ROOT environment variable should point where the par4all-priv git working copy is.)
#endif

# Add this to the TeX path:
INSERT_TEXINPUTS=::$(TEX_ROOT)//:$(TEX_ROOT)/../Images//:$(TEX_ROOT)/../presentations/2009/nVidia-GPU_Technology_Conference-2009:$(P4A_ROOT)/doc/figures//:$(COURSE_ROOT):../../..:../../../Multithread/2008-09-Astellia_3_jours
#APPEND_TEXINPUTS=$(TEX_ROOT)//:$(TEX_ROOT)/../Images//
include $(TEX_ROOT)/Makefiles/beamer.mk

pub:
	# To publish temporary versions on my web page:
	mkdir -p $(HOME)/public_html/CINES
	rsync -a $(DOCUMENT)-expose.pdf $(DOCUMENT)-copie.pdf $(HOME)/public_html/CINES

sources:
	tar zcvf SME-C.tar.gz pragma_SMECY.tex pragma_SMECY.pdf SMECY_pragma_slides-HPC-expose.pdf examples/{2D_example.c,example_helper.c,example_helper.h,Makefile,pragma_example.c,remapping_example.c,smecy.c,smecy.h}

