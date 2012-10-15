#include <stdlib.h>
#include "example_helper.h"

// Problem size
enum { WIDTH = 500, HEIGHT = 200, LINE_SIZE = 100 };

/* Apply some pixel value inversion in a 1D array
 */
void
invert_vector(int line_size,
	      int input_line[line_size],
	      int output_line[line_size]) {
  for(int i = 0; i < line_size; i++)
    output_line[i] = 500 - input_line[i];
}


/* The main host program controlling and representing the whole
   application */
int main(int argc, char* argv[]) {
  int image[HEIGHT][WIDTH];
  unsigned char output[HEIGHT][WIDTH];

  // Initialize with some values
  init_image(WIDTH, HEIGHT, image);

  // Draw 70 horizontal lines and map operation on 8 PEs:
#pragma omp parallel for num_threads(8)
  for(int proc = 0; proc < 70; proc++)
    // Each iteration is on a different PE in parallel:
#pragma smecy map(PE, proc & 7)			\
              arg(2, in, [1][LINE_SIZE])	\
              arg(3, out, [1][LINE_SIZE])
    // Invert an horizontal line:
    invert_vector(LINE_SIZE,
		  &image[HEIGHT - 20 - proc][WIDTH/2 + 2*proc],
		  &image[HEIGHT - 20 - proc][WIDTH/2 + 2*proc]);

  /* Here we guess we have 5 hardware accelerators and we launch
     operations on them: */
#pragma omp parallel for num_threads(5)
  for(int proc = 0; proc < 5; proc++) {
    /* This is need to express the fact that our accelerator only accept
       continuous data but we want apply them on non contiguous data in
       the array */
    int input_line[LINE_SIZE];
    int output_line[LINE_SIZE];
    /* We need to remap data in the good shape. The compiler should use
       the remapping information to generate DMA transfer for example and
       remove input_line array */
    SMECY_remap_int2D_to_int1D(HEIGHT, WIDTH, HEIGHT/3, 30 + 20*proc,
			       LINE_SIZE, 1, image,
			       LINE_SIZE, input_line);
    // Each iteration is on a different PE in parallel:
#pragma smecy map(PE, proc) arg(2, in, [LINE_SIZE]) arg(3, out, [LINE_SIZE])
    invert_vector(LINE_SIZE, input_line, output_line);
    SMECY_remap_int1D_to_int2D(LINE_SIZE, output_line,
			       HEIGHT, WIDTH, HEIGHT/3, 30 + 20*proc,
			       LINE_SIZE, 1, image);
  }

  // Convert int image to char image:
  normalize_to_char(WIDTH, HEIGHT, image, output);

  write_pgm_image("remapping_example-output.pgm", WIDTH, HEIGHT, output);

  return EXIT_SUCCESS;
}
