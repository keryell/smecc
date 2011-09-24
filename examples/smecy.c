#include "smecy.h"

void square_symmetry_smecy(int width, int height, int *image,
		     int square_size, int x_offset, int y_offset) {
  // Can be executed in parallel
#pragma omp parallel for
  for(int i = 0; i < square_size/2; i++)
    for(int j = 0; j < square_size; j++) {
      int tmp = image[(y_offset + i)*height + x_offset + j];
      image[(y_offset + i)*height + x_offset + j] = image[(y_offset + square_size - i)*height + x_offset + j];
      image[(y_offset + square_size - i)*height + x_offset + j] = tmp;
    }
}
