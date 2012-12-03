# A makefile to ease compilation of SME-C with smecc

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

# Add some flags to previously ones, if any:
SMECY_FLAGS+=$(SMECY_DEBUG) -I$(SMECY_INC)
CFLAGS+=--std=c99 -fopenmp -g $(SMECY_FLAGS) $(MCA_INCLUDE) $(MORE_FLAGS)
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

accel_smecy_%.c: %.c
	smecc -smecy -smecy-accel --std=c99 -rose:C99_only \
		-rose:skipfinalCompileStep -rose:C_output_language \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<
	mv rose_$*.c $@

run_%: %
	./$<

# Produce a specialized MCAPI expansion for host side:
%_host.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(CFLAGS) -DSMECY_MCAPI -DSMECY_MCAPI_HOST $< > $@

# Produce a specialized MCAPI expansion for fabric side:
%_fabric.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(CFLAGS) -DSMECY_MCAPI -DSMECY_MCAPI_PE $< > $@

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
		*.o *.E
