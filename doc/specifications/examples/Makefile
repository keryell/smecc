TARGETS=2D_example remapping_example pipeline_example \
	init_a init_a.p4a.c p4a_wrapper_init_array.cl \
	distribute_PEs distribute_PEs.p4a.c \
	independent_clusters independent_clusters.p4a.c \
	round_robin_clusters round_robin_clusters.p4a.c \
	stream_cluster_PEs stream_cluster_PEs.p4a.c \
	systolic_PEs systolic_PEs.p4a.c

all: $(TARGETS)

run:
	./init_a
	./2D_example
	./remapping_example
	./pipeline_example

images: run remapping_example-output.png 2D_example-output.png

# To rebuild headers:
headers: example_helper.h smecy.h

clean:
	rm *.o $(TARGETS)

# Guess we have GNU make and gcc, so use all the default variable names
# and implicit rules (look at "10.2 Catalogue of Implicit Rules" in the
# GNU make manual).

#CFLAGS = -std=c99 -g -Wall -fopenmp -fmudflapth
CFLAGS = -std=c99 -g -fopenmp -Wall
#LDLIBS = -lgomp -lmudflapth
LDLIBS = -lgomp


init_a.p4a.c p4a_wrapper_init_array.cl: init_a.c
	p4a --opencl init_a.c


%.p4a.c : %.c
	p4a --opencl $*.c


# They need some other .o objects to be built:
2D_example: example_helper.o
remapping_example: example_helper.o smecy.o

# To build automatically the .h. This is not used by default because some
# people do not have cproto:
%.h: %.c
	cproto $< > $@

%.png: %.pgm
	pnmtopng -transparent black $< > $@
