# To run smecc with ROSE in verbose mode, try:
# make ROSE_FLAGS="-rose:verbose 3"

# Think to use a make xyz.E to produce a preprocessed output to understand
# what is really compiled

# Where is smecy.h:
SMECY_INC=../runtime

# -fopenmp

# All the sources to compile:
# Skip map-tab_cpp.C.bug
LOCAL_SRC= hello_world.c map-tab.c \
	multiple_compilation_unit_a.c multiple_compilation_unit_b.c \
	simple_map.c input.c
# There is a link issue in the Makefile that use the C linker instead of C++:
# input_cpp.C simple_map_cpp.C
# multiple_compilation_unit_a_cpp.C multiple_compilation_unit_b_cpp.C

# Describe how to compile the smecc generated versions on STHORM:
STHORM_smecy_hello_world_host=HOST_SRC='smecy_hello_world_host.c' HOST_EXE='smecy_hello_world_host'  FABRIC_SRC='accel_smecy_hello_world.c'

STHORM_smecy_simple_map_host=HOST_SRC='smecy_simple_map_host.c' HOST_EXE='smecy_simple_map_host'  FABRIC_SRC='accel_smecy_simple_map.c'

## Remove .c and .C extension to have executable names:
#LOCAL_BIN:=$(LOCAL_SRC:.c=)
#LOCAL_BIN:=$(LOCAL_BIN:.C=)
LOCAL_BIN= hello_world map-tab \
	multiple_compilation_unit_a \
	simple_map input
# input_cpp simple_map_cpp multiple_compilation_unit_a_cpp

include $(SMECY_INC)/use.mk

# To skip debug, use "make SMECY_DEBUG=" or uncomment the following line:
#SMECY_DEBUG=

multiple_compilation_unit_a: multiple_compilation_unit_b.o
STHORM_smecy_multiple_compilation_unit_a_host="HOST_SRC='smecy_multiple_compilation_unit_a_host.c smecy_multiple_compilation_unit_b_host.c' HOST_EXE='smecy_multiple_compilation_unit_a_host'  FABRIC_SRC='accel_smecy_multiple_compilation_unit_a_fabric.c accel_smecy_multiple_compilation_unit_b_fabric.c'"
smecy_multiple_compilation_unit_a: smecy_multiple_compilation_unit_b.o
smecy_multiple_compilation_unit_a_host: smecy_multiple_compilation_unit_b_host.o
accel_smecy_multiple_compilation_unit_a_fabric: accel_smecy_multiple_compilation_unit_b.o

multiple_compilation_unit_a_cpp: multiple_compilation_unit_b_cpp.o
smecy_multiple_compilation_unit_a_cpp: smecy_multiple_compilation_unit_b_cpp.o
smecy_multiple_compilation_unit_a_cpp_host: smecy_multiple_compilation_unit_b_cpp_host.o
accel_smecy_multiple_compilation_unit_a_cpp_fabric: accel_smecy_multiple_compilation_unit_b_cpp_fabric.o


#rose_multiple_compilation_unit_a.c + rose_multiple_compilation_unit_b.c: multiple_compilation_unit_a.c multiple_compilation_unit_b.c multiple_compilation_unit.h
#	smecc -c -smecy --std=c99 multiple_compilation_unit_a.c multiple_compilation_unit_b.c
