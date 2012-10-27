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

SMECY_FLAGS=$(SMECY_DEBUG) -I$(SMECY_INC)
CFLAGS=--std=c99 -fopenmp -g $(SMECY_FLAGS) $(MORE_FLAGS)
LDFLAGS=-fopenmp
CXXFLAGS=--std=c++0x -fopenmp -g $(SMECY_FLAGS) $(MORE_FLAGS)

# Have an explicit list since we have some program with multiple
# compilation units:

# smecc generate file beginning with "rose_":
LOCAL_ROSE=$(addprefix rose_,$(LOCAL_SRC))
LOCAL_ROSE_DOT=$(addsuffix .dot,$(LOCAL_SRC))
LOCAL_ROSE_PDF=$(addsuffix .pdf,$(LOCAL_SRC))

# Remove .c and .C extension to have executable names:
#LOCAL_ROSE_BIN:=$(LOCAL_ROSE:.c=)
#LOCAL_ROSE_BIN:=$(LOCAL_ROSE_BIN:.C=)
LOCAL_ROSE_BIN=$(addprefix rose_,$(LOCAL_BIN))

all: $(LOCAL_ROSE)

bin: $(LOCAL_BIN) $(LOCAL_ROSE_BIN)

rose_%.C: %.C
	smecc -smecy -c $(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<

rose_%.c: %.c
	smecc -c -smecy --std=c99 -rose:C99_only -rose:C_output_language \
		$(ROSE_FLAGS) $(SMECY_FLAGS) $(MORE_FLAGS) $<

rose_%: rose_%.c smecy.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

# Specific C++ compiler for C++ linking:
rose_%: rose_%.C smecy.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

run_%: %
	./$<

# Produce a CPP output to help debugging
%.E: %.[cC]
	# Keep comments in the output
	$(CPP) -CC $(SMECY_FLAGS) $(MORE_FLAGS) $< > $@

%.E: %.x
	$(CPP) -C $(SMECY_FLAGS) $(MORE_FLAGS) $< > $@


# Use :: so that a user of this Makefile can extend this rule
clean::
	rm -f $(LOCAL_ROSE) $(LOCAL_BIN) $(LOCAL_ROSE_BIN) \
		$(LOCAL_ROSE_DOT) $(LOCAL_ROSE_PDF) rose_transformation_* \
		*.o *.E
