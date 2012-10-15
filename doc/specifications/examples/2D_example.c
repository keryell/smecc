/* To compile this program on Linux, try:

   make CFLAGS='-std=c99 -Wall' example_2D

   To run:
   ./example_2D; echo $?
   It should print 0 if OK.

   You can even compile it to run on multicore SMP for free with

   make CFLAGS='-std=c99 -fopenmp -Wall' example_2D

   To verify there are really some clone() system calls that create the threads:
   strace -f ./example_2D ; echo $?

   You can notice that the #pragma smecy are ignored (the project is
   on-going :-) ) but that the program produces already correct results in
   sequential execution and parallel OpenMP execution.

   Enjoy!

   Ronan.Keryell@hpc-project.com
   for ARTEMIS SMECY European project.
*/

#include <stdlib.h>
#include "example_helper.h"


// Problem size
enum { WIDTH = 500, HEIGHT = 200 };


/* The main host program controlling and representing the whole
   application */
int main(int argc, char* argv[]) {
  int image[HEIGHT][WIDTH];
  unsigned char output[HEIGHT][WIDTH];

  // Initialize with some values
  init_image(WIDTH, HEIGHT, image);

#pragma omp parallel sections
  {
    // On one processor
    // We rewrite a small part of image:
#pragma smecy map(PE, 0) arg(3, inout, [HEIGHT][WIDTH]			\
			     /[HEIGHT/3:HEIGHT/3 + HEIGHT/2 - 1]	\
			     [WIDTH/8:WIDTH/8 + HEIGHT/2 - 1])
    square_symmetry(WIDTH, HEIGHT, image, HEIGHT/2, WIDTH/8, HEIGHT/3);

    // On another processor
#pragma omp section
    // Here let the compiler to guess the array size
#pragma smecy map(PE, 1) arg(3, inout, /[HEIGHT/4:HEIGHT/4 + HEIGHT/2 - 1] \
			     [3*WIDTH/8:3*WIDTH/8 + HEIGHT/2 - 1])
    square_symmetry(WIDTH, HEIGHT, image, HEIGHT/2, 3*WIDTH/4, HEIGHT/4);

    // On another processor
#pragma omp section
    // Here let the compiler to guess the array size
#pragma smecy map(PE, 1) arg(3, inout, /[2*HEIGHT/5:2*HEIGHT/5 + HEIGHT/2 - 1]	\
			     [WIDTH/2:WIDTH/2 + HEIGHT/2 - 1])
    square_symmetry(WIDTH, HEIGHT, image, HEIGHT/2, WIDTH/2, 2*HEIGHT/5);
  }
  // Here there is a synchronization because of the parallel part end

  // Since there
  normalize_to_char(WIDTH, HEIGHT, image, output);

  write_pgm_image("2D_example-output.pgm", WIDTH, HEIGHT, output);

  return EXIT_SUCCESS;
}
