/* The main host program controlling and representing the whole
   application */
int main(int argc, char* argv[]) {
  int image[HEIGHT][WIDTH];
  unsigned char output[HEIGHT][WIDTH];

  // Initialize with some values
  init(WIDTH, HEIGHT, image);

  
  for(in) {
    function_A(in_A, out_A);
    function_B(out_A, out_B);
    function_C(out_B, out_C);

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
