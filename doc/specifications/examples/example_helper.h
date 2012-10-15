/* example_helper.c */
void init_image(int width, int height, int array[height][width]);
void write_pgm_image(const char filename[], int width, int height, const unsigned char array[height][width]);
void normalize_to_char(int width, int height, int array[height][width], unsigned char output[height][width]);
void square_symmetry(int width, int height, int image[height][width], int square_size, int x_offset, int y_offset);
