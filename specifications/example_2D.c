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

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Problem size
enum { WIDTH = 500, HEIGHT = 200 };


/** Initialize a 2D array with some progressive values

    @param width is the size of the array in the second dimension

    @param height is the size of the array in the first dimension

    @param[out] array is the array to initialize

    Note that we could also use this [out] Doxygen information to avoid
    specifying it again in the #pragma...
*/
void init(int width, int height, int array[height][width]) {
  // Can be executed in parallel
#pragma omp parallel for
  for(int i = 0; i < height; i++)
    for(int j = 0; j < width; j++)
      // Initialize with stripes:
      array[i][j] = (i + 3*j) >> ((i - j) & 7);
}


/** Write the content of an array to Portable Gray Map image format (PGM)

    @param[in] filename is the name of the file to write into the image

    @param n is the size of the array in the first dimension

    @param m is the size of the array in the second dimension

    @param[in] array is the array to use as image content. Note we could
    infer the [in] information and communication directions directly from
    "const" qualifier
*/
void write_pgm_image(const char filename[], int width, int height,
		     const unsigned char array[height][width]) {
  FILE * fp;

  char * comments = "# This is an image generated by the " __FILE__
    " program.\n"
    "# SMECY ARTEMIS European project.\n";
  // Open the image file for writing:
  if ((fp = fopen(filename, "w")) == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  /* Write the PGM header which begins with, in ASCII decimal, the
     width, the height and the maximum gray value (255 here): */
  fprintf(fp,"P5\n%d %d\n%s%d\n", width, height, comments, UCHAR_MAX);

  for(int i = 0; i < height; i++)
    for(int j = 0; j < width; j++)
      // Write a pixel value:
      fputc(array[i][j], fp);

  // Close the file:
  fclose(fp);
}


/* Apply a vertical symmetry to a subsquare in an image

*/
void square_symmetry(int width, int height, int image[height][width],
		     int square_size, int x_offset, int y_offset) {
  // Can be executed in parallel
#pragma omp parallel for
  for(int i = 0; i < square_size/2; i++)
    for(int j = 0; j < square_size; j++) {
      int tmp = image[y_offset + i][x_offset + j];
      image[y_offset + i][x_offset + j] =
	image[y_offset + square_size - i][x_offset + j];
      image[y_offset + square_size - i][x_offset + j] = tmp;
    }
}


/** Normalize an array of integer values into an array of unsigned char

    This is typically used to generate a gray image from arbitrary data.

 */
void normalize_to_char(int width, int height, int array[height][width],
		       unsigned char output[height][width]) {
  /* First find the minimum and maximum values of array for
     later normalization: */
  // Initialize the minimum value to the biggest integer:
  int minimum = INT_MAX;
  // Initialize the maximum value to the smallest integer:
  int maximum = INT_MIN;
#pragma omp parallel for reduction(min:minimum) reduction(max:maximum)
  for(int i = 0; i < height; i++)
    for(int j = 0; j < width; j++) {
      int v = array[i][j];
      if (v < minimum) minimum = v;
      else if (v > maximum) maximum = v;
    }

  // Now do the normalization
  float f = UCHAR_MAX/(float)(maximum - minimum);
#pragma omp parallel for
  for(int i = 0; i < height; i++)
    for(int j = 0; j < width; j++)
      output[i][j] = (array[i][j] - minimum)*f;
}


/* The main host program controlling and representing the whole
   application */
int main(int argc, char* argv[]) {
  int image[HEIGHT][WIDTH];
  unsigned char output[HEIGHT][WIDTH];

  // Initialize with some values
  init(WIDTH, HEIGHT, image);

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

  write_pgm_image("output.pgm", WIDTH, HEIGHT, output);

  return EXIT_SUCCESS;
}