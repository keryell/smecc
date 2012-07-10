# To run smecc with ROSE in verbose mode, try:
# make ROSE_FLAGS="-rose:verbose 3"

# Where is smecy.h:
SMECY_INC=../runtime

# -fopenmp

CFLAGS=--std=c99 -fopenmp -DSMECY_VERBOSE -g -I$(SMECY_INC)
CXXFLAGS=--std=c++0x -fopenmp -DSMECY_VERBOSE -g -I$(SMECY_INC)

# All the sources to compile:
# Skip map-tab_cpp.C.bug
LOCAL_SRC= hello_world.c input.c input_cpp.C map-tab.c \
	multiple_compilation_unit_a.c multiple_compilation_unit_b.c \
	multiple_compilation_unit_a_cpp.C multiple_compilation_unit_b_cpp.C \
	simple_map.C smecy.c

# Remove .c and .C extension to have executable names:
LOCAL_BIN:=$(LOCAL_SRC:.c=)
LOCAL_BIN:=$(LOCAL_BIN:.C=)

# smecc generate file beginning with "rose_":
LOCAL_ROSE=$(addprefix rose_,$(LOCAL_SRC))
LOCAL_ROSE_DOT=$(addsuffix .dot,$(LOCAL_SRC))
LOCAL_ROSE_PDF=$(addsuffix .pdf,$(LOCAL_SRC))

# Remove .c and .C extension to have executable names:
LOCAL_ROSE_BIN:=$(LOCAL_ROSE:.c=)
LOCAL_ROSE_BIN:=$(LOCAL_ROSE_BIN:.C=)

all: $(LOCAL_ROSE)

rose_%.C: %.C
	smecc -smecy -c $(ROSE_FLAGS) -I$(SMECY_INC) $<

rose_%.c: %.c
	smecc -c -smecy --std=c99 -rose:C99_only -rose:C_output_language \
		$(ROSE_FLAGS) -I$(SMECY_INC) $<

rose_%: rose_%.c smecy.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

# Specific C++ compiler for C++ linking:
rose_%: rose_%.C smecy.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

run_%: %
	./$<

multiple_compilation_unit_a: multiple_compilation_unit_b.o
rose_multiple_compilation_unit_a: rose_multiple_compilation_unit_b.o

multiple_compilation_unit_a_cpp: multiple_compilation_unit_b_cpp.o
rose_multiple_compilation_unit_a_cpp: rose_multiple_compilation_unit_b_cpp.o


#rose_multiple_compilation_unit_a.c + rose_multiple_compilation_unit_b.c: multiple_compilation_unit_a.c multiple_compilation_unit_b.c multiple_compilation_unit.h
#	smecc -c -smecy --std=c99 multiple_compilation_unit_a.c multiple_compilation_unit_b.c

clean:
	rm -f $(LOCAL_ROSE) $(LOCAL_BIN) $(LOCAL_ROSE_BIN) \
		$(LOCAL_ROSE_DOT) $(LOCAL_ROSE_PDF) rose_transformation_* *.o
