TARGETS=init_a init_a.p4a.c p4a_wrapper_init_array.cl
all: $(TARGETS)

run:
	./init_a

images: run remapping_example-output.png 2D_example-output.png

# To rebuild headers:
headers: example_helper.h smecy.h

clean:
	rm *.o $(TARGETS)

# Guess we have GNU make and gcc, so use all the default variable names
# and implicit rules (look at "10.2 Catalogue of Implicit Rules" in the
# GNU make manual).

CFLAGS = -std=c99 -g -fopenmp -Wall
LDLIBS = -lgomp


init_a.p4a.c p4a_wrapper_init_array.cl: init_a.c
	p4a --opencl init_a.c


# They need some other .o objects to be built:
2D_example: example_helper.o
remapping_example: example_helper.o smecy.o

# To build automatically the .h. This is not used by default because some
# people do not have cproto:
%.h: %.c
	cproto $< > $@

%.png: %.pgm
	pnmtopng -transparent black $< > $@
