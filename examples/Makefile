# To run smecc with ROSE in verbose mode, try:
# make ROSE_FLAGS="-rose:verbose 3"


# -fopenmp

CFLAGS=--std=c99 -DSMECY_VERBOSE -g
CXXFLAGS=--std=c++0x -DSMECY_VERBOSE -g

LOCAL_TARGET= rose_hello_world.c rose_input.c rose_input.C rose_map-tab.C \
	rose_simple_map.C rose_smecy.c rose_multiple_compilation_unit_b.c \
	rose_multiple_compilation_unit_a.c

all: $(LOCAL_TARGET)

rose_%.C: %.C
	smecc -smecy -c $(ROSE_FLAGS) $<

rose_%.c: %.c
	smecc -c -smecy --std=c99 -rose:C99_only -rose:C_output_language $(ROSE_FLAGS)$<

rose_%: rose_%.C

rose_%: rose_%.o smecy.o


#rose_multiple_compilation_unit_a.c + rose_multiple_compilation_unit_b.c: multiple_compilation_unit_a.c multiple_compilation_unit_b.c multiple_compilation_unit.h
#	smecc -c -smecy --std=c99 multiple_compilation_unit_a.c multiple_compilation_unit_b.c

clean:
	rm -f $(LOCAL_TARGET)
