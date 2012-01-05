# -fopenmp

LOCAL_TARGET= rose_hello_world.c rose_input.c rose_input.C rose_map-tab.C rose_simple_map.C rose_smecy.c rose_multiple_compilation_unit_a.c

all: $(LOCAL_TARGET)

rose_%.C: %.C
	smecc -smecy -c $<


rose_%.c: %.c
	smecc -c -smecy --std=c99 $<

rose_multiple_compilation_unit_a.c + rose_multiple_compilation_unit_b.c: multiple_compilation_unit_a.c multiple_compilation_unit_b.c multiple_compilation_unit.h
	smecc -c -smecy --std=c99 $+

clean:
	rm -f $(LOCAL_TARGET)
