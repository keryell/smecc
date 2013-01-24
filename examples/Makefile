# To run smecc with ROSE in verbose mode, try:
# make ROSE_FLAGS="-rose:verbose 3"

# Think to use a make xyz.E to produce a preprocessed output to understand
# what is really compiled

# Where is smecy.h:
SMECY_INC=../runtime

# -fopenmp

# All the sources to compile:
# Skip map-tab_cpp.C.bug
LOCAL_SRC= hello_world.c input.c input_cpp.C map-tab.c \
	multiple_compilation_unit_a.c multiple_compilation_unit_b.c \
	multiple_compilation_unit_a_cpp.C multiple_compilation_unit_b_cpp.C \
	simple_map.c simple_map_cpp.C

## Remove .c and .C extension to have executable names:
#LOCAL_BIN:=$(LOCAL_SRC:.c=)
#LOCAL_BIN:=$(LOCAL_BIN:.C=)
LOCAL_BIN= hello_world input input_cpp map-tab \
	multiple_compilation_unit_a \
	multiple_compilation_unit_a_cpp \
	simple_map simple_map_cpp

include $(SMECY_INC)/use.mk

# To skip debug, use "make SMECY_DEBUG=" or uncomment the following line:
#SMECY_DEBUG=

multiple_compilation_unit_a: multiple_compilation_unit_b.o
smecy_multiple_compilation_unit_a: smecy_multiple_compilation_unit_b.o

multiple_compilation_unit_a_cpp: multiple_compilation_unit_b_cpp.o
smecy_multiple_compilation_unit_a_cpp: smecy_multiple_compilation_unit_b_cpp.o


#rose_multiple_compilation_unit_a.c + rose_multiple_compilation_unit_b.c: multiple_compilation_unit_a.c multiple_compilation_unit_b.c multiple_compilation_unit.h
#	smecc -c -smecy --std=c99 multiple_compilation_unit_a.c multiple_compilation_unit_b.c
