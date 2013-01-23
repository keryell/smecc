# A Makefile to ease compilation of SME-C with smecc
# SMECY ARTEMIS European project
# Ronan Keryell @ silkan.com

# To run smecc with ROSE in verbose mode, try:
# make ROSE_FLAGS="-rose:verbose 3"

# Think to use a make xyz.E to produce a preprocessed output to understand
# what is really compiled

# Where is smecy.h:
# You have to set this before using this Makefile
#SMECY_INC=../runtime

# To put more flags, use make MORE_FLAGS=...

# To skip debug, use "make SMECY_DEBUG="
SMECY_DEBUG=-DSMECY_VERBOSE

# By default use MCA API reference implementation
MCA_INCLUDE?=`mcapi-config --cflags`
MCAPI_LINK?=`mcapi-config --libs`
MRAPI_LINK?=`mrapi-config --libs`

# Add some flags to previously ones, if any:
SMECY_FLAGS+=$(SMECY_DEBUG) -I$(SMECY_INC)
CFLAGS+=--std=c99 -fopenmp -g $(SMECY_FLAGS) $(MCA_INCLUDE) $(MORE_FLAGS)

# More flags to compile with MCAPI on the host side
CFLAGS_MCAPI=-DSMECY_MCAPI -DSMECY_MCAPI_HOST
# More flags to compile with MCAPI on the accelerator side
CFLAGS_MCAPI_ACCEL=-DSMECY_MCAPI

LDFLAGS+=-fopenmp
CXXFLAGS+=--std=c++0x -fopenmp -g $(SMECY_FLAGS) $(MCA_INCLUDE) $(MORE_FLAGS)
LDLIBS+=$(MCAPI_LINK) $(MRAPI_LINK)


# Have an explicit list since we have some program with multiple
# compilation units:

# smecc generate file beginning with "smecy_" and "smecy_accel_" for the
# host and accelerator sides respectively:
LOCAL_SMECY=$(addprefix smecy_,$(LOCAL_SRC)) \
	$(addprefix accel_smecy_,$(LOCAL_SRC))
LOCAL_SMECY_DOT=$(addsuffix .dot,$(LOCAL_SRC))
LOCAL_SMECY_PDF=$(addsuffix .pdf,$(LOCAL_SRC))

# Remove .c and .C extension to have executable names:
#LOCAL_ROSE_BIN:=$(LOCAL_ROSE:.c=)
#LOCAL_ROSE_BIN:=$(LOCAL_ROSE_BIN:.C=)
LOCAL_SMECY_BIN=$(addprefix smecy_,$(LOCAL_BIN))

all: $(LOCAL_SMECY)

bin: $(LOCAL_BIN) $(LOCAL_SMECY_BIN)

smecy_%.C: %.C
	smecc -smecy -rose:skipfinalCompileStep $(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.C $@

smecy_%.c: %.c
	smecc -smecy --std=c99 -rose:C99_only \
		-rose:skipfinalCompileStep -rose:C_output_language \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.c $@

accel_smecy_%.C: %.C
	smecc -smecy -smecy-accel -rose:skipfinalCompileStep \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.C $@
	# Post process the output:
	smecc_post_processor $@
	# Append the automatically generated file (if any) with the
	# dispatch system on the PEs to the post-processed version back
	# into the target file:
	if [ -r $*.C-smecy_dispatch ]; then \
	   cat $@.smecc_pp $*.C-smecy_dispatch > $@ ; \
	else \
	   mv $@.smecc_pp $@ ; \
	fi

accel_smecy_%.c: %.c
	smecc -smecy -smecy-accel --std=c99 -rose:C99_only \
		-rose:skipfinalCompileStep -rose:C_output_language \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.c $@
	# Post process the output:
	smecc_post_processor $@
	# Append the automatically generated file (if any) with the
	# dispatch system on the PEs to the post-processed version back
	# into the target file:
	if [ -r $*.c-smecy_dispatch ]; then \
	   cat $@.smecc_pp $*.c-smecy_dispatch > $@ ; \
	else \
	   mv $@.smecc_pp $@ ; \
	fi

run_%: %
	./$<

# Produce a specialized MCAPI expansion for host side smecy_... source files:
%_host.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(CFLAGS) $(CFLAGS_MCAPI) $< > $@

# Produce a specialized MCAPI expansion for fabric side accel_smecy_... source files:
%_fabric.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(CFLAGS) $(CFLAGS_MCAPI_ACCEL) $< > $@

accel_%: CFLAGS += $(CFLAGS_MCAPI_ACCEL)

# Produce a CPP output to help debugging
%.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(CFLAGS) $< > $@

%.E: %.x
	$(CPP) -CC $(CFLAGS) $< > $@


# Use :: so that a user of this Makefile can extend this rule
clean::
	rm -f $(LOCAL_SMECY) $(LOCAL_BIN) $(LOCAL_SMECY_BIN) \
		$(LOCAL_SMECY_DOT) $(LOCAL_SMECY_PDF) rose_transformation_* \
		*.o *.E *-smecy_dispatch *.smecc_pp
